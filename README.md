# BullsAndCows

This is a project with a game of bulls and cows. The project is divided into one model for the game and 3 executable projects:
1. Strategy Calculation - forms a json file used by the computer at a hard level.
2. Simulation - a project for subsetting the exact average number of steps for each used level of computer difficulty in the game.
3. Game View - the game itself, with 3 modes:

A regular game of bulls and cows only for the player.
A game where only the computer plays.
A game of the player against the computer.
Computer difficulty
is divided into 5 difficulty levels
1. The easiest level. The computer gives out random numbers
2. Easy level. The computer conducts a simple analysis of several previous attempts.
3. Medium level. Analyzes all past attempts and chooses from possible options.
4. Hard level. Loads a pre-compiled json file with a full strategy for moves.
