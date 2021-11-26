/* Copyright (C) 2021 Alice Shelton <AliceAShelton@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include "invaders.h"

uint8_t readAddress(i8080 *cpu, uint16_t address);
void writeAddress(i8080 *cpu, uint16_t address, uint8_t value);
uint8_t readPort(i8080 *cpu, uint8_t port);
void writePort(i8080 *cpu, uint8_t port);

invaders *initializeMachine(FILE *f) {
    invaders *machine = malloc(sizeof(invaders));
    
    machine->addressSpace = malloc(0x10000);
    machine->videoBuffer = malloc(0xE000);

    if(machine->addressSpace == NULL || machine->videoBuffer == NULL) {
        printf("Failed to initialized!\n");
        exit(10);
    }
    
    for(int i = 0; i < 0x10000; i++) {
        machine->addressSpace[i] = 0;
    }
    
    for(int i = 0; i < 0xE000; i++) {
        machine->videoBuffer[i] = 0;
    }

    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    fread(machine->addressSpace, 1, filesize, f);

    machine->cpu = create8080();
    machine->cpu->machine = machine;
    machine->cpu->readAddress = readAddress;
    machine->cpu->writeAddress = writeAddress;
    machine->cpu->readPort = readPort;
    machine->cpu->writePort = writePort;

    machine->shiftNum = 0;
    machine->shiftOffset = 0;

    machine->inputOne = 0b00001110;
    machine->inputTwo = 0b00001000;

    return machine;
}

void freeMachine(invaders *machine) {
    free(machine->cpu);
    free(machine->addressSpace);
    free(machine->videoBuffer);
    free(machine);
}

uint8_t readAddress(i8080 *cpu, uint16_t address) {
    return cpu->machine->addressSpace[address & 0x3FFF];
}

void writeAddress(i8080 *cpu, uint16_t address, uint8_t value) {
    if((address & 0x3FFF) > 0x1FFF) {
        cpu->machine->addressSpace[address & 0x3FFF] = value;
        if((address & 0x3FFF) > 0x23FF) {
            for(uint8_t i = 0; i < 8; i++) {
                if(value & (1 << i)) {
                    uint16_t originalOffset = (address - 0x2400) * 8 + i;
                    uint16_t newOffset = 57120 - 224 * (originalOffset % 256) + originalOffset / 256;
                    cpu->machine->videoBuffer[newOffset] = 0xFF;
                } else {
                    uint16_t originalOffset = (address - 0x2400) * 8 + i;
                    uint16_t newOffset = 57120 - 224 * (originalOffset % 256) + originalOffset / 256;
                    cpu->machine->videoBuffer[newOffset] = 0x00;
                }
            }
        }
    }
}

uint8_t readPort(i8080 *cpu, uint8_t port) {
    switch (port) {
        case 0:
            return cpu->machine->inputOne;
        case 1:
            return cpu->machine->inputTwo;
        case 2:
            return 0b00000000;
        case 3:
            return (cpu->machine->shiftNum >> (8 - cpu->machine->shiftOffset) & 0xFF);
        default:
            printf("Unknown input port %i\n", port);
            exit(20);
    }    
}

void writePort(i8080 *cpu, uint8_t port) {
    switch (port) {
        case 2:
            cpu->machine->shiftOffset = cpu->a & 0x07;
            return;
        case 3: //Ports 3 and 5 are sound output ports, not emulated and ignored
            return;
        case 4:
            cpu->machine->shiftNum >>= 8;
            cpu->machine->shiftNum += (cpu->a << 8);
            return;
        case 5: //Ports 3 and 5 are sound output ports, not emulated and ignored
            return;
        case 6: //Watchdog, not required for emulation
            return;
        default:
            printf("Unknown output port %i\n", port);
            exit(20);
    }
}
