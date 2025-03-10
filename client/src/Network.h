#pragma once

#include <memory>
#include <thread>
#include <vector>

class Network
{
public:
    Network();
    void Initialize(bool* isRunning);

private:
    std::unique_ptr<std::thread> networkThread;
    std::vector<struct Vector2> otherPositions;
};
