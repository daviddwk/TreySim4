#include "text.hpp"
#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>
#include <chrono>
#include <queue>

namespace Eend = Eendgine;

class TextBoxQueue {
    public:
        static void construct();
        static void destruct();
        static TextBoxQueue& get();

        void queue(
            std::string thumbnail, Font font, std::string text, float seconds,
            bool clickToContinue);
        void update();

    private:
        class TextBox {
            public:
                TextBox(
                    std::string thumbnail, Font font, std::string text, float seconds,
                    bool clickToContinue);
                ~TextBox();
                const float duration;
                const bool clickToContinue;

                Eend::PanelId arrow;

            private:
                Eend::PanelId m_background;
                Eend::PanelId m_thumbnail;
                Font m_font;
                Text m_text;
        };

        struct TextBoxParams {
                std::string thumbnail;
                Font font;
                std::string text;
                float duration;
                bool clickToContinue;
        };

        TextBoxQueue();
        ~TextBoxQueue();

        TextBoxQueue(const TextBoxQueue&) = delete;
        TextBoxQueue& operator=(const TextBoxQueue&) = delete;

        TextBoxQueue(TextBoxQueue&& other) = delete;
        TextBoxQueue& operator=(TextBoxQueue&& other) = delete;

        inline static TextBoxQueue* m_instance = nullptr;

        bool m_continue = false;
        std::chrono::steady_clock::time_point m_startTime;
        TextBox* m_textBox = NULL;
        std::queue<TextBoxParams> m_textBoxQueue;
};
