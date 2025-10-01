#include "Console_io.h"
extern Console console;
//extern Telnet telnet;
extern int myScroll;
extern T2Input t2input;


extern "C" void console_char_in(char ch){
	console.put_c(ch);
}

extern "C" void console_str_in(const char* str){
	console.putstr(str);
}

extern "C" void console_str_with_length_in(const char* str, int length){
	console.putstr(str, length);
}

extern "C" void console_char_out(char ch){
    if (t2input.input_mode == 0) {
        // Output to telnet socket
        char temp_str[2] = {ch, 0};
        //telnet.send_data(temp_str);
    } else {
        // Įvestis leidžiama tik kai nesame peržiūros režime
        if (myScroll == 0) {
            int len = strlen(t2input.str_buf);

            if (len < BUF_SIZE) {
                if (ch == '\177') { // Backspace
                    if (len > 0) {
                        t2input.str_buf[len - 1] = (char)'\0';
                        console_str_in("\b \b");
                    }
                } else {
                    if (t2input.input_done == 0)
                        t2input.str_buf[len] = ch;

                    console_char_in(ch);
                }
            }
        } else {
            // Jei vartotojas paspaudė simbolį peržiūros režime:
            // grįžtame į darbinę būseną (paskutinę eilutę)
            myScroll = 0;
            console.cursor_y = console.terminal_h - 1; // ✅ taisyta
            console.cursor_x = 0;

            console.clean_history();
            console.erase_display(2);
            console.reset();

            // Įvesti simbolį į darbinę vietą
            console_char_out(ch);
        }
    }
}

extern "C" void console_str_out(const char* str){
	for(unsigned int i = 0; i < strlen(str); i++){
		console_char_out(str[i]);
	}
}

extern "C" void console_str_with_length_out(const char* str, int length){
	if(t2input.input_mode == 1 && str == "\r\n")
		t2input.input_done = 1;

	console_str_out((char*)str);
}
