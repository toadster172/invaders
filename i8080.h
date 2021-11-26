#ifndef i8080_H
#define i8080_H

#include <stdio.h>
#include "i8080Type.h"

//Constructs an Intel 8080 CPU and returns a pointer to it
i8080 *create8080(void);
void executeInstruction(i8080 *cpu);

#endif
