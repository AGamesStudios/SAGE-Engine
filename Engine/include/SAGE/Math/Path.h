#pragma once

#include "SAGE/Math/Vector2.h"
#include <vector>
#include <cmath>

namespace SAGE {

enum class PathType {
    Linear,     // Straight lines between points
    CatmullRom, // Smooth curve passing through points
    Circle      // Parametric circle/ellipse
};

class Path {
public:
    PathType type = PathType::Linear;
    std::vector<Vector2> points;
    bool closed = false;

    // For Circle type
    Vector2 center;
    float radiusX = 0.0f;
    float radiusY = 0.0f;

    Path() = default;

    // Factory methods
    static Path CreateLinear(const std::vector<Vector2>& points, bool closed = false) {
        Path p;
        p.type = PathType::Linear;
        p.points = points;
        p.closed = closed;
        return p;
    }

    static Path CreateCircle(const Vector2& center, float radius) {
        Path p;
        p.type = PathType::Circle;
        p.center = center;
        p.radiusX = radius;
        p.radiusY = radius;
        p.closed = true;
        return p;
    }

    static Path CreateEllipse(const Vector2& center, float radiusX, float radiusY) {
        Path p;
        p.type = PathType::Circle;
        p.center = center;
        p.radiusX = radiusX;
        p.radiusY = radiusY;
        p.closed = true;
        return p;
    }

    // Get point at normalized parameter t [0, 1]
    Vector2 GetPoint(float t) const {
        if (type == PathType::Circle) {
            float angle = t * 6.28318530718f; // 2*PI
            return {
                center.x + std::cos(angle) * radiusX,
                center.y + std::sin(angle) * radiusY
            };
        }
        
        if (points.empty()) return Vector2::Zero();
        if (points.size() == 1) return points[0];

        if (type == PathType::Linear) {
            // Handle closed loop
            int numSegments = closed ? (int)points.size() : (int)points.size() - 1;
            if (numSegments <= 0) return points[0];

            float totalT = t * numSegments;
            int currentPoint = (int)totalT;
            float segmentT = totalT - currentPoint;

            int nextPoint = currentPoint + 1;
            if (closed) {
                currentPoint %= points.size();
                nextPoint %= points.size();
            } else {
                if (currentPoint >= numSegments) {
                    currentPoint = numSegments - 1;
                    nextPoint = numSegments;
                    segmentT = 1.0f;
                }
            }

            Vector2 p0 = points[currentPoint];
            Vector2 p1 = points[nextPoint];
            
            // Lerp
            return {
                p0.x + (p1.x - p0.x) * segmentT,
                p0.y + (p1.y - p0.y) * segmentT
            };
        }

        // TODO: CatmullRom
        return points[0];
    }
};

} // namespace SAGE
