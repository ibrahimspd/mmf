#pragma once

#include "IPathDataService.hpp"

namespace Walking::Webwalker
{
    class DaxWalkerDataServiceEXPLV : public IPathDataService
    {
    private:

        static std::string sendPost(Tile start, Tile dest);

    public:

        DaxWalkerDataServiceEXPLV();

        std::vector<Tile> RequestPath(Tile start, Tile dest) override;
        std::vector<Tile> RequestPathToBank(Tile start) override;
        std::vector<Tile> RequestPath(std::string apiKey, std::string apiSecret, Tile start, Tile dest) override;
        std::vector<Tile> RequestPathToBank(std::string apiKey, std::string apiSecret, Tile start) override;
    };
}