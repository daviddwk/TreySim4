#include "text.hpp"
#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>
#include <chrono>
#include <print>
#include <queue>

namespace Eend = Eendgine;

class TextBox {
    public:
        TextBox(std::string thumbnail, Font font, std::string text, float seconds,
            bool clickToContinue);
        ~TextBox();
        const float duration;
        const bool clickToContinue;

        Eend::PanelId _arrow;

    private:
        Eend::PanelId _background;
        Eend::PanelId _thumbnail;
        Font _font;
        Text _text;
};

struct TextBoxParams {
        std::string thumbnail;
        Font font;
        std::string text;
        float duration;
        bool clickToContinue;
};

class TextBoxQueue {
    public:
        static void queue(std::string thumbnail, Font font, std::string text, float seconds,
            bool clickToContinue);
        static void update();

    private:
        inline static bool _continue = false;
        inline static std::chrono::steady_clock::time_point _startTime;
        inline static TextBox* _textBox = NULL;
        inline static std::queue<TextBoxParams> _textBoxQueue;
};
