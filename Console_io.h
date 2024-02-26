#pragma once
#include "main.h"
#include "Console.h"
//#include "Telnet.h"
#include "T2Input.h"

extern "C" void console_char_in(char ch);
extern "C" void console_str_in(const char* str);
extern "C" void console_str_with_length_in(const char* str, int length);

extern "C" void console_char_out(char ch);
extern "C" void console_str_out(const char* str);
extern "C" void console_str_with_length_out(const char* str, int length);