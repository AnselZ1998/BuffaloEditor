// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <deque>
#include <string>

namespace buffalo::editor
{
enum class LogLevel
{
    Trace = 0,
    Debug,
    Info,
    Warning,
    Error
};

// Lightweight editor log.
// Logs stay in memory by default and are only written to disk through SaveLog().
class EditorLog
{
public:
    void Trace(const std::string& message);
    void Debug(const std::string& message);
    void Info(const std::string& message);
    void Warning(const std::string& message);
    void Error(const std::string& message);

    // Compatibility with existing code. Write() behaves as Info().
    void Write(const std::string& message);

    void SetLevel(LogLevel level);
    LogLevel GetLevel() const;

    bool SaveLog(const std::string& path) const;

    void Clear();
    const std::deque<std::string>& Lines() const;
    std::uint64_t Revision() const;

private:
    void Write(LogLevel level, const std::string& message);

    static const char* LevelName(LogLevel level);
    static std::string CurrentTime();

    static constexpr std::size_t MaxLines = 200;

    std::deque<std::string> lines_;
    LogLevel level_ = LogLevel::Info;
    std::uint64_t revision_ = 0;
};
} // namespace buffalo::editor