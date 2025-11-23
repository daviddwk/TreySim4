#include <Eendgine/fatalError.hpp>

#include <filesystem>
#include <fstream>
#include <json/json.h>
#include <optional>
#include <stb/stb_image.h>

#include "Eendgine/types.hpp"
#include "text.hpp"

Text::Text(Font font, std::string text, Eend::Point position, float scale, float width)
    : m_text(text), m_width(width), m_position(position), m_scale(scale), m_fontPath(""),
      m_charColumns({std::nullopt}) {

    // auto rootPath = std::filesystem::path("resources") / std::filesystem::path("fonts");
    m_fontPath = std::filesystem::path("fonts");
    switch (font) {
    case Font::daniel:
        m_fontPath = m_fontPath / std::filesystem::path("daniel");
        break;
    }
    m_texture = Eend::loadTexture("resources" / m_fontPath / "font.png");

    // open json
    Json::Value root;
    std::filesystem::path metadataPath = "resources" / m_fontPath / "metadata.json";
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
        if (!jsonChar.isArray()) continue;
        const unsigned int firstColumn = jsonChar[0].asUInt();
        const unsigned int lastColumn = jsonChar[1].asUInt();

        bool first_after_last = firstColumn > lastColumn;
        bool exceeded_max = (firstColumn >= static_cast<unsigned int>(m_texture.width)) ||
                            (lastColumn >= static_cast<unsigned int>(m_texture.width));

        if (!first_after_last && !exceeded_max) {
            m_charColumns[ch] = std::tie(firstColumn, lastColumn);
        } else {
            Eend::fatalError("malformatted font metadat\n");
            // print error and move on
        }
    }

    // this should point to a folder with just font pngs?
    // need to unwrap optional here
    for (size_t char_idx = 0; char_idx < m_text.length(); ++char_idx) {
        m_panelIds.push_back(Eend::Entities::panels().insert(m_fontPath));
    }
    updateText();
}

Text::~Text() {
    for (const Eend::PanelId& id : m_panelIds) {
        Eend::Entities::panels().erase(id);
    }
}

void Text::setText(const std::string& text) {
    if (m_text == text) return;

    m_text = text;

    for (const Eend::PanelId& id : m_panelIds) {
        Eend::Entities::panels().erase(id);
    }
    m_panelIds.clear();
    for (size_t char_idx = 0; char_idx < m_text.length(); ++char_idx) {
        m_panelIds.push_back(Eend::Entities::panels().insert(m_fontPath));
    }
    updateText();
}

void Text::setPosition(Eend::Point position) {

    m_position = position;
    updateText();
}

void Text::setScale(float scale) {

    m_scale = scale;
    updateText();
}

void Text::updateText() {
    float horizontal = 0.0f;
    float vertical = 0.0f;
    for (std::string::size_type char_idx = 0; char_idx < m_text.length(); ++char_idx) {
        char glyph = m_text[char_idx];
        if (glyph == '\n') {
            horizontal = 0;
            vertical += m_scale;
            continue;
        }
        if (!m_charColumns[glyph].has_value()) {
            glyph = '?';
            if (!m_charColumns[glyph].has_value()) {
                std::cerr << "Missing glyph and no '?' available so skipping." << std::endl;
                continue;
            }
        }
        float wordHorizontal = horizontal;
        if (glyph == ' ') {
            for (std::string::size_type word_idx = char_idx + 1; word_idx < m_text.length();
                 ++word_idx) {
                char wordGlyph = m_text[word_idx];
                if (wordGlyph == ' ' || wordGlyph == '\n') {
                    break;
                }
                if (!m_charColumns[wordGlyph].has_value()) {
                    wordGlyph = '?';
                    if (!m_charColumns[wordGlyph].has_value()) {
                        std::cerr << "Missing glyph and no '?' available so skipping." << std::endl;
                        continue;
                    }
                }
                unsigned int firstColumn;
                unsigned int lastColumn;
                std::tie(firstColumn, lastColumn) = m_charColumns[wordGlyph].value();
                unsigned int charWidth = (lastColumn - firstColumn) + 1;
                wordHorizontal += ((float)charWidth / (float)m_texture.height) * m_scale;
            }
        }
        if (wordHorizontal > m_width) {
            horizontal = 0;
            vertical += m_scale;
        }

        unsigned int firstColumn;
        unsigned int lastColumn;
        std::tie(firstColumn, lastColumn) = m_charColumns[glyph].value();
        unsigned int charWidth = (lastColumn - firstColumn) + 1;

        Eend::Panel* panelRef = Eend::Entities::panels().getRef(m_panelIds[char_idx]);

        if (horizontal + ((float)charWidth / (float)m_texture.height) * m_scale > m_width) {
            horizontal = 0;
            vertical += m_scale;
        }
        panelRef->setPosition(
            Eend::Point(m_position.x + horizontal, m_position.y + vertical, m_position.z));
        horizontal += ((float)charWidth / (float)m_texture.height) * m_scale;

        panelRef->setScale(
            Eend::Scale2D(m_scale * ((float)charWidth / (float)m_texture.height), m_scale));

        panelRef->cropTexture(
            Eend::Point2D(firstColumn, 0.0f),
            Eend::Point2D(lastColumn + 1.0f, m_texture.height));
    }
}
