// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/app/EditorApplication.h"
#include "core/EngineRuntime.h"
#include "editor/services/EditorContext.h"
#include "editor/services/EditorLog.h"
#include "editor/services/SelectionService.h"
#include "editor/viewport/ViewportController.h"
#include "editor/panels/viewport/ViewportPanel.h"
#include "editor/panels/hierarchy/HierarchyPanel.h"
#include "editor/panels/inspector/InspectorPanel.h"
#include "editor/panels/log/LogPanel.h"
#include "editor/panels/simulation/SimulationPanel.h"
#include "render/opengl/OpenGLRenderBackend.h"
#include "editor/ui/EditorTheme.h"
#include "editor/ui/EditorUI.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QDockWidget>
#include <QElapsedTimer>
#include <QOpenGLContext>
#include <QSettings>
#include <QSurfaceFormat>
#include <QStatusBar>
#include <QTimer>
#include <exception>
#include <memory>
#include <string>

namespace buffalo::editor
{
int EditorApplication::Run(int argc, char* argv[])
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QSurfaceFormat glFormat;
    glFormat.setVersion(3, 3);
    glFormat.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(glFormat);
    QApplication application(argc, argv);
    QApplication::setApplicationName("BuffaloEditor");
    QApplication::setOrganizationName("Buffalo");
    const bool themeReady = EditorTheme::Apply(application);

    EngineRuntime runtime;
    std::string startupError;
    try
    {
        if (!runtime.StartUp())
        {
            startupError = "Scene could not load.";
        }
    }
    catch (const std::exception& error)
    {
        startupError = std::string("Runtime startup failed: ") + error.what();
    }

    EditorLog log;
    SelectionService selection;
    EditorContext context(runtime, selection, log);
    log.Info(startupError.empty() ? "Buffalo Runtime ready. Software backend registered." : startupError);

    if (!themeReady)
    {
        log.Error("Could not load the Buffalo dark theme resource.");
    }
    if (EditorTheme::Logo().isNull())
    {
        log.Error("Original buffalo_logo.png is not installed yet.");
    }

    const QDir output(QCoreApplication::applicationDirPath());
    const bool capturePreview = application.arguments().contains("--capture-preview");
    const bool smokeTest = application.arguments().contains("--smoke-test");
    const bool requestOpenGL = application.arguments().contains("--opengl");
    const bool transientRun = capturePreview || smokeTest;
    int result = 0;

    {
        ViewportController viewport(context.runtime, context.log);
        // Temporary unique ownership is released when the Qt shell adopts each panel.
        auto viewportPanel = std::make_unique<ViewportPanel>(viewport);
        auto hierarchyPanel = std::make_unique<HierarchyPanel>(context.selection, runtime.IsStarted());
        auto inspectorPanel = std::make_unique<InspectorPanel>(context.selection);
        auto logPanel = std::make_unique<LogPanel>(context.log);
        auto simulationPanel = std::make_unique<SimulationPanel>(runtime);
        auto* simulationWidget = simulationPanel.get();
        bool openGLRegistered = false;
        std::size_t openGLBackendIndex = 0;
        viewportPanel->SetOpenGLReadyCallback([&runtime, &log, &openGLRegistered, &openGLBackendIndex, requestOpenGL](QOpenGLContext* shareContext)
        {
            if (openGLRegistered)
            {
                return;
            }
            // Qt creates the context; the optional backend owns its rendering resources.
            try
            {
                runtime.GetRenderSystem().AddBackend(std::make_unique<OpenGLRenderBackend>(shareContext));
                openGLRegistered = true;
                openGLBackendIndex = runtime.GetRenderSystem().BackendCount() - 1;
                if (requestOpenGL)
                {
                    runtime.GetRenderSystem().SelectBackend(openGLBackendIndex);
                }
                log.Info("OpenGL backend ready. Choose OpenGL from the viewport renderer selector.");
            }
            catch (const std::exception& error)
            {
                log.Warning(std::string("OpenGL unavailable; software rendering remains active: ") + error.what());
            }
        });
        auto* viewportWidget = viewportPanel.get();
        auto* logWidget = logPanel.get();

        EditorUI editor;
        editor.SetCentralWidget(viewportPanel.release());
        auto* hierarchyDock = editor.AddDock("Hierarchy", "HierarchyDock", hierarchyPanel.release(), Qt::LeftDockWidgetArea);
        auto* inspectorDock = editor.AddDock("Inspector", "InspectorDock", inspectorPanel.release(), Qt::RightDockWidgetArea);
        auto* logDock = editor.AddDock("Log", "LogDock", logPanel.release(), Qt::BottomDockWidgetArea);
        auto* simulationDock = editor.AddDock("Simulation", "SimulationDock", simulationPanel.release(), Qt::BottomDockWidgetArea);
        editor.resizeDocks({hierarchyDock, inspectorDock}, {230, 280}, Qt::Horizontal);
        editor.splitDockWidget(logDock, simulationDock, Qt::Vertical);
        editor.setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
        editor.setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
        editor.resizeDocks({logDock, simulationDock}, {160, 110}, Qt::Vertical);

        editor.CaptureDefaultLayout();

        auto* frameAll = new QAction("Frame all", &editor);
        QObject::connect(frameAll, &QAction::triggered, &editor, [&viewport]()
        {
            viewport.OnInput({InputAction::FrameAll});
        });
        editor.AddWorkspaceAction(frameAll);
        QSettings settings(output.filePath("buffalo-layout.ini"), QSettings::IniFormat);
        if (!transientRun)
        {
            editor.RestoreLayout(settings);
        }
        if (!startupError.empty())
        {
            editor.statusBar()->showMessage("Runtime unavailable - check Log");
        }
        log.Info("Buffalo Editor ready. Grid is an editor overlay.");

        QElapsedTimer simulationClock;
        simulationClock.start();
        QTimer refresh;
        QObject::connect(&refresh, &QTimer::timeout, &editor, [viewportWidget, logWidget, simulationWidget, &runtime, &simulationClock]()
        {
            runtime.Update(static_cast<double>(simulationClock.restart()) / 1000.0);
            simulationWidget->Refresh();
            viewportWidget->Refresh();
            logWidget->Refresh();
        });
        editor.show();
        refresh.start(16);
        if (transientRun)
        {
            QTimer::singleShot(1200, &editor, [&]()
            {
                const bool saved = !capturePreview || editor.grab().save(output.filePath("buffalo-editor-preview.png"));
                const bool requestedBackendReady = !requestOpenGL || (openGLRegistered && runtime.GetRenderSystem().ActiveBackend() == openGLBackendIndex);
                application.exit(saved && runtime.IsStarted() && requestedBackendReady ? 0 : 1);
            });
        }
        result = application.exec();
        refresh.stop();
        runtime.GetRenderSystem().ShutDown();
        if (!transientRun)
        {
            editor.SaveLayout(settings);
        }
    } // Qt panels and the controller die before Runtime shutdown.

    selection.Clear();
    runtime.ShutDown();
    return result;
}
} // namespace buffalo::editor
