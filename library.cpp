#include <Game/Core.hpp>
#include <thread>
#include "HelperMethods.h"
#include <Webwalker.hpp>
#include <Communication/DaxWalkerDataServiceEXPLV.hpp>

using namespace Walking;

void Setup() {
    ScriptInfo Info;
    Info.Name = "Fungi";
    Info.Description = "Picks fungi and banks";
    Info.Version = "Version";
    Info.Category = "Category";
    Info.Author = "Author";
    Info.UID = "UID";
    Info.ForumPage = "forum.alpacabot.org";
}

static std::vector<std::string> ringsOfDueling = {"Ring of dueling(8)", "Ring of dueling(7)", "Ring of dueling(6)",
                                                  "Ring of dueling(5)", "Ring of dueling(4)", "Ring of dueling(3)",
                                                  "Ring of dueling(2)", "Ring of dueling(1)"};
static std::vector<Tile> AREA_BANK = {Tile(3351, 3175, 0), Tile(3393, 3142, 0)};
static std::vector<Tile> AREA_FUNGI = {Tile(3400, 3386, 0), Tile(3471, 3463, 0)};
static std::vector<Tile> AREA_OUTSIDE_GATE = {Tile(3425, 3467, 0), Tile(3456, 3458, 0)};
static std::vector<Tile> FUNGI_LOG_AREA = {Tile(3419, 3441, 0), Tile(3423, 3436, 0)};
static std::vector<Tile> FUNGI_LOG_TILE = {Tile(3421, 3439, 0)};
static std::vector<Tile> BANK_BOX_AREA = {Tile(3366, 3172, 0), Tile(3372, 3168, 0)};
static std::vector<Tile> OUTSIDE_PORTAL_AREA = {Tile(3352, 3166, 0), Tile(3361, 3160, 0)};
Area clanWars = Area(AREA_BANK);
Area fungiArea = Area(AREA_FUNGI);
Tile inPortal = Tile(3327, 4751, 0);
Tile fungiLogTile = Tile(3421, 3439, 0);
Area fungiLogArea = Area(FUNGI_LOG_AREA);
Tile outsidePortalTile = Tile(3354, 3163, 0);
Area outsidePortalArea(OUTSIDE_PORTAL_AREA);
static int32_t Trips = 0;
static int32_t StartLevel = 0;
static int FungiPickedUp;
static std::string Status = "Starting";
static std::thread *StatusThread = nullptr;

bool ObtainSickle() {
    return !Equipment::Contains("Silver sickle (b)");
}

bool shouldPickUp;

bool IsMoving() {
    Internal::Player P = Internal::GetLocalPlayer();
    if (P)
        return (P.GetPoseAnimationID() != P.GetSubPoseAnimationID());
    return false;
}

void PaintStatus() {
    while (!Terminate) {
        std::vector<Box> B = Inventory::GetSlotBoxes();
        Box B1 = Box(B[0].X, B[0].Y, B[27].GetX2() + 5 - B[0].X, B[27].GetY2() - B[0].Y);
        Paint::Clear();
        Paint::DrawString("Time running: " + MillisToHumanShort(GetScriptTimer().GetTimeElapsed()),
                          Point(B[0].X + 5, B[0].Y + 5), 255, 255, 255, 255);
        Paint::DrawString("Breaked for: " + MillisToHumanShort(BreakHandler::GetBreakTimer().GetTimeElapsed()),
                          Point(B[4].X + 5, B[4].Y + 5), 255, 255, 255, 255);
        Paint::DrawString("Breaks taken: " + std::to_string(BreakHandler::GetBreakCounter().GetIterations()),
                          Point(B[8].X + 5, B[8].Y + 5), 255, 255, 255, 255);
        Paint::DrawString("Status: " + Status,
                          Point(B[12].X + 5, B[12].Y + 5), 255, 255, 255, 255);
        Paint::DrawString("Trips: " + std::to_string(Trips),
                          Point(B[16].X + 5, B[20].Y + 5), 255, 255, 255, 255);
        Paint::DrawString("CurrentWorld: " + std::to_string(Worlds::GetCurrent()) + ")",
                          Point(B[20].X + 5, B[24].Y + 5), 255, 255, 255, 255);
        Paint::SwapBuffer();
        Wait(1000);
    }
}

bool ObtainDuelingRing() {
    return !Equipment::ContainsAny(ringsOfDueling);
}

bool ObtainTeleportToFungi() {
    return !Inventory::Contains("Salve graveyard teleport") &&
           (HelperMethods::InArea(clanWars) || Prayer::GetPoints() > 30) && !Inventory::Contains("Mort myre fungus") &&
           !HelperMethods::InArea(AREA_FUNGI);
}

bool TeleportToClanWars() {
    return !HelperMethods::InArea(clanWars) && (Inventory::IsFull() || (Prayer::GetPoints() == 0 && !shouldPickUp) &&
                                                                       Players::GetLocal().GetTile() != inPortal);
}

bool WalkTroughPortal() {
    return HelperMethods::InArea(clanWars) && !Inventory::Contains("Mort myre fungus") &&
           Inventory::Contains("Salve graveyard teleport") && Prayer::GetPoints() < 40;

}

bool BankFungi() {
    return HelperMethods::InArea(clanWars) && Inventory::Contains("Mort myre fungus");
}

bool TeleportToFungi() {
    return (Prayer::GetPoints() > 0 || Players::GetLocal().GetTile() == inPortal) &&
           Inventory::Contains("Salve graveyard teleport") && !HelperMethods::InArea(AREA_FUNGI);
}


bool InArea(const Area &Tiles) {
    return false;
}

bool PickUpFungi(const std::string &Name) {
    std::vector<Interactable::GameObject> Fungi = GameObjects::GetAll(Name);
    for (auto fun : Fungi) {
        if (fungiLogArea.Contains(fun.GetTile())) {
            return shouldPickUp = true;
        }
    }
    return shouldPickUp = false;
}


bool WalkToFungiLog() {
    return HelperMethods::InArea(AREA_FUNGI) &&
           Players::GetLocal().GetTile() != fungiLogTile && !shouldPickUp;
}

bool CastBloom() {
    return Players::GetLocal().GetTile() == fungiLogTile && Prayer::GetPoints() != 0 && !shouldPickUp;
}

bool clickOnTile() {
    return HelperMethods::InArea(FUNGI_LOG_AREA) && Players::GetLocal().GetTile() != fungiLogTile && !shouldPickUp;
}

bool OnStart() {
    Webwalker::SetDataService(new Webwalker::DaxWalkerDataServiceEXPLV());
    if (!StatusThread) {
        StatusThread = new std::thread(PaintStatus);
    }
    RandomHandler::SetCheckDelay(5000);
    SetLoopDelay(50);
    return true;
}


bool Loop() {
    Debug::Info << "LOOPING" << std::endl;

    if (Mainscreen::IsLoggedIn()) {
        if (!HelperMethods::IsInteracting()) {
            BreakHandler::Break();
            RandomHandler::Check();
        }
        if (ObtainSickle()) {
            Debug::Info << "Obtain sickle" << std::endl;
            if (!Bank::IsOpen() && !Inventory::Contains("Silver sickle (b)")) {
                HelperMethods::OpenBank();
                Wait(NormalRandom(800, 500 * 0.2));
            } else if (Bank::Contains("Silver sickle (b)")) {
                Bank::Withdraw("Silver sickle (b)", 1);
                Wait(NormalRandom(800, 500 * 0.2));
            } else if (Inventory::Contains("Silver sickle (b)")) {
                if (Bank::IsOpen()) {
                    Bank::Close();
                    WaitFunc(1000, 10, [=]() -> bool { return Bank::IsOpen(); }, false);
                }
                if (!Inventory::IsOpen()) {
                    Inventory::Open();
                    WaitFunc(1000, 10, [=]() -> bool { return Inventory::IsOpen(); }, true);
                } else {
                    Inventory::Equip("Silver sickle (b)");
                    WaitFunc(1000, 10, [=]() -> bool { return Equipment::Contains("Silver sickle (b)"); }, true);
                }
            }
        } else if (ObtainDuelingRing()) {
            if (HelperMethods::InArea(BANK_BOX_AREA)) {
                Debug::Info << "Obtain dueling ring" << std::endl;
                if (!Bank::IsOpen() && !Inventory::Contains(2552)) {
                    HelperMethods::OpenBank();
                    WaitFunc(1000, 10, [=]() -> bool { return Bank::IsOpen(); }, true);
                } else if (!Inventory::IsEmpty() && !Inventory::Contains(2552)) {
                    if (Bank::DepositAll()) {
                        WaitFunc(1000, 10, [=]() -> bool { return Inventory::IsEmpty(); }, true);
                    }
                } else if (Bank::Contains(2552) && !Inventory::Contains(2552)) {
                    Bank::Withdraw(2552, 1);
                    WaitFunc(1000, 10, [=]() -> bool { return Inventory::Contains(2552); }, true);
                } else if (Inventory::Contains(2552)) {
                    if (Bank::IsOpen()) {
                        Bank::Close();
                        WaitFunc(1000, 10, [=]() -> bool { return Bank::IsOpen(); }, false);
                    }
                    if (!Inventory::IsOpen()) {
                        Inventory::Open();
                        WaitFunc(1000, 10, [=]() -> bool { return Inventory::IsOpen(); }, true);
                    } else {
                        Inventory::Equip(2552);
                        WaitFunc(1000, 10, [=]() -> bool { return Equipment::Contains(2552); }, true);
                    }
                }
            } else {
                Webwalker::WalkToBank(Walking::Webwalker::CLAN_WARS_BANK);
                WaitFunc(1000, 10, [=]() -> bool { return Mainscreen::IsMoving(); }, false);
            }
        } else if (BankFungi()) {
            Debug::Info << "Bank fungi" << std::endl;
            if (HelperMethods::InArea(BANK_BOX_AREA)) {
                if (!Bank::IsOpen()) {
                    HelperMethods::OpenBank();
                    WaitFunc(1000, 10, [=]() -> bool { return Bank::IsOpen(); }, true);
                } else {
                    Bank::DepositAll();
                    Trips++;
                    WaitFunc(1000, 10, [=]() -> bool { return Inventory::IsEmpty(); }, true);
                }
            } else {
                Webwalker::WalkToBank(Walking::Webwalker::CLAN_WARS_BANK);
                WaitFunc(1000, 10, [=]() -> bool { return Mainscreen::IsMoving(); }, false);
            }
        } else if (ObtainTeleportToFungi()) {
            Debug::Info << "Obtain teleport" << std::endl;
            if (HelperMethods::InArea(BANK_BOX_AREA)) {
                if (!Bank::IsOpen() && !Inventory::Contains("Salve graveyard teleport")) {
                    HelperMethods::OpenBank();
                    WaitFunc(1000, 10, [=]() -> bool { return Bank::IsOpen(); }, true);
                } else if (!Inventory::Contains("Salve graveyard teleport") && !Inventory::IsEmpty()) {
                    Debug::Info << "Depositing all" << std::endl;
                    Bank::DepositAll();
                    WaitFunc(1000, 10, [=]() -> bool { return Inventory::IsEmpty(); }, true);

                } else if (Bank::Contains("Salve graveyard teleport") &&
                           !Inventory::Contains("Salve graveyard teleport")) {
                    Bank::Withdraw("Salve graveyard teleport", 1);
                    WaitFunc(1000, 10, [=]() -> bool { return Inventory::Contains("Salve graveyard teleport"); }, true);

                    Debug::Info << "Withdrawing all" << std::endl;
                } else if (Inventory::Contains("Salve graveyard teleport")) {
                    if (Bank::IsOpen()) {
                        Bank::Close();
                        Debug::Info << "Closing the fuck9ing bank" << std::endl;
                        WaitFunc(1000, 10, [=]() -> bool { return Bank::IsOpen(); }, false);
                    }
                }
            } else {
                Webwalker::WalkToBank(Walking::Webwalker::CLAN_WARS_BANK);
                WaitFunc(1000, 10, [=]() -> bool { return IsMoving(); }, false);
            }
        } else if (WalkTroughPortal()) {
            Debug::Info << "Walk trough portal" << std::endl;
            if (HelperMethods::InArea(outsidePortalArea)) {
                if (Bank::IsOpen()) {
                    Bank::Close();
                    WaitFunc(1000, 10, [=]() -> bool { return Bank::IsOpen(); }, false);
                } else {
                    HelperMethods::InteractWithGameObject("Free-for-all portal", "Enter");
                    WaitFunc(2000, 200, [=]() -> bool { return Players::GetLocal().GetTile() == inPortal; }, true);

                }
            } else {
                Point tile = Internal::TileToMinimap(outsidePortalTile);
                Interact::Click(tile);
                WaitFunc(1000, 50, [=]() -> bool { return IsMoving(); }, false);
            }
        } else if (TeleportToClanWars()) {
            if (!Equipment::IsOpen()) {
                Equipment::Open();
                WaitFunc(1000, 50, [=]() -> bool { return Equipment::IsOpen(); }, true);
            } else {
                if (Equipment::GetItem(Equipment::SLOT::RING).Interact("Clan Wars")) {
                    WaitFunc(3000, 100, [=]() -> bool { return InArea(clanWars); }, true);
                }
            }
        } else if (TeleportToFungi()) {
            Debug::Info << "Teleport to fungi" << std::endl;
            if (Bank::IsOpen()) {
                Bank::Close();
                WaitFunc(1000, 50, [=]() -> bool { return Bank::IsOpen(); }, false);
            } else if (!Inventory::IsOpen()) {
                Inventory::Open();
                WaitFunc(1000, 50, [=]() -> bool { return Inventory::IsOpen(); }, true);
            } else if (!Bank::IsOpen()) {
                if (Inventory::GetItem("Salve graveyard teleport").Interact()) {
                    WaitFunc(1000, 100, [=]() -> bool { return InArea(fungiArea); }, true);
                }

            }
        } else if (clickOnTile()) {
            Debug::Info << "Clicking on tile" << std::endl;
            Point tile = Internal::TileToMinimap(fungiLogTile);
            Interact::Click(tile);
            WaitFunc(1000, 100, [=]() -> bool { return Mainscreen::IsMoving(); }, false);
        } else if (WalkToFungiLog()) {
            Debug::Info << "Walking to log" << std::endl;
            Webwalker::WalkTo(fungiLogTile);
            WaitFunc(1000, 100, [=]() -> bool { return Mainscreen::IsMoving(); }, false);
            return true;
        } else if (CastBloom()) {
            Debug::Info << "Cast bloom" << std::endl;
            if (!Equipment::IsOpen()) {
                Equipment::Open();
                WaitFunc(1000, 50, [=]() -> bool { return Equipment::IsOpen(); }, true);
            } else {
                Equipment::GetItem(Equipment::SLOT::WEAPON).Interact("Bloom");
                WaitFunc(1000, 50, [=]() -> bool { return PickUpFungi("Fungi on log"); }, true);
                shouldPickUp = true;
            }
        } else if (PickUpFungi("Fungi on log")) {
            Debug::Info << "Picking up fungi" << std::endl;
            if (HelperMethods::InteractWithFungi("Fungi on log", "Pick", FUNGI_LOG_AREA, &FungiPickedUp)) {
                Wait(NormalRandom(800, 500 * 0.2));
                FungiPickedUp++;
            }

        }
    } else {
        Login::LoginPlayer();
        WaitFunc(1000, 50, [=]() -> bool { return Mainscreen::IsLoggedIn(); }, true);
    }
    return true;
}

bool OnBreak() {
    Antiban::MouseOffClient();
    return true;
}

void OnEnd() {
    if (StatusThread) {
        if (StatusThread->joinable())
            StatusThread->join();
        delete StatusThread;
        StatusThread = nullptr;
    }
}