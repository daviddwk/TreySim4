#include "text.hpp"
#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>

class HealthBar {
    public:
        HealthBar();
        ~HealthBar();

        bool damage(unsigned int damage);
        void heal(unsigned int heal);

    private:
        void updateBar();

        Eend::PanelId _background;
        Eend::PanelId _bar;
        unsigned int _health;
};
