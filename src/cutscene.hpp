#include <memory>
#include <optional>

enum class CutsceneType { testOne };

class Cutscene {
    public:
        virtual ~Cutscene() = default;
        virtual bool update() = 0;
};

class CutscenePlayer {
    public:
        static void start(CutsceneType cutscene);
        // returns true if cutscene playing
        static bool update();

    private:
        inline static std::optional<std::unique_ptr<Cutscene>> activeCutscene = std::nullopt;
};

class TestOne : public Cutscene {
    public:
        TestOne() {};
        ~TestOne() {};
        bool update() { return false; };
};
