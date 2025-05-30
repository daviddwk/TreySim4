#include "textBox.hpp"

TextBox::TextBox(std::string thumbnail, Font font, std::string text, float seconds)
    : duration(seconds),
      _background(Eend::Entities::PanelBatch::insert(std::filesystem::path("textbox/background"))),
      _thumbnail(Eend::Entities::PanelBatch::insert(std::filesystem::path("textbox/thumbnail"))),
      _font(font), _text(font, text, Eend::Point(300.0f, 750.0f, 0.0f), 50.0f, 800.0f) {
    Eend::Panel* backgroundRef = Eend::Entities::PanelBatch::getRef(_background);
    Eend::Panel* thumbnailRef = Eend::Entities::PanelBatch::getRef(_thumbnail);
    assert(backgroundRef != NULL);
    assert(thumbnailRef != NULL);

    backgroundRef->setScale(Eend::Scale2D(1000.0f, 200.0f));
    backgroundRef->setPosition(Eend::Point(100.0f, 750.0f, -10.0f));

    thumbnailRef->setTexture(thumbnail);
    thumbnailRef->setScale(Eend::Scale2D(200.0f, 200.0f));
    thumbnailRef->setPosition(Eend::Point(100.0f, 750.0f, 100.0f));
}

TextBox::~TextBox() {
    Eend::Entities::PanelBatch::erase(_background);
    Eend::Entities::PanelBatch::erase(_thumbnail);
}

void TextBoxQueue::queue(std::string thumbnail, Font font, std::string text, float seconds) {
    _textBoxQueue.push(TextBoxParams(thumbnail, font, text, seconds));
}

void TextBoxQueue::update() {
    if (_textBox) {
        auto now = std::chrono::steady_clock::now();
        float secondsElapsed =
            std::chrono::duration_cast<std::chrono::seconds>(now - _startTime).count();
        if (secondsElapsed > _textBox->duration) {
            delete _textBox;
            _textBox = NULL;
        }
    }
    if (_textBox == NULL) {
        if (!_textBoxQueue.empty()) {
            _startTime = std::chrono::steady_clock::now();
            const TextBoxParams params = _textBoxQueue.front();
            _textBoxQueue.pop();
            _textBox = new TextBox(params.thumbnail, params.font, params.text, params.duration);
        }
    }
}
