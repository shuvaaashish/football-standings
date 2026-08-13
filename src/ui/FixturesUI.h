#ifndef FIXTURES_UI_H
#define FIXTURES_UI_H

#include "App.h"
#include "ui/DbWorker.h"
#include "ui/UiCache.h"
#include "ui/UiState.h"

namespace UI { namespace FixturesUI {
    void render(AppState& appState, DbWorker& dbWorker, UiCache& uiCache, ViewerUiState& viewerUi);
} }

#endif
