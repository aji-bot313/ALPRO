#include <iostream>
using namespace std;

// Global counter for attempts
int attempts = 0;

// Simple random number generator
int getRandomNumber() {
    static int seed = 1;
    seed = (seed * 7 + 3) % 100;
    return (seed % 10) + 1;
}

// Overload #1: Give hint based on guess
void giveHint(int secret, int guess) {
    if (guess < secret) {
        cout << "Too low! Try a higher number." << endl;
    } else if (guess > secret) {
        cout << "Too high! Try a lower number." << endl;
    }
}

// Overload #2: Give special hint
void giveHint(int secret, char type) {
    if (type == 'e') {
        int low = secret - 2;
        int high = secret + 2;
        if (low < 1) low = 1;
        if (high > 10) high = 10;
        cout << "Hint: Number is between " << low << " and " << high << endl;
    } else if (type == 'h') {
        if (secret % 2 == 0) {
            cout << "Hint: Number is even" << endl;
        } else {
            cout << "Hint: Number is odd" << endl;
        }
    }
}

// Function to get valid integer input
int getValidInput() {
    int value;
    cin >> value;
    
    // If input fails (user entered a letter)
    if (cin.fail()) {
        cin.clear();  // Clear the error flag
        cin.ignore(1000, '\n');  // Remove bad input from buffer
        return -1;  // Return invalid value
    }
    
    return value;
}

// Recursive guessing function
bool playGame(int secret, int maxGuesses) {
    // Check if player lost
    if (attempts >= maxGuesses) {
        cout << "\nGame Over! The number was " << secret << endl;
        return false;
    }
    
    int guess;
    cout << "\nGuess #" << (attempts + 1) << " (1-10): ";
    
    guess = getValidInput();
    
    // Input validation
    if (guess < 1 || guess > 10) {
        cout << "Please enter a number between 1 and 10!" << endl;
        return playGame(secret, maxGuesses);
    }
    
    attempts++;
    
    // Check if guess is correct
    if (guess == secret) {
        cout << "Correct! You got it in " << attempts << " tries!" << endl;
        return true;
    }
    
    // Give regular hint
    giveHint(secret, guess);
    
    // Every 2 wrong guesses, offer special hint
    if (attempts % 2 == 0 && attempts < maxGuesses) {
        char choice;
        cout << "Want a special hint? (e=easy, h=hard, n=no): ";
        cin >> choice;
        
        if (choice == 'e' || choice == 'h') {
            giveHint(secret, choice);
        }
    }
    
    // Recursive call
    return playGame(secret, maxGuesses);
}

int main() {
    char playAgain;
    
    do {
        // Reset attempts for new game
        attempts = 0;
        
        // Generate new random number for this game
        int secret = getRandomNumber();
        int maxGuesses = 3;
        
        cout << "\n======================================" << endl;
        cout << "   RECURSIVE NUMBER GUESSING GAME" << endl;
        cout << "======================================" << endl;
        cout << "I'm thinking of a number between 1-10" << endl;
        cout << "You have 3 guesses!" << endl;
        cout << "======================================" << endl;
        
        // Start the game
        bool won = playGame(secret, maxGuesses);
        
        // Show results
        cout << "\n======================================" << endl;
        cout << "              GAME OVER" << endl;
        cout << "======================================" << endl;
        cout << "Number was: " << secret << endl;
        cout << "Total attempts: " << attempts << endl;
        cout << "Result: " << (won ? "YOU WIN" : "YOU LOSE") << endl;
        cout << "======================================" << endl;
        
        // Clear input buffer before asking play again
        cin.ignore(1000, '\n');
        
        // Ask to play again
        cout << "\nPlay again? (y/n): ";
        cin >> playAgain;
        
    } while (playAgain == 'y' || playAgain == 'Y');
    
    cout << "\nThanks for playing!" << endl;
    
    return 0;
}
