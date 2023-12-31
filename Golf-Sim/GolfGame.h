#pragma once

#include <vector>
#include <array>
#include <memory>

#include <iostream>  // for debugging only

#include "Player.h"

struct Card {
	enum class Rank: int { ace = 1, two, three, four, five, six, seven, eight, nine, ten, jack, queen, king} rank;
	enum class Suit: int { club, diamond, heart, spade} suit;
	bool flipped;
	int score();
	friend std::ostream& operator<< (std::ostream& stream, const Card& card);
	bool operator==(const Card&) const = default;
};

class GolfGame {
	static constexpr std::size_t ROWS = 2;
	static constexpr std::size_t COLS = 3;
	static constexpr std::size_t DECKS = 1;
	static constexpr int GO_OUT_PENALTY = 10;
	static constexpr bool VERBOSE = false;
	typedef std::array<Card, ROWS * COLS> Hand;
	int turnsTillDone;
	int goOutScore;

	std::vector<Card> deck;
	std::vector<Card> discard;
	std::vector<Hand> hands;
	std::vector<std::shared_ptr<Player>> players;
	int numPlayers, currentPlayer;

	std::vector<Card> createDeck();
	int scoreHand(const Hand hand);
	void deal();
	void print();
	void endGame();
	bool done();

public:
	GolfGame(std::vector<std::shared_ptr<Player>> players);

	void shuffleDeck();
	Card drawCard() { Card c = deck.back(); deck.pop_back(); return c; }
	bool canDrawDisCard() { return discard.size(); }
	Card drawDisCard() { Card c = discard.back(); discard.pop_back(); return c; }
	void discardCard(Card card) { card.flipped = true;  discard.push_back(card); }
	void placeCard(Card card, int index) { 
		discardCard(hands[currentPlayer][index]); 
		hands[currentPlayer][index] = card; }
	void flipCard(int index) { hands[currentPlayer][index].flipped = true; }
	bool inLastTurn() { return turnsTillDone >= 0; }

	bool turn();
};

