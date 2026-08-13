#ifndef LEAGUES_UI_H
#define LEAGUES_UI_H

#include "App.h"
#include "ui/DbWorker.h"
#include "ui/UiCache.h"
#include "ui/UiState.h"

namespace UI { namespace LeaguesUI {
    void render(AppState& appState, DbWorker& dbWorker, UiCache& uiCache, ViewerUiState& viewerUi);
} }

#endif
