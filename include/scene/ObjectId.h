// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once
#include <atomic>
#include <cstdint>
using ObjectId = std::uint64_t;
using SceneId = ObjectId;
inline ObjectId NewObjectId()
{
    static std::atomic<ObjectId> next{1};
    return next.fetch_add(1, std::memory_order_relaxed);
}
