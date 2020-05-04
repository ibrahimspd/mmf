#pragma once

#include "IPathDataService.hpp"

namespace Walking::Webwalker
{
    class DaxWalkerDataServiceTCP : public IPathDataService
    {
    private:

        std::string address;
        std::string port;

        std::vector<std::string> SplitString(std::string text, char delim);
        std::vector<int> SplitInt(std::string text, char delim);
        std::vector<Tile> ParseResponse(std::string response);

    public:

        DaxWalkerDataServiceTCP(std::string IpAddress, std::string port);

        std::vector<Tile> RequestPath(Tile start, Tile dest) override;
        std::vector<Tile> RequestPathToBank(Tile start) override;
        std::vector<Tile> RequestPath(std::string apiKey, std::string apiSecret, Tile start, Tile dest) override;
        std::vector<Tile> RequestPathToBank(std::string apiKey, std::string apiSecret, Tile start) override;
    };
}