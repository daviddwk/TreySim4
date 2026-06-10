#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>
namespace Eend = Eendgine;

class Menu {
    public:
        static void construct();
        static void destruct();
        static Menu& get();

        void update(bool& start, bool& menu);

        Menu(const Menu&) = delete;
        Menu& operator=(const Menu&) = delete;

    private:
        Menu();
        ~Menu();

        inline static Menu* _instance = nullptr;

        Eend::PanelId m_startButton = Eend::Entities::panels().insert("resources/menu/startButton");
        Eend::PanelId m_exitButton = Eend::Entities::panels().insert("resources/menu/exitButton");
};
