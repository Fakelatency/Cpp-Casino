#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <map>
#include <limits>
#include <chrono>
#include <cstdlib>

using namespace std;

const vector<string> suits = {"♥", "♦", "♠", "♣"};
const vector<string> ranks = {"Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King", "Ace"};
const map<string, int> values = {
    {"Two", 2}, {"Three", 3}, {"Four", 4}, {"Five", 5}, {"Six", 6}, {"Seven", 7}, {"Eight", 8},
    {"Nine", 9}, {"Ten", 10}, {"Jack", 10}, {"Queen", 10}, {"King", 10}, {"Ace", 11}
};

class Deck;
class Hand;

class Card {
public:
    string suit;
    string rank;

    Card(string s, string r) : suit(s), rank(r) {}

    friend ostream& operator<<(ostream& os, const Card& card) {
        os << card.rank << " of " << card.suit;
        return os;
    }
};

class Hand {
public:
    vector<Card> cards;
    int value = 0;
    int aces = 0;

    void add_card(const Card& card) {
        cards.push_back(card);
        value += values.at(card.rank);

        if (card.rank == "Ace") {
            aces++;
        }
        adjust_for_ace();
    }

    void adjust_for_ace() {
        while (value > 21 && aces > 0) {
            value -= 10;
            aces--;
        }
    }

    void clear() {
        cards.clear();
        value = 0;
        aces = 0;
    }
};

class Deck {
private:
    mt19937 rng;

public:
    vector<Card> deck;

    Deck() {
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        rng = mt19937(seed);

        for (const auto& suit : suits) {
            for (const auto& rank : ranks) {
                deck.emplace_back(suit, rank);
            }
        }
    }

    void shuffle() {
        std::shuffle(deck.begin(), deck.end(), rng);
    }

    Card deal() {
        if (deck.empty()) {
            throw runtime_error("Dealing from an empty deck!");
        }
        Card dealtCard = deck.back();
        deck.pop_back();
        return dealtCard;
    }
};

class Player {
public:
    int balance;
    int bet;

    Player(int initial_balance = 1000) : balance(initial_balance), bet(0) {}

    void win_bet() {
        balance += bet;
    }

    void lose_bet() {
        balance -= bet;
    }
};

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void take_bet(Player& player) {
    while (true) {
        cout << "How much would you like to bet? ";
        cin >> player.bet;

        if (cin.fail()) {
            cout << "Invalid input. Please enter a number." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            player.bet = 0;
        } else if (player.bet <= 0) {
             cout << "Bet must be a positive amount." << endl;
        }
         else if (player.bet > player.balance) {
            cout << "Sorry, your bet cannot exceed your balance of " << player.balance << endl;
        } else {
             cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        }
    }
}

void hit(Deck& deck, Hand& hand) {
    hand.add_card(deck.deal());
}

bool hit_or_stand(Deck& deck, Hand& hand) {
    while (true) {
        cout << "Would you like to Hit or Stand? Enter h or s: ";
        string choice;
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (!choice.empty()) {
             char lower_choice = tolower(choice[0]);
             if (lower_choice == 'h') {
                 hit(deck, hand);
                 return true;
             } else if (lower_choice == 's') {
                 cout << "Player stands. Dealer is playing." << endl;
                 return false;
             }
        }
        cout << "Sorry, please enter 'h' or 's'." << endl;
    }
}

void show_some(const Hand& player_hand, const Hand& dealer_hand) {
    cout << "\nDealer's Hand:" << endl;
    cout << " <card hidden>" << endl;
    if (dealer_hand.cards.size() > 1) {
         cout << ' ' << dealer_hand.cards[1] << endl;
    }
    cout << "\nPlayer's Hand:" << endl;
    for(const auto& card : player_hand.cards) {
        cout << ' ' << card << endl;
    }
    cout << "Player's Hand = " << player_hand.value << endl;
}

void show_all(const Hand& player_hand, const Hand& dealer_hand) {
    cout << "\nDealer's Hand:" << endl;
     for(const auto& card : dealer_hand.cards) {
        cout << ' ' << card << endl;
    }
    cout << "Dealer's Hand = " << dealer_hand.value << endl;

    cout << "\nPlayer's Hand:" << endl;
    for(const auto& card : player_hand.cards) {
        cout << ' ' << card << endl;
    }
    cout << "Player's Hand = " << player_hand.value << endl;
}

void player_busts() {
    cout << "Player busts!" << endl;
}

void player_wins() {
    cout << "Player wins!" << endl;
}

void dealer_busts() {
    cout << "Dealer busts!" << endl;
}

void dealer_wins() {
    cout << "Dealer wins!" << endl;
}

void push() {
    cout << "Dealer and Player tie! It's a push." << endl;
}


int main() {
    Player player(1000);

    while (true) {
        clear_screen();
        cout << "Welcome to Blackjack! Your current balance is: " << player.balance << endl;

        if (player.balance <= 0) {
            cout << "You're out of money! Thanks for playing." << endl;
            break;
        }

        take_bet(player);

        Deck game_deck;
        game_deck.shuffle();

        Hand player_hand;
        player_hand.add_card(game_deck.deal());
        player_hand.add_card(game_deck.deal());

        Hand dealer_hand;
        dealer_hand.add_card(game_deck.deal());
        dealer_hand.add_card(game_deck.deal());

        show_some(player_hand, dealer_hand);

        bool player_turn = true;

        while (player_turn) {
             if (player_hand.value == 21) {
                cout << "Player has Blackjack!" << endl;
                player_turn = false;
                break;
             }

            player_turn = hit_or_stand(game_deck, player_hand);

            if (player_turn) {
                 show_some(player_hand, dealer_hand);
            }

            if (player_hand.value > 21) {
                show_some(player_hand, dealer_hand);
                player_busts();
                player.lose_bet();
                player_turn = false;
            }
        }


        if (player_hand.value <= 21) {
            while (dealer_hand.value < 17) {
                cout << "Dealer hits..." << endl;
                hit(game_deck, dealer_hand);
            }

            show_all(player_hand, dealer_hand);

            if (dealer_hand.value > 21) {
                dealer_busts();
                player.win_bet();
            } else if (dealer_hand.value > player_hand.value) {
                dealer_wins();
                player.lose_bet();
            } else if (dealer_hand.value < player_hand.value) {
                player_wins();
                player.win_bet();
            } else {
                push();
            }
        }

        cout << "\nYour current balance is: " << player.balance << endl;

        while(true) {
            cout << "Would you like to play another hand? Enter 'y' or 'n': ";
            string new_game_choice;
            cin >> new_game_choice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (!new_game_choice.empty()) {
                char lower_choice = tolower(new_game_choice[0]);
                if (lower_choice == 'y') {
                   break;
                } else if (lower_choice == 'n') {
                    cout << "Thanks for playing!" << endl;
                    return 0;
                }
            }
            cout << "Invalid input. Please enter 'y' or 'n'." << endl;
        }
    }

    return 0;
}