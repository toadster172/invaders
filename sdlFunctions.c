/* Copyright (C) 2020, 2021 Alice Shelton <AliceAShelton@gmail.com>
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
#include <math.h>

#include "sdlFunctions.h"

uint8_t scancodeConvertor(SDL_Scancode scancode);

void graphicsInit(SDL_Window **window, SDL_Renderer **renderer, SDL_Texture **texture, uint8_t scale) {
    SDL_Init(SDL_INIT_VIDEO);
    *window = SDL_CreateWindow("Invaders", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 224 * scale, 256 * scale, SDL_WINDOW_SHOWN);
    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    *texture = SDL_CreateTexture(*renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, 224, 256);
    SDL_SetRenderDrawColor(*renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_Delay(1000 / 60);
    SDL_RenderClear(*renderer);
    SDL_RenderPresent(*renderer);
}

void updateFrame(invaders *machine, SDL_Renderer *renderer, SDL_Texture *texture) {
    SDL_RenderClear(renderer);
    SDL_UpdateTexture(texture, NULL, machine->videoBuffer, 224);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

uint8_t queryKey(uint8_t key) {
    const uint8_t *keyboard = SDL_GetKeyboardState(NULL);
    SDL_PumpEvents();
    switch(key) {
        case 0x0: 
            return keyboard[SDL_SCANCODE_A];
        case 0x1:
            return keyboard[SDL_SCANCODE_D];
        case 0x2:
            return keyboard[SDL_SCANCODE_5];
        case 0x3:
            return keyboard[SDL_SCANCODE_L];
        case 0x4:
            return keyboard[SDL_SCANCODE_1];
        case 0x5:
            return keyboard[SDL_SCANCODE_ESCAPE];
    }
    return 0;
}

void graphicsEnd(SDL_Window **window, SDL_Renderer **renderer, SDL_Texture **texture) {
    SDL_DestroyTexture(*texture);
    SDL_DestroyRenderer(*renderer);
    SDL_DestroyWindow(*window);
    SDL_Quit();
}
