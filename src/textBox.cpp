#include "textBox.hpp"

TextBoxQueue::TextBox::TextBox(
    std::string thumbnail, Font font, std::string text, float seconds, bool clickToContinue)
    : duration(seconds), clickToContinue(clickToContinue),
      arrow(Eend::Entities::panels().insert(std::filesystem::path("textbox/arrow"))),
      m_background(Eend::Entities::panels().insert(std::filesystem::path("textbox/background"))),
      m_thumbnail(Eend::Entities::panels().insert(std::filesystem::path("textbox/thumbnail"))),
      m_font(font), m_text(font, text, Eend::Point(300.0f, 750.0f, 0.0f), 50.0f, 800.0f) {

    Eend::Panel* backgroundRef = Eend::Entities::panels().getRef(m_background);
    Eend::Panel* thumbnailRef = Eend::Entities::panels().getRef(m_thumbnail);
    Eend::Panel* arrowRef = Eend::Entities::panels().getRef(arrow);

    assert(backgroundRef != NULL);
    assert(thumbnailRef != NULL);
    assert(arrowRef != NULL);

    backgroundRef->setScale(Eend::Scale2D(1000.0f, 200.0f));
    backgroundRef->setPosition(Eend::Point(100.0f, 750.0f, -10.0f));

    thumbnailRef->setTexture(thumbnail);
    thumbnailRef->setScale(Eend::Scale2D(200.0f, 200.0f));
    thumbnailRef->setPosition(Eend::Point(100.0f, 750.0f, 100.0f));

    arrowRef->setTexture("inactive");
    arrowRef->setScale(Eend::Scale2D(100.0f, 100.0f));
    arrowRef->setPosition(Eend::Point(1100.0f, 750.0f, 100.0f));
}

TextBoxQueue::TextBox::~TextBox() {
    Eend::Entities::panels().erase(m_background);
    Eend::Entities::panels().erase(m_thumbnail);
    Eend::Entities::panels().erase(arrow);
}

void TextBoxQueue::construct() {
    assert(m_instance == nullptr);
    m_instance = new TextBoxQueue;
}

void TextBoxQueue::destruct() {
    assert(m_instance != nullptr);
    delete m_instance;
    m_instance = nullptr;
}

TextBoxQueue& TextBoxQueue::get() {
    assert(m_instance != nullptr);
    return *m_instance;
}

void TextBoxQueue::queue(
    std::string thumbnail, Font font, std::string text, float seconds, bool clickToContinue) {
    m_textBoxQueue.push(TextBoxParams(thumbnail, font, text, seconds, clickToContinue));
}

void TextBoxQueue::update() {
    if (m_textBox) {
        if (m_continue) {
            Eend::Panel* arrowRef = Eend::Entities::panels().getRef(m_textBox->arrow);
            if (arrowRef->isClicked() == Eend::Panel::MouseStatus::CLICK) {
                delete m_textBox;
                m_textBox = NULL;
                m_continue = false;
                return;
            }
        }

        auto now = std::chrono::steady_clock::now();
        float secondsElapsed =
            std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime).count();

        if (secondsElapsed > m_textBox->duration) {
            if (m_textBox->clickToContinue == false) {
                delete m_textBox;
                m_textBox = NULL;
            } else {
                // change box to continue texture
                Eend::Panel* arrowRef = Eend::Entities::panels().getRef(m_textBox->arrow);
                arrowRef->setTexture("active");
                m_continue = true;
            }
        }
    } else {
        if (!m_textBoxQueue.empty()) {
            m_startTime = std::chrono::steady_clock::now();
            const TextBoxParams params = m_textBoxQueue.front();
            m_textBoxQueue.pop();
            m_textBox = new TextBox(
                params.thumbnail, params.font, params.text, params.duration,
                params.clickToContinue);
        }
    }
}
