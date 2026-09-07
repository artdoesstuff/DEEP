#ifndef DEEP_SAVE_H
#define DEEP_SAVE_H

#include "game.h"

/* Returns 0 on success */
int save_game(const GameState *g, const char *path);
int load_game(GameState *g, const char *path);

#endif
