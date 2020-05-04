#pragma once

#include <Game/Core.hpp>

namespace Walking::BlindWalker
{
    bool WalkPath(const std::vector<Tile>& tiles, int offset=1);
    std::vector<Tile> GeneratePath(const Tile& start, const Tile& dest);
    std::vector<Tile> GeneratePath(const Tile& dest);
}