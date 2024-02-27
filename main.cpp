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

VMCHAR login[100] = {};
VMCHAR password[100] = {};
VMCHAR command[100] = {};
//VMBOOL missingConfigFile = VM_FALSE;
VMBOOL missingConfigFile = VM_TRUE;
VMBOOL flightMode = VM_FALSE;
VMCHAR text[220] = {};
VMCHAR text1[100] = {};
VMCHAR text11[100] = {};
char *test2;
char *test3;
VMCHAR text2[89] = " is not recognized as an internal or external command, operable program or batch file.\n";
VMCHAR text3[41] = "MRE console primitive [Version ALPHA].\n";
VMCHAR text4[174] = "about        - show program information\nhelp         - list available commands\necho         - print string\ncls          - clear screen\nexit         - quits the program\n";
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
void handle_penevt(VMINT event, VMINT x, VMINT y);

void create_app_txt_path(VMWSTR text, VMSTR extt);
void checkFileExist(void);
VMINT parseText(VMSTR text);
void timer1(int a);
char * strsubstr(char * str , int from, int count);
void trim(char *reslt_data, char *inp_data);

void vm_main(void){
	scr_w = vm_graphic_get_screen_width(); 
	scr_h = vm_graphic_get_screen_height();

	console.init();
	//telnet.init();
	t2input.init();

	vm_reg_sysevt_callback(handle_sysevt);
	vm_reg_keyboard_callback(handle_keyevt);
	vm_reg_pen_callback(handle_penevt);

        //checkFileExist();
        //if (vm_sim_card_count() == 99) { flightMode = VM_TRUE; }
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
		//console_str_in("\nPlease enter the host name/ip: ");
		port = -2;
	} else if (port == -2 && t2input.input_done == 1){
		strcpy(ip, t2input.str_buf); // Save the ip

                trim(text11, ip);

		//console_str_in("\nPlease enter the port: ");

                strncpy(text1, text11, 5);

                if (vm_string_equals_ignore_case(text1, "echo ") == 0)
                {
                   test2 = text11;
                   test3 = strsubstr(test2 , 5, strlen(test2)-5);
                   strcpy(text, test3);
                   strcat(text, "\n");
                   console_str_in(text);
                } 
                else if (vm_string_equals_ignore_case(text11, "exit") == 0)
                {
                   //vm_delete_timer(prompt_timer_id);
                   vm_exit_app();
                }
                else if (vm_string_equals_ignore_case(text11, "help") == 0)
                {
                   sprintf(text, "%s%s", text3, text4);
                   console_str_in(text);
                }
                else if (vm_string_equals_ignore_case(text11, "echo") == 0)
                {
                   sprintf(text, "ECHO is on.\n");
                   console_str_in(text);
                }
                else if (vm_string_equals_ignore_case(text11, "about") == 0)
                {
                   sprintf(text, "%s", text3);
                   console_str_in(text);
                }

                else if (vm_string_equals_ignore_case(text11, "cls") == 0)
                {
                   console.erase_display(2);
                   console.reset();
                }

                else if (strlen(text11) != 0)
                {
                   sprintf(text, "'%s'%s", text11, text2);
                   console_str_in(text);
                }
                else {}
	
		// Free the buffer
		t2input.free_buffer();
                strcpy(text11, "");
                strcpy(text1, "");
                strcpy(text, "");
                port = -3;
		//port = -1;
	//} else if (port == -1 && t2input.input_done == 1){
		//port = strtoi(t2input.str_buf); // Convert port to number
		
		//if (port < 0) {
			// Invaild port or number
		//	console_str_in("\nInvaild port! Please enter the port: ");
		
			// Free the buffer
		//	t2input.free_buffer();
		//} else {
			//telnet.connect_to(ip, port); // Try connecting to host

			//timeout_timer_id = vm_create_timer(1000, timeout_f);
			//vm_delete_timer(prompt_timer_id); // Remove the prompt
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
			//console_str_in("_");


                        if (flightMode == VM_TRUE) {

			    console_str_in("Turn off flight mode !\n");
			    vm_create_timer_ex(3000,timer1);

                        } else if (missingConfigFile == VM_TRUE) {

		            prompt_timer_id = vm_create_timer(1000, prompt); // Check the prompt for every 1 second

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

void handle_penevt(VMINT event, VMINT x, VMINT y){
	t2input.handle_penevt(event, x, y);
	draw();
}

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

    ptr = text;

    while (*ptr != '\0' || counter1 == 5) {

           if (*ptr == '\r') {ptr++;}
           if (*ptr == '\n') {

              counter = counter + 1;

              if (counter == 1) {strcpy(ip, nauj_strng);}
              if (counter == 2) {port = strtoi(nauj_strng);}
              if (counter == 3) {strcpy(login, nauj_strng);}
              if (counter == 4) {strcpy(password, nauj_strng);}
              if (counter == 5) {strcpy(command, nauj_strng);}

              counter1 = counter;

              strcpy(nauj_strng, "");
              ptr++;
           }

           sprintf(vns_simbl, "%c", *ptr);
           strcat(nauj_strng, vns_simbl);
    ptr++;

    }

    if (counter == 0) {strcpy(ip, nauj_strng);}
    if (counter == 1) {port = strtoi(nauj_strng);}
    if (counter == 2) {strcpy(login, nauj_strng);}
    if (counter == 3) {strcpy(password, nauj_strng);}
    if (counter == 4) {strcpy(command, nauj_strng);}

    if (strlen(ip) < 1 || port < 0) { missingConfigFile = VM_TRUE;}

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