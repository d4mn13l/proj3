#include "interactables.h"
#include "game.h"
#include "player.h"
#include "text.h"

void main_console(u8 *data) {
	if (player_get_item_count(ITEM_CAPTIANS_HAND) == 0) {
		game_set_state(GAME_STATE_TEXT, (void *) text[2], NULL);
	} else {
		game_set_state(GAME_STATE_TEXT, (void *) text[3], NULL);
	}
}
