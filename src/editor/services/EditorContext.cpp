// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/services/EditorContext.h"

namespace buffalo::editor
{
EditorContext::EditorContext(EngineRuntime& runtime, SelectionService& selection, EditorLog& log)
    : runtime(runtime), selection(selection), log(log)
{
}
} // namespace buffalo::editor
