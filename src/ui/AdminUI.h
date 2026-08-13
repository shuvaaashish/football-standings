#ifndef ADMIN_UI_H
#define ADMIN_UI_H

#include "App.h"
#include "ui/DbWorker.h"
#include "ui/UiCache.h"
#include "ui/UiState.h"
#include "User.h"

namespace UI { namespace AdminUI {

void render(AppState& appState, DbWorker& dbWorker, UiCache& uiCache, AdminUiState& adminUi, User* currentUser);

} }

#endif
