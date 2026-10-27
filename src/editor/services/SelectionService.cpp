// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/services/SelectionService.h"
namespace buffalo::editor
{
SelectionService::SelectionService(QObject* parent)
    : QObject(parent)
{
}
const SelectionTarget& SelectionService::Selected() const
{
    return selected_;
}
void SelectionService::Select(const SelectionTarget& target)
{
    if (selected_ != target)
    {
        selected_ = target;
        emit SelectionChanged();
    }
}
void SelectionService::Clear()
{
    Select({});
}
} // namespace buffalo::editor
