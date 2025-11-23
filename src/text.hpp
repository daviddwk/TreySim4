#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/texture.hpp>
#include <Eendgine/types.hpp>
#include <filesystem>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace Eend = Eendgine;

enum class Font { daniel };

class Text {
    public:
        Text(Font font, std::string text, Eend::Point position, float scale, float width);
        ~Text();

        void setText(const std::string& text);
        void setPosition(Eend::Point position);
        void setScale(float scale);

        Eend::Point getPosition() { return m_position; };
        float getScale() { return m_scale; };

    private:
        void updateText();
        std::string m_text;
        float m_width;
        std::vector<Eend::PanelId> m_panelIds;
        Eend::Point m_position;
        float m_scale;
        std::filesystem::path m_fontPath;
        Eend::Texture m_texture;
        std::array<
            std::optional<std::tuple<unsigned int, unsigned int>>,
            std::numeric_limits<char>::max() + 1>
            m_charColumns;
};
