#include "fbullcowgame.h"
#include <map>

// to make syntax Unreal friendly
#define TMap std::map
using int32 = int;

/**
 * @brief FBullCowGame::GetMaxTries
 * @return
 */
int32 FBullCowGame::GetMaxTries() const {
  TMap<int32, int32> WordLengthToMaxTries{{3, 4},  {4, 6},  {5, 9},
                                          {6, 13}, {7, 16}, {8, 20}};
  return WordLengthToMaxTries[MyHiddenWord.length()];
}

/**
 * @brief FBullCowGame::GetCurrentTry
 * @return
 */
int32 FBullCowGame::GetCurrentTry() const { return MyCurrentTry; }

/**
 * @brief FBullCowGame::GetMyHiddenWordLength
 * @return
 */
int32 FBullCowGame::GetMyHiddenWordLength() const {
  return MyHiddenWord.length();
}

/**
 * @brief FBullCowGame::IsGameWon
 * @return
 */
bool FBullCowGame::IsGameWon() const { return bIsGameWon; }

/**
 * @brief FBullCowGame::FBullCowGame
 */
FBullCowGame::FBullCowGame() { Reset(); }

/**
 * @brief FBullCowGame::~FBullCowGame
 */
FBullCowGame::~FBullCowGame() {
  MyMaxTries = 0;
  MyCurrentTry = 0;
}

/**
 * @brief FBullCowGame::Reset
 */
void FBullCowGame::Reset() {
  bIsGameWon = false;
  const FString HIDDEN_WORD = "planet"; // this must be an isogram (otherwise
                                        // the game will be very hard)
  MyHiddenWord = HIDDEN_WORD;

  MyCurrentTry = 1;

  return;
}

/**
 * @brief FBullCowGame::CheckGuessValidity
 * @param Guess
 * @return EGuessStatus
 */
EGuessStatus FBullCowGame::CheckGuessValidity(FString Guess) const {
  // guess has wrong length
  if (Guess.length() != MyHiddenWord.length())
    return EGuessStatus::Wrong_Length;
  // guess is not an isogram
  if (!IsIsogram(Guess)) {
    return EGuessStatus::Not_Isogram;
  }
  // guess is not all lower case
  if (!IsLowerCase(Guess)) {
    return EGuessStatus::Not_Lowercase;
  } else
    return EGuessStatus::OK;
}

/**
 * @brief FBullCowGame::SubbmitValidGuees
 * @param Guess
 * @return? counts # of Tries  and compares the letters
 */
FBullCowCount FBullCowGame::SubbmitValidGuees(FString Guess) {
  MyCurrentTry++;
  FBullCowCount BullCowCount;
  int32 WordLength = MyHiddenWord.length();
  int32 GuessLength = Guess.length();

  // loop through all letters in the hidden word
  for (int32 MHWChar = 0; MHWChar < WordLength; ++MHWChar) {
    // compare against the guess
    for (int32 GChar = 0; GChar < GuessLength; ++GChar) {
      // if the match
      if (MyHiddenWord[MHWChar] == Guess[GChar]) {
        if (MHWChar == GChar)   // if they are in the same place
          BullCowCount.Bulls++; // increment bulls
        else
          BullCowCount.Cows++; // must be cows
      }
    }
  }
  if (WordLength == BullCowCount.Bulls) {
    bIsGameWon = true;
  } else {
    bIsGameWon = false;
  }
  return BullCowCount;
}

/**
 * @brief FBullCowGame::IsIsogram
 * @param Word
 * @return bool
 */
bool FBullCowGame::IsIsogram(FString Word) const {
  // treat 0 and 1 letter strings as isograms
  if (Word.length() <= 1) {
    return true;
  }
  // setup our map
  TMap<char, bool> LetterSeen;

  // loop through all the letters of the word
  for (auto Letter : Word) {
    Letter = tolower(Letter); // handle mixed case
    // if the letter is in the map
    if (LetterSeen[Letter]) {
      return false;
    }    // we do NOT have an isogram
    else // otherwise
    {
      LetterSeen[Letter] = true; // add the letter to the map as seen
    }
  }

  return true;
}

/**
 * @brief FBullCowGame::IsLowerCase
 * @param Word
 * @return
 */
bool FBullCowGame::IsLowerCase(FString Word) const {
  for (auto Letter : Word) {
    if (!islower(Letter)) { // if not lowercase letter
      return false;
    }
  }
  return true;
}
