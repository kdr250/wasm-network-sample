#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #include <emscripten/html5.h>
#endif

#include "Game.h"

int main(int argc, char* argv[])
{
    Game game;

    if (!game.Initialize())
    {
        return EXIT_FAILURE;
    }

#ifdef __EMSCRIPTEN__
    auto callback = [](void* arg)
    {
        Game* pGame = reinterpret_cast<Game*>(arg);
        if (!pGame->IsRunning())
        {
            emscripten_cancel_main_loop();
            pGame->Shutdown();
            return;
        }
        pGame->MainLoop();
    };
    emscripten_set_main_loop_arg(callback, &game, 60, true);
#else
    while (game.IsRunning())
    {
        game.MainLoop();
    }
    game.Shutdown();
#endif

    return EXIT_SUCCESS;
}
