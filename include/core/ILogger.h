// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once
#include <string>
namespace buffalo
{
// Borrowed optional sink; owner outlives Runtime. Called on the editor thread.
class ILogger
{
public:
    virtual ~ILogger() = default;
    virtual void Debug(const std::string& message) = 0;
    virtual void Info(const std::string& message) = 0;
    virtual void Warning(const std::string& message) = 0;
    virtual void Error(const std::string& message) = 0;
};
} // namespace buffalo
