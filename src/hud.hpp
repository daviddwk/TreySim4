#include "text.hpp"

class Hud {
    public:
        static void construct();
        static void destruct();
        static Hud& get();

        void setDeathText(bool set);

        void update(float dt);

    private:
        Hud();
        ~Hud();

        Text m_deathText;
        Text m_testText;
        Eend::PanelId m_exitPanel;

        Hud(const Hud&) = delete;
        Hud& operator=(const Hud&) = delete;

        Hud(Hud&& other) = delete;
        Hud& operator=(Hud&& other) = delete;

        inline static Hud* m_instance = nullptr;
};
