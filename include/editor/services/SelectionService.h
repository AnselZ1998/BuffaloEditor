// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QString>

namespace buffalo::editor
{
// An editor selection token, not ownership of a Scene/Model/Camera.
// v1 supports the scene root; object IDs can be added with real scene editing.
class SelectionService final : public QObject
{
    Q_OBJECT

public:
    explicit SelectionService(QObject* parent = nullptr);
    const QString& SelectedId() const;
    void Select(const QString& id);
    void Clear();

signals:
    void SelectionChanged();

private:
    QString selectedId_;
};
} // namespace buffalo::editor
