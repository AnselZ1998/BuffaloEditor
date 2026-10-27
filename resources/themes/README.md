# Theme organization

Load order is buffalo_dark.qss (application shell), controls/inputs.qss (shared controls), then panels/panels.qss (panel-specific rules). The final layer is panels/viewport/viewportbar.qss. ApplyPanelStyle remains available for isolated future overrides.

Use stable object names for panel-specific selectors. Add every new stylesheet to buffalo_resources in CMakeLists.txt; Qt QSS does not resolve CSS @import. Keep asset/resource paths below resources/. Existing MIT/SPDX headers remain in stylesheets.
