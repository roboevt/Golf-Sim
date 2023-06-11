#include "GolfGame.h"
#include "RandomPlayer.h"

int main(void) {
	std::shared_ptr<Player> rand(new RandomPlayer());

	std::vector<std::shared_ptr<Player>> players;
	players.push_back(rand);
	players.push_back(rand);
	players.push_back(rand);

	GolfGame game(players);
	while(game.turn()) { }
	return 0;
}