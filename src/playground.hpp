#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>

#include "portal.hpp"

namespace Eend = Eendgine;

class Playground {

    public:
        Playground() {};
        ~Playground() {
            for (auto& board : boards)
                Eend::Entities::boards().erase(std::get<Eend::BoardId>(board));
            for (Eend::StatueId& statue : statues)
                Eend::Entities::statues().erase(statue);
            for (auto& doll : dolls)
                Eend::Entities::dolls().erase(std::get<Eend::DollId>(doll));
        }

        std::vector<Eend::Rectangle> collision;
        std::vector<Portal> portals;
        std::vector<std::tuple<Eend::BoardId, float>> boards;
        std::vector<Eend::StatueId> statues;
        std::vector<std::tuple<Eend::DollId, float>> dolls;
};
