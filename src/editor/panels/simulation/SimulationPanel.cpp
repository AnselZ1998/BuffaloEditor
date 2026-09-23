// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT
#include "editor/panels/simulation/SimulationPanel.h"
#include "core/EngineRuntime.h"
#include "simulation/SimulationSystem.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>

namespace buffalo::editor
{
namespace
{
constexpr int FramesPerSecond = 24;
constexpr int LastFrame = 240;

// A frame ruler with direct click/drag seeking; no scene or simulation dependency.
class TimelineSlider final : public QSlider
{
public:
    explicit TimelineSlider(QWidget* parent)
        : QSlider(Qt::Horizontal, parent)
    {
        setRange(0, LastFrame);
        setPageStep(FramesPerSecond);
        setSingleStep(1);
        setFixedHeight(43);
        setFocusPolicy(Qt::StrongFocus);
        setAccessibleName("Simulation timeline");
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), palette().window());
        const int left = 16;
        const int span = std::max(1, width() - 32);
        const int trackY = 32;
        painter.setPen(QColor("#4a5058"));
        painter.drawLine(left, trackY, left + span, trackY);
        for (int frame = 0; frame <= LastFrame; frame += 6)
        {
            const int x = left + qRound(static_cast<double>(frame) / LastFrame * span);
            const bool major = frame % FramesPerSecond == 0;
            painter.setPen(QColor(major ? "#8e97a3" : "#555e68"));
            painter.drawLine(x, trackY - (major ? 9 : 4), x, trackY + 3);
            if (major)
            {
                painter.drawText(QRect(x - 17, 2, 34, 18), Qt::AlignCenter, QString::number(frame));
            }
        }
        const int cursor = left + qRound(static_cast<double>(value()) / LastFrame * span);
        const QColor highlight = isEnabled() ? QColor("#d0a767") : QColor("#747b85");
        painter.setPen(QPen(highlight, 2));
        painter.drawLine(cursor, 18, cursor, 40);
        painter.setBrush(highlight);
        painter.drawPolygon(QPolygon({QPoint(cursor - 5, 18), QPoint(cursor + 5, 18), QPoint(cursor, 23)}));
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() != Qt::LeftButton)
        {
            QSlider::mousePressEvent(event);
            return;
        }
        setFocus(Qt::MouseFocusReason);
        setSliderDown(true);
        SeekAt(event->position().x());
        event->accept();
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        if (isSliderDown())
        {
            SeekAt(event->position().x());
            event->accept();
        }
        else
        {
            QSlider::mouseMoveEvent(event);
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton && isSliderDown())
        {
            SeekAt(event->position().x());
            setSliderDown(false);
            event->accept();
        }
        else
        {
            QSlider::mouseReleaseEvent(event);
        }
    }

private:
    void SeekAt(qreal x)
    {
        const double normalized = (x - 16.0) / std::max(1, width() - 32);
        setValue(std::clamp(qRound(normalized * LastFrame), 0, LastFrame));
    }
};
} // namespace

SimulationPanel::SimulationPanel(EngineRuntime& runtime, QWidget* parent)
    : QWidget(parent), runtime_(runtime)
{
    setObjectName("SimulationPanel");
    setMinimumHeight(102);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 3, 10, 5);
    layout->setSpacing(2);
    timeline_ = new TimelineSlider(this);
    timeline_->setObjectName("SimulationTimeline");
    timeline_->setToolTip("Seek frames 0-240 at 24 fps. Backward seeking resets and replays this CPU demo.");
    layout->addWidget(timeline_);

    auto* controls = new QHBoxLayout;
    controls->setSpacing(5);
    reset_ = new QPushButton("|<  Reset", this);
    play_ = new QPushButton(">  Play", this);
    pause_ = new QPushButton("||  Pause", this);
    step_ = new QPushButton(">|  Step", this);
    play_->setObjectName("SimulationPlay");
    pause_->setObjectName("SimulationPause");
    step_->setObjectName("SimulationStep");
    reset_->setObjectName("SimulationReset");
    step_->setToolTip("Pause and advance to the next display frame (24 fps)");
    reset_->setToolTip("Pause and restore the initial simulation state");
    controls->addWidget(reset_);
    controls->addWidget(play_);
    controls->addWidget(pause_);
    controls->addWidget(step_);
    controls->addSpacing(10);
    controls->addWidget(new QLabel("Frame", this));
    frame_ = new QSpinBox(this);
    frame_->setObjectName("SimulationFrame");
    frame_->setRange(0, LastFrame);
    frame_->setKeyboardTracking(false);
    frame_->setFixedWidth(70);
    controls->addWidget(frame_);
    controls->addWidget(new QLabel("/ 240   |   24 fps", this));
    controls->addStretch();
    state_ = new QLabel(this);
    state_->setObjectName("SimulationState");
    time_ = new QLabel(this);
    time_->setObjectName("SimulationTime");
    controls->addWidget(state_);
    controls->addSpacing(10);
    controls->addWidget(time_);
    layout->addLayout(controls);

    connect(play_, &QPushButton::clicked, this, [this]()
    {
        if (runtime_.GetSimulationSystem().Time() >= static_cast<double>(LastFrame) / FramesPerSecond)
        {
            runtime_.ResetSimulation();
        }
        runtime_.PlaySimulation();
        Refresh();
    });
    connect(pause_, &QPushButton::clicked, this, [this]()
    {
        runtime_.PauseSimulation();
        Refresh();
    });
    connect(step_, &QPushButton::clicked, this, [this]()
    {
        SeekFrame(std::min(LastFrame, static_cast<int>(std::floor(runtime_.GetSimulationSystem().Time() * FramesPerSecond + 1e-8)) + 1));
    });
    connect(reset_, &QPushButton::clicked, this, [this]()
    {
        runtime_.ResetSimulation();
        Refresh();
    });
    connect(timeline_, &QSlider::sliderPressed, this, [this]()
    {
        runtime_.PauseSimulation();
        Refresh();
    });
    connect(timeline_, &QSlider::valueChanged, this, [this](int value)
    {
        SeekFrame(value);
    });
    connect(frame_, &QSpinBox::valueChanged, this, [this](int value)
    {
        SeekFrame(value);
    });
    Refresh();
}

void SimulationPanel::SeekFrame(int frame)
{
    if (!runtime_.IsStarted() || !runtime_.GetSimulationSystem().HasBodies())
    {
        return;
    }
    runtime_.PauseSimulation();
    frame = std::clamp(frame, 0, LastFrame);
    const auto& simulation = runtime_.GetSimulationSystem();
    const auto target = static_cast<std::uint64_t>(
        std::llround(static_cast<double>(frame) / FramesPerSecond / simulation.FixedStep()));
    if (target < simulation.StepCount())
    {
        runtime_.ResetSimulation();
    }
    // Bounded to ten seconds / 1200 steps for this tiny demo; no reverse integration.
    const auto steps = target - simulation.StepCount();
    for (std::uint64_t step = 0; step < steps; ++step)
    {
        runtime_.StepSimulation();
    }
    Refresh();
}

void SimulationPanel::Refresh()
{
    const auto& simulation = runtime_.GetSimulationSystem();
    const bool available = runtime_.IsStarted() && simulation.HasBodies();
    if (available && simulation.IsPlaying() && simulation.Time() >= static_cast<double>(LastFrame) / FramesPerSecond)
    {
        SeekFrame(LastFrame);
        return;
    }
    const bool playing = simulation.IsPlaying();
    play_->setEnabled(available && !playing);
    pause_->setEnabled(available && playing);
    step_->setEnabled(available && !playing && simulation.Time() < static_cast<double>(LastFrame) / FramesPerSecond);
    reset_->setEnabled(available);
    timeline_->setEnabled(available);
    frame_->setEnabled(available);
    state_->setText(!available ? "No simulation body" : playing ? "Playing"
                                                                : "Paused");
    time_->setText(QString("%1 s   |   %2 steps")
            .arg(simulation.Time(), 0, 'f', 3)
            .arg(static_cast<qulonglong>(simulation.StepCount())));
    const int currentFrame = std::clamp(qRound(simulation.Time() * FramesPerSecond), 0, LastFrame);
    const QSignalBlocker sliderBlocker(timeline_);
    const QSignalBlocker frameBlocker(frame_);
    timeline_->setValue(currentFrame);
    frame_->setValue(currentFrame);
}
} // namespace buffalo::editor