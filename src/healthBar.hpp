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

        unsigned int getHealth();

    private:
        const unsigned int M_HEALTH_MAX = 100;
        const float M_BAR_SCALE_MAX = 500.0f;

        void update();

        Eend::PanelId m_background;
        Eend::PanelId m_bar;
        unsigned int m_health;
};
