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

using namespace std;


const vector<string> suits = {"♥", "♦", "♠", "♣"};
const vector<string> ranks = {"Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King", "Ace"};
const map<string, int> values = {
    {"Two", 2}, {"Three", 3}, {"Four", 4}, {"Five", 5}, {"Six", 6}, {"Seven", 7}, {"Eight", 8},
    {"Nine", 9}, {"Ten", 10}, {"Jack", 10}, {"Queen", 10}, {"King", 10}, {"Ace", 11}
};

class Card {
public:
    string suit;
    string rank;
    Card(string s, string r) : suit(s), rank(r) {}
    friend ostream& operator<<(ostream& os, const Card& card) {
        os << card.rank << " of " << card.suit;
        return os;
    }
    int getValue() const {
        try { return values.at(rank); } catch (const out_of_range&) { return 0; }
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
        if (card.rank == "Ace") aces++;
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
        for (const auto& s : suits) for (const auto& r : ranks) deck.emplace_back(s, r);
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
    void win_bet() { balance += bet; } // For 1:1 games
    void lose_bet() { balance -= bet; } // For 1:1 games or simple loss
    bool place_bet(int amount) {
        if (amount > 0 && amount <= balance) { bet = amount; return true; }
        if (amount > balance) cout << "Bet exceeds balance." << endl;
        if (amount <= 0) cout << "Bet must be positive." << endl;
        return false;
    }
};

// --- Utility Functions ---

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
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (cin.peek() == '\n') cin.ignore();
}

// --- Game Implementations ---

void hit_blackjack(Deck& deck, Hand& hand) { hand.add_card(deck.deal()); }
bool hit_or_stand_blackjack(Deck& deck, Hand& hand) { /* ... */ return false; }
void show_some_blackjack(const Hand& player_hand, const Hand& dealer_hand) { /* ... */ }
void show_all_blackjack(const Hand& player_hand, const Hand& dealer_hand) { /* ... */ }
void player_busts_blackjack() { cout << "Player busts!" << endl; }
void player_wins_blackjack() { cout << "Player wins!" << endl; }
void dealer_busts_blackjack() { cout << "Dealer busts!" << endl; }
void dealer_wins_blackjack() { cout << "Dealer wins!" << endl; }
void push_blackjack() { cout << "Dealer and Player tie! It's a push." << endl; }


void play_blackjack(Player& player) {
    clear_screen(); cout << "--- Welcome to Blackjack ---" << endl;

    if (player.balance <= 0) { cout << "Insufficient funds." << endl; press_enter_to_continue(); return; }

    if (!take_bet_generic(player, "Blackjack")) return;
    
    Deck game_deck; game_deck.shuffle(); Hand player_hand, dealer_hand;
    try { /* Deal cards */ player_hand.add_card(game_deck.deal()); dealer_hand.add_card(game_deck.deal()); player_hand.add_card(game_deck.deal()); dealer_hand.add_card(game_deck.deal()); } catch(...) { /* Handle error */ return; }
    show_some_blackjack(player_hand, dealer_hand);
    bool player_turn = true, player_busted = false, player_blackjack = (player_hand.value == 21);
    if (player_blackjack) { cout << "\nPlayer Blackjack!" << endl; player_turn = false; }
    while (player_turn) { 
        string choice_str; cout << "Hit or Stand? (h/s): "; getline(cin, choice_str);
        if (!choice_str.empty()){
            char lower_choice = tolower(choice_str[0]);
            if (lower_choice == 'h'){ hit_blackjack(game_deck, player_hand); show_some_blackjack(player_hand, dealer_hand); }
            else if(lower_choice == 's'){ player_turn = false; cout << "Player stands." << endl;}
            else { cout << "Invalid choice." << endl; continue;}
        } else { cout << "Invalid choice." << endl; continue; }

        if (player_hand.value > 21) { player_busts_blackjack(); player.lose_bet(); player_busted = true; player_turn = false;}
    }
    if (!player_busted) { 
        cout << "\n--- Dealer's Turn ---" << endl;
        while (dealer_hand.value < 17) { try { hit_blackjack(game_deck, dealer_hand); } catch (...) { break;} }
        show_all_blackjack(player_hand, dealer_hand);
        bool dealer_blackjack = (dealer_hand.cards.size() == 2 && dealer_hand.value == 21);
        if (dealer_hand.value > 21) { dealer_busts_blackjack(); if (!(player_blackjack && dealer_blackjack)) player.win_bet(); }
        else if (player_blackjack && !dealer_blackjack) { player_wins_blackjack(); player.win_bet(); }
        else if (dealer_blackjack && !player_blackjack) { dealer_wins_blackjack(); player.lose_bet(); }
        else if (dealer_hand.value > player_hand.value) { dealer_wins_blackjack(); player.lose_bet(); }
        else if (dealer_hand.value < player_hand.value) { player_wins_blackjack(); player.win_bet(); }
        else { push_blackjack(); }
    }
    cout << "\nRound over. Balance: " << player.balance << endl; press_enter_to_continue();
}

void play_high_low(Player& player) {
    clear_screen(); cout << "--- Welcome to High/Low ---" << endl;
    if (player.balance <= 0) { cout << "Insufficient funds." << endl; press_enter_to_continue(); return; }
    if (!take_bet_generic(player, "High/Low")) return;
    Deck game_deck; game_deck.shuffle();
    if (game_deck.size() < 2) { cout << "Not enough cards." << endl; press_enter_to_continue(); return; }
    Card first_card = game_deck.deal(); cout << "\nFirst card: " << first_card << endl;
    char guess = ' '; while (guess != 'h' && guess != 'l') { /* Get guess */ cout << "Higher (h) or Lower (l)? "; string gs; getline(cin, gs); if(!gs.empty()) guess=tolower(gs[0]); else cout << "Invalid." << endl;}
    Card second_card = game_deck.deal(); cout << "Next card: " << second_card << endl;
    int v1 = first_card.getValue(), v2 = second_card.getValue();
    bool correct = (v2 > v1 && guess == 'h') || (v2 < v1 && guess == 'l');
    if (v1 == v2) { cout << "\nTie! You lose." << endl; player.lose_bet(); }
    else if (correct) { cout << "\nCorrect!" << endl; player.win_bet(); }
    else { cout << "\nIncorrect." << endl; player.lose_bet(); }
    cout << "\nRound over. Balance: " << player.balance << endl; press_enter_to_continue();
}

void play_slots(Player& player) {
    clear_screen(); cout << "--- Welcome to Simple Slots ---" << endl;
    const vector<string> symbols = {"🍒", "🍋", "🍊", "🔔", "BAR", " 7 "};
    const map<string, int> payouts = {{"🍒", 2}, {"🍋", 3}, {"🍊", 5}, {"🔔", 10}, {"BAR", 20}, {" 7 ", 50}};
    static mt19937 rng(chrono::system_clock::now().time_since_epoch().count());
    uniform_int_distribution<int> dist(0, symbols.size() - 1);

    while (true) { 
        clear_screen(); cout << "--- Simple Slots ---" << endl;
        if (player.balance <= 0) { cout << "Insufficient funds." << endl; press_enter_to_continue(); return; }
        cout << "Current Balance: " << player.balance << endl;
        if (!take_bet_generic(player, "Simple Slots")) return;

        while (true) { 
            clear_screen(); cout << "--- Simple Slots ---" << endl;
            cout << "Balance: " << player.balance << " | Bet: " << player.bet << endl;
            if (player.balance < player.bet) { cout << "\nInsufficient balance for bet." << endl; this_thread::sleep_for(chrono::seconds(2)); break; }

            vector<string> result; cout << "\nSpinning..." << endl; this_thread::sleep_for(chrono::milliseconds(700));
            cout << "[ "; for (int i = 0; i < 3; ++i) { result.push_back(symbols[dist(rng)]); cout << result[i] << (i < 2 ? " | " : ""); } cout << " ]" << endl << endl;

            bool win = false;
            if (result[0] == result[1] && result[1] == result[2]) {
                string sym = result[0]; int multiplier = 0;
                try { multiplier = payouts.at(sym); win = true; int winnings = player.bet * multiplier; int profit = winnings - player.bet; player.balance += profit; cout << "!!! JACKPOT !!! x" << multiplier << " Win: " << winnings << endl; }
                catch (...) { cout << "Payout Error. Lose." << endl; player.balance -= player.bet; }
            }
            if (!win) { cout << "Sorry, lose." << endl; player.balance -= player.bet; }
            cout << "Balance after spin: " << player.balance << endl;
            if (player.balance <= 0) { cout << "\nOut of money!" << endl; press_enter_to_continue(); return; }

            cout << "\nOptions: [Enter]/[s] Spin | [c] Change bet | [q] Quit: "; string action; getline(cin, action);
            char cmd = action.empty() ? 's' : tolower(action[0]);
            if (cmd == 'q') return;
            if (cmd == 'c') break;
            if (cmd == 's') continue;
            cout << "Invalid. Spinning again..." << endl; this_thread::sleep_for(chrono::seconds(1)); continue;
        }
    }
}



const vector<string> symbols_3x3 = {" 🍒  ", " 🍋 ", " 🍊 ", " 🔔  ", " ⛔" ," ♿", "⭐" } ; 
const string WILD_SYMBOL = "⭐";
const map<string, int> payouts_3x3 = {
    {" 🍒  ", 2}, {" 🍋 ", 3}, {" 🍊 ", 5}, {" 🔔  ", 7}, {" ⛔", 10}, {" ♿", 15}, {WILD_SYMBOL, 25} 
};


const vector<vector<pair<int, int>>> win_lines_3x3 = {
    {{0, 0}, {0, 1}, {0, 2}}, // Top Horizontal
    {{1, 0}, {1, 1}, {1, 2}}, // Middle Horizontal
    {{2, 0}, {2, 1}, {2, 2}}, // Bottom Horizontal
    {{0, 0}, {1, 1}, {2, 2}}, // Diagonal TL-BR
    {{0, 2}, {1, 1}, {2, 0}}  // Diagonal TR-BL
};

int check_line(const vector<vector<string>>& grid,
               const vector<pair<int, int>>& line_coords,
               const map<string, int>& payouts)
{
    string s1 = grid[line_coords[0].first][line_coords[0].second];
    string s2 = grid[line_coords[1].first][line_coords[1].second];
    string s3 = grid[line_coords[2].first][line_coords[2].second];

    if (s1 == s2 && s2 == s3) {
        try { return payouts.at(s1); } catch(...) { return 0; } 
    }

    string effective_symbol = "";
    int wild_count = 0;
    int non_wild_count = 0;

    if (s1 == WILD_SYMBOL) wild_count++; else { effective_symbol = s1; non_wild_count++; }
    if (s2 == WILD_SYMBOL) wild_count++; else { if (effective_symbol == "" || effective_symbol == s2) {effective_symbol = s2; non_wild_count++;} else non_wild_count = 10; }
    if (s3 == WILD_SYMBOL) wild_count++; else { if (effective_symbol == "" || effective_symbol == s3) {effective_symbol = s3; non_wild_count++;} else non_wild_count = 10; } 
    if (wild_count > 0 && wild_count + non_wild_count == 3 && non_wild_count >= 1 && effective_symbol != "") {
        try { return payouts.at(effective_symbol); } catch(...) { return 0; } 
    }

    return 0;
}


void play_slots_3x3(Player& player) {
    clear_screen();
    cout << "--- Welcome to 3x3 Slots ---" << endl;

    static mt19937 rng(chrono::system_clock::now().time_since_epoch().count() + 1); 
    uniform_int_distribution<int> dist(0, symbols_3x3.size() - 1);

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
                break; // Break inner loop to go back to taking bet
            }

            // --- Spin Logic ---
            vector<vector<string>> grid(3, vector<string>(3));
            cout << "\nSpinning..." << endl;
            this_thread::sleep_for(chrono::milliseconds(700));

            const int cell_width = 6;
            string h_separator = "+";
            for (int i = 0; i < 3; ++i) {
                h_separator += string(cell_width + 2, '-') + "+"; 
            }

            cout << h_separator << endl;
            for (int r = 0; r < 3; ++r) {
                cout << "| "; // Left edge
                for (int c = 0; c < 3; ++c) {
                    grid[r][c] = symbols_3x3[dist(rng)];
                    cout << "  " << left << setw(cell_width) << grid[r][c] << "  |";
                }
                cout << endl;
                cout << h_separator << endl;
            }
            cout << endl;

            int total_payout_multiplier = 0;
            vector<int> winning_lines_indices;

            for(int i = 0; i < win_lines_3x3.size(); ++i) {
                int line_payout = check_line(grid, win_lines_3x3[i], payouts_3x3);
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
                cout << "Total Payout: " << total_winnings << " (Net Gain: " << net_change << ")" << endl;
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

            // --- User Action Prompt ---
            cout << "\nOptions: [Enter] or [s] to spin again | [c] Change bet | [q] Quit slots: ";
            string action;
            getline(cin, action);

            char command = ' ';
            if (!action.empty()) command = tolower(action[0]);
            else command = 's';

            if (command == 'q') return;
            if (command == 'c') break; 
            if (command == 's') continue; 
            else {
                cout << "Invalid command. Spinning again..." << endl;
                this_thread::sleep_for(chrono::seconds(1));
                continue;
            }
        }
    } 
}

int main() {
    Player player(1000);
    cin.sync();

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
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input." << endl; this_thread::sleep_for(chrono::seconds(1)); continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
            case 1: play_blackjack(player); break;
            case 2: play_high_low(player); break;
            case 3: play_slots(player); break; 
            case 4: play_slots_3x3(player); break; 
            case 5: cout << "\nCurrent balance: " << player.balance << endl; press_enter_to_continue(); break;
            case 6: cout << "\nThanks for playing! Final balance: " << player.balance << endl; return 0;
            default: cout << "Invalid choice." << endl; this_thread::sleep_for(chrono::seconds(1)); break;
        }
    }
    return 0;
}
