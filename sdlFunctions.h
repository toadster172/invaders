#ifndef SDLFUNCTIONS_H
#define SDLFUNCTIONS_H

#include <stdint.h>
#include <SDL.h>
#include "invaders.h"

void graphicsInit(SDL_Window **window, SDL_Renderer **renderer, SDL_Texture **texture, uint8_t scale);
void updateFrame(invaders *machine, SDL_Renderer *renderer, SDL_Texture *texture);
uint8_t queryKey(uint8_t key);
void graphicsEnd(SDL_Window **window, SDL_Renderer **renderer, SDL_Texture **texture);

#endif
