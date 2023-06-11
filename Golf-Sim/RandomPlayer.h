#pragma once
#include "Player.h"


class RandomPlayer : public Player {
public:
	void setup(GolfGame* game) override;
	void play(GolfGame* game) override;
};

