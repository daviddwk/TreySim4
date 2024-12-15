#pragma once

#include <Eendgine/entityBatches.hpp>
#include <string>

namespace Eend = Eendgine;

class Stage {
    public:
        Stage(std::string stagePath);
        ~Stage();

    private:
        std::vector<Eend::BoardId> _boardIds;
        std::vector<Eend::StatueId> _statueIds;
        std::vector<Eend::DollId> _dollIds;
};
