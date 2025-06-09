#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>

#include "text.hpp"

class HealthBar {
    public:
        HealthBar();
        ~HealthBar();

        bool damage(unsigned int damage);
        void heal(unsigned int heal);

    private:
        void update();

        Eend::PanelId _background;
        Eend::PanelId _bar;
        unsigned int _health;
};
