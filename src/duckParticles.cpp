#include "Eendgine/types.hpp"
#include "duckParticles.hpp"

Eend::Particles::Properties getJumpParticleProperties() {
    Eend::Particles::Behavior behavior =
        [](int seed, std::chrono::milliseconds time) -> std::optional<Eend::Particles::State> {
        constexpr int LIFETIME_BASE = 1000;
        constexpr int LIFETIME_SPREAD = 500;

        constexpr float SCALE_BASE = 3.0f;
        constexpr float SCALE_SPREAD = 2.0f;

        constexpr float SPEED_FACTOR = 0.3f;

        const int lifetimeMs = LIFETIME_BASE + (seed % LIFETIME_SPREAD);

        if (time.count() < lifetimeMs) {
            const unsigned int bits = 8;
            const unsigned int max = std::pow(2, bits);

            const float elapsedMs = static_cast<float>(time.count());

            auto extractU8 = [](int seed, unsigned int idx) { return (seed >> (8 * idx)); };
            auto posOrNeg = [](int seed) { return (seed % 2) ? 1.0f : -1.0f; };

            const float randX = static_cast<float>(extractU8(seed, 0));
            const float flipX = posOrNeg(extractU8(seed, 0));
            const float x = (elapsedMs / fmod(randX, max)) * flipX;

            const float randY = static_cast<float>(extractU8(seed, 1));
            const float flipY = posOrNeg(extractU8(seed, 1));
            const float y = (elapsedMs / fmod(randY, max)) * flipY;

            const float randZ = static_cast<float>(extractU8(seed, 2));
            const float flipZ = posOrNeg(extractU8(seed, 2));
            const float z = (elapsedMs / fmod(randZ, max)) * flipZ;

            const Eend::Point position = Eend::Point(x, y, z) * SPEED_FACTOR;

            const float scaleSpread = std::fmod(static_cast<float>(seed), SCALE_SPREAD);
            const float scaleFade = (lifetimeMs - static_cast<float>(time.count())) / lifetimeMs;

            const Eend::Scale2D scale = Eend::Scale2D((SCALE_BASE + scaleSpread) * scaleFade);

            return std::make_optional(Eend::Particles::State(position, scale, 0.0f));
        }
        return std::nullopt;
    };
    return Eend::Particles::Properties("duck/boards/poo", behavior);
}

Eend::Particles::Properties getKickParticleProperties(Duck::Direction direction) {

    const std::filesystem::path boardPath("duck/boards/kick");

    // clang-format off
    Eend::Vector2D directionVector = Eend::Vector2D(0.0f);
    switch (direction) {
    case Duck::Direction::up:
        directionVector = Eend::Vector2D(                0.0f,                1.0f);
        break;
    case Duck::Direction::upRight:
        directionVector = Eend::Vector2D(  Eend::INV_SQRT_TWO,  Eend::INV_SQRT_TWO);
        break;
    case Duck::Direction::right:
        directionVector = Eend::Vector2D(                1.0f,                0.0f);
        break;
    case Duck::Direction::downRight:
        directionVector = Eend::Vector2D(  Eend::INV_SQRT_TWO, -Eend::INV_SQRT_TWO);
        break;
    case Duck::Direction::down:
        directionVector = Eend::Vector2D(                0.0f,               -1.0f);
        break;
    case Duck::Direction::downLeft:
        directionVector = Eend::Vector2D( -Eend::INV_SQRT_TWO, -Eend::INV_SQRT_TWO);
        break;
    case Duck::Direction::left:
        directionVector = Eend::Vector2D(               -1.0f,                0.0f);
        break;
    case Duck::Direction::upLeft:
        directionVector = Eend::Vector2D( -Eend::INV_SQRT_TWO,  Eend::INV_SQRT_TWO);
        break;
    }
    // clang-format on

    const Eend::Particles::Behavior behavior =
        [directionVector](
            int seed,
            std::chrono::milliseconds time) -> std::optional<Eend::Particles::State> {
        constexpr int LIFETIME_BASE = 250;
        constexpr int LIFETIME_SPREAD = 100;

        constexpr float SCALE_BASE = 3.0f;
        constexpr float SCALE_SPREAD = 2.0f;

        constexpr float DISTANCE_BASE = 10.0f;

        constexpr float HEIGHT_BASE = 5.0f;

        constexpr float POSITION_SPREAD = 0.3f;

        const int lifetimeMs = LIFETIME_BASE + (seed % LIFETIME_SPREAD);

        if (time.count() < lifetimeMs) {

            const float elapsedMs = static_cast<float>(time.count());
            const float lifetimeRatio = elapsedMs / lifetimeMs;

            const float maxU8 = static_cast<float>(std::pow(2, 8));
            auto extractU8 = [](int seed, unsigned int idx) { return (seed >> (8 * idx)) & 0xFF; };
            auto posOrNeg = [](int seed) { return (seed % 2) ? 1.0f : -1.0f; };

            const float spreadRatioX = static_cast<float>(extractU8(seed, 0)) / maxU8;
            const float spreadX = POSITION_SPREAD * spreadRatioX * posOrNeg(extractU8(seed, 0));
            const float x = (directionVector.x + spreadX) * DISTANCE_BASE * lifetimeRatio;

            const float spreadRatioY = static_cast<float>(extractU8(seed, 1)) / maxU8;
            const float spreadY = POSITION_SPREAD * spreadRatioY * posOrNeg(extractU8(seed, 1));
            const float y = (directionVector.y + spreadY) * DISTANCE_BASE * lifetimeRatio;

            const float z = (HEIGHT_BASE + (HEIGHT_BASE * POSITION_SPREAD)) * lifetimeRatio;

            const Eend::Point position = Eend::Point(x, y, z);

            const float spreadRatioScale = static_cast<float>(extractU8(seed, 2)) / maxU8;

            const Eend::Scale2D scale =
                Eend::Scale2D(SCALE_BASE) + (SCALE_SPREAD * spreadRatioScale);

            const unsigned int frame =
                lifetimeRatio < 0.333f ? 0 : (lifetimeRatio < 0.666f ? 1 : 2);

            return std::make_optional(Eend::Particles::State(position, scale, frame));
        }
        return std::nullopt;
    };

    return Eend::Particles::Properties(boardPath, behavior);
}
