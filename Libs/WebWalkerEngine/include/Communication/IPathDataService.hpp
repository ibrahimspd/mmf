#pragma once

#include <Game/Core.hpp>

namespace Walking
{
    class IPathDataService
    {
    public:
        virtual std::vector<Tile> RequestPath(Tile start, Tile dest) = 0;
        virtual std::vector<Tile> RequestPathToBank(Tile start) = 0;
        virtual std::vector<Tile> RequestPath(std::string apiKey, std::string apiSecret, Tile start, Tile dest) = 0;
        virtual std::vector<Tile> RequestPathToBank(std::string apiKey, std::string apiSecret, Tile start) = 0;
    };
}