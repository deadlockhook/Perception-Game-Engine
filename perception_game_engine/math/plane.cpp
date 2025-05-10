#include "plane.h"
#include "matrix4x4.h"

plane_t plane_t::transform(const matrix4x4& mat) const {
    matrix4x4 inv_transpose = mat.inverse_affine().transposed();
    vector4 p = to_vector4();
    vector4 tp = inv_transpose.transform_vec4(p);
    return plane_t(vector3(tp.x, tp.y, tp.z), tp.w).normalized();
}
