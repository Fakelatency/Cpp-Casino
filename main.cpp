#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <map>
#include <limits>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <iomanip>
#include <utility>
#include <memory>

using namespace std;

enum class Suit { HEARTS, DIAMONDS, SPADES, CLUBS };
enum class Rank { TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE };

const vector<Suit> ALL_SUITS = {Suit::HEARTS, Suit::DIAMONDS, Suit::SPADES, Suit::CLUBS};
string suitToString(Suit s) {
    switch (s) {
        case Suit::HEARTS: return "♥";
        case Suit::DIAMONDS: return "♦";
        case Suit::SPADES: return "♠";
        case Suit::CLUBS: return "♣";
        default: return "?";
    }
}

const vector<Rank> ALL_RANKS = {
    Rank::TWO, Rank::THREE, Rank::FOUR, Rank::FIVE, Rank::SIX, Rank::SEVEN,
    Rank::EIGHT, Rank::NINE, Rank::TEN, Rank::JACK, Rank::QUEEN, Rank::KING, Rank::ACE
};
string rankToString(Rank r) {
    switch (r) {
        case Rank::TWO: return "Two";
        case Rank::THREE: return "Three";
        case Rank::FOUR: return "Four";
        case Rank::FIVE: return "Five";
        case Rank::SIX: return "Six";
        case Rank::SEVEN: return "Seven";
        case Rank::EIGHT: return "Eight";
        case Rank::NINE: return "Nine";
        case Rank::TEN: return "Ten";
        case Rank::JACK: return "Jack";
        case Rank::QUEEN: return "Queen";
        case Rank::KING: return "King";
        case Rank::ACE: return "Ace";
        default: return "Unknown Rank";
    }
}

const map<Rank, int> card_values = {
    {Rank::TWO, 2}, {Rank::THREE, 3}, {Rank::FOUR, 4}, {Rank::FIVE, 5},
    {Rank::SIX, 6}, {Rank::SEVEN, 7}, {Rank::EIGHT, 8}, {Rank::NINE, 9},
    {Rank::TEN, 10}, {Rank::JACK, 10}, {Rank::QUEEN, 10}, {Rank::KING, 10},
    {Rank::ACE, 11}
};

class Card {
public:
    Suit suit;
    Rank rank;
    Card(Suit s, Rank r) : suit(s), rank(r) {}
    friend ostream& operator<<(ostream& os, const Card& card) {
        os << rankToString(card.rank) << " of " << suitToString(card.suit);
        return os;
    }
    int getValue() const {
        try {
            return card_values.at(rank);
        } catch (const out_of_range&) {
            return 0;
        }
    }
};

class Hand {
public:
    vector<Card> cards;
    int value = 0;
    int aces = 0;
    void add_card(const Card& card) {
        cards.push_back(card);
        value += card.getValue();
        if (card.rank == Rank::ACE) aces++;
        adjust_for_ace();
    }
    void adjust_for_ace() {
        while (value > 21 && aces > 0) { value -= 10; aces--; }
    }
    void clear() { cards.clear(); value = 0; aces = 0; }
};

class Deck {
private:
    mt19937 rng;
public:
    vector<Card> deck;
    Deck() {
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        rng = mt19937(seed);
        for (Suit s : ALL_SUITS) {
            for (Rank r : ALL_RANKS) {
                deck.emplace_back(s, r);
            }
        }
    }
    void shuffle() { std::shuffle(deck.begin(), deck.end(), rng); }
    Card deal() {
        if (deck.empty()) throw runtime_error("Dealing empty deck");
        Card c = deck.back(); deck.pop_back(); return c;
    }
    size_t size() const { return deck.size(); }
};

class Player {
public:
    int balance;
    int bet;
    Player(int initial_balance = 1000) : balance(initial_balance), bet(0) {}
    void win_bet() { balance += bet; }
    void apply_blackjack_win() { balance += static_cast<int>(bet * 1.5); }
    void lose_bet() { balance -= bet; }
    bool place_bet(int amount) {
        if (amount > 0 && amount <= balance) { bet = amount; return true; }
        if (amount > balance) cout << "Bet (" << amount << ") exceeds balance (" << balance << ")." << endl;
        if (amount <= 0) cout << "Bet must be positive." << endl;
        return false;
    }
};

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

bool take_bet_generic(Player& player, const string& game_name) {
    while (true) {
        cout << "Your balance: " << player.balance << ". Enter bet for " << game_name << " (or 0 to go back): ";
        int bet_amount;
        cin >> bet_amount;

        if (cin.fail()) {
            cout << "Invalid input. Please enter a number." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (bet_amount == 0) return false;
        if (player.place_bet(bet_amount)) return true;
    }
}

void press_enter_to_continue() {
    cout << "\nPress Enter to continue...";
    string dummy;
    getline(cin, dummy);
    if (cin.peek() == '\n' && cin.eof()) cin.clear(); // Handle edge case for empty buffer before EOF
    else if (cin.peek() == '\n') cin.ignore();

}

class Game {
public:
    virtual ~Game() = default;
    virtual void play(Player& player) = 0;
protected:
    void display_welcome_message(const string& game_name) {
        clear_screen();
        cout << "--- Welcome to " << game_name << " ---" << endl;
    }
};

class BlackjackGame : public Game {
public:
    void play(Player& player) override {
        display_welcome_message("Blackjack");

        if (player.balance <= 0) {
            cout << "Insufficient funds." << endl;
            press_enter_to_continue();
            return;
        }

        if (!take_bet_generic(player, "Blackjack")) {
            return;
        }

        Deck game_deck;
        game_deck.shuffle();
        Hand player_hand, dealer_hand;

        try {
            player_hand.add_card(game_deck.deal());
            dealer_hand.add_card(game_deck.deal());
            player_hand.add_card(game_deck.deal());
            dealer_hand.add_card(game_deck.deal());
        } catch (const runtime_error& e) {
            cout << "Error dealing cards: " << e.what() << endl;
            press_enter_to_continue();
            return;
        }

        show_some(player_hand, dealer_hand);

        bool player_turn_active = true;
        bool player_busted = false;
        bool player_has_blackjack = (player_hand.value == 21 && player_hand.cards.size() == 2);
        bool dealer_has_blackjack = (dealer_hand.value == 21 && dealer_hand.cards.size() == 2);

        if (player_has_blackjack) {
            cout << "\nPlayer Blackjack!" << endl;
            show_all(player_hand, dealer_hand);
            if (dealer_has_blackjack) {
                cout << "Dealer also has Blackjack! It's a push." << endl;
            } else {
                cout << "Player wins with Blackjack (pays 3:2)!" << endl;
                player.apply_blackjack_win();
            }
            player_turn_active = false;
        }


        while (player_turn_active) {
            string choice_str;
            cout << "Hit or Stand? (h/s): ";
            getline(cin, choice_str);
            if (!choice_str.empty()) {
                char lower_choice = tolower(choice_str[0]);
                if (lower_choice == 'h') {
                    player_hand.add_card(game_deck.deal());
                    show_some(player_hand, dealer_hand);
                } else if (lower_choice == 's') {
                    player_turn_active = false;
                    cout << "Player stands with " << player_hand.value << "." << endl;
                } else {
                    cout << "Invalid choice. Please enter 'h' or 's'." << endl;
                    continue;
                }
            } else {
                cout << "Invalid choice. Please enter 'h' or 's'." << endl;
                continue;
            }

            if (player_hand.value > 21) {
                cout << "Player busts with " << player_hand.value << "!" << endl;
                player.lose_bet();
                player_busted = true;
                player_turn_active = false;
            } else if (player_hand.value == 21) {
                cout << "Player has 21!" << endl;
                player_turn_active = false;
            }
        }

        if (!player_busted && !player_has_blackjack) {
            cout << "\n--- Dealer's Turn ---" << endl;
            show_all(player_hand, dealer_hand);
             this_thread::sleep_for(chrono::milliseconds(500));


            if (dealer_has_blackjack && !player_has_blackjack) {
                 cout << "Dealer has Blackjack! Dealer wins." << endl;
                 player.lose_bet();
            } else {
                while (dealer_hand.value < 17) {
                    cout << "Dealer hits." << endl;
                    this_thread::sleep_for(chrono::seconds(1));
                    try {
                        dealer_hand.add_card(game_deck.deal());
                        show_all(player_hand, dealer_hand);
                    } catch (const runtime_error& e) {
                        cout << "Error during dealer's turn: " << e.what() << endl;
                        break;
                    }
                    if (dealer_hand.value > 21) break;
                }

                if (dealer_hand.value > 21) {
                    cout << "Dealer busts with " << dealer_hand.value << "! Player wins." << endl;
                    player.win_bet();
                } else {
                    cout << "Dealer stands with " << dealer_hand.value << "." << endl;
                    if (dealer_hand.value > player_hand.value) {
                        cout << "Dealer wins." << endl;
                        player.lose_bet();
                    } else if (dealer_hand.value < player_hand.value) {
                        cout << "Player wins." << endl;
                        player.win_bet();
                    } else {
                        cout << "It's a push! Bets are returned." << endl;
                    }
                }
            }
        }
        cout << "\nRound over. Your balance: " << player.balance << endl;
        press_enter_to_continue();
    }

private:
    void show_some(const Hand& p_hand, const Hand& d_hand) {
        cout << "\n--- Current Hands ---" << endl;
        cout << "Player's Hand: ";
        for (const auto& card : p_hand.cards) cout << card << " ";
        cout << "(Value: " << p_hand.value << ")" << endl;
        if (!d_hand.cards.empty()) {
            cout << "Dealer's Showing: " << d_hand.cards[0] << " [Hidden Card]" << endl;
        }
    }
    void show_all(const Hand& p_hand, const Hand& d_hand) {
        cout << "\n--- Final Hands ---" << endl;
        cout << "Player's Hand: ";
        for (const auto& card : p_hand.cards) cout << card << " ";
        cout << "(Value: " << p_hand.value << ")" << endl;
        cout << "Dealer's Hand: ";
        for (const auto& card : d_hand.cards) cout << card << " ";
        cout << "(Value: " << d_hand.value << ")" << endl;
    }
};

class HighLowGame : public Game {
public:
    void play(Player& player) override {
        display_welcome_message("High/Low");
        if (player.balance <= 0) {
            cout << "Insufficient funds." << endl;
            press_enter_to_continue();
            return;
        }
        if (!take_bet_generic(player, "High/Low")) return;

        Deck game_deck;
        game_deck.shuffle();
        if (game_deck.size() < 2) {
            cout << "Not enough cards to play High/Low." << endl;
            press_enter_to_continue();
            return;
        }
        Card first_card = game_deck.deal();
        cout << "\nFirst card: " << first_card << " (Value: " << first_card.getValue() << ")" << endl;
        char guess = ' ';
        while (guess != 'h' && guess != 'l') {
            cout << "Will the next card be Higher (h) or Lower (l)? ";
            string gs;
            getline(cin, gs);
            if (!gs.empty()) guess = tolower(gs[0]);
            else cout << "Invalid input. Please enter 'h' or 'l'." << endl;
        }
        Card second_card = game_deck.deal();
        cout << "Next card: " << second_card << " (Value: " << second_card.getValue() << ")" << endl;
        int v1 = first_card.getValue();
        int v2 = second_card.getValue();

        if (v1 == v2) {
            cout << "\nIt's a tie! Values are the same. You lose your bet." << endl;
            player.lose_bet();
        } else {
            bool correct = (v2 > v1 && guess == 'h') || (v2 < v1 && guess == 'l');
            if (correct) {
                cout << "\nCorrect!" << endl;
                player.win_bet();
            } else {
                cout << "\nIncorrect." << endl;
                player.lose_bet();
            }
        }
        cout << "\nRound over. Your balance: " << player.balance << endl;
        press_enter_to_continue();
    }
};

class SlotsGame : public Game {
private:
    mt19937 rng;
    const vector<string> symbols = {"🍒", "🍋", "🍊", "🔔", "BAR", " 7 "};
    const map<string, int> payouts = {{"🍒", 2}, {"🍋", 3}, {"🍊", 4}, {"🔔", 5}, {"BAR", 10}, {" 7 ", 20}};
public:
    SlotsGame() : rng(chrono::system_clock::now().time_since_epoch().count()) {}
    void play(Player& player) override {
        while (true) {
            clear_screen();
            cout << "--- Simple Slots ---" << endl;
            if (player.balance <= 0) {
                cout << "Insufficient funds to play Simple Slots." << endl;
                press_enter_to_continue();
                return;
            }
            cout << "Current Balance: " << player.balance << endl;
            if (!take_bet_generic(player, "Simple Slots")) {
                return;
            }

            while (true) {
                clear_screen();
                cout << "--- Simple Slots ---" << endl;
                cout << "Balance: " << player.balance << " | Bet: " << player.bet << endl;

                if (player.balance < player.bet) {
                    cout << "\nInsufficient balance for the current bet of " << player.bet << "." << endl;
                    cout << "Please change your bet." << endl;
                    this_thread::sleep_for(chrono::seconds(2));
                    break;
                }

                uniform_int_distribution<int> dist(0, symbols.size() - 1);
                vector<string> result;
                cout << "\nSpinning..." << endl;
                this_thread::sleep_for(chrono::milliseconds(700));
                cout << "[ ";
                for (int i = 0; i < 3; ++i) {
                    result.push_back(symbols[dist(rng)]);
                    cout << result[i] << (i < 2 ? " | " : "");
                }
                cout << " ]" << endl << endl;

                bool win_this_spin = false;
                if (result[0] == result[1] && result[1] == result[2]) {
                    string sym = result[0];
                    int multiplier = 0;
                    try {
                        multiplier = payouts.at(sym);
                        win_this_spin = true;
                        int winnings = player.bet * multiplier;
                        int profit = winnings - player.bet;
                        player.balance += profit;

                        cout << "!!! JACKPOT !!! You matched three " << sym << " symbols!" << endl;
                        cout << "Payout Multiplier: x" << multiplier << endl;
                        cout << "You win: " << winnings << " (Profit: " << profit << ")" << endl;
                    } catch (const out_of_range&) {
                        cout << "Payout Error. This spin is a loss." << endl;
                        player.balance -= player.bet;
                    }
                }

                if (!win_this_spin) {
                    cout << "Sorry, no win this spin." << endl;
                    player.balance -= player.bet;
                }
                cout << "Balance after spin: " << player.balance << endl;

                if (player.balance <= 0) {
                    cout << "\nYou've run out of money!" << endl;
                    press_enter_to_continue();
                    return;
                }

                cout << "\nOptions: [Enter]/[s] Spin again | [c] Change bet | [q] Quit Simple Slots: ";
                string action_str;
                getline(cin, action_str);
                char cmd = action_str.empty() ? 's' : tolower(action_str[0]);

                if (cmd == 'q') return;
                if (cmd == 'c') break;
                if (cmd == 's') continue;
                
                cout << "Invalid command. Spinning again..." << endl;
                this_thread::sleep_for(chrono::seconds(1));
            }
        }
    }
};

const vector<string> Slots3x3Game_symbols_3x3_data = {" 🍒  ", " 🍋 ", " 🍊 ", " 🔔  ", " ⛔" ," ♿", "⭐"};
const string Slots3x3Game_WILD_SYMBOL_DATA = "⭐";
const map<string, int> Slots3x3Game_payouts_3x3_data = {
    {" 🍒  ", 2}, {" 🍋 ", 3}, {" 🍊 ", 5}, {" 🔔  ", 7}, {" ⛔", 10}, {" ♿", 15}, {Slots3x3Game_WILD_SYMBOL_DATA, 25}
};
const vector<vector<pair<int, int>>> Slots3x3Game_win_lines_3x3_data = {
    {{0, 0}, {0, 1}, {0, 2}}, {{1, 0}, {1, 1}, {1, 2}}, {{2, 0}, {2, 1}, {2, 2}},
    {{0, 0}, {1, 1}, {2, 2}}, {{0, 2}, {1, 1}, {2, 0}}
};

class Slots3x3Game : public Game {
private:
    mt19937 rng;
    static int check_line_static(const vector<vector<string>>& grid,
                          const vector<pair<int, int>>& line_coords) {
        string s1 = grid[line_coords[0].first][line_coords[0].second];
        string s2 = grid[line_coords[1].first][line_coords[1].second];
        string s3 = grid[line_coords[2].first][line_coords[2].second];

        if (s1 == s2 && s2 == s3 && s1 != Slots3x3Game_WILD_SYMBOL_DATA) {
            try { return Slots3x3Game_payouts_3x3_data.at(s1); } catch(...) { return 0; }
        }
        if (s1 == Slots3x3Game_WILD_SYMBOL_DATA && s2 == Slots3x3Game_WILD_SYMBOL_DATA && s3 == Slots3x3Game_WILD_SYMBOL_DATA) {
            try { return Slots3x3Game_payouts_3x3_data.at(Slots3x3Game_WILD_SYMBOL_DATA); } catch(...) { return 0; }
        }

        string effective_symbol = "";
        int wild_count = 0;
        vector<string> line_symbols = {s1, s2, s3};
        vector<string> non_wild_symbols;

        for(const auto& sym : line_symbols) {
            if (sym == Slots3x3Game_WILD_SYMBOL_DATA) {
                wild_count++;
            } else {
                non_wild_symbols.push_back(sym);
            }
        }

        if (wild_count > 0 && wild_count < 3) {
            if (non_wild_symbols.empty()) return 0;
            bool all_same_non_wild = true;
            if (non_wild_symbols.size() > 1) {
                for (size_t i = 1; i < non_wild_symbols.size(); ++i) {
                    if (non_wild_symbols[i] != non_wild_symbols[0]) {
                        all_same_non_wild = false;
                        break;
                    }
                }
            }
            if (all_same_non_wild) {
                effective_symbol = non_wild_symbols[0];
                try { return Slots3x3Game_payouts_3x3_data.at(effective_symbol); } catch(...) { return 0; }
            }
        }
        return 0;
    }

public:
    Slots3x3Game() : rng(chrono::system_clock::now().time_since_epoch().count() + 1) {}
    void play(Player& player) override {
         while (true) {
            clear_screen();
            cout << "--- 3x3 Slots ---" << endl;
            if (player.balance <= 0) {
                cout << "You have no money left to play!" << endl;
                press_enter_to_continue();
                return;
            }
            cout << "Current Balance: " << player.balance << endl;
            if (!take_bet_generic(player, "3x3 Slots")) {
                return;
            }

            while (true) {
                clear_screen();
                cout << "--- 3x3 Slots ---" << endl;
                cout << "Balance: " << player.balance << " | Bet: " << player.bet << endl;

                if (player.balance < player.bet) {
                    cout << "\nNot enough balance for the current bet of " << player.bet << "." << endl;
                    cout << "Please change your bet." << endl;
                    this_thread::sleep_for(chrono::seconds(2));
                    break;
                }

                uniform_int_distribution<int> dist(0, Slots3x3Game_symbols_3x3_data.size() - 1);
                vector<vector<string>> grid(3, vector<string>(3));
                cout << "\nSpinning..." << endl;
                this_thread::sleep_for(chrono::milliseconds(700));

                const int cell_width = 6;
                string h_separator = "+";
                for (int i = 0; i < 3; ++i) h_separator += string(cell_width + 2, '-') + "+";

                cout << h_separator << endl;
                for (int r = 0; r < 3; ++r) {
                    cout << "|";
                    for (int c = 0; c < 3; ++c) {
                        grid[r][c] = Slots3x3Game_symbols_3x3_data[dist(rng)];
                        cout << " " << left << setw(cell_width) << grid[r][c] << " |";
                    }
                    cout << endl;
                    if (r < 2) cout << h_separator << endl;
                }
                 cout << h_separator << endl;
                 cout << endl;

                int total_payout_multiplier = 0;
                vector<int> winning_lines_indices;

                for(size_t i = 0; i < Slots3x3Game_win_lines_3x3_data.size(); ++i) {
                    int line_payout = check_line_static(grid, Slots3x3Game_win_lines_3x3_data[i]);
                    if (line_payout > 0) {
                        total_payout_multiplier += line_payout;
                        winning_lines_indices.push_back(i + 1);
                    }
                }

                if (total_payout_multiplier > 0) {
                    int total_winnings = player.bet * total_payout_multiplier;
                    int net_change = total_winnings - player.bet;
                    player.balance += net_change;

                    cout << "!!! WIN !!! on line(s): ";
                    for(size_t i = 0; i < winning_lines_indices.size(); ++i) {
                        cout << winning_lines_indices[i] << (i < winning_lines_indices.size() - 1 ? ", " : "");
                    }
                    cout << endl;
                    cout << "Total Payout Multiplier: x" << total_payout_multiplier << endl;
                    cout << "You win: " << total_winnings << " (Net Gain: " << net_change << ")" << endl;
                } else {
                    cout << "Sorry, no winning lines this spin." << endl;
                    player.balance -= player.bet;
                }
                cout << "Balance after spin: " << player.balance << endl;

                if (player.balance <= 0) {
                    cout << "\nYou've run out of money!" << endl;
                    press_enter_to_continue();
                    return;
                }

                cout << "\nOptions: [Enter]/[s] Spin again | [c] Change bet | [q] Quit 3x3 Slots: ";
                string action_str;
                getline(cin, action_str);
                char cmd = action_str.empty() ? 's' : tolower(action_str[0]);

                if (cmd == 'q') return;
                if (cmd == 'c') break;
                if (cmd == 's') continue;
                
                cout << "Invalid command. Spinning again..." << endl;
                this_thread::sleep_for(chrono::seconds(1));
            }
        }
    }
};

int main() {
    Player player(100000);

    map<int, unique_ptr<Game>> games;
    games[1] = make_unique<BlackjackGame>();
    games[2] = make_unique<HighLowGame>();
    games[3] = make_unique<SlotsGame>();
    games[4] = make_unique<Slots3x3Game>();

    while (true) {
        clear_screen();
        cout << "====== CASINO MAIN MENU ======" << endl;
        cout << "Player Balance: " << player.balance << endl;
        cout << "-----------------------------" << endl;
        cout << "1. Play Blackjack" << endl;
        cout << "2. Play High/Low" << endl;
        cout << "3. Play Simple Slots (1x3)" << endl;
        cout << "4. Play 3x3 Slots" << endl;
        cout << "5. View Balance" << endl;
        cout << "6. Quit" << endl;
        cout << "-----------------------------" << endl;
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number." << endl;
            this_thread::sleep_for(chrono::seconds(1));
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 6) {
            cout << "\nThanks for playing! Final balance: " << player.balance << endl;
            break;
        }
        if (choice == 5) {
            cout << "\nCurrent balance: " << player.balance << endl;
            press_enter_to_continue();
            continue;
        }

        auto game_it = games.find(choice);
        if (game_it != games.end()) {
            game_it->second->play(player);
        } else {
            cout << "Invalid choice. Please try again." << endl;
            this_thread::sleep_for(chrono::seconds(1));
        }
    }
    return 0;
}