// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/services/EditorLog.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace buffalo::editor
{
void EditorLog::Trace(const std::string& message)
{
    Write(LogLevel::Trace, message);
}

void EditorLog::Debug(const std::string& message)
{
    Write(LogLevel::Debug, message);
}

void EditorLog::Info(const std::string& message)
{
    Write(LogLevel::Info, message);
}

void EditorLog::Warning(const std::string& message)
{
    Write(LogLevel::Warning, message);
}

void EditorLog::Error(const std::string& message)
{
    Write(LogLevel::Error, message);
}

void EditorLog::Write(const std::string& message)
{
    Info(message);
}

void EditorLog::SetLevel(LogLevel level)
{
    level_ = level;
}

LogLevel EditorLog::GetLevel() const
{
    return level_;
}

bool EditorLog::SaveLog(const std::string& path) const
{
    std::ofstream file(path);

    if (!file.is_open())
    {
        return false;
    }

    for (const std::string& line : lines_)
    {
        file << line << '\n';
    }

    return file.good();
}

void EditorLog::Clear()
{
    lines_.clear();
    ++revision_;
}

const std::deque<std::string>& EditorLog::Lines() const
{
    return lines_;
}

std::uint64_t EditorLog::Revision() const
{
    return revision_;
}

void EditorLog::Write(LogLevel level, const std::string& message)
{
    if (level < level_)
    {
        return;
    }

    std::ostringstream stream;
    stream << "[" << CurrentTime() << "] "
           << "[" << LevelName(level) << "] "
           << message;

    lines_.push_back(stream.str());

    if (lines_.size() > MaxLines)
    {
        lines_.pop_front();
    }

    ++revision_;
}

const char* EditorLog::LevelName(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Trace:
        return "TRACE";

    case LogLevel::Debug:
        return "DEBUG";

    case LogLevel::Info:
        return "INFO ";

    case LogLevel::Warning:
        return "WARN ";

    case LogLevel::Error:
        return "ERROR";
    }

    return "UNKNOWN";
}

std::string EditorLog::CurrentTime()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};

#ifdef _WIN32
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif

    std::ostringstream stream;
    stream << std::put_time(&localTime, "%H:%M:%S");
    return stream.str();
}
} // namespace buffalo::editor