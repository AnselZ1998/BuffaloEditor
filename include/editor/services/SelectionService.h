// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once
#include "scene/ObjectId.h"
#include <QObject>
namespace buffalo::editor
{
enum class SelectionKind
{
    None,
    Scene,
    Model,
    Camera,
    Light
};
struct SelectionTarget
{
    SelectionKind kind = SelectionKind::None;
    SceneId sceneId = 0;
    ObjectId objectId = 0;
    bool operator==(const SelectionTarget& other) const
    {
        return kind == other.kind && sceneId == other.sceneId && objectId == other.objectId;
    }
    bool operator!=(const SelectionTarget& other) const
    {
        return !(*this == other);
    }
};
class SelectionService final : public QObject
{
    Q_OBJECT
public:
    explicit SelectionService(QObject* parent = nullptr);
    const SelectionTarget& Selected() const;
    void Select(const SelectionTarget& target);
    void Clear();
signals:
    void SelectionChanged();

private:
    SelectionTarget selected_;
};
} // namespace buffalo::editor
Q_DECLARE_METATYPE(buffalo::editor::SelectionTarget)
