#ifndef GUARD_PLATFORM_DEVELOPER_MAP_CAMERA_H
#define GUARD_PLATFORM_DEVELOPER_MAP_CAMERA_H

#ifdef PLATFORM_SDL2

#ifdef __ANDROID__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include <stdbool.h>

bool DeveloperMapCamera_Open(SDL_Renderer *renderer);
void DeveloperMapCamera_Close(void);
bool DeveloperMapCamera_IsOpen(void);
bool DeveloperMapCamera_HandleEvent(const SDL_Event *event, SDL_Window *window, SDL_Renderer *renderer, const SDL_Rect *viewport);
void DeveloperMapCamera_Draw(SDL_Renderer *renderer, const SDL_Rect *viewport);
void DeveloperMapCamera_UpdateWindowTitle(SDL_Window *window);

#endif // PLATFORM_SDL2

#endif // GUARD_PLATFORM_DEVELOPER_MAP_CAMERA_H
