#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>
const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 3;
const byte d6 = 5;
const byte d7 = 4;
const byte buzzerPin = 13;
const byte LCDbrightnessPin = 6;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
const byte matrixSize = 8;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); 
byte matrixBrightness = 1;
byte aboutScrollText = 0;
byte HTPscrollText = 0;
byte upDownArrows[8] = {
  B00100,
  B01110,
  B11111,
  B00000,
  B00000,
  B11111,
  B01110,
  B00100
};
byte block[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};  
const byte matrixMenu[5][8] = {
  { 0b00100000,
    0b00110000,
    0b00111000,
    0b00111100,
    0b00111100,
    0b00111000,
    0b00110000,
    0b00100000 },
  { 0b01111100,
    0b00111000,
    0b00010000,
    0b01111100,
    0b01010100,
    0b00111000,
    0b00010000,
    0b00111000 },
  { 0b00010000,
    0b00111000,
    0b01110000,
    0b11110000,
    0b01001000,
    0b00000100,
    0b00000010,
    0b00000001 },
  { 0b00000000,
    0b00011000,
    0b00100100,
    0b00000100,
    0b00001000,
    0b00001000,
    0b00000000,
    0b00001000 },
  { 0b00000000,
    0b00001000,
    0b00000000,
    0b00011000,
    0b00001000,
    0b00001000,
    0b00001000,
    0b00011100 }
};
const byte matrixHappy[8] = {
  0b00100100,
  0b00100100,
  0b00100100,
  0b00100100,
  0b00000000,
  0b10000001,
  0b01000010,
  0b00111100
};
byte matrix[matrixSize][matrixSize] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};
const String menuOptions[5] = {
  "Start game",
  "Leaderboard",
  "Settings",
  "How to play",
  "About"
};
const String settingsOptions[6] = {
  "Player name",
  "LCD bright.",
  "Matrix bright.",
  "Difficulty",
  "Sounds",
  "Reset HS"
};
char nameLetters[3] = {
  'A', 'A', 'A'
};
byte namePos = 0;
byte menuCurrentItem = 0;
const byte pinSW = 2;
const byte pinX = A0;
const byte pinY = A1;
bool swState = LOW;
bool lastSwState = LOW;
byte state = 0;
byte switchState = HIGH;
int xValue = 0;
byte subMenuOption = 0;
int yValue = 0;
bool joyBackToMiddleX = LOW;
bool joyBackToMiddleY = LOW;
const int minThreshold = 400;
const int maxThreshold = 600;
unsigned long lastDebounceTime = 0;
unsigned long prevScrollTime = 0;
const byte debounceDelay = 50;
byte score = 0;
byte xPos = 0;
byte yPos = 0;
byte xLastPos = 0;
byte yLastPos = 0;
byte lastFoodRow = 0;
byte lastFoodColumn = 0;
byte scrollTextPosition = 0;
byte currentFoodRow = 0;
byte leaderboard = 0;
byte leaderboardPos = 0;
byte settings = 0;
byte beatHighscore = 0;
byte settingsPos = 0;
byte currentFoodColumn = 0;
byte startGame = 0;
const String messageAbout = "Made by Vlad Toader. Github user: @vladfxstoader";
const String messageHTP = "Eat blinking food to increase score. The game ends when the snake hits itself.";
byte LCDbrightness;
byte difficulty;
byte sounds;
unsigned long lastBlink = 0;
byte blinkLetter = LOW;
const int blinkInterval = 250;
unsigned long lastMoved = 0;
byte highscores[5];
String highscoreNames[5] = { "", "", "", "", "" };
unsigned long lastLetterBlink = 0;
byte i;
byte headRow = 0;
byte headColumn = 1;
byte snakeLength = 2;
int body[30][2];
int updateInterval;
unsigned long lastGameUpdate = 0;
int directionRow = 0;  // directions: (-1, 0) - up, (0, 1) - right
int directionColumn = 1; // (1, 0) - down, (0, -1) - left

void resetToMenu() {
  lcd.clear();
  lcd.print("<MENU>");
  lcd.setCursor(15, 0);
  lcd.write(byte(0));
  lcd.setCursor(0, 1);
  lcd.print(">");
  lcd.print(menuOptions[menuCurrentItem]);
  lc.clearDisplay(0);
  for (int row = 0; row < matrixSize; row++) {
    lc.setRow(0, row, matrixMenu[menuCurrentItem][row]);
  }
}

void gameDifficulty() {
  if (difficulty == 1)
    updateInterval = 500;
  else if (difficulty == 2 || difficulty == 3)
    updateInterval = 250;
}

bool foodInBody(byte row, byte column) {
  for (i = 0; i < snakeLength; ++i)
    if (body[i][0] == row && body[i][1] == column)
      return true;
  return false;
}

// function used for the game logic
void game() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  if (xValue < minThreshold && directionRow == 0) {
    directionRow = 1;
    directionColumn = 0;
  }
  if (xValue > maxThreshold && directionRow == 0) {
    directionRow = -1;
    directionColumn = 0;
  }
  if (yValue < minThreshold && directionColumn == 0) {
    directionRow = 0;
    directionColumn = -1;
  }
  if (yValue > maxThreshold && directionColumn == 0) {
    directionRow = 0;
    directionColumn = 1;
  }
  if (millis() - lastBlink > blinkInterval && difficulty != 3) {
    matrix[currentFoodRow][currentFoodColumn] = !matrix[currentFoodRow][currentFoodColumn];
    lastBlink = millis();
  }
  else if (difficulty == 3) {
    if (matrix[currentFoodRow][currentFoodColumn] == 1) {
      if (millis() - lastBlink > blinkInterval) {
        matrix[currentFoodRow][currentFoodColumn] = 0;
        lastBlink = millis();
      }
    }
    else {
      if (millis() - lastBlink > 3 * blinkInterval) {
        matrix[currentFoodRow][currentFoodColumn] = 1;
        lastBlink = millis();
      }
    }
  }
   if(millis() - lastGameUpdate > updateInterval){
    lastGameUpdate = millis();
    int result = updatePositions();
    if (result == -1)
      return;
  }
  updateMatrix();
}

// function that generates new food (blinking point on the matrix)
void generateFood() {
  lastFoodRow = currentFoodRow;
  lastFoodColumn = currentFoodColumn;
  currentFoodRow = random(0, matrixSize);
  currentFoodColumn = random(0, matrixSize);
  while (foodInBody(currentFoodRow, currentFoodColumn)) {
    currentFoodRow = random(0, matrixSize);
    currentFoodColumn = random(0, matrixSize);
  }
  matrix[lastFoodRow][lastFoodColumn] = 0;
  matrix[currentFoodRow][currentFoodColumn] = 1;
}

// function that updates the matrix
void updateMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, matrix[row][col]);
    }
  }
}

// function that resets the matrix
void resetMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
        matrix[row][col] = 0;
    }
  }
}

// function that updates the positions according to the movement
// of the joystick
int updatePositions() {
  int newHeadRow = headRow + directionRow;
  int newHeadColumn = headColumn + directionColumn;
  if (newHeadRow == matrixSize) {
    newHeadRow = 0;
  }
  else if (newHeadRow == -1) {
    newHeadRow = matrixSize - 1;
  }
  else if (newHeadColumn == matrixSize) {
    newHeadColumn = 0;
  }
  else if (newHeadColumn == -1) {
    newHeadColumn = matrixSize - 1;
  }
  for (i = 0; i < snakeLength; ++i) {
    if (body[i][0] == newHeadRow && body[i][1] == newHeadColumn) {
      // snake hits itself
      if (sounds == 1) {
        tone(buzzerPin, 150);
        delay(500);
        noTone(buzzerPin);
      }
      exitGame();
      return -1;
    }
  }
  if (newHeadRow == currentFoodRow && newHeadColumn == currentFoodColumn) {
    if (sounds == 1) {
      tone(buzzerPin, 350);
      delay(150);
      noTone(buzzerPin);
    }
    score++;
    snakeLength++;
    if (difficulty == 2 || difficulty == 3) {
      snakeLength++;
      score++;
    }
    lcd.setCursor(7, 1);
    lcd.print(score);
    generateFood();
  }
  else {
    for (i = 1; i < snakeLength; ++i) {
      body[i - 1][0] = body[i][0];
      body[i - 1][1] = body[i][1];
    }
  }
  body[snakeLength - 1][0] = newHeadRow;
  body[snakeLength - 1][1] = newHeadColumn;
  headRow = newHeadRow;
  headColumn = newHeadColumn;
  matrix[body[0][0]][body[0][1]] = 0;
  matrix[headRow][headColumn] = 1;
  return 0;
}

// function used after the game is finished
void exitGame() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, true);
    }
  }
  directionRow = 0;
  directionColumn = 1;
  lastGameUpdate = 0;
  headRow = 0;
  headColumn = 1;
  snakeLength = 2;
  for (i=0; i<40; ++i) {
    body[i][0] = -1;
    body[i][1] = -1;
  }
  resetMatrix();
  body[0][0] = 0;
  body[0][1] = 0;
  body[1][0] = 0;
  body[1][1] = 1;
  lcd.clear();
  menuCurrentItem = 0;
  subMenuOption = 0;
  startGame = 0;
  lcd.print(nameLetters[0]);
  lcd.print(nameLetters[1]);
  lcd.print(nameLetters[2]);
  lcd.print(", you died!");
  lcd.setCursor(0, 1);
  lcd.print("Your score: ");
  lcd.print(score);
  if (sounds == 1) {
    tone(buzzerPin, 300);
    delay(500);
    tone(buzzerPin, 500);
    delay(500);
    noTone(buzzerPin);
  }
  else {
    delay(1000);
  }
  String newName = "";
  newName += nameLetters[0];
  newName += nameLetters[1];
  newName += nameLetters[2];
  updateHStoEEPROM(score, newName);
  getHighscores();
  score = 0;
  delay(4000);
  if (beatHighscore != 0) {
    for (int row = 0; row < matrixSize; row++) {
      lc.setRow(0, row, matrixHappy[row]);
    }
    lcd.clear();
    lcd.print("WOW! You are #");
    lcd.print(beatHighscore);
    lcd.setCursor(0, 1);
    lcd.print("on leaderboard!");
    beatHighscore = 0;
    if (sounds == 1)
      song();
    state = 4;
  }
  else {
    state = 1;
    resetToMenu();
  }
}

/*
in EEPROM, the memory will be put like this:
1 - LCD brightness level (0-5, multiplied by 51 and write it to pin)
2 - Matrix brightness
3 - Difficulty
4 - Sounds on/off
5 - First highscore
6 -> 9 - First highscore name
10 - Second highscore
11 -> 14 - Second highscore name
15 - Third highscore
16 -> 19 - Third highscore name
20 - Fourth highscore
21 -> 24 - Fourth highscore name
25 - Fifth highscore
26 -> 29 - Fifth highscore name
*/

// function that gets the settings values from EEPROM
void getSettingsFromEEPROM() {
  LCDbrightness = EEPROM.read(1);
  matrixBrightness = EEPROM.read(2);
  difficulty = EEPROM.read(3);
  sounds = EEPROM.read(4);
}

// function used for initialising the leaderboard with 0
void putInitialHStoEEPROM() {
  for (i = 1; i <= 5; ++i) {
    EEPROM.update(i * 5, 0);
    EEPROM.update(i * 5 + 1, 'N');
    EEPROM.update(i * 5 + 2, 'a');
    EEPROM.update(i * 5 + 3, 'N');
  }
}

// function that updates the highscores at the end of the game
void updateHStoEEPROM(int newScore, String newName) {
  if (newScore > highscores[0]) {
    beatHighscore = 1;
    EEPROM.update(25, EEPROM.read(20));
    EEPROM.update(26, EEPROM.read(21));
    EEPROM.update(27, EEPROM.read(22));
    EEPROM.update(28, EEPROM.read(23));

    EEPROM.update(20, EEPROM.read(15));
    EEPROM.update(21, EEPROM.read(16));
    EEPROM.update(22, EEPROM.read(17));
    EEPROM.update(23, EEPROM.read(18));

    EEPROM.update(15, EEPROM.read(10));
    EEPROM.update(16, EEPROM.read(11));
    EEPROM.update(17, EEPROM.read(12));
    EEPROM.update(18, EEPROM.read(13));

    EEPROM.update(10, EEPROM.read(5));
    EEPROM.update(11, EEPROM.read(6));
    EEPROM.update(12, EEPROM.read(7));
    EEPROM.update(13, EEPROM.read(8));

    EEPROM.update(5, newScore);
    EEPROM.update(6, newName[0]);
    EEPROM.update(7, newName[1]);
    EEPROM.update(8, newName[2]);
  } 
  else if (newScore > highscores[1]) {
    beatHighscore = 2;
    EEPROM.update(25, EEPROM.read(20));
    EEPROM.update(26, EEPROM.read(21));
    EEPROM.update(27, EEPROM.read(22));
    EEPROM.update(28, EEPROM.read(23));

    EEPROM.update(20, EEPROM.read(15));
    EEPROM.update(21, EEPROM.read(16));
    EEPROM.update(22, EEPROM.read(17));
    EEPROM.update(23, EEPROM.read(18));

    EEPROM.update(15, EEPROM.read(10));
    EEPROM.update(16, EEPROM.read(11));
    EEPROM.update(17, EEPROM.read(12));
    EEPROM.update(18, EEPROM.read(13));

    EEPROM.update(10, newScore);
    EEPROM.update(11, newName[0]);
    EEPROM.update(12, newName[1]);
    EEPROM.update(13, newName[2]);
  } 
  else if (newScore > highscores[2]) {
    beatHighscore = 3;
    EEPROM.update(25, EEPROM.read(20));
    EEPROM.update(26, EEPROM.read(21));
    EEPROM.update(27, EEPROM.read(22));
    EEPROM.update(28, EEPROM.read(23));

    EEPROM.update(20, EEPROM.read(15));
    EEPROM.update(21, EEPROM.read(16));
    EEPROM.update(22, EEPROM.read(17));
    EEPROM.update(23, EEPROM.read(18));

    EEPROM.update(15, newScore);
    EEPROM.update(16, newName[0]);
    EEPROM.update(17, newName[1]);
    EEPROM.update(18, newName[2]);
  } 
  else if (newScore > highscores[3]) {
    beatHighscore = 4;
    EEPROM.update(25, EEPROM.read(20));
    EEPROM.update(26, EEPROM.read(21));
    EEPROM.update(27, EEPROM.read(22));
    EEPROM.update(28, EEPROM.read(23));

    EEPROM.update(20, newScore);
    EEPROM.update(21, newName[0]);
    EEPROM.update(22, newName[1]);
    EEPROM.update(23, newName[2]);
  } 
  else if (newScore > highscores[4]) {
    beatHighscore = 5;
    EEPROM.update(25, newScore);
    EEPROM.update(26, newName[0]);
    EEPROM.update(27, newName[1]);
    EEPROM.update(28, newName[2]);
  }
}

// function that retrieves the highscores from EEPROM
void getHighscores() {
  for (i = 1; i <= 5; ++i) {
    highscores[i - 1] = EEPROM.read(i * 5);
    String name = "";
    name += char(EEPROM.read(i * 5 + 1));
    name += char(EEPROM.read(i * 5 + 2));
    name += char(EEPROM.read(i * 5 + 3));
    highscoreNames[i - 1] = name;
  }
}

// function that plays a song used for the welcome screen
void song() {
  int melody[] = {
    262, 196, 196, 220, 196, 0, 247, 262
  };
  int noteDurations[] = {
    4, 8, 8, 4, 4, 4, 4, 4
  };
  for (i = 0; i < 8; i++) {
    tone(buzzerPin, melody[i], 1000 / noteDurations[i]);
    delay(1000 / noteDurations[i] * 1.30);
    noTone(buzzerPin);
  }
}

// function used to blink the current letter when the player
// chooses their name
void blinkLetterName() {
  if (state == 3 && settings == 1 && settingsPos == 0) {
    if (millis() - lastLetterBlink > 400) {
      lastLetterBlink = millis();
      blinkLetter = !blinkLetter;
    }
    lcd.setCursor(namePos + 4, 1);
    if (blinkLetter == HIGH) {
      lcd.print(nameLetters[namePos]);
    } 
    else {
      lcd.print(" ");
    }
  }
}

// function used for different uses of the button of the joystick
void buttonLogic() {
  if (swState != lastSwState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (swState != switchState) {
      switchState = swState;
      if (switchState == LOW) {
        if (state == 0 && startGame == 0 || state == 4) {  // past the welcome screen
          state = 1;
          lcd.clear();
          resetToMenu();
          if (sounds == 1) {
            tone(buzzerPin, 300);
            delay(200);
            noTone(buzzerPin);
          };
        } 
        else if (subMenuOption == 1) {  // when the game starts
          startGame = 1;
          if (sounds == 1) {
            tone(buzzerPin, 300);
            delay(200);
            noTone(buzzerPin);
          };
          lcd.clear();
          lcd.print("PLAYER: ");
          lcd.print(nameLetters[0]);
          lcd.print(nameLetters[1]);
          lcd.print(nameLetters[2]);
          lcd.setCursor(0, 1);
          lcd.print("SCORE: ");
          lcd.print(score);
          lc.clearDisplay(0);  // clear screen
          generateFood();
        } 
        else if (state == 3 && startGame == 0 && settings == 1 && settingsPos == 0) {  // after the player saves their name
          state = 2;
          lcd.clear();
          lcd.print("<SETTINGS>");
          lcd.setCursor(15, 0);
          lcd.write(byte(0));
          lcd.setCursor(0, 1);
          lcd.print(">");
          lcd.print(settingsOptions[settingsPos]);
          lc.clearDisplay(0);
          for (int row = 0; row < matrixSize; row++) {
            lc.setRow(0, row, matrixMenu[menuCurrentItem][row]);
          }
          if (sounds == 1) {
            tone(buzzerPin, 400);
            delay(200);
            noTone(buzzerPin);
          };
        }
        else if (state == 3 && startGame == 0 && settings == 1 && settingsPos == 5) {  // after the player resets the leaderboard
          putInitialHStoEEPROM();
          getHighscores();
          state = 2;
          lcd.clear();
          lcd.print("<SETTINGS>");
          lcd.setCursor(15, 0);
          lcd.write(byte(0));
          lcd.setCursor(0, 1);
          lcd.print(">");
          lcd.print(settingsOptions[settingsPos]);
          lc.clearDisplay(0);
          for (int row = 0; row < matrixSize; row++) {
            lc.setRow(0, row, matrixMenu[menuCurrentItem][row]);
          }
          if (sounds == 1) {
            tone(buzzerPin, 400);
            delay(200);
            noTone(buzzerPin);
          };
        }
      }
    }
  }
  lastSwState = swState;
}

// function used for different uses of the y axis of the joystick
void yAxisLogic() {
  if (yValue > maxThreshold && joyBackToMiddleY == LOW && state == 1 && startGame == 0) {  // selecting an option from the menu
    state = 2;
    if (menuCurrentItem == 0) {
      subMenuOption = 1;
      lcd.clear();
      lcd.print("<START GAME>");
      lcd.setCursor(0, 1);
      lcd.print("Press to start");
      if (sounds == 1) {
        tone(buzzerPin, 300);
        delay(200);
        noTone(buzzerPin);
      };
    } 
    else if (menuCurrentItem == 1) {
      lcd.clear();
      lcd.print("<LEADERBOARD>");
      lcd.setCursor(15, 0);
      lcd.write(byte(0));
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(leaderboardPos + 1);
      lcd.print(".");
      lcd.print(highscoreNames[leaderboardPos]);
      lcd.print(" - ");
      lcd.print(highscores[leaderboardPos]);
      lcd.print(" pts");
      if (sounds == 1) {
        tone(buzzerPin, 300);
        delay(200);
        noTone(buzzerPin);
      };
      leaderboard = 1;
    } 
    else if (menuCurrentItem == 2) {
      lcd.clear();
      lcd.print("<SETTINGS>");
      lcd.setCursor(15, 0);
      lcd.write(byte(0));
      settings = 1;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(">");
      lcd.print(settingsOptions[settingsPos]);
      if (sounds == 1) {
        tone(buzzerPin, 300);
        delay(200);
        noTone(buzzerPin);
      };
    } 
    else if (menuCurrentItem == 3) {
      lcd.clear();
      lcd.print("<HOW TO PLAY>");
      lcd.setCursor(0, 1);
      HTPscrollText = 1;
      if (sounds == 1) {
        tone(buzzerPin, 300);
        delay(200);
        noTone(buzzerPin);
      };
    } 
    else if (menuCurrentItem == 4) {
      lcd.clear();
      lcd.print("<ABOUT>    SNAKE");
      lcd.setCursor(0, 1);
      aboutScrollText = 1;
      if (sounds == 1) {
        tone(buzzerPin, 300);
        delay(200);
        noTone(buzzerPin);
      };
    }
    joyBackToMiddleY = HIGH;
  } 
  else if (yValue < minThreshold && joyBackToMiddleY == LOW && state == 2 && startGame == 0) {  // moving back to the main menu
    state = 1;
    aboutScrollText = 0;
    HTPscrollText = 0;
    scrollTextPosition = 0;
    leaderboardPos = 0;
    subMenuOption = 0;
    leaderboard = 0;
    settingsPos = 0;
    settings = 0;
    lcd.clear();
    resetToMenu();
    if (sounds == 1) {
      tone(buzzerPin, 400);
      delay(200);
      noTone(buzzerPin);
    };
    joyBackToMiddleY = HIGH;
  } 
  else if (yValue > maxThreshold && joyBackToMiddleY == LOW && state == 2 && startGame == 0 && settings == 1) {  // choosing an option from the settings submenu
    state = 3;
    lcd.clear();
    if (settingsPos == 0) {
      lcd.print("<NAME>  Press to");
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
      lcd.print("   ");
      lcd.print(nameLetters[0]);
      lcd.print(nameLetters[1]);
      lcd.print(nameLetters[2]);
      lcd.print("     SAVE");
      if (sounds == 1) {
        tone(buzzerPin, 300);
        delay(200);
        noTone(buzzerPin);
      };
    } 
    else if (settingsPos == 1) {
      lcd.print("<LCD BRIGHT.>");
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
      lcd.setCursor(4, 1);
      lcd.print("-");
      for (i = 0; i < LCDbrightness; ++i)
        lcd.write(byte(1));
      lcd.setCursor(10, 1);
      lcd.print("+");
      if (sounds == 1) {
        tone(buzzerPin, 300);
        delay(200);
        noTone(buzzerPin);
      };
    } 
    else if (settingsPos == 2) {
      for (int row = 0; row < matrixSize; row++) {
        for (int col = 0; col < matrixSize; col++) {
          lc.setLed(0, row, col, true);
        }
      }
      lcd.print("<MATRIX BRIGHT.>");
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
      lcd.setCursor(4, 1);
      lcd.print("-");
      for (i = 0; i < matrixBrightness; ++i)
        lcd.write(byte(1));
      lcd.setCursor(10, 1);
      lcd.print("+");
      if (sounds == 1) {
        tone(buzzerPin, 300);
        delay(200);
        noTone(buzzerPin);
      };
    } 
    else if (settingsPos == 3) {
      lcd.print("<DIFFICULTY>");
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
      lcd.print(" ");
      if (difficulty == 1) {
        lcd.print("EASY");
      }
      else if (difficulty == 2) {
        lcd.print("MEDIUM");
      }
      else if (difficulty == 3) {
        lcd.print("HARD");
      }
      if (sounds == 1) {
        tone(buzzerPin, 300);
        delay(200);
        noTone(buzzerPin);
      };
    } 
    else if (settingsPos == 4) {
      lcd.print("<SOUNDS>");
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
      lcd.print(" ");
      if (sounds == 1)
        lcd.print("ON");
      else
        lcd.print("OFF");
      if (sounds == 1) {
        tone(buzzerPin, 300);
        delay(200);
        noTone(buzzerPin);
      };
    }
    else if (settingsPos == 5) {
      lcd.print("<RESET HS>");
      lcd.setCursor(0, 1);
      lcd.print("Press to reset");
      if (sounds == 1) {
        tone(buzzerPin, 300);
        delay(200);
        noTone(buzzerPin);
      };
    }
    joyBackToMiddleY = HIGH;
  } 
  else if (yValue < minThreshold && joyBackToMiddleY == LOW && state == 3 && startGame == 0 && settings == 1 and settingsPos != 0) {  // going back to the settings submenu
    state = 2;
    lcd.clear();
    lcd.print("<SETTINGS>");
    lcd.setCursor(15, 0);
    lcd.write(byte(0));
    lcd.setCursor(0, 1);
    lcd.print(">");
    lcd.print(settingsOptions[settingsPos]);
    lc.clearDisplay(0);
    for (int row = 0; row < matrixSize; row++) {
      lc.setRow(0, row, matrixMenu[menuCurrentItem][row]);
    }
    if (sounds == 1) {
      tone(buzzerPin, 400);
      delay(200);
      noTone(buzzerPin);
    };
    joyBackToMiddleY = HIGH;
  } 
  else if (yValue < minThreshold && joyBackToMiddleY == LOW && state == 3 && startGame == 0 && settings == 1 and settingsPos == 0) {  // moving left through the name field
    if (namePos > 0) {
      lcd.setCursor(namePos + 4, 1);
      lcd.print(nameLetters[namePos]);
      namePos--;
    }
    joyBackToMiddleY = HIGH;
  } 
  else if (yValue > maxThreshold && joyBackToMiddleY == LOW && state == 3 && startGame == 0 && settings == 1 and settingsPos == 0) {  // moving right through the name field
    if (namePos < 2) {
      lcd.setCursor(namePos + 4, 1);
      lcd.print(nameLetters[namePos]);
      namePos++;
    }
    joyBackToMiddleY = HIGH;
  } 
  else if (joyBackToMiddleY == HIGH && yValue < maxThreshold && yValue > minThreshold && startGame == 0) {
    joyBackToMiddleY = LOW;
  }
}

// function used for different uses of the x axis of the joystick
void xAxisLogic() {
  if (xValue < minThreshold && joyBackToMiddleX == LOW && state == 1 && startGame == 0 && leaderboard == 0) {  // moving down through the menu
    if (menuCurrentItem < 4) {
      menuCurrentItem++;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(">");
      lcd.print(menuOptions[menuCurrentItem]);
      lc.clearDisplay(0);
      for (int row = 0; row < matrixSize; row++) {
        lc.setRow(0, row, matrixMenu[menuCurrentItem][row]);
      }
      if (sounds == 1) {
        tone(buzzerPin, 200);
        delay(100);
        noTone(buzzerPin);
      };
    } 
    else if (menuCurrentItem == 4) {
      menuCurrentItem = 0;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(">");
      lcd.print(menuOptions[menuCurrentItem]);
      lc.clearDisplay(0);
      for (int row = 0; row < matrixSize; row++) {
        lc.setRow(0, row, matrixMenu[menuCurrentItem][row]);
      }
      if (sounds == 1) {
        tone(buzzerPin, 200);
        delay(100);
        noTone(buzzerPin);
      };
    }
    joyBackToMiddleX = HIGH;
  } 
  else if (xValue > maxThreshold && joyBackToMiddleX == LOW && state == 1 && startGame == 0 && leaderboard == 0) {  // moving up through the menu
    if (menuCurrentItem > 0) {
      menuCurrentItem--;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(">");
      lcd.print(menuOptions[menuCurrentItem]);
      lc.clearDisplay(0);
      for (int row = 0; row < matrixSize; row++) {
        lc.setRow(0, row, matrixMenu[menuCurrentItem][row]);
      }
      if (sounds == 1) {
        tone(buzzerPin, 200);
        delay(100);
        noTone(buzzerPin);
      };
    } 
    else if (menuCurrentItem == 0) {
      menuCurrentItem = 4;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(">");
      lcd.print(menuOptions[menuCurrentItem]);
      lc.clearDisplay(0);
      for (int row = 0; row < matrixSize; row++) {
        lc.setRow(0, row, matrixMenu[menuCurrentItem][row]);
      }
      if (sounds == 1) {
        tone(buzzerPin, 200);
        delay(100);
        noTone(buzzerPin);
      };
    }
    joyBackToMiddleX = HIGH;
  } 
  else if (xValue < minThreshold && joyBackToMiddleX == LOW && state == 2 && startGame == 0 && leaderboard == 1) {  // moving down through the leaderboard
    if (leaderboardPos < 4) {
      leaderboardPos++;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(leaderboardPos + 1);
      lcd.print(".");
      lcd.print(highscoreNames[leaderboardPos]);
      lcd.print(" - ");
      lcd.print(highscores[leaderboardPos]);
      lcd.print(" pts");
      if (sounds == 1) {
        tone(buzzerPin, 200);
        delay(100);
        noTone(buzzerPin);
      };
    }
    joyBackToMiddleX = HIGH;
  } 
  else if (xValue > maxThreshold && joyBackToMiddleX == LOW && state == 2 && startGame == 0 && leaderboard == 1) {  // moving up through the leaderboard
    if (leaderboardPos > 0) {
      leaderboardPos--;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(leaderboardPos + 1);
      lcd.print(".");
      lcd.print(highscoreNames[leaderboardPos]);
      lcd.print(" - ");
      lcd.print(highscores[leaderboardPos]);
      lcd.print(" pts");
      if (sounds == 1) {
        tone(buzzerPin, 200);
        delay(100);
        noTone(buzzerPin);
      };
    }
    joyBackToMiddleX = HIGH;
  } 
  else if (xValue < minThreshold && joyBackToMiddleX == LOW && state == 2 && startGame == 0 && settings == 1) {  // moving down through the settings submenu
    if (settingsPos < 5) {
      settingsPos++;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(">");
      lcd.print(settingsOptions[settingsPos]);
      if (sounds == 1) {
        tone(buzzerPin, 200);
        delay(100);
        noTone(buzzerPin);
      };
    }
    else if (settingsPos == 5) {
      settingsPos = 0;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(">");
      lcd.print(settingsOptions[settingsPos]);
      if (sounds == 1) {
        tone(buzzerPin, 200);
        delay(100);
        noTone(buzzerPin);
      };
    }
    joyBackToMiddleX = HIGH;
  } 
  else if (xValue > maxThreshold && joyBackToMiddleX == LOW && state == 2 && startGame == 0 && settings == 1) {  // moving up through the settings submenu
    if (settingsPos > 0) {
      settingsPos--;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(">");
      lcd.print(settingsOptions[settingsPos]);
      if (sounds == 1) {
        tone(buzzerPin, 200);
        delay(100);
        noTone(buzzerPin);
      };
    }
    else if (settingsPos == 0) {
      settingsPos = 5;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(">");
      lcd.print(settingsOptions[settingsPos]);
      if (sounds == 1) {
        tone(buzzerPin, 200);
        delay(100);
        noTone(buzzerPin);
      };
    }
    joyBackToMiddleX = HIGH;
  } 
  else if (xValue < minThreshold && joyBackToMiddleX == LOW && state == 3 && startGame == 0 && settings == 1) {  // different values when in the settings submenu
    if (settingsPos == 0) {                                                                                        // scrolling through letters while in the input of the name
      if (nameLetters[namePos] > 'A') {
        nameLetters[namePos]--;
        lcd.setCursor(namePos + 4, 1);
        lcd.print(nameLetters[namePos]);
      } 
      else if (nameLetters[namePos] == 'A') {
        nameLetters[namePos] = 'Z';
        lcd.setCursor(namePos + 4, 1);
        lcd.print(nameLetters[namePos]);
      }
    } 
    else if (settingsPos == 1) {  // changing the brightness
      if (LCDbrightness > 1) {
        LCDbrightness--;
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
        lcd.setCursor(4, 1);
        lcd.print("-");
        for (i = 0; i < LCDbrightness; ++i)
          lcd.write(byte(1));
        lcd.setCursor(10, 1);
        lcd.print("+");
        analogWrite(LCDbrightnessPin, LCDbrightness * 51);
        EEPROM.update(1, LCDbrightness);
      }
    } 
    else if (settingsPos == 2) {  // changing the brightness
      if (matrixBrightness > 1) {
        matrixBrightness--;
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
        lcd.setCursor(4, 1);
        lcd.print("-");
        for (i = 0; i < matrixBrightness; ++i)
          lcd.write(byte(1));
        lcd.setCursor(10, 1);
        lcd.print("+");
        lc.setIntensity(0, 3 * matrixBrightness);
        EEPROM.update(2, matrixBrightness);
      }
    } 
    else if (settingsPos == 3) {  // changing the difficulty
      if (difficulty > 1) {
        difficulty--;
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
        lcd.print(" ");
        if (difficulty == 1) {
          lcd.print("EASY");
        }
        else if (difficulty == 2) {
          lcd.print("MEDIUM");
        }
        else if (difficulty == 3) {
          lcd.print("HARD");
        }
        EEPROM.update(3, difficulty);
      }
    } 
    else if (settingsPos == 4) {  // changing the sounds
      if (sounds == 1) {
        sounds = 0;
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
        lcd.print(" ");
        lcd.print("OFF");
        EEPROM.update(4, sounds);
      }
    }
    joyBackToMiddleX = HIGH;
  } 
  else if (xValue > maxThreshold && joyBackToMiddleX == LOW && state == 3 && startGame == 0 && settings == 1) {  // different values when in the settings submenu
    if (settingsPos == 0) {
      if (nameLetters[namePos] < 'Z') {
        nameLetters[namePos]++;
        lcd.setCursor(namePos + 4, 1);
        lcd.print(nameLetters[namePos]);
      } 
      else if (nameLetters[namePos] == 'Z') {
        nameLetters[namePos] = 'A';
        lcd.setCursor(namePos + 4, 1);
        lcd.print(nameLetters[namePos]);
      }
    } 
    else if (settingsPos == 1) {
      if (LCDbrightness < 5) {
        LCDbrightness++;
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
        lcd.setCursor(4, 1);
        lcd.print("-");
        for (i = 0; i < LCDbrightness; ++i)
          lcd.write(byte(1));
        lcd.setCursor(10, 1);
        lcd.print("+");
        analogWrite(LCDbrightnessPin, LCDbrightness * 51);
        EEPROM.update(1, LCDbrightness);
      }
    } 
    else if (settingsPos == 2) {
      if (matrixBrightness < 5) {
        matrixBrightness++;
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
        lcd.setCursor(4, 1);
        lcd.print("-");
        for (i = 0; i < matrixBrightness; ++i)
          lcd.write(byte(1));
        lcd.setCursor(10, 1);
        lcd.print("+");
        lc.setIntensity(0, 3 * matrixBrightness);
        EEPROM.update(2, matrixBrightness);
      }
    } 
    else if (settingsPos == 3) {
      if (difficulty < 3) {
        difficulty++;
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
        lcd.print(" ");
        if (difficulty == 1) {
        lcd.print("EASY");
        }
        else if (difficulty == 2) {
          lcd.print("MEDIUM");
        }
        else if (difficulty == 3) {
          lcd.print("HARD");
        }
        EEPROM.update(3, difficulty);
      }
    } 
    else if (settingsPos == 4) {
      if (sounds == 0) {
        sounds = 1;
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
        lcd.print(" ");
        lcd.print("ON");
        EEPROM.update(4, sounds);
      }
    }
    joyBackToMiddleX = HIGH;
  } 
  else if (joyBackToMiddleX == HIGH && xValue < maxThreshold && xValue > minThreshold && startGame == 0) {
    joyBackToMiddleX = LOW;
  }
}

void scrollTextAbout() {
  if (aboutScrollText == 1 && scrollTextPosition < messageAbout.length() - 15) {
    lcd.setCursor(0, 1);
    if (millis() - prevScrollTime >= 400) {
      prevScrollTime = millis();
      lcd.setCursor(0, 1);
      lcd.print(messageAbout.substring(scrollTextPosition, 16 + scrollTextPosition));
      scrollTextPosition++;
    }
  }
}

void scrollTextHTP() {
  if (HTPscrollText == 1 && scrollTextPosition < messageHTP.length() - 15) {
    lcd.setCursor(0, 0);
    lcd.print("<HOW TO PLAY>   ");
    lcd.setCursor(0, 1);
    if (millis() - prevScrollTime >= 400) {
      prevScrollTime = millis();
      lcd.setCursor(0, 1);
      lcd.print(messageHTP.substring(scrollTextPosition, 16 + scrollTextPosition));
      scrollTextPosition++;
    }
  }
}

void setup() {
  body[0][0] = 0;
  body[0][1] = 0;
  body[1][0] = 0;
  body[1][1] = 1;
  getSettingsFromEEPROM();
  gameDifficulty();
  analogWrite(LCDbrightnessPin, LCDbrightness * 51);
  pinMode(pinSW, INPUT_PULLUP);
  Serial.begin(9600);
  lc.shutdown(0, false);
  lc.setIntensity(0, 3 * matrixBrightness);
  lc.clearDisplay(0);
  for (int row = 0; row < matrixSize; row++) {
    lc.setRow(0, row, matrixHappy[row]);
  }
  randomSeed(analogRead(A2));
  lcd.begin(16, 2);
  lcd.print("     SNAKE!");
  lcd.setCursor(0, 1);
  lcd.print(" Press to begin");
  getHighscores();
  lcd.createChar(0, upDownArrows);
  lcd.createChar(1, block);
  if (sounds == 1)
    song();
}

void loop() {
  if (startGame == 1) {
    gameDifficulty();
    game();
  } 
  else {
    blinkLetterName();
    scrollTextAbout();
    scrollTextHTP();
    swState = digitalRead(pinSW);
    xValue = analogRead(pinX);
    yValue = analogRead(pinY);
    buttonLogic();
    yAxisLogic();
    xAxisLogic();
  }
}
