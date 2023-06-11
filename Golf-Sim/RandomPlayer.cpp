#include "RandomPlayer.h"
#include "GolfGame.h"
#include <random>

void RandomPlayer::setup(GolfGame* game) {
	game->flipCard(rand() % 3);
	game->flipCard((rand() % 3) + 3);
}

void RandomPlayer::play(GolfGame* game) {
	Card card;

	if (!game->canDrawDisCard() || rand() % 2) {  // draw a card?
		card = game->drawCard();
		if (rand() % 2) {  // place it?
			game->placeCard(card, rand() % 6);
		} else game->discardCard(card);
	}
	else {
		card = game->drawDisCard();
		game->placeCard(card, rand() % 6);
	}
}