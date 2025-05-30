#include "textBox.hpp"

TextBox::TextBox(
    std::string thumbnail, Font font, std::string text, float seconds, bool clickToContinue)
    : duration(seconds), clickToContinue(clickToContinue),
      _background(Eend::Entities::PanelBatch::insert(std::filesystem::path("textbox/background"))),
      _thumbnail(Eend::Entities::PanelBatch::insert(std::filesystem::path("textbox/thumbnail"))),
      _arrow(Eend::Entities::PanelBatch::insert(std::filesystem::path("textbox/arrow"))),
      _font(font), _text(font, text, Eend::Point(300.0f, 750.0f, 0.0f), 50.0f, 800.0f) {

    Eend::Panel* backgroundRef = Eend::Entities::PanelBatch::getRef(_background);
    Eend::Panel* thumbnailRef = Eend::Entities::PanelBatch::getRef(_thumbnail);
    Eend::Panel* arrowRef = Eend::Entities::PanelBatch::getRef(_arrow);

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

TextBox::~TextBox() {
    Eend::Entities::PanelBatch::erase(_background);
    Eend::Entities::PanelBatch::erase(_thumbnail);
    Eend::Entities::PanelBatch::erase(_arrow);
}

void TextBoxQueue::queue(
    std::string thumbnail, Font font, std::string text, float seconds, bool clickToContinue) {
    _textBoxQueue.push(TextBoxParams(thumbnail, font, text, seconds, clickToContinue));
}

void TextBoxQueue::update() {
    if (_textBox) {
        if (_continue) {
            Eend::Panel* arrowRef = Eend::Entities::PanelBatch::getRef(_textBox->_arrow);
            if (arrowRef->isClicked() == Eend::Panel::MouseStatus::CLICK) {
                delete _textBox;
                _textBox = NULL;
                _continue = false;
                return;
            }
        }

        auto now = std::chrono::steady_clock::now();
        float secondsElapsed =
            std::chrono::duration_cast<std::chrono::seconds>(now - _startTime).count();

        if (secondsElapsed > _textBox->duration) {
            if (_textBox->clickToContinue == false) {
                delete _textBox;
                _textBox = NULL;
            } else {
                // change box to continue texture
                Eend::Panel* arrowRef = Eend::Entities::PanelBatch::getRef(_textBox->_arrow);
                arrowRef->setTexture("active");
                _continue = true;
            }
        }
    } else {
        if (!_textBoxQueue.empty()) {
            _startTime = std::chrono::steady_clock::now();
            const TextBoxParams params = _textBoxQueue.front();
            _textBoxQueue.pop();
            _textBox = new TextBox(params.thumbnail, params.font, params.text, params.duration,
                params.clickToContinue);
        }
    }
}
