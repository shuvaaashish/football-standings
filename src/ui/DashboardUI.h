#ifndef DASHBOARD_UI_H
#define DASHBOARD_UI_H

#include "ui/App.h"
#include "ui/UiState.h"
#include "ui/DbWorker.h"
#include "ui/UiCache.h"
#include "User.h"
#include "api/RealFootballService.h"

namespace UI { namespace DashboardUI {

void render(AppState& appState, DbWorker& dbWorker, UiCache& uiCache,
            FootballApi::RealFootballService& realFootballService,
            ViewerUiState& viewerUi, User* currentUser);

} }

#endif
