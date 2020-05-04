#pragma once

#include <Game/Core.hpp>

namespace Walking::WalkingUtils
{
    bool IsRunOn();
    bool SetRun(bool on);

    Tile GetRandomTile(const Tile& tile, int offset);

    bool IsNearTile(const Tile& tile, int distance);

    bool WaitUntilNearTile(const Tile& tile, int distance, int timeout=-1, int step=-1);
}