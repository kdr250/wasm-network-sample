#pragma once

#include <memory>
#include <thread>
#include <vector>

struct Vector2;

class Network
{
public:
    Network();
    bool Initialize(class Game* game);

private:
    std::unique_ptr<std::thread> mNetworkThread;
};
