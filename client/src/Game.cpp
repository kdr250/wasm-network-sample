#include "Game.h"
#include <algorithm>
#include <random>

Game::Game()
{
    std::random_device randomDevice;
    std::mt19937 mt(randomDevice());
    mId = mt();
}

bool Game::Initialize()
{
    int sdlResult = SDL_Init(SDL_INIT_VIDEO);
    if (sdlResult != 0)
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow("Wasm Network Sample",
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               WINDOW_WIDTH,
                               WINDOW_HEIGHT,
                               0);

    if (!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    mRenderer =
        SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!mRenderer)
    {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return false;
    }

    // Game specific
    mPaddlePosition.x = static_cast<float>(WINDOW_WIDTH) / 2.0f;
    mPaddlePosition.y = static_cast<float>(WINDOW_HEIGHT) / 2.0f;

    // Network
    bool networkResult = network.Initialize(this);
    if (!networkResult)
    {
        SDL_Log("Failed to initialize network...");
        return false;
    }

    return true;
}

void Game::MainLoop()
{
    ProcessInput();
    UpdateGame();
    GenerateOutput();
}

void Game::Shutdown()
{
    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

bool Game::IsRunning()
{
    return mIsRunning;
}

void Game::Receive(unsigned int id, float x, float y)
{
    // TODO
    SDL_Log("Receive! id: %i, x: %f, y: %f", id, x, y);
}

const unsigned int Game::GetId() const
{
    return mId;
}

const Vector2& Game::GetPosition() const
{
    return mPaddlePosition;
}

bool Game::IsAnyAction()
{
    return mAnyAction;
}

void Game::ProcessInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                mIsRunning = false;
                break;
        }
    }

    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_ESCAPE])
    {
        mIsRunning = false;
    }

    // Update paddle velocity
    mPaddleVelocity.x = 0.0f;
    mPaddleVelocity.y = 0.0f;
    if (state[SDL_SCANCODE_W])
    {
        mPaddleVelocity.y = -mPaddleSpeed;
    }
    if (state[SDL_SCANCODE_S])
    {
        mPaddleVelocity.y = mPaddleSpeed;
    }
    if (state[SDL_SCANCODE_A])
    {
        mPaddleVelocity.x = -mPaddleSpeed;
    }
    if (state[SDL_SCANCODE_D])
    {
        mPaddleVelocity.x = mPaddleSpeed;
    }

    mAnyAction = mPaddleVelocity.x != 0.0f || mPaddleVelocity.y != 0.0f;
}

void Game::UpdateGame()
{
    // Delta time is the difference in ticks from last frame (converted to seconds)
    float deltaTime = (SDL_GetTicks64() - mTickCount) / 1000.0f;

    // Clamp maximum delta time value
    if (deltaTime > 0.05f)
    {
        deltaTime = 0.05f;
    }

    // Update tick counts (for next frame)
    mTickCount = SDL_GetTicks64();

    // Update paddle position
    mPaddlePosition.x += mPaddleVelocity.x * deltaTime;
    mPaddlePosition.y += mPaddleVelocity.y * deltaTime;

    mPaddlePosition.x = std::clamp(mPaddlePosition.x,
                                   mPaddleThickness / 2.0f,
                                   WINDOW_WIDTH - mPaddleThickness / 2.0f);
    mPaddlePosition.y = std::clamp(mPaddlePosition.y,
                                   mPaddleThickness / 2.0f,
                                   WINDOW_HEIGHT - mPaddleThickness / 2.0f);
}

void Game::GenerateOutput()
{
#ifndef __EMSCRIPTEN__
    // Wait until 16ms has elapsed since last frame
    while (!SDL_TICKS_PASSED(SDL_GetTicks64(), mTickCount + 16))
        ;
#endif

    SDL_SetRenderDrawColor(mRenderer, 0, 0, 255, 255);

    SDL_RenderClear(mRenderer);

    SDL_SetRenderDrawColor(mRenderer, 255, 0, 0, 255);
    SDL_Rect paddle {static_cast<int>(mPaddlePosition.x - mPaddleThickness / 2),
                     static_cast<int>(mPaddlePosition.y - mPaddleThickness / 2),
                     mPaddleThickness,
                     mPaddleThickness};
    SDL_RenderFillRect(mRenderer, &paddle);

    SDL_RenderPresent(mRenderer);
}
