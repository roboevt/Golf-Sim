#include "GolfGame.h"
#include <algorithm>
#include <random>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h> //ugh, for emojis
#endif

using std::cout, std::endl;  // I'm tired of writing std::

int Card::score() {
	switch (this->rank) {
	case Rank::jack:
		return -2;
	case Rank::queen:
		return 10;
	case Rank::king:
		return 0;
	default:
		return static_cast<int>(this->rank);
	}
}

GolfGame::GolfGame(std::vector<std::shared_ptr<Player>> players) : 
	players(players), numPlayers(static_cast<int>(players.size())), turnsTillDone(-1),
	goOutScore(0) {
	if (numPlayers < 2 || numPlayers * 6 >= 52 * DECKS) {
		cout << "Invalid number of players" << endl;
		exit(1);  // TODO better usage message method
	}
	srand(static_cast<uint32_t>(time(0)));
	deck = createDeck();
	shuffleDeck();
	discard.reserve(52);
	hands.resize(numPlayers);

	deal();
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);  // for emojis
#endif
	if (VERBOSE) {
		cout << "Initial state:" << endl;
		print();
	}
}

void GolfGame::shuffleDeck() {
	static auto rng = std::default_random_engine{};
	rng.seed(static_cast<uint32_t>(time(0)));
	std::shuffle(std::begin(deck), std::end(deck), rng);
}

std::vector<Card> GolfGame::createDeck() {
	std::vector<Card> deck;
	deck.resize(52 * DECKS);
	int i = 0;

	for (int r = static_cast<int>(Card::Rank::ace); r <= static_cast<int>(Card::Rank::king); r++) {
		for (int s = static_cast<int>(Card::Suit::club); s <= static_cast<int>(Card::Suit::spade); s++) {
			// am I dumb? Why must this for loop be <= and not the rank?
			Card card{ static_cast<Card::Rank>(r), static_cast<Card::Suit>(s), true };
			deck[i++] = card;
		}
	}

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
		return stream << ranks[static_cast<std::size_t>(card.rank)] 
					  << suits[static_cast<std::size_t>(card.suit)];
	else return stream << "🂠 ";
}

void GolfGame::print() {
	cout << "Current player: " << currentPlayer + 1 << endl;
	cout << "▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔" << endl;
	cout << "  Deck: |";
	for (Card card : deck) cout << card << "|";
	cout << "\n\n  ";

	for (int p = 0; p < numPlayers; p++) {
		cout << "   Player " << p + 1 << ":\t";
	}
	cout << "\n";
	for (int row = 0; row < ROWS; row++) {
		cout << "  |";
		for (int p = 0; p < numPlayers; p++) {
			for (int col = 0; col < COLS; col++) {
				cout << hands[p][col + row * COLS] << "|";
			}
			if(p < numPlayers - 1) cout << "\t |";
		}
		cout << endl;
	}

	cout << "\n\n  Discard: |";
	for (Card card : discard) cout << card << "|";
	cout << "\n\n\n";
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

	if (deck.empty()) {
		deck.swap(discard);
		shuffleDeck();
	}

	if (done()) return false;
	else {
		// loop through players
		currentPlayer = ++currentPlayer % numPlayers;
		if (VERBOSE) print();
	}
	return true;
}

void GolfGame::endGame() {
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
			if (score < goOutScore) winningPlayer = p;
		}
		players[p].get()->addScore(score);
	}

	if (minScore <= goOutScore) goOutScore += GO_OUT_PENALTY;
	else winningPlayer = goOutPlayer;
	players[goOutPlayer].get()->addScore(goOutScore);

	cout << " ▁▁▁▁▁▁▁▁▁▁▁▁\n"
		<< "▕ Game over! ▏\n"
		<< " ▔▔▔▔▔▔▔▔▔▔▔▔" << endl;
	print();

	cout << "Scores:\n  ";
	for (int p = 0; p < numPlayers; p++) {
		int score = (p == goOutPlayer) ? goOutScore : scoreHand(hands[p]);
		cout << "Player " << p + 1 << " (" << score <<
			(p == numPlayers - 1 ? ")" : "), ");
	}

	cout << "\nWinner: " << winningPlayer + 1;
	cout << " (" << scoreHand(hands[winningPlayer]) << ")" << endl;
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
			endGame();
			return true;
		}
	}
}