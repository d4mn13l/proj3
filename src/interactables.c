#include "interactables.h"
#include "game.h"
#include "player.h"
#include "shared.h"
#include "text.h"

void interactable_main_console(u8 *data) {
	if (player_get_item_count(ITEM_CAPTIANS_HAND) == 0) {
		game_set_state(GAME_STATE_TEXT, (void *) text[2], NULL);
	} else {
		game_set_state(GAME_STATE_TEXT, (void *) text[3], NULL);
	}
}


void interactable_mine_door(u8 *data) {
	if (g_game.flags & GAME_MINE_UNLOCKED) {
		// goto mine
		game_win();
		// when ur too lazy to actually make the game
	} else {
		g_game.player.thinking = "the door to the mines. i shouldnt go there now.";
		g_game.player.think_for = 4.2f;
	}
}


void interactable_escape_pod(u8 *data) {
	if (g_game.flags & GAME_ESCAPE_PODS_UNLOCKED) {
		game_win();
	} else {
		g_game.player.thinking = "theyre locked. the controls are probably at the main console on the bridge.";
		g_game.player.think_for = 5.0f;
	}
}

