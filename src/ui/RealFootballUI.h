#ifndef REAL_FOOTBALL_UI_H
#define REAL_FOOTBALL_UI_H

#include "api/RealFootballService.h"

namespace UI { namespace RealFootballUI {

void renderCompetitions(FootballApi::RealFootballService& service);
void renderMatches(FootballApi::RealFootballService& service);
void renderStandings(FootballApi::RealFootballService& service);

} }

#endif
