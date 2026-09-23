// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

namespace buffalo
{
class EngineRuntime;
}

namespace buffalo::editor
{
class SelectionService;
class EditorLog;

// Long-lived, explicit references. No registration, lookup or owned services.
struct EditorContext
{
    EditorContext(EngineRuntime& runtime, SelectionService& selection, EditorLog& log);
    EngineRuntime& runtime;
    SelectionService& selection;
    EditorLog& log;
};
} // namespace buffalo::editor
