#include <Eendgine/fatalError.hpp>

#include <filesystem>
#include <fstream>
#include <json/json.h>
#include <limits>
#include <optional>
#include <print>
#include <stb/stb_image.h>

#include "Eendgine/types.hpp"
#include "text.hpp"

Text::Text(Font font, std::string text, Eend::Point position, float scale, float width)
    : _text(text), _width(width), _position(position), _scale(scale), _fontPath(""),
      _charColumns({std::nullopt}) {

    // auto rootPath = std::filesystem::path("resources") / std::filesystem::path("fonts");
    _fontPath = std::filesystem::path("fonts");
    switch (font) {
    case DANIEL:
        _fontPath = _fontPath / std::filesystem::path("daniel");
        break;
    }
    _texture = Eend::loadTexture("resources" / _fontPath / "font.png");

    // open json
    Json::Value root;
    std::filesystem::path metadataPath = "resources" / _fontPath / "metadata.json";
    std::ifstream metadata(metadataPath);
    if (!metadata.is_open()) {
        Eend::fatalError("could not open: " + metadataPath.string());
    }
    try {
        metadata >> root;
    } catch (...) {
        Eend::fatalError("improper json: " + metadataPath.string());
    }

    // ' ' to '~' see https://www.ascii-code.com/
    for (char ch = 32; ch < 127; ++ch) {
        Json::Value jsonChar = root[std::format("{}", ch)];
        if (!jsonChar.isArray())
            continue;
        const unsigned int firstColumn = jsonChar[0].asUInt();
        const unsigned int lastColumn = jsonChar[1].asUInt();

        bool first_after_last = firstColumn > lastColumn;
        bool exceeded_max = (firstColumn >= _texture.width) || (lastColumn >= _texture.width);

        if (!first_after_last && !exceeded_max) {
            _charColumns[ch] = std::tie(firstColumn, lastColumn);
        } else {
            Eend::fatalError("malformatted font metadat\n");
            // print error and move on
        }
    }

    // this should point to a folder with just font pngs?
    // need to unwrap optional here
    for (size_t char_idx = 0; char_idx < _text.length(); ++char_idx) {
        _panelIds.push_back(Eend::Entities::getPanels().insert(_fontPath));
    }
    updateText();
}

Text::~Text() {
    for (const Eend::PanelId& id : _panelIds) {
        Eend::Entities::getPanels().erase(id);
    }
}

void Text::setText(const std::string& text) {
    if (_text == text)
        return;

    _text = text;

    for (const Eend::PanelId& id : _panelIds) {
        Eend::Entities::getPanels().erase(id);
    }
    _panelIds.clear();
    for (size_t char_idx = 0; char_idx < _text.length(); ++char_idx) {
        _panelIds.push_back(Eend::Entities::getPanels().insert(_fontPath));
    }
    updateText();
}

void Text::setPosition(Eend::Point position) {

    _position = position;
    updateText();
}

void Text::setScale(float scale) {

    _scale = scale;
    updateText();
}

void Text::updateText() {
    float horizontal = 0.0f;
    float vertical = 0.0f;
    for (std::string::size_type char_idx = 0; char_idx < _text.length(); ++char_idx) {
        char glyph = _text[char_idx];
        if (glyph == '\n') {
            horizontal = 0;
            vertical += _scale;
            continue;
        }
        if (!_charColumns[glyph].has_value()) {
            glyph = '?';
            if (!_charColumns[glyph].has_value()) {
                std::cerr << "Missing glyph and no '?' available so skipping." << std::endl;
                continue;
            }
        }
        float wordHorizontal = horizontal;
        if (glyph == ' ') {
            for (std::string::size_type word_idx = char_idx + 1; word_idx < _text.length();
                 ++word_idx) {
                char wordGlyph = _text[word_idx];
                if (wordGlyph == ' ' || wordGlyph == '\n') {
                    break;
                }
                if (!_charColumns[wordGlyph].has_value()) {
                    wordGlyph = '?';
                    if (!_charColumns[wordGlyph].has_value()) {
                        std::cerr << "Missing glyph and no '?' available so skipping." << std::endl;
                        continue;
                    }
                }
                unsigned int firstColumn;
                unsigned int lastColumn;
                std::tie(firstColumn, lastColumn) = _charColumns[wordGlyph].value();
                unsigned int charWidth = (lastColumn - firstColumn) + 1;
                wordHorizontal += ((float)charWidth / (float)_texture.height) * _scale;
            }
        }
        if (wordHorizontal > _width) {
            horizontal = 0;
            vertical += _scale;
        }

        unsigned int firstColumn;
        unsigned int lastColumn;
        std::tie(firstColumn, lastColumn) = _charColumns[glyph].value();
        unsigned int charWidth = (lastColumn - firstColumn) + 1;

        Eend::Panel* panelRef = Eend::Entities::getPanels().getRef(_panelIds[char_idx]);

        if (horizontal + ((float)charWidth / (float)_texture.height) * _scale > _width) {
            horizontal = 0;
            vertical += _scale;
        }
        panelRef->setPosition(
            Eend::Point(_position.x + horizontal, _position.y + vertical, _position.z));
        horizontal += ((float)charWidth / (float)_texture.height) * _scale;

        panelRef->setScale(
            Eend::Scale2D(_scale * ((float)charWidth / (float)_texture.height), _scale));

        panelRef->cropTexture(
            Eend::Point2D(firstColumn, 0.0f), Eend::Point2D(lastColumn + 1.0f, _texture.height));
    }
}
