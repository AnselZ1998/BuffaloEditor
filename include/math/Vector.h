#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <ostream>

// =====================================================
// TVector2
// =====================================================

template<typename T>
class TVector2
{
public:
    union
    {
        struct
        {
            T x, y;
        };

        T v[2];
    };

public:
    TVector2()
        : x(T(0)), y(T(0))
    {
    }

    explicit TVector2(T inValue)
        : x(inValue), y(inValue)
    {
    }

    TVector2(T inX, T inY)
        : x(inX), y(inY)
    {
    }

    float Length() const
    {
        return std::sqrt(static_cast<float>(x * x + y * y));
    }

    TVector2 Normalized() const
    {
        float length = Length();

        if (length <= 0.000001f)
        {
            return TVector2(T(0), T(0));
        }

        return TVector2(
            static_cast<T>(x / length),
            static_cast<T>(y / length)
        );
    }

    void NormalizeSelf()
    {
        float length = Length();

        if (length <= 0.000001f)
        {
            return;
        }

        x = static_cast<T>(x / length);
        y = static_cast<T>(y / length);
    }

    static T Dot(const TVector2& a, const TVector2& b)
    {
        return a.x * b.x + a.y * b.y;
    }

    TVector2 operator+(const TVector2& rhs) const
    {
        return TVector2(x + rhs.x, y + rhs.y);
    }

    TVector2 operator-(const TVector2& rhs) const
    {
        return TVector2(x - rhs.x, y - rhs.y);
    }

    TVector2 operator*(T scalar) const
    {
        return TVector2(x * scalar, y * scalar);
    }

    TVector2& operator+=(const TVector2& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    TVector2& operator-=(const TVector2& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    TVector2& operator*=(T scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    T& operator[](int index)
    {
        assert(index >= 0 && index < 2);
        return v[index];
    }

    const T& operator[](int index) const
    {
        assert(index >= 0 && index < 2);
        return v[index];
    }
};

// =====================================================
// TVector3
// =====================================================

template<typename T>
class TVector3
{
public:
    union
    {
        struct
        {
            T x, y, z;
        };

        T v[3];
    };

public:
    TVector3()
        : x(T(0)), y(T(0)), z(T(0))
    {
    }

    explicit TVector3(T inValue)
        : x(inValue), y(inValue), z(inValue)
    {
    }

    TVector3(T inX, T inY, T inZ)
        : x(inX), y(inY), z(inZ)
    {
    }

    float Length() const
    {
        return std::sqrt(static_cast<float>(x * x + y * y + z * z));
    }

    TVector3 Normalized() const
    {
        float length = Length();

        if (length <= 0.000001f)
        {
            return TVector3(T(0), T(0), T(0));
        }

        return TVector3(
            static_cast<T>(x / length),
            static_cast<T>(y / length),
            static_cast<T>(z / length)
        );
    }

    void NormalizeSelf()
    {
        float length = Length();

        if (length <= 0.000001f)
        {
            return;
        }

        x = static_cast<T>(x / length);
        y = static_cast<T>(y / length);
        z = static_cast<T>(z / length);
    }

    static T Dot(const TVector3& a, const TVector3& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static TVector3 Cross(const TVector3& a, const TVector3& b)
    {
        return TVector3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    static TVector3 GetMax(const TVector3& a, const TVector3& b, const TVector3& c)
    {
        return TVector3(
            (std::max)(a.x, (std::max)(b.x, c.x)),
            (std::max)(a.y, (std::max)(b.y, c.y)),
            (std::max)(a.z, (std::max)(b.z, c.z))
        );
    }

    static TVector3 GetMin(const TVector3& a, const TVector3& b, const TVector3& c)
    {
        return TVector3(
            (std::min)(a.x, (std::min)(b.x, c.x)),
            (std::min)(a.y, (std::min)(b.y, c.y)),
            (std::min)(a.z, (std::min)(b.z, c.z))
        );
    }

    TVector3 operator+(const TVector3& rhs) const
    {
        return TVector3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    TVector3 operator-(const TVector3& rhs) const
    {
        return TVector3(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    TVector3 operator*(T scalar) const
    {
        return TVector3(x * scalar, y * scalar, z * scalar);
    }

    TVector3& operator=(const TVector3& rhs)
    {
        x = rhs.x;
        y = rhs.y;
        z = rhs.z;
        return *this;
    }

    TVector3& operator+=(const TVector3& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    TVector3& operator-=(const TVector3& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    TVector3& operator*=(T scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    T& operator[](int index)
    {
        assert(index >= 0 && index < 3);
        return v[index];
    }

    const T& operator[](int index) const
    {
        assert(index >= 0 && index < 3);
        return v[index];
    }
};

// =====================================================
// TVector4
// =====================================================

template<typename T>
class TVector4
{
public:
    union
    {
        struct
        {
            T x, y, z, w;
        };

        T v[4];
    };

public:
    TVector4()
        : x(T(0)), y(T(0)), z(T(0)), w(T(1))
    {
    }

    explicit TVector4(T inValue)
        : x(inValue), y(inValue), z(inValue), w(inValue)
    {
    }

    TVector4(T inX, T inY, T inZ, T inW = T(1))
        : x(inX), y(inY), z(inZ), w(inW)
    {
    }

    explicit TVector4(const T* inData)
        : x(inData[0]), y(inData[1]), z(inData[2]), w(inData[3])
    {
    }

    TVector4(const TVector3<T>& xyz, T inW = T(1))
        : x(xyz.x), y(xyz.y), z(xyz.z), w(inW)
    {
    }

    TVector3<T> XYZ() const
    {
        return TVector3<T>(x, y, z);
    }

    static TVector4 FromPoint(const TVector3<T>& p)
    {
        return TVector4(p.x, p.y, p.z, T(1));
    }

    static TVector4 FromDirection(const TVector3<T>& d)
    {
        return TVector4(d.x, d.y, d.z, T(0));
    }

    TVector3<T> PerspectiveDivide() const
    {
        assert(std::abs(static_cast<float>(w)) > 0.000001f);

        return TVector3<T>(
            static_cast<T>(x / w),
            static_cast<T>(y / w),
            static_cast<T>(z / w)
        );
    }

    static TVector3<T> GetMax(const TVector4& a, const TVector4& b, const TVector4& c)
    {
        return TVector3<T>(
            (std::max)(a.x, (std::max)(b.x, c.x)),
            (std::max)(a.y, (std::max)(b.y, c.y)),
            (std::max)(a.z, (std::max)(b.z, c.z))
        );
    }

    static TVector3<T> GetMin(const TVector4& a, const TVector4& b, const TVector4& c)
    {
        return TVector3<T>(
            (std::min)(a.x, (std::min)(b.x, c.x)),
            (std::min)(a.y, (std::min)(b.y, c.y)),
            (std::min)(a.z, (std::min)(b.z, c.z))
        );
    }

    TVector4 operator+(const TVector4& rhs) const
    {
        return TVector4(
            x + rhs.x,
            y + rhs.y,
            z + rhs.z,
            w + rhs.w
        );
    }

    TVector4 operator-(const TVector4& rhs) const
    {
        return TVector4(
            x - rhs.x,
            y - rhs.y,
            z - rhs.z,
            w - rhs.w
        );
    }

    TVector4 operator*(T scalar) const
    {
        return TVector4(
            x * scalar,
            y * scalar,
            z * scalar,
            w * scalar
        );
    }

    TVector4& operator+=(const TVector4& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }

    TVector4& operator-=(const TVector4& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }

    TVector4& operator*=(T scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    T& operator[](int index)
    {
        assert(index >= 0 && index < 4);
        return v[index];
    }

    const T& operator[](int index) const
    {
        assert(index >= 0 && index < 4);
        return v[index];
    }
};

// =====================================================
// ostream
// =====================================================

template<typename T>
std::ostream& operator<<(std::ostream& stream, const TVector2<T>& value)
{
    stream << value.x << " " << value.y;
    return stream;
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const TVector3<T>& value)
{
    stream << value.x << " " << value.y << " " << value.z;
    return stream;
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const TVector4<T>& value)
{
    stream << value.x << " " << value.y << " " << value.z << " " << value.w;
    return stream;
}

// =====================================================
// Common aliases
// =====================================================

using Vector2 = TVector2<float>;
using Vector3 = TVector3<float>;
using Vector4 = TVector4<float>;

using Vector2f = TVector2<float>;
using Vector3f = TVector3<float>;
using Vector4f = TVector4<float>;

using Vector2i = TVector2<int>;
using Vector3i = TVector3<int>;
using Vector4i = TVector4<int>;