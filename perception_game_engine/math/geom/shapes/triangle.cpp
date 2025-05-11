#include "../geom.h"
#include "../../math_defs.h"

triangle_t::triangle_t()
    : a(vector3::zero()), b(vector3::unit_x()), c(vector3::unit_y()) {
}

triangle_t::triangle_t(const vector3& a_, const vector3& b_, const vector3& c_)
    : a(a_), b(b_), c(c_) {
}

vector3 triangle_t::normal() const {
    return (b - a).cross(c - a).safe_normalized();
}

vector3 triangle_t::centroid() const {
    return (a + b + c) / 3.0;
}

double triangle_t::area() const {
    return 0.5 * (b - a).cross(c - a).length();
}

double triangle_t::perimeter() const {
    return (b - a).length() + (c - b).length() + (a - c).length();
}

bool triangle_t::is_degenerate(double epsilon) const {
    return area() < epsilon;
}

bool triangle_t::contains_point(const vector3& p, double epsilon) const {
    double u, v, w;
    barycentric_coords(p, u, v, w);
    return u >= -epsilon && v >= -epsilon && w >= -epsilon;
}

void triangle_t::barycentric_coords(const vector3& p, double& u, double& v, double& w) const {
    vector3 v0 = b - a;
    vector3 v1 = c - a;
    vector3 v2 = p - a;

    double d00 = v0.dot(v0);
    double d01 = v0.dot(v1);
    double d11 = v1.dot(v1);
    double d20 = v2.dot(v0);
    double d21 = v2.dot(v1);

    double denom = d00 * d11 - d01 * d01;
    if (std::abs(denom) < 1e-12) {
        u = v = w = 1.0 / 3.0;
        return;
    }

    double inv_denom = 1.0 / denom;
    v = (d11 * d20 - d01 * d21) * inv_denom;
    w = (d00 * d21 - d01 * d20) * inv_denom;
    u = 1.0 - v - w;
}

vector3 triangle_t::from_barycentric(double u, double v, double w) const {
    return a * u + b * v + c * w;
}

vector3 triangle_t::closest_point(const vector3& p) const {
    const vector3 ab = b - a;
    const vector3 ac = c - a;
    const vector3 ap = p - a;

    double d1 = ab.dot(ap);
    double d2 = ac.dot(ap);

    if (d1 <= 0.0 && d2 <= 0.0) return a;

    const vector3 bp = p - b;
    double d3 = ab.dot(bp);
    double d4 = ac.dot(bp);

    if (d3 >= 0.0 && d4 <= d3) return b;

    double vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0) {
        double v = d1 / (d1 - d3);
        return a + ab * v;
    }

    const vector3 cp = p - c;
    double d5 = ab.dot(cp);
    double d6 = ac.dot(cp);

    if (d6 >= 0.0 && d5 <= d6) return c;

    double vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0) {
        double w = d2 / (d2 - d6);
        return a + ac * w;
    }

    double va = d3 * d6 - d5 * d4;
    if (va <= 0.0 && (d4 - d3) >= 0.0 && (d5 - d6) >= 0.0) {
        double w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + (c - b) * w;
    }

    double denom = 1.0 / (va + vb + vc);
    double v = vb * denom;
    double w = vc * denom;
    return a + ab * v + ac * w;
}

double triangle_t::distance_to_point(const vector3& p) const {
    return (closest_point(p) - p).length();
}

bool triangle_t::intersects_ray(const ray_t& ray, double* out_t, double epsilon) const {
    const vector3 edge1 = b - a;
    const vector3 edge2 = c - a;
    const vector3 h = ray.direction.cross(edge2);
    double det = edge1.dot(h);

    if (std::abs(det) < epsilon)
        return false;

    double inv_det = 1.0 / det;
    vector3 s = ray.origin - a;
    double u = s.dot(h) * inv_det;
    if (u < -epsilon || u > 1.0 + epsilon)
        return false;

    vector3 q = s.cross(edge1);
    double v = ray.direction.dot(q) * inv_det;
    if (v < -epsilon || u + v > 1.0 + epsilon)
        return false;

    double t = edge2.dot(q) * inv_det;
    if (t < 0.0)
        return false;

    if (out_t) *out_t = t;
    return true;
}

vector3 triangle_t::get_vertex(int i) const {
    switch (i) {
    case 0: return a;
    case 1: return b;
    case 2: return c;
    default: return vector3::zero();
    }
}

vector3 triangle_t::get_edge(int i) const {
    switch (i) {
    case 0: return b - a;
    case 1: return c - b;
    case 2: return a - c;
    default: return vector3::zero();
    }
}

triangle_t triangle_t::flipped() const {
    return triangle_t(a, c, b);
}

bool triangle_t::is_front_facing(const vector3& view_dir) const {
    return normal().dot(view_dir) < 0.0;
}

bool triangle_t::equals(const triangle_t& other, double epsilon) const {
    return a.equals(other.a, epsilon) &&
        b.equals(other.b, epsilon) &&
        c.equals(other.c, epsilon);
}
