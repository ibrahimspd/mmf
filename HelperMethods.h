//
// Created by ibra on 2020-04-17.
//

#ifndef FISHER_HELPERMETHODS_H
#define FISHER_HELPERMETHODS_H

#endif //FISHER_HELPERMETHODS_H

#include <Game/Core.hpp>

namespace HelperMethods {
    inline bool IsNearTile(Tile tile, int distance) {
        return tile.DistanceFrom(Minimap::GetPosition()) <= distance && Minimap::GetPosition().Plane == tile.Plane;
    }

    bool InArea(const Area &Tiles) {
        return Area(Tiles).Contains(Minimap::GetPosition());
    }

    bool InConvex(const Convex &Point) {
        return Convex(Point).Contains(Minimap::GetMiddle());
    }

    bool WalkToTile(const std::vector<Tile> &tileVector, const Tile &Tiles) {
        if (Players::GetLocal().GetTile() != Tiles) {
            if (Minimap::WalkPath(Pathfinding::FindPathTo(tileVector.back()), 0, 2)) {
                return true;
            } else {
                if (Minimap::WalkPath(tileVector, 0, 2))
                    return true;
            }
        }
        return true;
    }

    bool WalkToArea(const Area &Tiles, const std::vector<Tile> &BackupPath) {
        if (!InArea(Tiles)) {
            if (Minimap::WalkPath(Pathfinding::FindPathTo(BackupPath.back()), 0, 2)) {
                return true;//WaitFunc(10000, 100, IsMoving, false);
            } else {
                if (Minimap::WalkPath(BackupPath, 0, 2))
                    return true;//WaitFunc(10000, 100, IsMoving, false);
            }
        }
        return true;
    }

    bool InteractWithWallObject(const int &Name, const std::string &Action) {
        std::vector<Interactable::WallObject> wallObject = WallObjects::GetAll(Name);
        for (auto p : wallObject) {
            if (p.GetVisibility() < 0.3)
                Camera::RotateTo(p.GetTile(), 0);

            if (p.GetVisibility() < 0.3)
                Minimap::WalkPath(Pathfinding::FindWalkableTiles(p.GetTile()), 4, 2);

            if (p) {
                p.Interact(Action);
                Wait(NormalRandom(1500, 500 * 0.2));

            }
        }
        return false;
    }

    bool InteractWithGameObject(const std::string &Name, const std::string &Action) {
        std::vector<Interactable::GameObject> gameObject = GameObjects::GetAll(Name);
        for (auto p : gameObject) {

            if (p.GetVisibility() < 0.3)
                Minimap::WalkPath(Pathfinding::FindWalkableTiles(p.GetTile()), 4, 2);
            if (p) {
                p.Interact(Action);
            }
        }
        return false;
    }

    static int getAmountPerHour(int numberOfItems, long runtimeInMilliseconds) {
        return (int) (numberOfItems * ((double) 3600000 / runtimeInMilliseconds));
    }

    bool InteractWithFungi(const std::string &Name, const std::string &Action, const Area &area, int *count) {
        std::vector<Interactable::GameObject> gameObject = GameObjects::GetAll(Name);
        Interactable::GameObject gameObject1;
        Interactable::GameObject gameObject2;
        Interactable::GameObject gameObject3;
        for (auto p : gameObject) {
            if (p.GetVisibility() < 0.3)
                Camera::RotateTo(p.GetTile(), 0);

            if (p.GetTile() == Tile(3420, 3438, 0)) {
                gameObject1 = p;
            }
            if (p.GetTile() == Tile(3422, 3438, 0)) {
                gameObject2 = p;
            }
            if (p.GetTile() == Tile(3421, 3440, 0)) {
                gameObject3 = p;
            }
//                if (area.Contains(p.GetTile())) {
//                    p.Interact(Action);
//                    Wait(NormalRandom(1500, 500 * 0.2));
//                }
        }
        if (gameObject1) {
            if (gameObject1.Interact()) {
                Wait(NormalRandom(800, 500 * 0.2));
                count++;
            }

        }
        if (gameObject2) {
            if (gameObject2.Interact()) {
                Wait(NormalRandom(800, 500 * 0.2));
                count++;
            }

        }
        if (gameObject3) {
            if (gameObject3.Interact()) {
                Wait(NormalRandom(800, 500 * 0.2));
                count++;
            }

        }
        return false;
    }

    bool InteractNPC(const std::string &Name, const std::string &Action) {
        std::vector<Interactable::NPC> Mobs = NPCs::GetAll(Name);

        for (auto Mob : Mobs) {
            if (Mob.GetVisibility() < 0.3)
                Camera::RotateTo(Mob.GetTile(), 0);

            if (Mob.GetVisibility() < 0.3)
                Minimap::WalkPath(Pathfinding::FindWalkableTiles(Mob.GetTile()), 4, 2);

            if (Mob.Interacting(Players::GetLocal())) {
                Debug::Info << "We are already interecting with this NPC" << std::endl;
                return Mob.Interact(Action);
            }

            if (Mob.Interacting()) {
                Debug::Info << "Mob is interacting with someone else" << std::endl;
                continue;
            }
            if (Internal::GetHealthPercentage(Mob) == 0) {
                //Debug::Info << "Mob is already dead" << std::endl;
                continue;
            }

            return Mob.Interact(Action);
        }
        return false;
    }

    bool IsInteracting() {
        Internal::Player P = Internal::GetLocalPlayer();
        if (P)
            return (P.GetInteractIndex() != -1);
        return false;
    }

    inline bool OpenBank() {
        Interactable::GameObject bankObject = GameObjects::Get([](Interactable::GameObject object) -> bool {
            if (object.GetTile().DistanceFrom(Minimap::GetPosition()) > 10) return false;

            bool objectNameIsValid = object.GetName().find("Bank booth") != std::string::npos ||
                                     object.GetName().find("Bank chest") != std::string::npos;

            if (objectNameIsValid) {
                for (auto &op : object.GetInfo().GetActions()) {
                    if (op.find("Bank") != std::string::npos || op.find("Use") != std::string::npos)
                        return true;
                }
            }

            return false;
        });

        if (Bank::IsEnterPinOpen()) {
            Bank::EnterPin();
            return true;
        }

        if (bankObject) {
            for (auto &op : bankObject.GetInfo().GetActions()) {
                if (op.find("Bank") != std::string::npos || op.find("Use") != std::string::npos)
                    return bankObject.Interact(op);
            }
        } else {
            Interactable::WallObject GeBankBooth = WallObjects::Get([](Interactable::WallObject object) -> bool {
                bool objectNameIsValid = object.GetName().find("Grand Exchange booth") != std::string::npos;

                if (objectNameIsValid) {
                    for (auto &op : object.GetInfo().GetActions()) {
                        if (op.find("Bank") != std::string::npos)
                            return true;
                    }
                }

                return false;
            });

            if (GeBankBooth) {
                for (auto &op : GeBankBooth.GetInfo().GetActions()) {
                    if (op.find("Bank") != std::string::npos) {
                        Debug::Info << op << std::endl;
                        return GeBankBooth.Interact(op);
                    }
                }
            }
        }

        return false;
    }

    inline bool WaitUntilInBank() {
        return WaitFunc(UniformRandom(2000, 3500), UniformRandom(50, 450), []() -> bool { return Bank::IsOpen(); });
    }
}