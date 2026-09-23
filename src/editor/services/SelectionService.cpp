// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/services/SelectionService.h"

namespace buffalo::editor
{
SelectionService::SelectionService(QObject* parent)
    : QObject(parent)
{
}

const QString& SelectionService::SelectedId() const
{
    return selectedId_;
}

void SelectionService::Select(const QString& id)
{
    if (selectedId_ != id)
    {
        selectedId_ = id;
        emit SelectionChanged();
    }
}

void SelectionService::Clear()
{
    Select({});
}
} // namespace buffalo::editor
