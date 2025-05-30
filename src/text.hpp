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

enum Font { daniel };

class Text {
    public:
        Text(Font font, std::string text, Eend::Point position, float scale, float width);
        ~Text();

        void setText(const std::string& text);
        void setPosition(Eend::Point position);
        void setScale(float scale);

        Eend::Point getPosition() { return _position; };
        float getScale() { return _scale; };

    private:
        void updateText();
        std::string _text;
        float _width;
        std::vector<Eend::PanelId> _panelIds;
        Eend::Point _position;
        float _scale;
        std::filesystem::path _fontPath;
        Eend::Texture _texture;
        std::array<std::optional<std::tuple<unsigned int, unsigned int>>,
            std::numeric_limits<char>::max() + 1>
            _charColumns;
};
