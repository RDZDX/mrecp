#include "main.h"
#include "Console.h"
#include "Console_io.h"
//#include "Telnet.h"
#include "T2Input.h"
#include "vmtimer.h"

//Global
int scr_w = 0, scr_h =0;
VMUINT8 *layer_bufs[2] = {0,0};
VMINT layer_hdls[2] = {-1,-1};

VMCHAR command[100] = {};
//VMCHAR ip[100] = {};
VMCHAR portx[100] = {};
VMCHAR login[100] = {};
VMCHAR password[100] = {};

//VMBOOL missingConfigFile = VM_FALSE;
VMBOOL missingConfigFile = VM_TRUE;
VMBOOL flightMode = VM_FALSE;
VMBOOL startup = VM_FALSE;
VMCHAR text[220] = {};
VMCHAR text11[100] = {};

VMCHAR text22[100] = {};
VMCHAR text33[100] = {};
VMCHAR text44[100] = {};
VMCHAR text55[100] = {};

VMCHAR my_path[100] = {};
VMCHAR my_file_name[100] = {};
VMINT lenght_555;
VMINT lenght_666;

VMCHAR text6[100];
VMCHAR text3[41] = "MRE console primitive [Version 1.00]\n\n";
VMCHAR text4[150] = "about        - show program information\nhelp         - list available commands\necho         - print string\ndir          - list directory content\n";
VMCHAR text5[172] = "cd           - change current directory\npath         - sets a path\nmkdir        - creates a directory\nrmdir        - deletes a directory\ncopy         - copies a file\n";
VMCHAR text10[195] = "rename       - renames a file\nmove         - move a file\ndel          - deletes a file\ntype         - displays file contents\ncls          - clear screen\nexit         - quits the program\n\n";
VMCHAR text88[44] = "The syntax of the command is incorrect.\n\n";
VMINT hnd;
VMINT test;
VMINT rrrrrrr = 0;
VMINT page11 = 0;
VMINT plus_line = 29; //dirtyhack: compensate 29 lines of ignored first display
struct vm_fileinfo_ext fileInfo;
VMWCHAR fullPath1[100]; 
VMWCHAR fullPath2[100];
VMWCHAR fullPath3[100];
VMWCHAR fullPath4[100];
VMUINT nread;
VMFILE f_read;

Console console;
//Telnet telnet;
T2Input t2input;

int main_timer_id = -1;

int prompt_timer_id = -1; // Timer for prompt
int timeout_timer_id = -1; // Timer for waiting telnet to connect
int timeout = 0; // Timeout counter

// Telnet host and port
char ip[BUF_SIZE];
int port = -3;

char port1[BUF_SIZE];

#ifndef WIN32
extern "C" void* malloc(int size){
	return vm_malloc(size);
}
extern "C" void free(void*prt){
	return vm_free(prt);
}

extern "C" void _sbrk(){}
extern "C" void _write(){}
extern "C" void _close(){}
extern "C" void _lseek(){}
extern "C" void _open(){}
extern "C" void _read(){}
extern "C" void _exit(){}
extern "C" void _getpid(){}
extern "C" void _kill(){}
extern "C" void _fstat(){}
extern "C" void _isatty(){}
#endif

void handle_sysevt(VMINT message, VMINT param);
void handle_keyevt(VMINT event, VMINT keycode);
//void handle_penevt(VMINT event, VMINT x, VMINT y);

void create_app_txt_path(VMWSTR text, VMSTR extt);
void checkFileExist(void);
VMINT parseText(VMSTR text);
VMINT parseText1(VMSTR text);
void timer1(int a);
char * strsubstr(char * str , int from, int count);
void trim(char *reslt_data, char *inp_data);
void create_app_txt_path1(void);
void create_supdir_path(VMWSTR result, VMWSTR source);
void create_search_path(VMWSTR result, VMWSTR source, VMSTR text);
VMINT cb(VMINT act, VMUINT32 total, VMUINT32 completed, VMINT hdl);
void trim_left_symbols(char *reslt_data, char *inp_data);
void extract_path(char *reslt_data, char *inp_data);

void vm_main(void){
	scr_w = vm_graphic_get_screen_width(); 
	scr_h = vm_graphic_get_screen_height();

	console.init();
	//telnet.init();
	t2input.init();

	vm_reg_sysevt_callback(handle_sysevt);
	vm_reg_keyboard_callback(handle_keyevt);
	//vm_reg_pen_callback(handle_penevt);

        checkFileExist();
        //if (vm_sim_card_count() == 99) { flightMode = VM_TRUE; }
        create_app_txt_path1();
}

void draw(){
	vm_graphic_fill_rect(layer_bufs[1], 0, 0, scr_w, scr_h, tr_color, tr_color);
	vm_graphic_line(layer_bufs[1], console.cursor_x*char_width, (console.cursor_y+1)*char_height,
		(console.cursor_x+1)*char_width, (console.cursor_y+1)*char_height, console.cur_textcolor);
	t2input.draw();
	vm_graphic_flush_layer(layer_hdls, 2);
} 

void timer(int tid){
	//telnet.update();
	draw();
}

void timeout_f(int tid){
	//if (telnet.is_connected == 1) {
	//	// Connected
	//	t2input.input_mode = 0; // Change the input mode to telnet
	//	vm_delete_timer(timeout_timer_id); // Delete the timeout counter
	//} else {
		timeout++; // Increase the timeout counter
		//timeout++; // Increase the timeout counter

		if(timeout > 22 && timeout < 24) {
			// After 30 seconds (you can change this duration) -> timeout and exit
			console_str_in("\nTimed out, exiting...");
		}

		if(timeout > 25) {
			// After 25 seconds (you can change this duration) -> timeout and exit
			vm_exit_app(); // Exit
		}
	//}
}

/*
 * We use port as the status flag before input it actual value
 * If port == -3 -> prompt for host input, then set port = -2
 * If port == -2 -> Check if the input has been done and if yes,
 * save the ip and prompt for port input, then set port = -1
 * If port == -1 -> Check if the input has been done, if yes set
 * the actual port value.
 */

void prompt(int tid) {
    if (port == -3) {
        // Prompt for host input
        // console_str_in("\nPlease enter the host name/ip: ");
        port = -2;

    } else if (port == -2 && t2input.input_done == 1) {
        strcpy(ip, t2input.str_buf);  // Save the ip
        trim(text11, ip);

        // console_str_in("\nPlease enter the port: ");

        parseText1(text11);

        if (vm_string_equals_ignore_case(text22, "echo") == 0 && strlen(text33) != 0) {
            sprintf(text, "%s %s %s", text33, text44, text55);
            trim(text, text);
            strcat(text, "\n\n");
            console_str_in(text);

        } else if (vm_string_equals_ignore_case(text22, "echo") == 0 && strlen(text33) == 0) {
            console_str_in("ECHO is on.\n\n");

        } else if (vm_string_equals_ignore_case(text22, "line") == 0 && strlen(text33) == 0) {
            sprintf(text, "Line: %d\n\n", my_intx + plus_line); //chimka trukstamu 29 pirmojo ekrano eiluciu skaiciaus kompensavimui bendrame kiekyje !
            console_str_in(text);

        } else if (vm_string_equals_ignore_case(text22, "type") == 0 && strlen(text33) == 0) {
            console_str_in(text88);

        } else if (vm_string_equals_ignore_case(text22, "type") == 0 && strlen(text33) != 0) {
            strcpy(text, "The system cannot find the file specified.");
            vm_ascii_to_ucs2(fullPath3, (strlen(text33) + 1) * 2, text33);
            vm_wstrcpy(fullPath2, fullPath1);
            vm_wstrcat(fullPath2, fullPath3);

            f_read = vm_file_open(fullPath2, MODE_READ, FALSE);

            if (f_read < 0) {
                vm_file_close(f_read);
                console_str_in(text);

            } else if (rrrrrrr == 1) {
                sprintf(text, "Used buffer space: %d/500, need empty !\n\n", my_intx);
                console_str_in(text);

            } else {
                while (!vm_file_is_eof(f_read)) {
                    vm_file_read(f_read, text, 219, &nread);
                    text[nread] = '\0';
                    console_str_in(text);

                    if (rrrrrrr == 0 && my_intx >= 500 - 5) {
                        // if (my_intx >= 500 - 5) {

                        rrrrrrr = 1;
                        console.clean_history();
                        console.erase_display(2);
                        console.reset();
                        plus_line = 0; //chimka trukstamu 29 pirmojo ekrano eiluciu skaiciaus kompensavimui bendrame kiekyje !
                    }
                }

                vm_file_close(f_read);
            }

            console_str_in("\n\n");

        } else if (vm_string_equals_ignore_case(text22, "exit") == 0 && strlen(text33) == 0) {
            // vm_delete_timer(prompt_timer_id);
            vm_exit_app();

        } else if (vm_string_equals_ignore_case(text22, "exit") == 0 && strlen(text33) != 0) {
            console_str_in(text88);

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && strlen(text33) == 0) {
            console_str_in(text4);
            console_str_in(text5);
            console_str_in(text10);

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && vm_string_equals_ignore_case(text33, "help") == 0 && strlen(text44) == 0) {
            console_str_in("help         - list available commands\n\n");

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && vm_string_equals_ignore_case(text33, "copy") == 0 && strlen(text44) == 0) {
            console_str_in("copy         - command description\n\n");

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && vm_string_equals_ignore_case(text33, "cls") == 0 && strlen(text44) == 0) {
            console_str_in("cls          - clear screen\n\n");

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && vm_string_equals_ignore_case(text33, "cd") == 0 && strlen(text44) == 0) {
            console_str_in("cd           - change current directory\n\n");

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && vm_string_equals_ignore_case(text33, "dir") == 0 && strlen(text44) == 0) {
            console_str_in("dir          - list directory content\n\n");

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && vm_string_equals_ignore_case(text33, "about") == 0 && strlen(text44) == 0) {
            console_str_in("about        - show program information\n\n");

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && vm_string_equals_ignore_case(text33, "exit") == 0 && strlen(text44) == 0) {
            console_str_in("exit         - quits the program\n\n");

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && vm_string_equals_ignore_case(text33, "type") == 0 && strlen(text44) == 0) {
            console_str_in("type         - displays file contents\n\n");

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && vm_string_equals_ignore_case(text33, "line") == 0 && strlen(text44) == 0) {
            console_str_in("line         - command description\n\n");

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && vm_string_equals_ignore_case(text33, "echo") == 0 && strlen(text44) == 0) {
            console_str_in("echo         - print string\n\n");

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && vm_string_equals_ignore_case(text33, "del") == 0 && strlen(text44) == 0) {
            console_str_in("del          - deletes a file\n\n");

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && vm_string_equals_ignore_case(text33, "mkdir") == 0 && strlen(text44) == 0) {
            console_str_in("mkdir        - creates a directory\n\n");

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && vm_string_equals_ignore_case(text33, "rmdir") == 0 && strlen(text44) == 0) {
            console_str_in("rmdir        - deletes a directory\n\n");

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && vm_string_equals_ignore_case(text33, "rename") == 0 && strlen(text44) == 0) {
            console_str_in("rename       - renames a file\n\n");

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && vm_string_equals_ignore_case(text33, "copy") == 0 && strlen(text44) == 0) {
            console_str_in("copy         - copies a file\n\n");

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && vm_string_equals_ignore_case(text33, "move") == 0 && strlen(text44) == 0) {
            console_str_in("move         - move a file\n\n");

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && vm_string_equals_ignore_case(text33, "path") == 0 && strlen(text44) == 0) {
            console_str_in("path         - sets a path\n\n");

        } else if (vm_string_equals_ignore_case(text22, "help") == 0 && strlen(text33) != 0) {
            console_str_in(text88);

        } else if (vm_string_equals_ignore_case(text22, "about") == 0 && strlen(text33) == 0) {
            console_str_in(text3);

        } else if (vm_string_equals_ignore_case(text22, "about") == 0 && strlen(text33) != 0) {
            console_str_in(text88);

        //} else if (vm_string_equals_ignore_case(text22, "dir") == 0 && strlen(text33) != 0) {
        // dir filename

        //} else if (vm_string_equals_ignore_case(text22, "dir") == 0 && strlen(text33) != 0) {
        // dir directoryname

        } else if (vm_string_equals_ignore_case(text22, "dir") == 0 && strlen(text33) == 0) {
            create_search_path(fullPath2, fullPath1, "*.*");
            test = 0;
            hnd = vm_find_first_ext(fullPath2, &fileInfo);
            vm_ucs2_to_ascii(text6, wstrlen(fileInfo.filefullname) + 1, fileInfo.filefullname);

            if (fileInfo.attributes == VM_FS_ATTR_DIR) {
                strcpy(text, "<DIR> ");
            } else {
                strcpy(text, "-r--- ");
            }

            strcat(text, text6);
            strcat(text, "\n");
            console_str_in(text);
            strcpy(text6, "");

            while (test == 0) {
                test = vm_find_next_ext(hnd, &fileInfo);

                if (test == 0) {
                    vm_ucs2_to_ascii(text6, wstrlen(fileInfo.filefullname) + 1, fileInfo.filefullname);

                    if (fileInfo.attributes == VM_FS_ATTR_DIR) {
                        strcpy(text, "<DIR> ");
                    } else {
                        strcpy(text, "-r--- ");
                    }

                    strcat(text, text6);
                    strcat(text, "\n");
                    console_str_in(text);
                    strcpy(text6, "");
                }
            }

            console_str_in("\n");

            vm_find_close_ext(hnd);
            vm_find_close_ext(test);

        } else if (vm_string_equals_ignore_case(text22, "cd") == 0 && vm_string_equals_ignore_case(text33, ".") == 0 || vm_string_equals_ignore_case(text22, "cd") == 0 && strlen(text33) == 0) {
            vm_ucs2_to_ascii(text, wstrlen(fullPath1) + 1, fullPath1);
            strcat(text, "\n\n");
            console_str_in(text);

        } else if (vm_string_equals_ignore_case(text22, "cd") == 0 && vm_string_equals_ignore_case(text33, "..") == 0) {
            create_supdir_path(fullPath1, fullPath1);
            vm_ucs2_to_ascii(text, wstrlen(fullPath1) + 1, fullPath1);
            strcat(text, "\n\n");
            console_str_in(text);

        } else if (vm_string_equals_ignore_case(text22, "cd") == 0 && strlen(text33) != 0 && vm_string_equals_ignore_case(text33, "..") != 0 && vm_string_equals_ignore_case(text33, ".") != 0) {

            strcpy(text, "The system cannot find the path specified.");
            trim_left_symbols(text6, text33);
            strcat(text6, "\\");
            vm_ascii_to_ucs2(fullPath3, (strlen(text6) + 1) * 2, text6);
            vm_wstrcpy(fullPath2, fullPath1);
            vm_wstrcat(fullPath2, fullPath3);

            if (vm_file_get_attributes(fullPath2) == VM_FS_ATTR_DIR) { // -1 file or directory not found
                wstrcpy(fullPath1, fullPath2);
                vm_ucs2_to_ascii(text, wstrlen(fullPath1) + 1, fullPath1);
            }

            strcat(text, "\n\n");
            console_str_in(text);

        } else if (vm_string_equals_ignore_case(text22, "cls") == 0 && strlen(text33) != 0) {
            console_str_in(text88);

        } else if (vm_string_equals_ignore_case(text22, "cls") == 0 &&  strlen(text33) == 0) {

            if (my_intx > 471) {
                console.clean_history();
                my_intx = 0;
            }

            console.erase_display(2);
            console.reset();
            rrrrrrr = 0;

        } else if (vm_string_equals_ignore_case(text22, "del") == 0 &&  strlen(text33) == 0) {
            console_str_in(text88);

        } else if (vm_string_equals_ignore_case(text22, "del") == 0 &&  strlen(text33) != 0 &&  strlen(text44) == 0) {
            create_search_path(fullPath2, fullPath1, text33);
            if (vm_file_get_attributes(fullPath2) != -1 || vm_file_get_attributes(fullPath2) != VM_FS_ATTR_DIR) {
               vm_file_delete(fullPath2);
               vm_ucs2_to_ascii(text, wstrlen(fullPath2) + 1, fullPath2);
               strcat(text, " - OK");
               strcat(text, "\n\n");
               console_str_in(text);
            } else {console_str_in("File not found !\n\n");}

        } else if (vm_string_equals_ignore_case(text22, "mkdir") == 0 &&  strlen(text33) == 0) {
            console_str_in(text88);

        } else if (vm_string_equals_ignore_case(text22, "mkdir") == 0 &&  strlen(text33) != 0 &&  strlen(text44) == 0) {
            create_search_path(fullPath2, fullPath1, text33);
            if (vm_file_get_attributes(fullPath2) == -1) {
               vm_file_mkdir(fullPath2);
               vm_ucs2_to_ascii(text, wstrlen(fullPath2) + 1, fullPath2);
               strcat(text, " - OK");
               strcat(text, "\n\n");
               console_str_in(text);
            } else {console_str_in("Fail, same directory or file exists !\n\n");}

        } else if (vm_string_equals_ignore_case(text22, "rmdir") == 0 &&  strlen(text33) == 0) {
            console_str_in(text88);

        } else if (vm_string_equals_ignore_case(text22, "rmdir") == 0 &&  strlen(text33) != 0 &&  strlen(text44) == 0) {
            create_search_path(fullPath2, fullPath1, text33);
            if (vm_file_get_attributes(fullPath2) == VM_FS_ATTR_DIR) {
               vm_file_rmdir(fullPath2);
               vm_ucs2_to_ascii(text, wstrlen(fullPath2) + 1, fullPath2);
               strcat(text, " - OK");
               strcat(text, "\n\n");
               console_str_in(text);
            } else {console_str_in("Directory not found !\n\n");}

        } else if (vm_string_equals_ignore_case(text22, "rename") == 0 &&  strlen(text33) == 0) {
            console_str_in(text88);

        } else if (vm_string_equals_ignore_case(text22, "rename") == 0 &&  strlen(text33) != 0 && strlen(text44) != 0 && strlen(text55) == 0) {
            create_search_path(fullPath2, fullPath1, text33);
            if (vm_file_get_attributes(fullPath2) != -1) {
               create_search_path(fullPath3, fullPath1, text44);
               if (vm_file_get_attributes(fullPath3) == -1) {
                  vm_file_rename(fullPath2, fullPath3);
                  vm_ucs2_to_ascii(text, wstrlen(fullPath3) + 1, fullPath3);
                  strcat(text, " - OK");
                  strcat(text, "\n\n");
                  console_str_in(text);
               }
            } else {console_str_in("Name of source or destination is incorrect !\n\n");}

        } else if (vm_string_equals_ignore_case(text22, "move") == 0 &&  strlen(text33) == 0) {
            console_str_in(text88);

        } else if (vm_string_equals_ignore_case(text22, "copy") == 0 &&  strlen(text33) == 0) {
            console_str_in(text88);

        } else if (vm_string_equals_ignore_case(text22, "split") == 0 &&  strlen(text33) == 0) {
            console_str_in(text88);
            //some code..

        } else if (vm_string_equals_ignore_case(text22, "start") == 0 &&  strlen(text33) == 0) {
            console_str_in(text88);
            //some code..

        } else if (vm_string_equals_ignore_case(text22, "batch") == 0 &&  strlen(text33) == 0) {
            console_str_in(text88);
            //some code..

        } else if (vm_string_equals_ignore_case(text22, "path") == 0 &&  strlen(text33) == 0) {
            vm_ucs2_to_ascii(text, wstrlen(fullPath1) + 1, fullPath1);
            strcat(text, "\n\n");
            console_str_in(text);

        } else if (vm_string_equals_ignore_case(text22, "path") == 0 && strlen(text33) != 0 && strlen(text44) == 0) {
            trim_left_symbols(text6, text33);
            strcat(text6, "\\");
            vm_ascii_to_ucs2(fullPath2, (strlen(text6) + 1) * 2, text6);

            if (vm_file_get_attributes(fullPath2) == VM_FS_ATTR_DIR) {
                wstrcpy(fullPath1, fullPath2);
                vm_ucs2_to_ascii(text, wstrlen(fullPath1) + 1, fullPath1);
                strcat(text, "\n\n");
                console_str_in(text);
            } else {console_str_in("Directory not found !\n\n");}

        } else if (vm_string_equals_ignore_case(text22, "copy") == 0 && strlen(text33) != 0 && strlen(text44) != 0 && strlen(text55) == 0) {
            extract_path(my_path, text44);
            lenght_555 = strlen(my_path);
            lenght_666 = strlen(text44);
            strcat(text, "Name of source or destination is incorrect !\n\n");

            if (lenght_555 == 0) {
                create_search_path(fullPath2, fullPath1, text33);
                if (vm_file_get_attributes(fullPath2) != -1) {
                    create_search_path(fullPath3, fullPath1, text44);
                    if (vm_file_get_attributes(fullPath3) == -1) {
                        vm_file_copy(fullPath3, fullPath2, cb);
                        vm_ucs2_to_ascii(text, wstrlen(fullPath3) + 1, fullPath3);
                        strcat(text, " - OK");
                        strcat(text, "\n\n");
                    }
                }

            } else if (lenght_666 == lenght_555) {
                vm_ascii_to_ucs2(fullPath3, (strlen(my_path) + 1) * 2, my_path);
                if (vm_file_get_attributes(fullPath3) == VM_FS_ATTR_DIR) {
                    create_search_path(fullPath4, fullPath3, text33);
                    if (vm_file_get_attributes(fullPath4) == -1) {
                        create_search_path(fullPath2, fullPath1, text33);
                        if (vm_file_get_attributes(fullPath2) != -1) {
                            vm_file_copy(fullPath4, fullPath2, cb);
                            vm_ucs2_to_ascii(text, wstrlen(fullPath4) + 1, fullPath4);
                            strcat(text, " - OK");
                            strcat(text, "\n\n");
                        }
                    }
                }

            } else {
               strncpy(my_file_name, text44 + lenght_555, (lenght_666 - lenght_555));  // '\0' on end ? my_file_name[1 + lenght_666 - lenght_555] = '\0'
                vm_ascii_to_ucs2(fullPath3, (strlen(my_path) + 1) * 2, my_path);
                if (vm_file_get_attributes(fullPath3) == VM_FS_ATTR_DIR) {
                    create_search_path(fullPath4, fullPath3, my_file_name);
                    if (vm_file_get_attributes(fullPath4) == -1) {
                        create_search_path(fullPath2, fullPath1, text33);
                        if (vm_file_get_attributes(fullPath2) != -1) {
                            vm_file_copy(fullPath4, fullPath2, cb);
                            vm_ucs2_to_ascii(text, wstrlen(fullPath4) + 1, fullPath4);
                            strcat(text, " - OK");
                            strcat(text, "\n\n");
                        }
                    }
                }
            }

            console_str_in(text);

        } else if (vm_string_equals_ignore_case(text22, "move") == 0 && strlen(text33) != 0 && strlen(text44) != 0 && strlen(text55) == 0) {
            extract_path(my_path, text44);
            lenght_555 = strlen(my_path);
            lenght_666 = strlen(text44);
            strcat(text, "Name of source or destination is incorrect !\n\n");

            if (lenght_555 == 0) {
                create_search_path(fullPath2, fullPath1, text33);
                if (vm_file_get_attributes(fullPath2) != -1) {
                    create_search_path(fullPath3, fullPath1, text44);
                    if (vm_file_get_attributes(fullPath3) == -1) {
                        vm_file_copy(fullPath3, fullPath2, cb);
                        vm_file_delete(fullPath2);
                        vm_ucs2_to_ascii(text, wstrlen(fullPath3) + 1, fullPath3);
                        strcat(text, " - OK");
                        strcat(text, "\n\n");
                    }
                }

            } else if (lenght_666 == lenght_555) {
                vm_ascii_to_ucs2(fullPath3, (strlen(my_path) + 1) * 2, my_path);
                if (vm_file_get_attributes(fullPath3) == VM_FS_ATTR_DIR) {
                    create_search_path(fullPath4, fullPath3, text33);
                    if (vm_file_get_attributes(fullPath4) == -1) {
                        create_search_path(fullPath2, fullPath1, text33);
                        if (vm_file_get_attributes(fullPath2) != -1) {
                            vm_file_copy(fullPath4, fullPath2, cb);
                            vm_file_delete(fullPath2);
                            vm_ucs2_to_ascii(text, wstrlen(fullPath4) + 1, fullPath4);
                            strcat(text, " - OK");
                            strcat(text, "\n\n");
                        }
                    }
                }

            } else {
               strncpy(my_file_name, text44 + lenght_555, (lenght_666 - lenght_555)); // '\0' on end ? my_file_name[1 + lenght_666 - lenght_555] = '\0'
                vm_ascii_to_ucs2(fullPath3, (strlen(my_path) + 1) * 2, my_path);
                if (vm_file_get_attributes(fullPath3) == VM_FS_ATTR_DIR) {
                    create_search_path(fullPath4, fullPath3, my_file_name);
                    if (vm_file_get_attributes(fullPath4) == -1) {
                        create_search_path(fullPath2, fullPath1, text33);
                        if (vm_file_get_attributes(fullPath2) != -1) {
                            vm_file_copy(fullPath4, fullPath2, cb);
                            vm_file_delete(fullPath2);
                            vm_ucs2_to_ascii(text, wstrlen(fullPath4) + 1, fullPath4);
                            strcat(text, " - OK");
                            strcat(text, "\n\n");
                        }
                    }
                }
            }

            console_str_in(text);

        } else if (strlen(text11) != 0) {
            sprintf(text, "%s is not recognized as an internal or external command, operable program or batch file.\n\n", text11);
            console_str_in(text);
        } else {
        }

        // Free the buffer
        t2input.free_buffer();
        strcpy(text11, "");
        strcpy(text, "");
        //strcpy(text9, "");
        strcpy(text22, "");
        strcpy(text33, "");
        strcpy(text44, "");
        strcpy(text55, "");
        strcpy(my_file_name, "");
        strcpy(my_path, "");
        //strcpy(text12, "");
        strcpy(text6, "");
        lenght_555 = 0;
        lenght_666 = 0;

        //vm_wstrcpy(fullPath2, "");
        //wvm_wstrcpy(fullPath3, "");
        //vm_wstrcpy(fullPath4, "");

        port = -3;

        // port = -1;
        //} else if (port == -1 && t2input.input_done == 1){
        // port = strtoi(t2input.str_buf); // Convert port to number

        // if (port < 0) {
        //  Invaild port or number
        //	console_str_in("\nInvaild port! Please enter the port: ");

        // Free the buffer
        //	t2input.free_buffer();
        //} else {
        // telnet.connect_to(ip, port); // Try connecting to host

        // timeout_timer_id = vm_create_timer(1000, timeout_f);
        // vm_delete_timer(prompt_timer_id); // Remove the prompt
        //}
    }
}

void handle_sysevt(VMINT message, VMINT param) {
	switch (message) {
	case VM_MSG_CREATE:
	case VM_MSG_ACTIVE:
		layer_hdls[0] = vm_graphic_create_layer(0, 0, scr_w, scr_h, -1);
		layer_hdls[1] = vm_graphic_create_layer(0, 0, scr_w, scr_h, tr_color);
		
		vm_graphic_set_clip(0, 0, scr_w, scr_h);

		layer_bufs[0]=vm_graphic_get_layer_buffer(layer_hdls[0]);
		layer_bufs[1]=vm_graphic_get_layer_buffer(layer_hdls[1]);

		vm_switch_power_saving_mode(turn_off_mode);

		console.scr_buf=layer_bufs[0];
		console.draw_all();

		t2input.scr_buf=layer_bufs[1];
		t2input.layer_handle=layer_hdls[1];

		if(message == VM_MSG_CREATE){ //only when app start
			// Prompt for host & port to connect
			t2input.input_mode = 1; // Get input from keyboard to buffer

			//console_str_in("Welcome to Telnet Client\n");
			//console_str_in("Written by Ximik_Boda & TelnetVXP contributors\n");

                        if (flightMode == VM_TRUE) {

			    console_str_in("Turn off flight mode !\n");
			    vm_create_timer_ex(3000,timer1);

                        } else if (missingConfigFile == VM_TRUE) {

                            console_str_in(text3);
		            prompt_timer_id = vm_create_timer(1000, prompt); // Check the prompt for every 1 second

                        } else if (missingConfigFile == VM_FALSE) {

		            prompt_timer_id = vm_create_timer(1000, prompt); // Check the prompt for every 1 second
                            strcpy(t2input.str_buf, command);
                            t2input.send_c("\r\n");

		       } else {
                            sprintf(port1, "%d", port);
			    console_str_in("Opening: ");
			    console_str_in(ip);
			    console_str_in(":");
			    console_str_in(port1);
			    //telnet.connect_to(ip, port); // Try connecting to host
			    timeout_timer_id = vm_create_timer(1000, timeout_f);

                       }

		}
		if(main_timer_id==-1)
			main_timer_id = vm_create_timer(1000/15, timer); //15 fps
		break;
		
	case VM_MSG_PAINT:
		draw();
		break;
		
	case VM_MSG_INACTIVE:
		vm_switch_power_saving_mode(turn_on_mode);
		if( layer_hdls[0] != -1 ){
			vm_graphic_delete_layer(layer_hdls[0]);
			vm_graphic_delete_layer(layer_hdls[1]);
		}
		if(main_timer_id!=-1)
			vm_delete_timer(main_timer_id);
		break;		case VM_MSG_QUIT:
		if( layer_hdls[0] != -1 ){
			vm_graphic_delete_layer(layer_hdls[0]);
			vm_graphic_delete_layer(layer_hdls[1]);
		}
		if(main_timer_id!=-1)
			vm_delete_timer(main_timer_id);
		break;	
	}
}

void handle_keyevt(VMINT event, VMINT keycode) {
#ifdef WIN32
	if(keycode>=VM_KEY_NUM1&&keycode<=VM_KEY_NUM3)
		keycode+=6;
	else if(keycode>=VM_KEY_NUM7&&keycode<=VM_KEY_NUM9)
		keycode-=6;
#endif
    //if (event == VM_KEY_EVENT_UP && keycode == VM_KEY_RIGHT_SOFTKEY) {
    //    if (layer_hdls[0] != -1) {
    //        vm_graphic_delete_layer(layer_hdls[0]);
    //        layer_hdls[0] = -1;
    //    }
    //    vm_exit_app();
    //}
	t2input.handle_keyevt(event, keycode);

}

/*
void handle_penevt(VMINT event, VMINT x, VMINT y){
	t2input.handle_penevt(event, x, y);
	draw();
}
*/

void create_app_txt_path(VMWSTR text, VMSTR extt) {

    VMWCHAR fullPath[100];
    VMWCHAR wfile_extension[4];

    vm_get_exec_filename(fullPath);
    vm_ascii_to_ucs2(wfile_extension, 8, extt);
    vm_wstrncpy(text, fullPath, vm_wstrlen(fullPath) - 3);
    vm_wstrcat(text, wfile_extension);

}

void checkFileExist(void) {

    VMFILE f_read;
    VMUINT nread;
    VMWCHAR file_pathw[100];
    VMCHAR new_data[500];

    create_app_txt_path(file_pathw, "txt");

    f_read = vm_file_open(file_pathw, MODE_READ, FALSE);

    if (f_read < 0) {

       vm_file_close(f_read);
       missingConfigFile = VM_TRUE;

    } else {

      vm_file_read(f_read, new_data, 500, &nread);
      new_data[nread] = '\0';
      vm_file_close(f_read);

      if (strlen(new_data) > 1) {

         parseText(new_data);
         missingConfigFile = VM_FALSE;

      } else {

        missingConfigFile = VM_TRUE;

      }

    }
}

VMINT parseText(VMSTR text) {

    VMCHAR vns_simbl[2] = {};
    VMCHAR nauj_strng[100] = {};
    VMINT counter = 0;
    VMINT counter1 = 0;
    VMCHAR *ptr;

    //VMCHAR command[100] = {};
    //VMCHAR ip[100] = {};
    //VMCHAR portx[100] = {};
    //VMCHAR login[100] = {};
    //VMCHAR password[100] = {};


    ptr = text;

    while (*ptr != '\0' || counter1 == 5) {

           if (*ptr == '\r') {ptr++;}
           if (*ptr == '\n') {

              counter = counter + 1;

              if (counter == 1) {strcpy(command, nauj_strng);}
              if (counter == 2) {strcpy(ip, nauj_strng);}
              //if (counter == 3) {port = strtoi(nauj_strng);}
              if (counter == 3) {strcpy(portx, nauj_strng);}
              if (counter == 4) {strcpy(login, nauj_strng);}
              if (counter == 5) {strcpy(password, nauj_strng);}

              counter1 = counter;

              strcpy(nauj_strng, "");
              ptr++;
           }

           sprintf(vns_simbl, "%c", *ptr);
           strcat(nauj_strng, vns_simbl);
    ptr++;

    }

    if (counter == 0) {strcpy(command, nauj_strng);}
    if (counter == 1) {strcpy(ip, nauj_strng);}
    //if (counter == 2) {port = strtoi(nauj_strng);}
    if (counter == 2) {strcpy(portx, nauj_strng);}
    if (counter == 3) {strcpy(login, nauj_strng);}
    if (counter == 4) {strcpy(password, nauj_strng);}

    //if (strlen(ip) < 1 || port < 0) { missingConfigFile = VM_TRUE;}
    //if (strlen(ip) < 1 || strlen(portx) < 0) { missingConfigFile = VM_TRUE;}

    return 0;
}

VMINT parseText1(VMSTR text) {

    VMCHAR vns_simbl[2] = {};
    VMCHAR nauj_strng[100] = {};
    VMINT counter = 0;
    VMINT counter1 = 0;
    VMCHAR *ptr;

    ptr = text;

    while (*ptr != '\0') {

           if (*ptr == ' ') {

              counter = counter + 1;

              if (counter == 1) {strcpy(text22, nauj_strng);}
              if (counter == 2) {strcpy(text33, nauj_strng);}
              if (counter == 3) {strcpy(text44, nauj_strng);}

              counter1 = counter;

              if (counter < 4) {strcpy(nauj_strng, "");}
              ptr++;
           }

           sprintf(vns_simbl, "%c", *ptr);
           strcat(nauj_strng, vns_simbl);
    ptr++;

    }

    if (counter == 0) {strcpy(text22, nauj_strng);}
    if (counter == 1) {strcpy(text33, nauj_strng);}
    if (counter == 2) {strcpy(text44, nauj_strng);}
    if (counter > 2) {strcpy(text55, nauj_strng);}

    return 0;
}

void timer1(int a){
     vm_delete_timer_ex(a);
     vm_exit_app();
}

char  * strsubstr(char * str , int from, int count) {

    char * result;

    if(str == NULL) return NULL;

    result = (char *)vm_malloc((count+1) * sizeof(char));

    if(result == NULL) return NULL;

    strncpy(result, str+from, count);
    result[count] = '\0';

    vm_free(result);
    return result;
}

void trim(char *reslt_data, char *inp_data) {
    int flag = 0;

    // from left
    while(*inp_data) {
        if(!isspace((unsigned char) *inp_data) && flag == 0) {
            *reslt_data++ = *inp_data;
            flag = 1;
        }
        inp_data++;
        if(flag == 1) {
            *reslt_data++ = *inp_data;
        }
    }

    // from right
    while(1) {
        reslt_data--;
        if(!isspace((unsigned char) *reslt_data) && flag == 0) {
            break;
        }
        flag = 0;
        *reslt_data = '\0';
    }
}

void create_app_txt_path1(void) {

    VMWCHAR fullPath[100];
//  VMWCHAR fullPath1[100];
    VMWCHAR wfile_extension[10];

    vm_get_exec_filename(fullPath);
    vm_get_path(fullPath, fullPath1);
}

void create_supdir_path(VMWSTR result, VMWSTR source) {

    VMINT addrBefo = 3;
    VMINT addrLast = 0;
    VMINT count1 = 0;
    VMWCHAR fullPathx2[100];
    VMWCHAR *ptr;

    if (wstrlen(source) < 4) {
    
       wstrcpy(fullPathx2, source);
       wstrcpy(result, fullPathx2);
       return;

    }

    ptr = source;

       while (*ptr != L'\0') {

            addrBefo = addrLast;

            if (*ptr == L'\\') {

               addrLast = count1 + 1;
 
            }
 
       count1 = count1 + 1;

       ptr++;

       }

    vm_wstrncpy(fullPathx2, source, addrBefo);
    wstrcpy(result, fullPathx2);

}

void create_search_path(VMWSTR result, VMWSTR source, VMSTR text) {

    VMWCHAR fullPathx1[100];
    VMWCHAR wtext[100];

    vm_ascii_to_ucs2(wtext, (strlen(text) + 1) * 2, text); //"*.*"
    vm_wstrcpy(fullPathx1, source);
    vm_wstrcat(fullPathx1, wtext);
    vm_wstrcpy(result, fullPathx1);

}

//char *trimLeft(char *string) {

//    char *c = string;
//    while (*c) {
//         c ++;
//   }
//    c--;
    // trim suffix
//    while ((*c) == '\\' ) {
//        *c = '\0';
//        c--;
//    }
//    return string;
//}

VMINT cb(VMINT act, VMUINT32 total, VMUINT32 completed, VMINT hdl){}

//char *trimLeftz(char *string) {
 
//    char *c = string;
//    while (*c) {
//         c ++;
//   }
//    c--;
    // trim suffix
//    while ((*c) != '\\' ) {
//        *c = '\0';
//        c--;
//        if ((*c) == '\\' ) {break;}
//    }
//    return string;
//}

void trim_left_symbols(char *reslt_data, char *inp_data) {

      while(*inp_data) {
            *reslt_data++ = *inp_data;
          inp_data++;
    }
     
     while(1) {
        reslt_data--;
               if('\\' != (unsigned char) *reslt_data) {
            break;
        }
        *reslt_data = '\0';
        }
}

void extract_path(char *reslt_data, char *inp_data) {

      while(*inp_data) {
            *reslt_data++ = *inp_data;
          inp_data++;
    }
     
     while(1) {
        reslt_data--;
               //if('\\' != (unsigned char) *reslt_data) {
               if('\\' == (unsigned char) *reslt_data) {
            break;
        }
        *reslt_data = '\0';  // Ar cia ?
        }
}
