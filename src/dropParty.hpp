#pragma once

#include "dropParty.hpp"

#include <filesystem>

#include "items.hpp"

class DropParty {
    public:
        DropParty(std::filesystem::path parkPath);
        void update(float dt);
};
