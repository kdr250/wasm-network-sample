#include "Network.h"
#include <SDL2/SDL.h>
#include "Game.h"

Network::Network() {}

void Network::Initialize(bool* isRunning)
{
    networkThread = std::make_unique<std::thread>(
        [isRunning]()
        {
            while (*isRunning)
            {
                SDL_Log("Hello world!");
            }
            SDL_Log("Finish!");
        });
    networkThread->detach();
}
