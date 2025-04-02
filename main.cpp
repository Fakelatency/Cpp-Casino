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
         try {
            return values.at(rank);
         } catch (const out_of_range& oor) {
            cerr << "Error: Invalid rank encountered: " << rank << endl;
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
    size_t size() const {
        return deck.size();
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

     bool place_bet(int amount) {
        if (amount > 0 && amount <= balance) {
            bet = amount;
            return true;
        }
         if (amount > balance) {
             cout << "Bet exceeds current balance." << endl;
         }
         if (amount <= 0) {
             cout << "Bet must be positive." << endl;
         }
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


bool take_bet_slots(Player& player) {
    while (true) {
        cout << "Your balance: " << player.balance << ". Enter bet amount (or 0 to go back): ";
        int bet_amount;
        cin >> bet_amount;

        if (cin.fail()) {
            cout << "Invalid input. Please enter a number." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue; 
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n'); 

        if (bet_amount == 0) {
            return false; 
        } else if (bet_amount < 0) {
             cout << "Bet must be a positive amount." << endl;
        } else if (bet_amount > player.balance) {
            cout << "Sorry, your bet cannot exceed your balance of " << player.balance << endl;
        } else {
             player.bet = bet_amount; 
             return true; 
        }
    }
}


void press_enter_to_continue() {
    cout << "\nPress Enter to continue...";
    char c;
    while(cin.get(c) && c != '\n');
    if (cin.peek() == '\n') cin.ignore();
}


void hit_blackjack(Deck& deck, Hand& hand) {
    hand.add_card(deck.deal());
}

bool hit_or_stand_blackjack(Deck& deck, Hand& hand) {
    while (true) {
        cout << "Would you like to Hit or Stand? Enter h or s: ";
        string choice;
        getline(cin, choice);

        if (!choice.empty()) {
             char lower_choice = tolower(choice[0]);
             if (lower_choice == 'h') {
                 hit_blackjack(deck, hand);
                 return true;
             } else if (lower_choice == 's') {
                 cout << "Player stands. Dealer is playing." << endl;
                 return false;
             }
        }
        cout << "Sorry, please enter 'h' or 's'." << endl;
    }
}

void show_some_blackjack(const Hand& player_hand, const Hand& dealer_hand) {
    cout << "\nCurrent Balance: " << player_hand.value << endl; 
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

void show_all_blackjack(const Hand& player_hand, const Hand& dealer_hand) {
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

void player_busts_blackjack() { cout << "Player busts!" << endl; }
void player_wins_blackjack() { cout << "Player wins!" << endl; }
void dealer_busts_blackjack() { cout << "Dealer busts!" << endl; }
void dealer_wins_blackjack() { cout << "Dealer wins!" << endl; }
void push_blackjack() { cout << "Dealer and Player tie! It's a push." << endl; }


void play_blackjack(Player& player) {
    clear_screen();
    cout << "--- Welcome to Blackjack ---" << endl;

    if (player.balance <= 0) {
        cout << "You have no money left to play Blackjack!" << endl;
        press_enter_to_continue();
        return;
    }

     while (true) {
        cout << "Your balance: " << player.balance << ". How much would you like to bet? ";
        int bet_amount;
        cin >> bet_amount;

        if (cin.fail()) {
            cout << "Invalid input. Please enter a number." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } else if (bet_amount <= 0) {
             cout << "Bet must be a positive amount." << endl;
             cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } else if (bet_amount > player.balance) {
            cout << "Sorry, your bet cannot exceed your balance of " << player.balance << endl;
             cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } else {
             player.bet = bet_amount;
             cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        }
    }


    Deck game_deck;
    game_deck.shuffle();

    Hand player_hand;
    Hand dealer_hand;

     try {
        player_hand.add_card(game_deck.deal());
        dealer_hand.add_card(game_deck.deal());
        player_hand.add_card(game_deck.deal());
        dealer_hand.add_card(game_deck.deal());
     } catch (const runtime_error& e) {
         cerr << "Error dealing initial cards: " << e.what() << endl;
         return;
     }


    show_some_blackjack(player_hand, dealer_hand);

    bool player_turn = true;
    bool player_busted = false;
    bool player_blackjack = (player_hand.value == 21);

    if (player_blackjack) {
         cout << "\nPlayer has Blackjack!" << endl;
         player_turn = false;
    }

    while (player_turn) {
        player_turn = hit_or_stand_blackjack(game_deck, player_hand);

        if (player_turn) {
            show_some_blackjack(player_hand, dealer_hand);
        }

        if (player_hand.value > 21) {
            show_some_blackjack(player_hand, dealer_hand);
            player_busts_blackjack();
            player.lose_bet();
            player_turn = false;
            player_busted = true;
        }
    }

    if (!player_busted) {
        cout << "\n--- Dealer's Turn ---" << endl;
        cout << "Dealer reveals: " << dealer_hand.cards[0] << endl;
        cout << "Dealer's Hand: ";
         for(const auto& card : dealer_hand.cards) cout << card << " ";
         cout << "(" << dealer_hand.value << ")" << endl;


        while (dealer_hand.value < 17) {
            cout << "Dealer hits..." << endl;
            try {
                 hit_blackjack(game_deck, dealer_hand);
                 cout << "Dealer draws: " << dealer_hand.cards.back() << endl;
                 cout << "Dealer's Hand total: " << dealer_hand.value << endl;
            } catch (const runtime_error& e) {
                cerr << "Error during dealer hit: " << e.what() << endl;
                break;
            }
        }

        show_all_blackjack(player_hand, dealer_hand);

         bool dealer_blackjack = (dealer_hand.cards.size() == 2 && dealer_hand.value == 21);

         // Standard win/loss logic assuming 1:1 payout for win_bet/lose_bet
         if (dealer_hand.value > 21) {
             dealer_busts_blackjack();
             if (!(player_blackjack && dealer_blackjack)) { // Player wins unless push BJ
                  player.win_bet();
             }
         } else if (player_blackjack && !dealer_blackjack) {
              player_wins_blackjack(); // Already announced BJ, confirm win
              player.win_bet(); // Add 3:2 logic here if needed later
         } else if (dealer_blackjack && !player_blackjack) {
              dealer_wins_blackjack();
              player.lose_bet();
         } else if (dealer_hand.value > player_hand.value) {
            dealer_wins_blackjack();
            player.lose_bet();
        } else if (dealer_hand.value < player_hand.value) {
            player_wins_blackjack();
            player.win_bet();
        } else { // Tie
            push_blackjack(); 
        }
    }

    cout << "\nRound over. Your balance is now: " << player.balance << endl;
    press_enter_to_continue();
}

void play_high_low(Player& player) {
    clear_screen();
    cout << "--- Welcome to High/Low ---" << endl;
    cout << "(Guess if the next card is higher or lower than the first. Ace is high. Ties lose.)" << endl;

     if (player.balance <= 0) {
        cout << "You have no money left to play High/Low!" << endl;
        press_enter_to_continue();
        return;
    }

     // High/Low specific take_bet
     while (true) {
        cout << "Your balance: " << player.balance << ". How much would you like to bet? ";
        int bet_amount;
        cin >> bet_amount;

        if (cin.fail()) {
            cout << "Invalid input. Please enter a number." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } else if (bet_amount <= 0) {
             cout << "Bet must be a positive amount." << endl;
             cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } else if (bet_amount > player.balance) {
            cout << "Sorry, your bet cannot exceed your balance of " << player.balance << endl;
             cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } else {
             player.bet = bet_amount;
             cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        }
    }


    Deck game_deck;
    game_deck.shuffle();

    if (game_deck.size() < 2) {
        cout << "Not enough cards in the deck to play!" << endl;
         press_enter_to_continue();
        return;
    }

    Card first_card = game_deck.deal();
    cout << "\nFirst card is: " << first_card << " (Value: " << first_card.getValue() << ")" << endl;

    char guess = ' ';
    while (guess != 'h' && guess != 'l') {
        cout << "Will the next card be higher (h) or lower (l)? ";
        string input;
        getline(cin, input); 
        if (!input.empty()) {
            guess = tolower(input[0]);
        }
        if (guess != 'h' && guess != 'l') {
            cout << "Invalid input. Please enter 'h' or 'l'." << endl;
        }
    }

    Card second_card = game_deck.deal();
    cout << "Next card is: " << second_card << " (Value: " << second_card.getValue() << ")" << endl;

    int first_val = first_card.getValue();
    int second_val = second_card.getValue();
    bool correct_guess = false;

    if (second_val > first_val && guess == 'h') {
        correct_guess = true;
    } else if (second_val < first_val && guess == 'l') {
        correct_guess = true;
    }

    if (correct_guess) {
        cout << "\nCorrect!" << endl;
        player.win_bet();
    } else if (second_val == first_val) {
        cout << "\nIt's a tie! You lose." << endl;
        player.lose_bet();
    }
     else {
        cout << "\nIncorrect." << endl;
        player.lose_bet();
    }

    cout << "\nRound over. Your balance is now: " << player.balance << endl;
    press_enter_to_continue();
}


void play_slots(Player& player) {
    clear_screen();
    cout << "--- Welcome to Slots ---" << endl;

    const vector<string> symbols = {"🍒", "🍋", "🍊", "🔔", "BAR", " 7 "};
    const map<string, int> payouts = {
        {"🍒", 2}, {"🍋", 3}, {"🍊", 5}, {"🔔", 10}, {"BAR", 20}, {" 7 ", 50}
    };
    static mt19937 rng(chrono::system_clock::now().time_since_epoch().count());
    uniform_int_distribution<int> dist(0, symbols.size() - 1);

    while (true) {
        clear_screen();
         cout << "--- Slots ---" << endl;
         if (player.balance <= 0) {
            cout << "You have no money left to play Slots!" << endl;
            press_enter_to_continue();
            return;
         }

        cout << "Current Balance: " << player.balance << endl;
        if (!take_bet_slots(player)) { 
             return; 
        }

        while (true) {
             clear_screen(); 
             cout << "--- Slots ---" << endl;
             cout << "Current Balance: " << player.balance << " | Current Bet: " << player.bet << endl;


             if (player.balance < player.bet) {
                 cout << "\nNot enough balance for the current bet of " << player.bet << "." << endl;
                 cout << "Please change your bet." << endl;
                 this_thread::sleep_for(chrono::seconds(2)); 
                 break; 
             }

            vector<string> result;
            cout << "\nSpinning..." << endl;
            this_thread::sleep_for(chrono::milliseconds(200));

            cout << "[ ";
            for (int i = 0; i < 3; ++i) {
                result.push_back(symbols[dist(rng)]);
                cout << result[i] << (i < 2 ? " | " : "");
                this_thread::sleep_for(chrono::milliseconds(500));
            }
            cout << " ]" << endl << endl;

            bool win = false;
            if (result[0] == result[1] && result[1] == result[2]) {
                string win_symbol = result[0];
                int multiplier = 0;
                try {
                    multiplier = payouts.at(win_symbol);
                    win = true;

                    int winnings = player.bet * multiplier;
                    int profit = winnings - player.bet;
                    player.balance += profit;

                    cout << "!!! JACKPOT !!! You matched three " << win_symbol << "'s!" << endl;
                    cout << "You win " << winnings << " (Profit: " << profit << ")" << endl;

                } catch (const out_of_range& oor) {
                    cerr << "Error: Winning symbol " << win_symbol << " not found in payouts map." << endl;
                    cout << "Sorry, you lose this time (Payout Error)." << endl;
                    player.balance -= player.bet; 
                }
            }

            if (!win) {
                cout << "Sorry, you lose this time." << endl;
                player.balance -= player.bet; 
            }

            cout << "Balance after spin: " << player.balance << endl;
   


         
             if (player.balance <= 0) {
                  cout << "\nYou've run out of money!" << endl;
                  press_enter_to_continue();
                  return;
             }

            // Prompt for next action
            cout << "\nOptions: [Enter] or [s] to spin again | [c] Change bet | [q] Quit slots: ";
            string action;
            getline(cin, action); 

            char command = ' ';
            if (!action.empty()) {
                command = tolower(action[0]);
            }

            if (command == 'q') {
                return; 
            } else if (command == 'c') {
                break; 
            } else if (command == 's' || action.empty()) {
                continue;
            } else {
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
        cout << "3. Play Slots" << endl;
        cout << "4. View Balance" << endl;
        cout << "5. Quit" << endl;
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


        switch (choice) {
            case 1:
                play_blackjack(player);
                break;
            case 2:
                play_high_low(player);
                break;
            case 3:
                 play_slots(player);
                 break;
            case 4:
                 cout << "\nYour current balance is: " << player.balance << endl;
                 press_enter_to_continue();
                 break;
            case 5:
                cout << "\nThanks for playing! Your final balance is: " << player.balance << endl;
                return 0;
            default:
                cout << "Invalid choice. Please try again." << endl;
                this_thread::sleep_for(chrono::seconds(1));
                break;
        }
    }

    return 0;
}