/* The game logic (no view code or user interaction)
 * The game is a simple guess the word game based on the Mastermind
*/

#ifndef FBULLCOWGAME_H
#define FBULLCOWGAME_H
#include <string>

// to make syntax Unreal friendly
using FString = std::string;
using int32 = int;

struct FBullCowCount
{
    int32 Bulls = 0;
    int32 Cows = 0;
};

enum class EGuessStatus
{
    Not_Valid,
    OK,
    Not_Isogram,
    Wrong_Length,
    Not_Lowercase
};

class FBullCowGame
{
public:
    FBullCowGame(); // c-tor
    ~FBullCowGame(); // d-tor
    void Reset();
    int32 GetMaxTries() const;
    int32 GetCurrentTry() const;
    bool IsGameWon() const;
    EGuessStatus CheckGuessValidity(FString) const;
    int32 GetMyHiddenWordLength() const;
    FBullCowCount SubbmitValidGuees(FString);

private:
    // see constructor for initialization
    int32 MyCurrentTry;
    int32 MyMaxTries;
    FString MyHiddenWord;
    bool IsIsogram(FString) const;
    bool IsLowerCase(FString) const;
    bool bIsGameWon;
};

#endif // FBULLCOWGAME_H
