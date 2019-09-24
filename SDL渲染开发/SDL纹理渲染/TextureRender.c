#include "SDL.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    
    int flag = 1;
    
    SDL_Window *window;                    // Declare a pointer
    SDL_Renderer *renderer;
    
    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2
    
    // Create an application window with the following settings:
    window = SDL_CreateWindow(
                              "An SDL2 window",                  // window title
                              SDL_WINDOWPOS_UNDEFINED,           // initial x position
                              SDL_WINDOWPOS_UNDEFINED,           // initial y position
                              640,                               // width, in pixels
                              480,                               // height, in pixels
                              SDL_WINDOW_SHOWN// flags - see below
                              );
    
    // Check that the window was successfully created
    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }
    
    renderer = SDL_CreateRenderer(window,-1,0);
    
    // The window is open: could enter program loop here (see SDL_PollEvent())
    int quit = 0;
    
    SDL_Texture *texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_TARGET,640,480);
    
    if (!texture) {
        SDL_Log("Failed to create Texture");
        goto dRender;
    }
    
    SDL_Rect rect;
    rect.w = 30;
    rect.h = 30;    
    do {
        quit += 1;
        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type) {
            case SDL_QUIT:
                quit = 0;
                break;
            default:
                SDL_Log("event type is %d",event.type);
        }
        
        rect.x = rand() % 600;
        
        rect.y = rand() % 450;
        
        SDL_SetRenderTarget(renderer,texture);
        //画背景
        SDL_SetRenderDrawColor(renderer,0,255,0,0);
    
        SDL_RenderClear(renderer);
        //画方块
        SDL_RenderDrawRect(renderer,&rect);
        
        SDL_SetRenderDrawColor(renderer,255,0,0,0);
        
        SDL_RenderFillRect(renderer,&rect);
        
        SDL_SetRenderTarget(renderer,NULL);
        
        SDL_RenderCopy(renderer,texture,NULL,NULL);
        
        SDL_RenderPresent(renderer);
        
    } while (quit > 0);
SDL_DestroyTexture(texture);

dRender:
    //destory renderer
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    
    // Close and destroy the window
    SDL_DestroyWindow(window);
    
    // Clean up
    SDL_Quit();
    return 0;
}
