#pragma once

#include <print>

#define CR_TERM_BLACK   "\x1B[31m"
#define CR_TERM_RED     "\x1B[31m"
#define CR_TERM_GREEN   "\x1B[32m"
#define CR_TERM_YELLOW  "\x1B[33m"
#define CR_TERM_BLUE    "\x1B[34m"
#define CR_TERM_MAGENTA "\x1B[35m"
#define CR_TERM_CYAN    "\x1B[36m"
#define CR_TERM_WHITE   "\x1B[37m"
#define CR_TERM_DEFAULT "\x1B[39m"

#define CR_TERM_RESET   "\x1B[0m"

#define CR_FLOG(FD, COLOR, FORMAT, ...) do { std::print(FD, COLOR FORMAT CR_TERM_RESET "\n" __VA_OPT__(,)__VA_ARGS__); } while(0)

#define CR_INFO(FORMAT,  ...) CR_FLOG(stdout, CR_TERM_DEFAULT, FORMAT, __VA_ARGS__)
#define CR_WARN(FORMAT,  ...) CR_FLOG(stderr, CR_TERM_YELLOW,  FORMAT, __VA_ARGS__)
#define CR_ERROR(FORMAT, ...) CR_FLOG(stderr, CR_TERM_RED,     FORMAT, __VA_ARGS__)

