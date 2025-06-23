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
        static void construct();
        static void destruct();
        static TextBoxQueue& get();

        TextBoxQueue(const TextBoxQueue&) = delete;
        TextBoxQueue& operator=(const TextBoxQueue&) = delete;

        void queue(std::string thumbnail, Font font, std::string text, float seconds,
            bool clickToContinue);
        void update();

    private:
        TextBoxQueue() = default;
        ~TextBoxQueue() = default;

        inline static TextBoxQueue* _instance = nullptr;

        bool _continue = false;
        std::chrono::steady_clock::time_point _startTime;
        TextBox* _textBox = NULL;
        std::queue<TextBoxParams> _textBoxQueue;
};
