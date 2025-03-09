#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #include <emscripten/html5.h>
#endif

SDL_Window* window     = nullptr;
SDL_Renderer* renderer = nullptr;
bool isRunning         = true;

void Terminate()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void MainLoop()
{
#ifdef __EMSCRIPTEN__
    if (!isRunning)
    {
        emscripten_cancel_main_loop();
        Terminate();
        return;
    }
#endif

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                isRunning = false;
                break;
        }
    }

    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_ESCAPE])
    {
        isRunning = false;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);
}

int main(int argc, char** argv)
{
    int sdlResult = SDL_Init(SDL_INIT_VIDEO);
    if (sdlResult != 0)
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    window = SDL_CreateWindow("Wasm Network Sample",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              1024,
                              768,
                              0);

    if (!window)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer)
    {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(MainLoop, 0, 1);
#else
    while (isRunning)
    {
        MainLoop();
    }
    Terminate();
#endif

    return EXIT_SUCCESS;
}
