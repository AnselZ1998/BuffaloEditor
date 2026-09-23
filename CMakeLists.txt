# MIT License - Copyright (c) 2026 AnselZ
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.20)
project(Buffalo VERSION 0.1.0 LANGUAGES CXX)

option(BUFFALO_BUILD_EDITOR "Build the Qt Widgets editor and OpenGL integration" ON)
option(BUFFALO_BUILD_TESTS "Build simulation, runtime and optional Qt regression tests" OFF)
option(BUFFALO_TEST_OPENGL "Register real-device OpenGL tests (requires a desktop graphics context)" OFF)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Use an out-of-source build: cmake -S . -B build-editor")
endif()

function(buffalo_configure_target target)
    target_compile_features(${target} PUBLIC cxx_std_17)
    target_include_directories(${target} PUBLIC "${PROJECT_SOURCE_DIR}/include")
    target_compile_definitions(${target} PUBLIC UNICODE _UNICODE NOMINMAX)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W3 /sdl /permissive- /utf-8)
    endif()
endfunction()

# The simulation library deliberately has no Scene, renderer, Qt or GPU dependency.
add_library(BuffaloSimulation STATIC
    src/simulation/SimulationSystem.cpp
    src/simulation/rigidbody/cpu/CpuRigidBodySolver.cpp
)
buffalo_configure_target(BuffaloSimulation)

# Runtime has no Qt/editor/window dependency. Legacy Win32 code is archived in bak/legacy-win32/.
add_library(BuffaloRuntime STATIC
    src/core/EngineRuntime.cpp
    src/core/SceneSimulationBinding.cpp
    src/math/BoundingVolume.cpp
    src/math/Matrix.cpp
    src/math/Quaternion.cpp
    src/assets/model/Mesh.cpp
    src/assets/model/Model.cpp
    src/assets/import/objParser.cpp
    src/render/RenderSystem.cpp
    src/render/material/Material.cpp
    src/render/shader/Shader.cpp
    src/render/software/Rasterizer.cpp
    src/render/software/SoftwareRenderer.cpp
    src/render/software/SoftwareRenderBackend.cpp
    src/scene/Camera.cpp
    src/scene/Scene.cpp
    src/scene/SceneManager.cpp
)
buffalo_configure_target(BuffaloRuntime)
target_link_libraries(BuffaloRuntime PUBLIC BuffaloSimulation)
target_compile_definitions(BuffaloRuntime PRIVATE
    BUFFALO_DEFAULT_MODEL_PATH="${PROJECT_SOURCE_DIR}/model/toy.obj")

if(BUFFALO_BUILD_EDITOR)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)
    # Qt integration is optional; the standalone Runtime and Simulation remain Qt-free.
    add_library(BuffaloOpenGL STATIC src/render/opengl/OpenGLRenderBackend.cpp)
    buffalo_configure_target(BuffaloOpenGL)
    target_link_libraries(BuffaloOpenGL PUBLIC BuffaloRuntime PRIVATE Qt6::OpenGL)
    set(buffalo_editor_sources
        include/editor/services/SelectionService.h
        src/editor/services/SelectionService.cpp
        src/editor/services/EditorContext.cpp
        src/editor/services/EditorLog.cpp
        src/editor/viewport/ViewportController.cpp
        src/editor/ui/EditorUI.cpp
        src/editor/ui/EditorTheme.cpp
        src/editor/panels/viewport/ViewportPanel.cpp
        src/editor/panels/viewport/ViewportSurface.cpp
        src/editor/panels/viewport/ViewportToolbar.cpp
        src/editor/panels/viewport/presentation/OpenGLFramePresenter.cpp
        src/editor/panels/simulation/SimulationPanel.cpp
        src/editor/panels/hierarchy/HierarchyPanel.cpp
        src/editor/panels/inspector/InspectorPanel.cpp
        src/editor/panels/log/LogPanel.cpp
    )
    set(buffalo_resources resources/themes/buffalo_dark.qss)
    list(APPEND buffalo_resources
        resources/shaders/opengl/mesh.vert
        resources/shaders/opengl/diffuse.frag)
    if(EXISTS "${PROJECT_SOURCE_DIR}/resources/icons/buffalo_logo.png")
        list(APPEND buffalo_resources resources/icons/buffalo_logo.png)
    endif()

    if(EXISTS "${PROJECT_SOURCE_DIR}/resources/icons/grid.png")
        list(APPEND buffalo_resources resources/icons/grid.png)
    endif()

    function(buffalo_configure_editor target)
        buffalo_configure_target(${target})
        set_target_properties(${target} PROPERTIES AUTOMOC ON)
        target_link_libraries(${target} PRIVATE BuffaloRuntime BuffaloOpenGL Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)
        qt_add_resources(${target} buffalo_resources PREFIX "/buffalo"
            BASE "${PROJECT_SOURCE_DIR}/resources" FILES ${buffalo_resources})
    endfunction()

    qt_add_executable(BuffaloEditor WIN32 src/app/editor_main.cpp src/editor/app/EditorApplication.cpp ${buffalo_editor_sources})
    buffalo_configure_editor(BuffaloEditor)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT BuffaloEditor)

    if(WIN32)
        get_target_property(buffalo_qmake Qt6::qmake IMPORTED_LOCATION)
        get_filename_component(buffalo_qt_bin "${buffalo_qmake}" DIRECTORY)
        find_program(BUFFALO_WINDEPLOYQT windeployqt HINTS "${buffalo_qt_bin}" NO_DEFAULT_PATH REQUIRED)
    endif()
    function(buffalo_deploy_qt target)
        if(WIN32)
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND "${BUFFALO_WINDEPLOYQT}" "--$<IF:$<CONFIG:Debug>,debug,release>"
                    --no-translations --no-opengl-sw --no-compiler-runtime --verbose 0 "$<TARGET_FILE:${target}>"
                COMMENT "Copying Qt runtime for ${target}" VERBATIM)
        endif()
    endfunction()
    buffalo_deploy_qt(BuffaloEditor)
endif()

# CPU tests never require a display or a GPU. Real-device GL tests are opt-in.
if(BUFFALO_BUILD_TESTS)
    enable_testing()
    add_executable(SimulationTests tests/simulation/SimulationTests.cpp)
    buffalo_configure_target(SimulationTests)
    target_link_libraries(SimulationTests PRIVATE BuffaloSimulation)
    add_test(NAME simulation_contract COMMAND SimulationTests)
    set_tests_properties(simulation_contract PROPERTIES TIMEOUT 30)
    add_executable(FrameworkTests tests/editor/FrameworkTests.cpp
        src/editor/viewport/ViewportController.cpp src/editor/services/EditorLog.cpp)
    buffalo_configure_target(FrameworkTests)
    target_link_libraries(FrameworkTests PRIVATE BuffaloRuntime)
    add_test(NAME framework_contract COMMAND FrameworkTests)
    set_tests_properties(framework_contract PROPERTIES TIMEOUT 120)

    if(BUFFALO_BUILD_EDITOR)
        find_package(Qt6 6.5 REQUIRED COMPONENTS Test)
        add_executable(QtPanelTests tests/editor/QtPanelTests.cpp ${buffalo_editor_sources})
        buffalo_configure_editor(QtPanelTests)
        target_link_libraries(QtPanelTests PRIVATE Qt6::Test)
        buffalo_deploy_qt(QtPanelTests)
        if(WIN32)
            add_custom_command(TARGET QtPanelTests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt6::QOffscreenIntegrationPlugin>"
                    "$<TARGET_FILE_DIR:QtPanelTests>/platforms/" VERBATIM)
        endif()
        add_test(NAME qt_panel_contract COMMAND QtPanelTests -platform offscreen)
        set_tests_properties(qt_panel_contract PROPERTIES TIMEOUT 60)
        add_test(NAME editor_smoke COMMAND BuffaloEditor --smoke-test -platform offscreen)
        set_tests_properties(editor_smoke PROPERTIES TIMEOUT 120)

        add_executable(OpenGLTests tests/render/OpenGLTests.cpp)
        buffalo_configure_editor(OpenGLTests)
        buffalo_deploy_qt(OpenGLTests)
        if(BUFFALO_TEST_OPENGL)
            add_test(NAME opengl_contract COMMAND OpenGLTests)
            set_tests_properties(opengl_contract PROPERTIES TIMEOUT 60 LABELS "gpu")
        endif()
    endif()
endif()
