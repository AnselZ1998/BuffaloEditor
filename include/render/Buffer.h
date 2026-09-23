#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <vector>

template<typename T>
class Buffer
{
public:
    Buffer() = default;

    Buffer(int width, int height)
    {
        Resize(width, height);
    }

    void Resize(int width, int height)
    {
        assert(width > 0);
        assert(height > 0);

        mWidth = width;
        mHeight = height;
        mPitchBytes = width * static_cast<int>(sizeof(T));

        mData.resize(
            static_cast<std::size_t>(width) *
            static_cast<std::size_t>(height)
        );
    }

    void Clear(const T& value)
    {
        std::fill(
            mData.begin(),
            mData.end(),
            value
        );
    }

    T& operator()(int x, int y)
    {
        assert(x >= 0 && x < mWidth);
        assert(y >= 0 && y < mHeight);

        return mData[
            static_cast<std::size_t>(y) *
                static_cast<std::size_t>(mWidth) +
                static_cast<std::size_t>(x)
        ];
    }

    const T& operator()(int x, int y) const
    {
        assert(x >= 0 && x < mWidth);
        assert(y >= 0 && y < mHeight);

        return mData[
            static_cast<std::size_t>(y) *
                static_cast<std::size_t>(mWidth) +
                static_cast<std::size_t>(x)
        ];
    }

    T* Data()
    {
        return mData.data();
    }

    const T* Data() const
    {
        return mData.data();
    }

    int GetWidth() const
    {
        return mWidth;
    }

    int GetHeight() const
    {
        return mHeight;
    }

    int GetPixelCount() const
    {
        return mWidth * mHeight;
    }

    int GetPitchBytes() const
    {
        return mPitchBytes;
    }

    bool IsValid() const
    {
        return mWidth > 0 &&
            mHeight > 0 &&
            !mData.empty();
    }

private:
    int mWidth = 0;
    int mHeight = 0;
    int mPitchBytes = 0;

    std::vector<T> mData;
};