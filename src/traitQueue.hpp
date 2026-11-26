#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/frameLimiter.hpp>

#include <filesystem>

namespace Eend = Eendgine;

template <typename T, float F> class TraitQueue {
    public:
        TraitQueue(T default_traits) : traits_total(default_traits) {};
        ~TraitQueue() {
            for (auto& [traits, panel_id] : items) {
                Eend::Entities::panels().erase(panel_id);
            }
        };

        TraitQueue(const TraitQueue&) = delete;
        TraitQueue(TraitQueue&&) = delete;
        TraitQueue& operator=(const TraitQueue&) = delete;
        TraitQueue& operator=(TraitQueue&&) = delete;

        T getTraitsTotal() const { return traits_total; }

        void update() {
            last_remove += Eend::FrameLimiter::get().deltaTime; // TODO didn't I make timers

            if (!isEmpty() && (last_remove > 5.0f)) {
                last_remove = 0.0f;
                popTrait();
            }
        }

        bool isEmpty() const { return items.empty(); }

        void addTrait(T newTraits, std::filesystem::path texture) {
            if (isEmpty()) {
                last_remove = 0.0f;
            }
            Eend::PanelId newPanelId = Eend::Entities::panels().insert(texture);
            traits_total += newTraits;
            items.emplace_back(newTraits, newPanelId);
            positionIcons();
        }

        void popTrait() { // TODO should total traits be passed or internal?
            if (!isEmpty()) {
                auto& item = items.front();
                T oldTraits = std::get<T>(item);
                traits_total -= oldTraits;
                Eend::Entities::panels().erase(std::get<Eend::PanelId>(item));
                items.erase(items.begin());
                positionIcons();
            }
        }

        void emptyTraits() {
            for (auto& [traits, panel_id] : items) {
                Eend::Entities::panels().erase(panel_id);
            }
            items = std::vector<std::tuple<T, Eend::PanelId>>();
        }

        void positionIcons() {
            int idx = 0;
            for (auto& [traits, panel_id] : items) {
                Eend::Panel* icon = Eend::Entities::panels().getRef(panel_id);
                icon->setScale(Eend::Scale2D(100.0f));
                icon->setPosition(Eend::Point(F, static_cast<float>(idx) * 55.0f, 0.0f));
                idx++;
            }
        }

    private:
        float last_remove = 0;
        T traits_total;
        std::vector<std::tuple<T, Eend::PanelId>> items;
};
