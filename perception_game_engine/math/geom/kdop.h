#pragma once

#include "../vector3.h"
#include "../aabb.h"
#include <array>
#include <cstddef>
#include <limits>

template <size_t N>
struct kdop_t {
    std::array<double, N> mins;
    std::array<double, N> maxs;

    kdop_t() {
        invalidate();
    }

    static const std::array<vector3, N>& get_axes();

    void invalidate() {
        for (size_t i = 0; i < N; ++i) {
            mins[i] = std::numeric_limits<double>::infinity();
            maxs[i] = -std::numeric_limits<double>::infinity();
        }
    }

    void expand(const vector3& point) {
        const auto& axes = get_axes();
        for (size_t i = 0; i < N; ++i) {
            double d = axes[i].dot(point);
            if (d < mins[i]) mins[i] = d;
            if (d > maxs[i]) maxs[i] = d;
        }
    }

    void expand(const kdop_t<N>& other) {
        for (size_t i = 0; i < N; ++i) {
            if (other.mins[i] < mins[i]) mins[i] = other.mins[i];
            if (other.maxs[i] > maxs[i]) maxs[i] = other.maxs[i];
        }
    }

    bool contains_point(const vector3& point) const {
        const auto& axes = get_axes();
        for (size_t i = 0; i < N; ++i) {
            double d = axes[i].dot(point);
            if (d < mins[i] || d > maxs[i])
                return false;
        }
        return true;
    }

    bool intersects(const kdop_t<N>& other) const {
        for (size_t i = 0; i < N; ++i) {
            if (maxs[i] < other.mins[i] || mins[i] > other.maxs[i])
                return false;
        }
        return true;
    }

    bool is_valid() const {
        for (size_t i = 0; i < N; ++i)
            if (mins[i] > maxs[i]) return false;
        return true;
    }

    void from_points(const vector3* points, size_t count) {
        invalidate();
        for (size_t i = 0; i < count; ++i)
            expand(points[i]);
    }

    aabb_t to_aabb() const {
        const auto& axes = get_axes();

        vector3 min_point(+INFINITY, +INFINITY, +INFINITY);
        vector3 max_point(-INFINITY, -INFINITY, -INFINITY);

        for (size_t i = 0; i < axes.size(); ++i) {
            vector3 axis = axes[i];

            vector3 p_min = axis * mins[i];
            vector3 p_max = axis * maxs[i];

            for (int j = 0; j < 3; ++j) {
                double min_c = std::min(p_min[j], p_max[j]);
                double max_c = std::max(p_min[j], p_max[j]);

                min_point[j] = std::min(min_point[j], min_c);
                max_point[j] = std::max(max_point[j], max_c);
            }
        }

        return aabb_t(min_point, max_point);
    }

    vector3 get_support_point(const vector3& axis) const {

        const auto& axes = get_axes();
        double best_dot = -INFINITY;
        vector3 best_point = {};

        for (size_t i = 0; i < axes.size(); ++i) {
            double dot_val = axis.dot(axes[i]);
            double d = (dot_val >= 0.0) ? maxs[i] : mins[i];
            vector3 p = axes[i] * d;

            double score = axis.dot(p);
            if (score > best_dot) {
                best_dot = score;
                best_point = p;
            }
        }

        return best_point;
    }

}; 

using kdop6_t = kdop_t<3>;
using kdop14_t = kdop_t<7>;
using kdop26_t = kdop_t<13>;
