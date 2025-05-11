#include "../geom.h"
#include "../../math_defs.h"

capsule_t::capsule_t()
    : a(vector3::zero()), b(vector3::unit_y()), radius(1.0) {
}

capsule_t::capsule_t(const vector3& a_, const vector3& b_, double r)
    : a(a_), b(b_), radius(r) {
}

double capsule_t::length() const {
    return (b - a).length();
}

double capsule_t::length_squared() const {
    return (b - a).length_squared();
}

vector3 capsule_t::center() const {
    return (a + b) * 0.5;
}

vector3 capsule_t::direction() const {
    return (b - a).safe_normalized();
}

aabb_t capsule_t::to_aabb() const {
    vector3 rvec(radius);
    aabb_t box;
    box.min = vector3::min(a, b) - rvec;
    box.max = vector3::max(a, b) + rvec;
    return box;
}

vector3 capsule_t::closest_point(const vector3& p) const {
    vector3 ab = b - a;
    double t = (p - a).dot(ab) / ab.length_squared();
    t = clamp(t, 0.0, 1.0);
    return a + ab * t;
}

double capsule_t::distance_to_point(const vector3& p) const {
    return (p - closest_point(p)).length() - radius;
}

bool capsule_t::contains_point(const vector3& p, double epsilon) const {
    return distance_to_point(p) <= epsilon;
}

bool capsule_t::intersects_sphere(const vector3& center, double r) const {
    double dist = (center - closest_point(center)).length();
    return dist <= (radius + r);
}

bool capsule_t::intersects_capsule(const capsule_t& other) const {
    const vector3& p1 = this->a;
    const vector3& q1 = this->b;
    const vector3& p2 = other.a;
    const vector3& q2 = other.b;

    vector3 d1 = q1 - p1;
    vector3 d2 = q2 - p2;
    vector3 r = p1 - p2;
    double a = d1.dot(d1);
    double e = d2.dot(d2);
    double f = d2.dot(r);

    double s, t;

    if (a <= 1e-8 && e <= 1e-8) {
        s = t = 0.0;
    }
    else if (a <= 1e-8) {
        s = 0.0;
        t = clamp(f / e, 0.0, 1.0);
    }
    else {
        double c = d1.dot(r);
        if (e <= 1e-8) {
            t = 0.0;
            s = clamp(-c / a, 0.0, 1.0);
        }
        else {
            double b = d1.dot(d2);
            double denom = a * e - b * b;

            if (denom != 0.0) {
                s = clamp((b * f - c * e) / denom, 0.0, 1.0);
            }
            else {
                s = 0.0;
            }

            t = (b * s + f) / e;

            if (t < 0.0) {
                t = 0.0;
                s = clamp(-c / a, 0.0, 1.0);
            }
            else if (t > 1.0) {
                t = 1.0;
                s = clamp((b - c) / a, 0.0, 1.0);
            }
        }
    }

    vector3 c1 = p1 + d1 * s;
    vector3 c2 = p2 + d2 * t;
    double dist = (c1 - c2).length();

    return dist <= (radius + other.radius);
}

bool capsule_t::intersects_ray(const ray_t& ray, double* out_t) const {

    vector3 ba = b - a;
    vector3 oa = ray.origin - a;
    vector3 rd = ray.direction;

    double baba = ba.dot(ba);
    double bard = ba.dot(rd);
    double baoa = ba.dot(oa);
    double rdoa = rd.dot(oa);
    double oaoa = oa.dot(oa);

    double a_val = baba - bard * bard;
    double b_val = baba * rdoa - baoa * bard;
    double c_val = baba * oaoa - baoa * baoa - baba * radius * radius;

    double h = b_val * b_val - a_val * c_val;
    if (h < 0.0)
        return false;

    h = std::sqrt(h);
    double t = (-b_val - h) / a_val;

    double y = baoa + t * bard;
    if (y > 0.0 && y < baba) {
        if (out_t) *out_t = t;
        return true;
    }

    vector3 end1 = a;
    vector3 end2 = b;
    bool hit_caps = false;

    vector3 ao = ray.origin - end1;
    double b2 = ao.dot(rd);
    double c2 = ao.dot(ao) - radius * radius;
    double h2 = b2 * b2 - c2;
    if (h2 > 0.0) {
        double t2 = -b2 - std::sqrt(h2);
        vector3 p = ray.origin + rd * t2;
        if ((p - end1).dot(ba) < 0.0) {
            if (out_t) *out_t = t2;
            return true;
        }
    }

    ao = ray.origin - end2;
    b2 = ao.dot(rd);
    c2 = ao.dot(ao) - radius * radius;
    h2 = b2 * b2 - c2;
    if (h2 > 0.0) {
        double t2 = -b2 - std::sqrt(h2);
        vector3 p = ray.origin + rd * t2;
        if ((p - end2).dot(ba) > 0.0) {
            if (out_t) *out_t = t2;
            return true;
        }
    }

    return false;
}

bool capsule_t::equals(const capsule_t& other, double epsilon) const {
    return a.equals(other.a, epsilon) &&
        b.equals(other.b, epsilon) &&
        std::abs(radius - other.radius) <= epsilon;
}

bool capsule_t::is_valid() const {
    return a.is_finite() &&
        b.is_finite() &&
        std::isfinite(radius) &&
        radius >= 0.0;
}


double capsule_t::volume() const {
    double h = length();
    double r = radius;
    double cyl_volume = M_PI * r * r * h;
    double sphere_volume = (4.0 / 3.0) * M_PI * r * r * r;
    return cyl_volume + sphere_volume;
}

double capsule_t::surface_area() const {
    double h = length();
    double r = radius;
    double cyl_area = 2.0 * M_PI * r * h;
    double sphere_area = 4.0 * M_PI * r * r;
    return cyl_area + sphere_area;
}

void capsule_t::transform(const matrix4x4& m) {
    a = m.transform_point(a);
    b = m.transform_point(b);

    double sx = m.transform_direction(vector3::unit_x()).length();
    double sy = m.transform_direction(vector3::unit_y()).length();
    double sz = m.transform_direction(vector3::unit_z()).length();
    double scale = (sx + sy + sz) / 3.0;

    radius *= scale;
}

vector3 capsule_t::normal_at(const vector3& p) const {
    vector3 q = closest_point(p);
    vector3 n = (p - q).safe_normalized();
    return n;
}

std::pair<vector3, vector3> capsule_t::get_segment() const {
    return { a, b };
}

capsule_t capsule_t::lerp(const capsule_t& c1, const capsule_t& c2, double t) {
    return capsule_t(
        ::lerp(c1.a, c2.a, t),
        ::lerp(c1.b, c2.b, t),
        ::lerp(c1.radius, c2.radius, t)
    );
}


