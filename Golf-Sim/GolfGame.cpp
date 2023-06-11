#include "GolfGame.h"
#include <algorithm>
#include <random>
#include <iostream>
#include <string>

#include <windows.h> //ugh, for emojis

int Card::score() {
	switch (rank) {
	case jack:
		return -2;
	case queen:
		return 10;
	case king:
		return 0;
	default:
		return rank;
	}
}

GolfGame::GolfGame(std::vector<std::shared_ptr<Player>> players) : 
	players(players), numPlayers(static_cast<int>(players.size())), turnsTillDone(-1),
	goOutScore(0) {
	if (numPlayers < 2 || numPlayers * 6 > 52) {  // Todo parameter for number of decks
		std::cout << "Invalid number of players" << std::endl;
		exit(1);  // TODO better usage message method
	}
	srand(static_cast<uint32_t>(time(0)));
	deck = createRandomDeck();
	discard.reserve(52);
	hands.resize(numPlayers);

	deal();

	SetConsoleOutputCP(CP_UTF8);  // for emojis
	std::cout << "Initial state:" << std::endl;
	print();
}

std::vector<Card> GolfGame::createRandomDeck() {
	std::vector<Card> deck;
	deck.resize(52 * DECKS);
	int i = 0;

	for (int r = Card::Rank::ace; r <= Card::Rank::king; r++) {
		for (int s = Card::Suit::club; s <= Card::Suit::spade; s++) {
			// am I dumb? Why must this for loop be <= and not the rank?
			Card card{ static_cast<Card::Rank>(r), static_cast<Card::Suit>(s), true };
			deck[i++] = card;
		}
	}

	auto rng = std::default_random_engine{};
	rng.seed(static_cast<uint32_t>(time(0)));
	std::shuffle(std::begin(deck), std::end(deck), rng);

	return deck;
}

void GolfGame::deal() {
	for (int p = 0; p < numPlayers; p++) {
		for (int i = 0; i < 6; i++) {
			hands[p][i] = this->drawCard();
			hands[p][i].flipped = false;
		}
		currentPlayer = p;
		players[p].get()->setup(this);
	}
	currentPlayer = 0;
}

std::ostream& operator<< (std::ostream& stream, const Card& card) {
	static const std::string ranks[] = {
	"unused", "A", "2", "3", "4", "5", "6", "7", "8", "9", "X", "J", "Q", "K" };

	static const std::string suits[] = {
	"♣️", "♦️", "♥️", "♠️" };
	if (card.flipped)
		return stream << ranks[card.rank] << suits[card.suit];
	else return stream << "🂠 ";
}

void GolfGame::print() {
	std::cout << "Current player: " << currentPlayer + 1 << std::endl;
	std::cout << "▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔" << std::endl;
	std::cout << "  Deck: |";
	for (Card card : deck) std::cout << card << "|";
	std::cout << "\n\n  ";

	for (int p = 0; p < numPlayers; p++) {
		std::cout << "   Player " << p + 1 << ":\t";
	}
	std::cout << "\n";
	for (int row = 0; row < ROWS; row++) {
		std::cout << "  |";
		for (int p = 0; p < numPlayers; p++) {
			for (int col = 0; col < COLS; col++) {
				std::cout << hands[p][col + row * COLS] << "|";
			}
			if(p < numPlayers - 1) std::cout << "\t |";
		}
		std::cout << std::endl;
	}

	std::cout << "\n\n  Discard: |";
	for (Card card : discard) std::cout << card << "|";
	std::cout << "\n\n\n";
}

int GolfGame::scoreHand(const Hand hand) {
	int totalScore = 0;

	for (int col = 0; col < COLS; col++) {
		bool colAllSame = true;
		Card prevCard = hand[col];
		int colScore = prevCard.score();

		for (int row = 1; row < ROWS; row++) {
			Card newCard = hand[col + row * COLS];
			colScore += newCard.score();

			if (newCard.rank != prevCard.rank) {
				colAllSame = false;
			}

			prevCard = newCard;
		}

		if (!colAllSame) {
			totalScore += colScore;
		}
	}
	return totalScore;
}

bool GolfGame::turn() {
	players[currentPlayer].get()->play(this);

	if (done()) return false;
	else {
		currentPlayer = ++currentPlayer % numPlayers;  // loop through players
		print();
	}
	return true;
}

bool GolfGame::done() {
	if (turnsTillDone < 0) {  // normal game phase
		for (const Hand& hand : hands) {
			char allFlipped = 1;
			for (const Card& card : hand) {
				allFlipped *= card.flipped;
			}
			if (allFlipped) {  // Current player went out first
				turnsTillDone = numPlayers - 1;
				goOutScore = scoreHand(hand);
			}
		}
		return false;
	}
	else {  // last round
		if (turnsTillDone) {
			turnsTillDone--;
			return false;
		}
		else {  // game over
			int goOutPlayer = currentPlayer;
			int minScore = INT_MAX;
			int winningPlayer = 0;

			for (int p = 0; p < numPlayers; p++) {
				if (p == goOutPlayer) continue;  // skip player who "went out"
				for (Card& card : hands[p]) {
					card.flipped = true;
				}
				int score = scoreHand(hands[p]);
				if (score < minScore) {
					minScore = score;
					if(score < goOutScore) winningPlayer = p;
				}
				players[p].get()->addScore(score);
			}
			if (minScore <= goOutScore) goOutScore += GO_OUT_PENALTY;
			else winningPlayer = goOutPlayer;
			players[goOutPlayer].get()->addScore(goOutScore);

			std::cout << " ▁▁▁▁▁▁▁▁▁▁▁▁\n"
				      << "▕ Game over! ▏\n" 
					  << " ▔▔▔▔▔▔▔▔▔▔▔▔" << std::endl;
			print();
			std::cout << "Scores:\n  ";
			for (int p = 0; p < numPlayers; p++) {
				int score = (p == goOutPlayer) ? goOutScore : scoreHand(hands[p]);
				std::cout << "Player " << p + 1 << " (" << score << 
					(p == numPlayers - 1 ? ")" : "), ");
			}
			std::cout << "\nWinner: " << winningPlayer + 1;
			std::cout << " (" << scoreHand(hands[winningPlayer]) << ")" << std::endl;
			
			return true;
		}
	}
}