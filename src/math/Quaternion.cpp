#include "math/Quaternion.h"
#include <cmath>

Quaternion& Quaternion::SetFromEuler(const Vector3& inEuler)
{
    float hx = inEuler.x * 0.5f;
    float hy = inEuler.y * 0.5f;
    float hz = inEuler.z * 0.5f;

    float cx = std::cos(hx);
    float sx = std::sin(hx);
    float cy = std::cos(hy);
    float sy = std::sin(hy);
    float cz = std::cos(hz);
    float sz = std::sin(hz);

    w = cz * cy * cx + sz * sy * sx;
    x = cz * cy * sx - sz * sy * cx;
    y = cz * sy * cx + sz * cy * sx;
    z = sz * cy * cx - cz * sy * sx;

    return *this;
}
