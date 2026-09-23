// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT
#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class QSlider;
class QSpinBox;
namespace buffalo
{
class EngineRuntime;
}

namespace buffalo::editor
{
// Editor playback and bounded reset/replay seeking. No solver or Scene knowledge.
class SimulationPanel final : public QWidget
{
public:
    explicit SimulationPanel(EngineRuntime& runtime, QWidget* parent = nullptr);
    void Refresh();

private:
    void SeekFrame(int frame);
    EngineRuntime& runtime_;
    QPushButton* play_ = nullptr;
    QPushButton* pause_ = nullptr;
    QPushButton* step_ = nullptr;
    QPushButton* reset_ = nullptr;
    QLabel* state_ = nullptr;
    QLabel* time_ = nullptr;
    QSlider* timeline_ = nullptr;
    QSpinBox* frame_ = nullptr;
};
} // namespace buffalo::editor