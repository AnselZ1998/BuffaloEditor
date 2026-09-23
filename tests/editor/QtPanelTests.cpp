// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/ui/EditorUI.h"
#include "editor/panels/log/LogPanel.h"
#include "editor/panels/viewport/ViewportPanel.h"
#include "editor/panels/simulation/SimulationPanel.h"
#include "core/EngineRuntime.h"

#include "editor/viewport/IViewport.h"
#include "editor/ui/EditorTheme.h"
#include "editor/services/SelectionService.h"
#include "editor/panels/hierarchy/HierarchyPanel.h"
#include "editor/panels/inspector/InspectorPanel.h"

#include <QApplication>
#include <QLabel>
#include <QSettings>
#include <QTemporaryDir>
#include <QTreeWidget>
#include <QToolButton>
#include <QComboBox>
#include <QDockWidget>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QSlider>
#include <QSpinBox>
#include <QTest>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

using namespace buffalo::editor;

namespace
{
void Require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

class ProbeViewport final : public IViewport
{
public:
    void OnInput(const InputEvent& event) override
    {
        events.push_back(event);
    }
    void SetSize(int width, int height) override
    {
        lastSize = QSize(width, height);
    }
    std::size_t BackendCount() const override
    {
        return 2;
    }
    const char* BackendName(std::size_t index) const override
    {
        return index == 0 ? "First" : "Second";
    }
    std::size_t ActiveBackend() const override
    {
        return active;
    }
    void SelectBackend(std::size_t index) override
    {
        active = index;
    }

    ViewportFrame RenderFrame() override
    {
        ++renders;
        return valid ? ViewportFrame{pixels.data(), 2, 2, 12, PixelFormat::BGRA8} : ViewportFrame{};
    }

    // Two padded rows: red/green, blue/white. Padding must never appear on screen.
    std::array<std::uint32_t, 6> pixels{0xffff0000u, 0xff00ff00u, 0u, 0xff0000ffu, 0xffffffffu, 0u};
    std::vector<InputEvent> events;
    QSize lastSize;
    std::size_t active = 0;
    int renders = 0;
    bool valid = true;
    ViewportShowFlags flags{false};
    ViewportShowFlags GetShowFlags() const override
    {
        return flags;
    }
    void SetShowFlags(const ViewportShowFlags& value) override
    {
        flags = value;
    }
};

void Drag(QWidget* surface, Qt::MouseButton button, InputAction expected, ProbeViewport& viewport)
{
    viewport.events.clear();
    const QPoint start(40, 40);
    const QPoint end(60, 50);
    QTest::mousePress(surface, button, Qt::NoModifier, start);
    QMouseEvent move(QEvent::MouseMove, QPointF(end), QPointF(surface->mapToGlobal(end)), Qt::NoButton, button, Qt::NoModifier);
    QApplication::sendEvent(surface, &move);
    QTest::mouseRelease(surface, button, Qt::NoModifier, end);
    Require(viewport.events.size() == 2, "Drag emits one movement and one navigation end");
    Require(viewport.events[0].action == expected && viewport.events[1].action == InputAction::NavigationEnd, "Mouse action mapping");
    Require(std::abs(viewport.events[0].x - 20.0f / surface->height()) < 0.0001f, "Input normalized by viewport height");
}
} // namespace

int main(int argc, char* argv[])
{
#ifdef _MSC_VER
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
    std::cerr << "Entering QApplication...\n";
    QApplication application(argc, argv);
    const bool themeReady = EditorTheme::Apply(application);
    try
    {
        std::cerr << "Checking Qt panel setup...\n";
        Require(themeReady && !application.styleSheet().isEmpty(), "QSS loads from the compiled resource");
        ProbeViewport viewport;
        EditorLog log;
        QWidget host;
        auto* layout = new QVBoxLayout(&host);
        auto* panel = new ViewportPanel(viewport, &host);
        auto* logPanel = new LogPanel(log, &host);
        layout->addWidget(panel, 2);
        layout->addWidget(logPanel, 1);
        host.resize(800, 700);
        host.show();
        host.activateWindow();
        QApplication::processEvents();
        auto* surface = panel->findChild<QWidget*>("ViewportImage");
        auto* selector = panel->findChild<QComboBox*>("RendererSelector");
        auto* messages = logPanel->findChild<QPlainTextEdit*>("LogMessages");
        Require(surface && selector && messages, "Panel controls exist");

        panel->Refresh();
        std::cerr << "Checking pixel format and copied storage...\n";
        Require(viewport.renders == 1 && viewport.lastSize.width() > 0, "Visible viewport renders through IViewport");
        const QSize size = surface->size();
        const QImage image = surface->grab().toImage();
        Require(image.pixelColor(image.width() / 4, image.height() / 4) == QColor(Qt::red), "BGRA red channel is preserved");
        Require(image.pixelColor(image.width() / 4, image.height() * 3 / 4) == QColor(Qt::blue), "Padded row pitch and vertical orientation are preserved");
        auto* grid = panel->findChild<QToolButton*>("GridToggle");
        Require(grid && !grid->isChecked(), "Grid button follows IViewport state");
        const auto untouchedPixels = viewport.pixels;
        QTest::mouseClick(grid, Qt::LeftButton);
        Require(viewport.flags.grid, "Grid button updates controller show flags");
        const QImage withGrid = surface->grab().toImage();
        Require(withGrid != image, "Grid is visible on the Qt surface");
        Require(viewport.pixels == untouchedPixels, "Grid overlay never modifies borrowed Runtime pixels");
        QTest::mouseClick(grid, Qt::LeftButton);
        Require(surface->grab().toImage() == image, "Grid toggle fully removes the overlay");
        viewport.pixels.fill(0xff000000u);
        const QImage copied = surface->grab().toImage();
        Require(copied.pixelColor(copied.width() / 4, copied.height() / 4) == QColor(Qt::red), "Qt owns a copy before renderer buffer reuse");
        viewport.valid = false;
        panel->Refresh();
        const QImage empty = surface->grab().toImage();
        Require(empty.pixelColor(4, 4) == QColor("#1b1e22"), "Invalid frame clears previous pixels");

        selector->setCurrentIndex(1);
        std::cerr << "Checking mouse input...\n";
        Require(viewport.active == 1, "Renderer selection reaches IViewport");
        Drag(surface, Qt::LeftButton, InputAction::Orbit, viewport);
        Drag(surface, Qt::RightButton, InputAction::Orbit, viewport);
        Drag(surface, Qt::MiddleButton, InputAction::Pan, viewport);

        viewport.events.clear();
        std::cerr << "Checking keyboard and focus...\n";
        QWheelEvent wheel(QPointF(40, 40), QPointF(surface->mapToGlobal(QPoint(40, 40))), QPoint(), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
        QApplication::sendEvent(surface, &wheel);
        Require(viewport.events.size() == 1 && viewport.events.back().action == InputAction::Zoom && viewport.events.back().x == 1.0f, "Wheel maps to zoom steps");
        surface->setFocus();
        QTest::keyClick(surface, Qt::Key_F);
        Require(viewport.events.back().action == InputAction::FrameAll, "F frames the scene");
        QTest::keyPress(surface, Qt::Key_W);
        QTest::qWait(10);
        panel->Refresh();
        Require(viewport.events.back().action == InputAction::Pan, "Held WASD pans using elapsed time");
        messages->setFocus();
        QApplication::processEvents();
        Require(viewport.events.back().action == InputAction::NavigationEnd, "Focus loss ends navigation");
        const auto inputCount = viewport.events.size();
        QTest::keyClick(messages, Qt::Key_F);
        panel->Refresh();
        Require(viewport.events.size() == inputCount, "Log focus cannot move the camera or leave held keys stuck");

        host.resize(1000, 800);
        std::cerr << "Checking size and visibility...\n";
        QApplication::processEvents();
        panel->Refresh();
        Require(surface->size() != size, "Viewport follows layout resize");
        Require(viewport.lastSize.width() == qRound(surface->width() * surface->devicePixelRatioF()), "Resize passes pixel dimensions to IViewport");
        panel->hide();
        const int renderCount = viewport.renders;
        panel->Refresh();
        Require(viewport.lastSize == QSize(0, 0) && viewport.renders == renderCount, "Hidden viewport stops rendering");
        panel->show();
        panel->Refresh();
        Require(viewport.renders == renderCount + 1, "Showing viewport resumes rendering");

        log.Write("<tag> & plain text");
        std::cerr << "Checking original Log integration...\n";
        logPanel->Refresh();
        Require(messages->toPlainText() == QString::fromStdString(log.Lines().back()) && messages->toPlainText().endsWith("<tag> & plain text"), "Existing log is displayed as plain text");
        for (int index = 0; index < 250; ++index)
        {
            log.Write("line " + std::to_string(index));
        }
        logPanel->Refresh();
        Require(messages->toPlainText().section('\n', 0, 0).endsWith(" line 50") && messages->toPlainText().count('\n') == 199, "Log refresh handles the bounded deque");
        log.Write("latest");
        logPanel->Refresh();
        Require(messages->toPlainText().endsWith("latest"), "Log updates when deque size stays at 200");
        auto* scrollbar = messages->verticalScrollBar();
        scrollbar->setValue(0);
        log.Write("while reading");
        logPanel->Refresh();
        Require(scrollbar->value() == 0, "Log preserves scroll position while reading");
        QTest::mouseClick(logPanel->findChild<QPushButton*>("ClearLog"), Qt::LeftButton);
        Require(log.Lines().empty() && messages->toPlainText().isEmpty(), "Clear updates both original Log and widget");
        host.hide();

        SelectionService selection;
        EditorUI editor;
        editor.SetCentralWidget(new ViewportPanel(viewport));
        auto* hierarchyPanel = new HierarchyPanel(selection, true);
        auto* inspectorPanel = new InspectorPanel(selection);
        editor.AddDock("Hierarchy", "HierarchyDock", hierarchyPanel, Qt::LeftDockWidgetArea);
        editor.AddDock("Inspector", "InspectorDock", inspectorPanel, Qt::RightDockWidgetArea);
        editor.AddDock("Log", "LogDock", new LogPanel(log), Qt::BottomDockWidgetArea);
        editor.CaptureDefaultLayout();
        std::cerr << "Checking dock layout...\n";
        editor.show();
        QApplication::processEvents();
        Require(editor.windowTitle() == "Buffalo Editor", "Buffalo branding");
        Require(editor.centralWidget() && editor.findChildren<QDockWidget*>().size() == 3, "Viewport plus three dock panels");
        auto* hierarchy = editor.findChild<QDockWidget*>("HierarchyDock");
        hierarchy->hide();
        editor.ResetLayout();
        Require(hierarchy->isVisible(), "Reset layout restores a closed panel");
        QTemporaryDir directory;
        Require(directory.isValid(), "Temporary layout directory exists");
        QSettings settings(directory.filePath("layout.ini"), QSettings::IniFormat);
        hierarchy->hide();
        editor.SaveLayout(settings);
        hierarchy->show();
        editor.RestoreLayout(settings);
        Require(!hierarchy->isVisible(), "Saved dock visibility is restored");
        editor.ResetLayout();
        auto* tree = hierarchyPanel->findChild<QTreeWidget*>("SceneHierarchy");
        tree->topLevelItem(0)->setSelected(true);
        Require(selection.SelectedId() == "scene", "Hierarchy writes shared selection state");
        auto* title = inspectorPanel->findChild<QLabel*>("SelectionTitle");
        Require(title->text() == "Selected: scene", "Inspector observes selection without knowing Hierarchy");
        selection.Clear();
        Require(tree->selectedItems().isEmpty() && title->text() == "Nothing selected", "Clearing shared selection updates both independent panels");
        std::cerr << "Checking simulation timeline transport and deterministic replay...\n";
        buffalo::EngineRuntime simulationRuntime;
        Require(simulationRuntime.StartUp(), "Simulation test runtime starts");
        SimulationPanel simulationPanel(simulationRuntime);
        simulationPanel.resize(1000, 110);
        simulationPanel.show();
        QApplication::processEvents();
        auto* timeline = simulationPanel.findChild<QSlider*>("SimulationTimeline");
        auto* currentFrame = simulationPanel.findChild<QSpinBox*>("SimulationFrame");
        auto* play = simulationPanel.findChild<QPushButton*>("SimulationPlay");
        auto* pause = simulationPanel.findChild<QPushButton*>("SimulationPause");
        auto* step = simulationPanel.findChild<QPushButton*>("SimulationStep");
        auto* reset = simulationPanel.findChild<QPushButton*>("SimulationReset");
        Require(timeline && currentFrame && play && pause && step && reset, "Timeline controls exist");
        const auto initialSceneBounds = simulationRuntime.GetSceneManager().GetCurrentScene()->GetWorldBounds();
        currentFrame->setValue(12);
        Require(simulationRuntime.GetSimulationSystem().StepCount() == 60, "Frame entry seeks real fixed physics steps");
        std::vector<buffalo::RigidBodyState> earlier;
        simulationRuntime.GetSimulationSystem().Readback(earlier);
        Require(!earlier.empty(), "Seeking produces a real body state");
        Require((simulationRuntime.GetSceneManager().GetCurrentScene()->GetWorldBounds().Center() - initialSceneBounds.Center()).Length() > 0.001f,
            "Timeline seeking synchronizes visible Scene transforms");

        QTest::mouseClick(play, Qt::LeftButton);
        Require(simulationRuntime.GetSimulationSystem().IsPlaying() && pause->isEnabled(), "Play resumes simulation");
        const QPoint timelineStart(16, 30);
        const QPoint timelineFrame60(16 + qRound((timeline->width() - 32) * 0.25), 30);
        QTest::mousePress(timeline, Qt::LeftButton, Qt::NoModifier, timelineStart);
        QMouseEvent scrubMove(QEvent::MouseMove, QPointF(timelineFrame60),
            QPointF(timeline->mapToGlobal(timelineFrame60)), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(timeline, &scrubMove);
        QTest::mouseRelease(timeline, Qt::LeftButton, Qt::NoModifier, timelineFrame60);
        Require(!simulationRuntime.GetSimulationSystem().IsPlaying(), "Scrubbing pauses simulation");
        Require(timeline->value() == 60 && simulationRuntime.GetSimulationSystem().StepCount() == 300,
            "Dragging the ruler to frame 60 advances 300 actual physics steps");
        currentFrame->setValue(12);
        std::vector<buffalo::RigidBodyState> replayed;
        simulationRuntime.GetSimulationSystem().Readback(replayed);
        Require(replayed.size() == earlier.size() && (replayed.front().position - earlier.front().position).Length() < 1e-6f && (replayed.front().velocity - earlier.front().velocity).Length() < 1e-6f,
            "Backward seek resets and deterministically replays position and velocity");
        QTest::mouseClick(reset, Qt::LeftButton);
        Require(simulationRuntime.GetSimulationSystem().StepCount() == 0 && currentFrame->value() == 0,
            "Reset returns body and timeline to the initial frame");
        QTest::mouseClick(step, Qt::LeftButton);
        Require(simulationRuntime.GetSimulationSystem().StepCount() == 5 && currentFrame->value() == 1,
            "Step advances one display frame while staying paused");
        simulationRuntime.ShutDown();
        simulationPanel.Refresh();
        Require(!timeline->isEnabled() && !play->isEnabled() && !currentFrame->isEnabled(),
            "Timeline cannot seek a stopped runtime");
        simulationPanel.hide();
        std::cout << "Passed: Qt pixels, input, dock layout and simulation timeline replay.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
