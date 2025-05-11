#include "../geom.h"
#include "../../math_defs.h"
#include "../../plane.h"

line_t::line_t(const vector3& pt, const vector3& dir)
    : point(pt), direction(dir) {
}

line_t line_t::from_points(const vector3& a, const vector3& b) {
    vector3 dir = b - a;
    return line_t(a, dir.normalized());
}

vector3 line_t::point_at(double t) const {
    return point + direction * t;
}

bool line_t::is_valid() const {
    return direction.is_finite() && point.is_finite() && std::abs(direction.length_squared() - 1.0) < 1e-6;
}

line_t line_t::reversed() const {
    return line_t(point, -direction);
}

double line_t::project_length(const vector3& pt) const {
    return (pt - point).dot(direction);
}

vector3 line_t::closest_point(const vector3& pt) const {
    return point + direction * project_length(pt);
}

double line_t::distance_to_point(const vector3& pt) const {
    return (closest_point(pt) - pt).length();
}

double line_t::distance_to_line(const line_t& other) const {
    const vector3 w0 = point - other.point;
    const double a = direction.dot(direction);     // == 1
    const double b = direction.dot(other.direction);
    const double c = other.direction.dot(other.direction); // == 1
    const double d = direction.dot(w0);
    const double e = other.direction.dot(w0);

    const double denom = a * c - b * b;
    if (std::abs(denom) < 1e-8) {
        // Lines are nearly parallel
        return ((w0 - direction * d)).length();
    }

    const double s = (b * e - c * d) / denom;
    const double t = (a * e - b * d) / denom;

    const vector3 p1 = point + direction * s;
    const vector3 p2 = other.point + other.direction * t;
    return (p1 - p2).length();
}

std::pair<vector3, vector3> line_t::closest_points_between(const line_t& other) const {
    const vector3 w0 = point - other.point;
    const double a = direction.dot(direction);
    const double b = direction.dot(other.direction);
    const double c = other.direction.dot(other.direction);
    const double d = direction.dot(w0);
    const double e = other.direction.dot(w0);

    const double denom = a * c - b * b;
    double s = 0.0, t = 0.0;

    if (std::abs(denom) > 1e-8) {
        s = (b * e - c * d) / denom;
        t = (a * e - b * d) / denom;
    }
    else {
        // Parallel lines
        s = 0.0;
        t = e / c;
    }

    return { point + direction * s, other.point + other.direction * t };
}

bool line_t::intersects_plane(const plane_t& plane, vector3& out_point) const {
    const double denom = direction.dot(plane.normal);
    if (std::abs(denom) < 1e-8)
        return false; // Parallel

    const double t = (plane.d - plane.normal.dot(point)) / denom;
    out_point = point + direction * t;
    return true;
}

bool line_t::operator==(const line_t& other) const {
    return point == other.point && direction == other.direction;
}

bool line_t::operator!=(const line_t& other) const {
    return !(*this == other);
}