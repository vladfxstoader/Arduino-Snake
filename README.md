# Snake

This repository is dedicated to the matrix project for the Introduction to Robotics course, taken in the 3rd year (2022-2023) at the Faculty of Mathematics and Computer Science, University of Bucharest.

# Backstory

I chose to recreate the classic game of Snake because it is, by far, the most popular game on Earth, and also because I have very fond memories of myself playing it on an old Nokia phone in my childhood.

# How to play

The game is very simple and intuitive to play: the snake must eat as much food as possible and avoid hitting itself.

The initial size of the snake is one, and the score increases along with its size. The snake can also pass through walls (when it exits the matrix, it reappears on the other side).

If the difficulty of the game is set to easy, the snake moves quite slow and its size increases by one when eating food. If the difficulty is set to medium, the snake moves faster and its length increases by two when eating. If the difficulty is set to hard, along with the challenges from the medium difficulty, the food stays turned off three times more than it stays on.

If the user beats one of the top 5 scores from the leaderboard, a message is shown on the LCD.

# How to use the menu

When the board is powered up, a greeting message appears on the LCD; the user must press the button of the joystick to access the menu.

The menu contains the following submenus:

* Start game
* Leaderboard - contains the top 5 scores (stored in EEPROM)
* Settings
    - Player name: the user can enter their name (old school 3-letter name)
    - LCD brightness: the user can set the brightness of the LCD (stored in EEPROM)
    - Matrix brightness: the user can set the brightness of the matrix (stored in EEPROM)
    - Difficulty: the user can choose between easy, medium and hard (stored in EEPROM)
    - Sounds: the user can choose between sounds on or off (stored in EEPROM)
    - Reset HS: the leaderboard is reset
* How to play - information about the game
* About - information about the author

# Components

* 8x8 LED matrix
* Matrix driver
* Joystick
* Buzzer
* LCD
* 10 uF 50V capacitor
* 104 ceramic capacitor
* Two potentiometers (one for the contrast of the LCD and one for the volume of the buzzer)
* Resistors
* Cables
* Arduino Uno Board

# Setup
![Setup](https://raw.githubusercontent.com/vladfxstoader/Arduino-Snake/main/setup.jpg)

# Demo
TODO