# Buffalo dependencies

Qt 6 Widgets is an external SDK discovered by CMake through `CMAKE_PREFIX_PATH` or `Qt6_DIR`.
No Qt source is vendored in this repository. The editor links dynamically, and `windeployqt`
copies the required runtime libraries and plugins alongside each built executable.

The previous Dear ImGui sources and Windows/D3D11 editor backends are no longer used.
Qt license and third-party notices are supplied with the installed Qt SDK.
