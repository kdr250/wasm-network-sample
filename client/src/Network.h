#pragma once

#include <memory>
#include <thread>
#include <vector>

struct Vector2;

class Network
{
public:
    Network();
    bool Initialize(bool* isRunning);

private:
    std::unique_ptr<std::thread> mNetworkThread;
    std::vector<Vector2> mOtherPositions;
};
