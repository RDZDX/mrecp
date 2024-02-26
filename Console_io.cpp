#include "Console_io.h"
extern Console console;
//extern Telnet telnet;
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
	if(t2input.input_mode == 0){
		// Output to telnet socket
		char temp_str[2] = {ch, 0};
		//telnet.send_data(temp_str);
	} else {
		int len = strlen(t2input.str_buf);

		if(len < BUF_SIZE){ // Prevent buffer overflow
			if(ch == '\177'){ // Telnet code if 177 for deleting character
				// Delete the last key
				if(len > 1) { // Only delete if there is something in the buffer
					t2input.str_buf[len - 1] = (char)'\0'; // Clear it
					console_char_in('\b\b'); // Sending \b\b for deleting character
				}
			} else {
				if(t2input.input_done == 0)
					t2input.str_buf[len] = ch;
				
				console_char_in(ch);
			}
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
