#pragma once
#include <optional>

#include <Eendgine/types.hpp>

namespace Eend = Eendgine;

bool pointOnRectangle(Eend::Point2D point, Eend::Rectangle rectangle);
float pointHeightOnTri(const Eend::Triangle& tri, const Eend::Point2D& point);
std::optional<Eend::Point> pointToSphereEdgeRelative(Eend::Point point, Eend::Sphere sphere);
