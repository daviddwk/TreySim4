#include <queue>

constexpr float DEFAULT_BOOT_RADIUS = 1.0f;
constexpr float DEFAULT_CIRCLE_RADIUS = 1.0f;
constexpr float DEFAULT_SPEED = 1.0f;

struct bootCircleTraits {
    public:
        bootCircleTraits(float bootRadius, float circleRadius, float speed)
            : bootRadius(bootRadius), circleRadius(circleRadius), speed(speed) {

              };
        bootCircleTraits& operator+=(const bootCircleTraits& rhs) {
            bootRadius += rhs.bootRadius;
            circleRadius += rhs.circleRadius;
            speed += rhs.speed;
            return *this;
        }
        bootCircleTraits& operator-=(const bootCircleTraits& rhs) {
            bootRadius -= rhs.bootRadius;
            circleRadius -= rhs.circleRadius;
            speed -= rhs.speed;
            return *this;
        }

    private:
        float bootRadius;
        float circleRadius;
        float speed;
};

class BootCircleItem { // TODO also inherit from item?
    public:
        const bootCircleTraits traits;
};

class bootCircle {
    public:
        bootCircle() : traits(DEFAULT_BOOT_RADIUS, DEFAULT_CIRCLE_RADIUS, DEFAULT_SPEED) {};
        bool enabled() { return !items.empty(); }

        void addItem(BootCircleItem item) {
            traits += item.traits;
            items.push(item);
        };
        void removeItem() {
            if (enabled()) {
                BootCircleItem item = items.front();
                traits -= item.traits;
                items.pop();
            }
        };

    private:
        bootCircleTraits traits;
        std::queue<BootCircleItem> items;
};
