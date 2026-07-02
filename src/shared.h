#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>

#include "3ds/os.h"
#include "game.h"

extern float g_delta;
extern TickCounter g_timer;

extern FILE *g_log_file;
extern game_t g_game;

#endif
