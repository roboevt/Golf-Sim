#pragma once

class GolfGame;

class Player {
public:
	virtual void setup(GolfGame* game) = 0;
	virtual void play(GolfGame* game) = 0;
};

