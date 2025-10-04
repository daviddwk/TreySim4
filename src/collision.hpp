#pragma once
#include <optional>

#include <Eendgine/types.hpp>

namespace Eend = Eendgine;

bool pointOnRectangle(Eend::Point2D point, Eend::Rectangle rectangle);
float pointHeightOnTri(const Eend::Triangle& tri, const Eend::Point2D& point);
std::optional<Eend::Vector> pointToSphereEdgeRelative(Eend::Point point, Eend::Sphere sphere);
std::optional<Eend::Vector>
pointToSphereSliceEdgeRelative(Eend::Point point, Eend::Sphere sphere, float angle, float spread);
