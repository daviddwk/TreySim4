#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>

namespace Eend = Eendgine;

class Playground {

    public:
        Playground(std::string playgroundName) : name(playgroundName), m_isEnabled(false) {};
        void enable() { m_isEnabled = true; };
        void disable() { m_isEnabled = false; };
        bool isEnabled() { return isEnabled(); };

    private:
        const std::string name;
        bool m_isEnabled;

        std::vector<Eend::Rectangle> m_collisionRectangles;
        std::vector<std::tuple<Eend::BoardId, float>> m_boards;
        std::vector<Eend::StatueId> m_statues;
        std::vector<std::tuple<Eend::DollId, float>> m_dolls;
};
