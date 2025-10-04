#include "collision.hpp"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/vector_angle.hpp>

#include <print>

bool pointOnRectangle(Eend::Point2D point, Eend::Rectangle rectangle) {
    float behindLeft = point.x - rectangle.upperLeft.x; // negative means in front
    float behindTop = point.y - rectangle.upperLeft.y;
    float behindRight = rectangle.lowerRight.x - point.x;
    float behindBottom = rectangle.lowerRight.y - point.y;

    if (behindLeft > 0 && behindTop > 0 && behindRight > 0 && behindBottom > 0)
        return true;
    return false;
}

std::optional<Eend::Vector> pointToSphereEdgeRelative(Eend::Point point, Eend::Sphere sphere) {
    Eend::Point difference = point - sphere.position;
    float distance = glm::length(difference);
    if (distance < sphere.radius) {
        float depth = sphere.radius - distance;
        return std::optional<Eend::Vector>(glm::normalize(difference) * (depth / sphere.radius));
    }
    return std::nullopt;
}

std::optional<Eend::Vector>
pointToSphereSliceEdgeRelative(Eend::Point point, Eend::Sphere sphere, float angle, float spread) {

    assert(spread < 360.0f);
    assert(spread >= 0.0f);

    assert(angle < 360);
    assert(angle >= 0);

    const std::optional<Eend::Vector> pointToEdge = pointToSphereEdgeRelative(point, sphere);

    if (pointToEdge) {
        Eend::Vector2D pointToEdge2D = Eend::Vector2D(pointToEdge->x, pointToEdge->y);
        // angle always between 0 and 180
        float toEdgeAngle = glm::degrees(glm::angle(Eend::Vector2D(0.0f, 1.0f), pointToEdge2D));
        std::print("toEdgeAngle {}\n", toEdgeAngle);
        // fixing that so its >= 0 and < 360
        if (pointToEdge2D.x < 0.0f) {
            toEdgeAngle = (180 - toEdgeAngle) + 180;
        }
        std::print("toEdgeAngle360 {}\n", toEdgeAngle);

        float maxAngle = angle + (spread / 2.0);
        float minAngle = angle - (spread / 2.0);
        maxAngle = (maxAngle >= 360.0f) ? (maxAngle - 360.0f) : maxAngle;
        minAngle = (minAngle < 0.0f) ? (minAngle + 360.0f) : minAngle;
        std::print("min max {} {}\n", minAngle, maxAngle);

        // if it doesn't wray around 0
        if (maxAngle > minAngle) {
            std::print("no wrap\n", toEdgeAngle);
            if ((toEdgeAngle < maxAngle) && (toEdgeAngle > minAngle)) {
                return pointToEdge;
            }
        } else {
            // implied >= 0.0 and < 360.0
            if ((toEdgeAngle < maxAngle) || (toEdgeAngle > minAngle)) {
                return pointToEdge;
            }
        }
    }

    return std::nullopt;
}

float pointHeightOnTri(const Eend::Triangle& tri, const Eend::Point2D& point) {
    // could use a tri construct here instead of 3 points

    // calc tri normal
    // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
    const Eend::Point u = tri.p2 - tri.p1;
    const Eend::Point v = tri.p3 - tri.p1;
    const Eend::Point normal = glm::cross(u, v);
    // calc d for point normal plane
    const float d = -((normal.x * tri.p1.x) + (normal.y * tri.p1.y) + (normal.z * tri.p1.z));
    // solve for point.z
    return -((normal.x * point.x) + (normal.y * point.y) + d) / normal.z;
}
