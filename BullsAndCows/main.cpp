/* This is the console executable, that makes use of the BullCow class
 * This acts as the view in a MVC pattern, and is responsible for all
 * user interactions. For game logic see the FBullCowGame class.
 */

#include <iostream>
#include <string>
#include "fbullcowgame.h"

// to make syntax Unreal friendly
using FText = std::string;
using int32 = int;

// function prototypes as outside a class
void PrintIntro();
void PlayGame();
void PrintGameSummary();
bool AskToPlayAgain();

FText GetValidGuess();
FBullCowGame BCGame; // instantiate a game, which we re-use across all plays

// the entry point of our application
/**
 * @brief main
 * @return
 */
int main()
{
    do {
        PrintIntro();
        PlayGame();
    } while (AskToPlayAgain());

    std::cout << std::endl;

    return 0; // exit the application
}

/**
 * @brief PrintIntro
 */
void PrintIntro() {
    std::cout << "Welcome to the Bulls and Cows game\n";
    std::cout << "Can you guess " << BCGame.GetMyHiddenWordLength();
    std::cout << " letters word I am thinking of?\n\n";
    return;
}

/**
 * @brief GetValidGuess
 * @return
 */
FText GetValidGuess() {

    FText Guess = "";
    EGuessStatus Status = EGuessStatus::Not_Valid;
    do
    {
        // get guess from player
        int32 CurrentTry = BCGame.GetCurrentTry();
        std::cout << "Try " << CurrentTry << ". Enter your guess: ";
        std::getline(std::cin, Guess);

        Status = BCGame.CheckGuessValidity(Guess);
        switch(Status)
        {
        case EGuessStatus::Not_Valid:
            std::cout << "Not Valid Status.\n\n";
            break;
        case EGuessStatus::Not_Isogram:
            std::cout << "Please write word without repeating letters.\n\n";
            break;
        case EGuessStatus::Not_Lowercase:
            std::cout << "Please write word with lover case.\n\n";
            break;
        case EGuessStatus::Wrong_Length:
            std::cout << "Please write " << BCGame.GetMyHiddenWordLength() << " letter word .\n\n";
            break;

        default:
            break;
        }
    }
    while (Status != EGuessStatus::OK);
    return Guess;
}

// plays a single game to complition
/**
 * @brief PlayGame
 */
void PlayGame()
{
    BCGame.Reset();
    int32 MaxTries = BCGame.GetMaxTries();

    // loop through till the game is won or no more tries
    while (!BCGame.IsGameWon() && BCGame.GetCurrentTry() <= MaxTries)
    {

        FText Guess = GetValidGuess(); // TODO make loop checking valid guesses
        FBullCowCount BullCowCount = BCGame.SubbmitValidGuees(Guess);


        // print number of bulls and cows
        std::cout << "Bulls : " << BullCowCount.Bulls << std::endl;
        std::cout << "Cows : " << BullCowCount.Cows<< std::endl;
        std::cout << std::endl;

    }
    PrintGameSummary();
}

/**
 * @brief AskToPlayAgain
 * @return
 */
bool AskToPlayAgain() {

    std::cout << "Do you want to play again ? (y/n)\n";
    FText Response = "";
    std::getline(std::cin, Response);

    return (Response[0] == 'Y') || (Response[0] == 'y');

}

/**
 * @brief PrintGameSummary
 */
void PrintGameSummary()
{
    if (BCGame.IsGameWon())
        std::cout << "Gratz U Won.\n";
    else
        std::cout << "Better LCK next time.\n";
}
