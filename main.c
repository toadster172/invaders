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

#include <stdio.h>
#include <SDL.h>
#include <time.h>
#include "invaders.h"
#include "sdlFunctions.h"

int main(int argc, char *argv[]) {
    if(argc == 1) {
        printf("Invalid number of command line arguments!\nUsage: invaders [Path to ROM]\n");
        return 1;
    }

    //Opens a file pointer to the location specified and reads it as binary
    FILE *rom = fopen(argv[1], "rb");
    if(rom == NULL) {
        printf("Error: File specified does not exist!\nUsage: invaders [Path to ROM]\n");
        return 2;
    }

    fseek(rom, 0, SEEK_END);

    if(ftell(rom) > 0xFFFF) {
        printf("Error: File too large to fit in memory of 8080\n");
        fclose(rom);
        return 3;
    }
    fseek(rom, 0, SEEK_SET);

    invaders *machine = initializeMachine(rom);
    fclose(rom);

    SDL_Window *window;
    SDL_Texture *texture;
    SDL_Renderer *renderer;
    graphicsInit(&window, &renderer, &texture, 4);

    clock_t freq;
    while(1) {
        freq = clock();
        
        machine->cpu->pendingInterrupt = 0xD7;
        while(machine->cpu->cycles < 16667) {
            executeInstruction(machine->cpu);
            if(machine->cpu->halt) {
                printf("CPU has halted!\n");
                graphicsEnd(&window, &renderer, &texture);
                freeMachine(machine);
                return 1;
            }
        }

        machine->cpu->cycles -= 16667;
        machine->cpu->pendingInterrupt = 0xCF;
        while(machine->cpu->cycles < 16667) {
            executeInstruction(machine->cpu);
            if(machine->cpu->halt) {
                printf("CPU has halted!\n");
                graphicsEnd(&window, &renderer, &texture);
                freeMachine(machine);
                return 1;
            }
        }
        
        machine->cpu->cycles -= 16667;

        updateFrame(machine, renderer, texture);
        
        if(queryKey(0)) {
            machine->inputTwo |= 32;
        } else {
            machine->inputTwo &= 0b11011111;
        }
        if(queryKey(1)) {
            machine->inputTwo |= 64;
        } else {
            machine->inputTwo &= 0b10111111;
        }
        if(queryKey(2)) {
            machine->inputTwo |= 1;
        } else {
            machine->inputTwo &= 0xFE;
        }
        if(queryKey(3)) {
            machine->inputTwo |= 16;
        } else {
            machine->inputTwo &= 0b11101111;
        }
        if(queryKey(4)) {
            machine->inputTwo |= 4;
        } else {
            machine->inputTwo &= 0b11111011;
        }
        if(queryKey(5)) {
            machine->cpu->halt = 1;
        }

        freq = clock() - freq;
        if((double) freq / CLOCKS_PER_SEC < 1 / 60.0) {
            SDL_Delay(((1.0 / 60.0) - ((double) freq / CLOCKS_PER_SEC)) * 1000);
        }
    }
}
