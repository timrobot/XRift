#ifndef __color_h__
#define __color_h__

#include <stdio.h>

#define ANSI_COLOR_RED      "\x1b[31m"
#define ANSI_COLOR_GREEN    "\x1b[32m"
#define ANSI_COLOR_YELLOW   "\x1b[33m"
#define ANSI_COLOR_BLUE     "\x1b[34m"
#define ANSI_COLOR_MAGENTA  "\x1b[35m"
#define ANSI_COLOR_CYAN     "\x1b[36m"
#define ANSI_COLOR_RESET    "\x1b[0m"
#define print_red(str)      { printf(ANSI_COLOR_RED     str ANSI_COLOR_RESET "\n"); }
#define print_green(str)    { printf(ANSI_COLOR_GREEN   str ANSI_COLOR_RESET "\n"); }
#define print_yellow(str)   { printf(ANSI_COLOR_YELLOW  str ANSI_COLOR_RESET "\n"); }
#define print_blue(str)     { printf(ANSI_COLOR_BLUE    str ANSI_COLOR_RESET "\n"); }
#define print_magenta(str)  { printf(ANSI_COLOR_MAGENTA str ANSI_COLOR_RESET "\n"); }
#define print_cyan(str)     { printf(ANSI_COLOR_CYAN    str ANSI_COLOR_RESET "\n"); }
#define color_red(str)      { ANSI_COLOR_RED      str ANSI_COLOR_RESET }
#define color_green(str)    { ANSI_COLOR_GREEN    str ANSI_COLOR_RESET }
#define color_yellow(str)   { ANSI_COLOR_YELLOW   str ANSI_COLOR_RESET }
#define color_blue(str)     { ANSI_COLOR_BLUE     str ANSI_COLOR_RESET }
#define color_magenta(str)  { ANSI_COLOR_MAGENTA  str ANSI_COLOR_RESET }
#define color_cyan(str)     { ANSI_COLOR_CYAN     str ANSI_COLOR_RESET }

#define print_error(str)      { fprintf(stderr, ANSI_COLOR_RED     str ANSI_COLOR_RESET "\n"); }
#define print_debug(str)      { fprintf(stderr, ANSI_COLOR_GREEN     str ANSI_COLOR_RESET "\n"); }

#endif
