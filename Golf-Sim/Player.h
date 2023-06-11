#pragma once
#include <vector>
#include <numeric>

class GolfGame;

class Player {
	std::vector<int> scores;
public:
	virtual void setup(GolfGame* game) = 0;
	virtual void play(GolfGame* game) = 0;
	void addScore(int score) { scores.push_back(score); }
	std::vector<int> getScores() { return scores; }
	int getScore() { return std::reduce(scores.begin(), scores.end()); }
};

