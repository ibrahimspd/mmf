#pragma once

#include <Game/Core.hpp>

#include <string>
#include "Communication/IPathDataService.hpp"
#include "Communication/DaxWalkerDataServiceTCP.hpp"
#include "Communication/DaxWalkerDataServiceHTTP.hpp"

namespace Walking::Webwalker
{
    static std::string apiKey = "";
    static std::string apiSecret = "";
    static IPathDataService *dataService = nullptr;

    enum Bank { VARROCK_WEST, VARROCK_EAST, FALADOR_EAST, FALADOR_WEST, DRAYNOR_BANK, CATHERBY_BANK, CAMELOT_BANK,
        ARDOUGNE_BANK, YANILLE_BANK, CASTLE_WARS_BANK, SHANTAY_PASS_BANK, ALKHARID_BANK, CANFIS_BANK, PORT_PHASMATYS,
        CLAN_WARS_BANK, LUMBRIDGE_BANK, EGDEVILLE_BANK, GNOME_STRONGHOLD_BANK, BARBARIAN_ASSULT_BANK };

    Tile BankToTile(int bank);
    bool WalkTo(const Tile& dest);
    bool WalkToBank();
    bool WalkToBank(Bank bank);

    void SetDataService(IPathDataService *service);
}