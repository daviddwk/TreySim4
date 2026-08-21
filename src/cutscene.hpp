#include "trey.hpp"

#include <memory>
#include <optional>
#include <print>

enum class CutsceneType { intro };

namespace Eend = Eendgine;

class Cutscene {
    public:
        virtual ~Cutscene() = default;
        virtual bool update(float dt) = 0;
};

class CutscenePlayer {
    public:
        static void start(CutsceneType cutscene);
        // returns true if cutscene playing
        static bool update();

    private:
        inline static std::optional<CutsceneType> nextCutscene = std::nullopt;
        inline static std::optional<std::unique_ptr<Cutscene>> activeCutscene = std::nullopt;
};

/*
class TestOne : public Cutscene {
    public:
        TestOne() : m_timer(Eend::Timer(3.0f)) {
            Eend::Point treyPosition = Trey::get().getPosition();
            std::print("test\n");
            Eend::Cameras::getScene().setPosition(treyPosition + Eend::Point(0.0f, -30.0f, 10.0f));
        };
        ~TestOne() {};
        bool update(float dt) {
            Eend::Point cameraPosition = Eend::Cameras::getScene().getPosition();
            Eend::Cameras::getScene().setPosition(cameraPosition + Eend::Point(dt));
            if (m_timer.update(dt)) return false;
            return true;
        };

    private:
        Eend::Timer m_timer;
};
*/

class Intro : public Cutscene {
    public:
        Intro();
        ~Intro();
        bool update(float dt);

    private:
        Eend::Timer m_timer;
};
