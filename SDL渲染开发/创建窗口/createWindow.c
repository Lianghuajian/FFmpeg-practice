#include <SDL.h>
#include <stdio.h>
int main(int argc , char * argv[])
{
SDL_Window *window = NULL;   
SDL_Renderer *render = NULL;
SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);   
SDL_Log("Hello world!");
window = SDL_CreateWindow("Hello World!",300,300,640,480,SDL_WINDOW_SHOWN);

if (!window)
{
SDL_Log("Failed to create window");
goto DWindow;
}

render = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);

if (!render)
{
SDL_Log("Failed to create render");
goto DWindow;
}


SDL_RenderClear(render);

SDL_SetRenderDrawColor(render,255,20,0,200);

SDL_RenderPresent(render);

SDL_Delay(5000);

DWindow:
SDL_DestroyWindow(window);
SDL_DestroyRenderer(render);
exit:
SDL_Quit();

return 0;
}
