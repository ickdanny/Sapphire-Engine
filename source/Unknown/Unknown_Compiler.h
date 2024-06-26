#ifndef UNKNOWN_COMPILER_H
#define UNKNOWN_COMPILER_H

#include <stdbool.h>

/*
 * compiles the specified Unknown source file and
 * returns the program; error on compiler error
 */
UNProgram unCompile(const char *fileName);

#endif