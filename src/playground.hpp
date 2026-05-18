#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>

#include "portal.hpp"

namespace Eend = Eendgine;

class Playground {

    public:
        Playground(std::string playgroundName) : name(playgroundName) {};
        ~Playground() {
            for (auto& board : m_boards)
                Eend::Entities::boards().erase(std::get<Eend::BoardId>(board));
            for (Eend::StatueId& statue : m_statues)
                Eend::Entities::statues().erase(statue);
            for (auto& doll : m_dolls)
                Eend::Entities::dolls().erase(std::get<Eend::DollId>(doll));
        }

        std::vector<Eend::Rectangle> m_collisionRectangles;
        std::vector<std::tuple<Eend::BoardId, float>> m_boards;
        std::vector<Eend::StatueId> m_statues;
        std::vector<std::tuple<Eend::DollId, float>> m_dolls;

        const std::string name;
};
