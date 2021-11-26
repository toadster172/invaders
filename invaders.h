#ifndef INVADERS_H
#define INVADERS_H

#include <stdio.h>
#include "i8080Type.h"
#include "i8080.h"

typedef struct _invaders {
    i8080 *cpu;
    uint8_t *addressSpace;
    uint8_t *videoBuffer;

    uint16_t shiftNum;
    uint8_t shiftOffset;

    uint8_t inputOne;
    uint8_t inputTwo;
} invaders;

invaders *initializeMachine(FILE *f);
void freeMachine(invaders *machine);

#endif
