#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>
#include "Network.h"

struct Vector2
{
    float x;
    float y;
};

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

    void Receive(int id, float x, float y);

    void SetId(int id);

    const unsigned int GetId() const;

    const Vector2& GetPosition() const;

    bool IsAnyAction();

private:
    // Herlper functions for the game loop
    void ProcessInput();

    void UpdateGame();

    void GenerateOutput();

    static constexpr int WINDOW_WIDTH  = 1024;
    static constexpr int WINDOW_HEIGHT = 768;

    // Window created by SDL
    SDL_Window* mWindow = nullptr;

    // Renderer for 2D drawing
    SDL_Renderer* mRenderer = nullptr;

    // Number of ticks since start of game
    Uint64 mTickCount = 0;

    // Game should continue to run
    bool mIsRunning = true;

    // Game specific
    Vector2 mPaddlePosition;
    Vector2 mPaddleVelocity;
    float mPaddleSpeed   = 200.0f;
    int mPaddleThickness = 50;

    Network network;
    int mId         = 0;
    bool mAnyAction = false;
    std::unordered_map<unsigned int, Vector2> otherPositions;
};
