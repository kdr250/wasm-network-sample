#pragma once

#include <SDL2/SDL.h>

class Game
{
public:
    Game();

    // Initailize game
    bool Initialize();

    // Runs the game loop until the game is over
    void MainLoop();

    // shutdown the game
    void Shutdown();

    bool IsRunning();

private:
    // Herlper functions for the game loop
    void ProcessInput();

    void UpdateGame();

    void GenerateOutput();

    // Window created by SDL
    SDL_Window* mWindow = nullptr;

    // Renderer for 2D drawing
    SDL_Renderer* mRenderer = nullptr;

    // Number of ticks since start of game
    Uint64 mTickCount = 0;

    // Game should continue to run
    bool mIsRunning = true;
};
