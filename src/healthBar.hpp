#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>
namespace Eend = Eendgine;

class HealthBar {
    public:
        HealthBar();
        ~HealthBar();

        bool damage(unsigned int damage);
        void heal(unsigned int heal);

    private:
        void update();

        Eend::PanelId m_background;
        Eend::PanelId m_bar;
        unsigned int m_health;
};
