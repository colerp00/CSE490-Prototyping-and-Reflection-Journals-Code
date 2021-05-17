/**
 * This code uses ParallaxJoystick.hpp from MakeAbility Lab:
 * 
 * ParallaxJoystick.hpp
 * https://github.com/makeabilitylab/arduino/blob/master/MakeabilityLab_Arduino_Library/src/ParallaxJoystick.hpp
 */

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ParallaxJoystick.hpp>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


const int JOYSTICK_UPDOWN_PIN = A1;
const int JOYSTICK_LEFTRIGHT_PIN = A0;

const uint16_t MAX_ANALOG_VAL = 1023;
const enum JoystickYDirection JOYSTICK_Y_DIR = RIGHT;

// Analog joystick
ParallaxJoystick _analogJoystick(JOYSTICK_UPDOWN_PIN, JOYSTICK_LEFTRIGHT_PIN, MAX_ANALOG_VAL, JOYSTICK_Y_DIR);

const uint8_t VIBROMOTOR_OUTPUT_PIN = 11;

const uint8_t SHOOT_BUTTON_INPUT_PIN = 7;
const uint8_t SHIELD_BUTTON_INPUT_PIN = 8;

const uint8_t TWO_HEALTH_LED_PIN = 6;
const uint8_t ONE_HEALTH_LED_PIN = 9;
const uint8_t ZERO_HEALTH_LED_PIN= 10;

// Used to keep track of the game's state
enum GAMESTATE {
  MENU,
  GAME,
  GAMEOVER
};

// Used to keep track of what side each ship
// and shot is on
enum SIDE {
  PLAYER = 1,
  ENEMY = -1,
  INVALID = 0
};

// Used to store info about each LED
typedef struct led_t {
  uint8_t _brightness;
  const uint8_t _outputPin;

  led_t::led_t(uint8_t b, uint8_t o) : _brightness(b), _outputPin(o) {}

  // Updates the brightness of this LED
  void updateBrightness() {
    if(_brightness == 0) {
      _brightness = 255;
    } else if(_brightness >= 225) {
      _brightness--;
    } else if(_brightness >= 175) {
      _brightness -= 3;
    } else if(_brightness >= 125) {
      _brightness -= 5;
    } else if(_brightness >= 25) {
      _brightness -= 7;
    }
  }

  // Resets this LED's brightness back to 0
  void resetBrightness() {
    _brightness = 0;
  }

  // Lights the corresponding LED connected to
  // this led_t struct's _outputPin
  void lightLED() {
    analogWrite(_outputPin, _brightness);
  }
} LED;

// Used to store information on all of the
// shots fired by the player and enemies
typedef struct shot_t {
  int8_t x, y;
  uint8_t len;
  int8_t xSpd, ySpd;
  SIDE side;
  bool valid;

  // Update the location of this shot
  void updateLoc() {
    x += side * xSpd;
    y += ySpd;
  }

  // Check if this shot is OOB
  bool checkOOB() {
    return (x <= 0 || x >= SCREEN_WIDTH);
  }

  // Makes this shot INVALID
  void makeInvalid() {
    x = -1;
    y = -1;
    len = 0;
    xSpd = 0;
    ySpd = 0;
    valid = false;
  }
  
} PlayerShot, EnemyShot;

// Used to store information on all of the
// ships in the game, including the player
// and all of the enemies
typedef struct ship_t {
  uint8_t x, y, wdth, hlfHght, hlth;
  uint16_t clDwn;
  SIDE side;
  unsigned long lastTs;

  ship_t::ship_t(int8_t xStrt, int8_t yStrt, uint8_t w,
                 uint8_t ht, uint16_t c, uint8_t hl, SIDE s) {
    x = xStrt;
    y = yStrt;
    wdth = w;
    hlfHght = ht;
    clDwn = c;
    hlth = hl;
    lastTs = 0;
    side = s;
  }

  // Constructs an invalid ship
  ship_t::ship_t() {
    x = 0;
    y = 0;
    wdth = 0;
    hlfHght = 0;
    clDwn = 0;
    lastTs = 0;
    side = INVALID;
  }

  // Updates the location of this ship
  void updateLoc(int xMove, int yMove) {
    x += xMove;
    y += yMove;
  }

  // Sets the location of this ship
  void setLoc(int xLoc, int yLoc) {
    x = xLoc;
    y = yLoc;
  }

  // Checks to see if the given shot is
  // touching this ship
  bool shotCollides(shot_t *shot) {
    bool condition = false;
    if(shot->valid) {
      if(shot->side == PLAYER) {
        condition = (shot->x >= x) && (shot->x <= x + wdth) &&
                    (shot->y <= y + hlfHght) && (shot->y >= y - hlfHght);
      } else if(shot->side == ENEMY) {
        condition = (shot->x <= x) && (shot->x >= x - wdth) &&
                    (shot->y <= y + hlfHght) && (shot->y >= y - hlfHght);
      }
    }
    
    if(condition) {
      shot->makeInvalid();
      hlth--;
    }
    
    return condition;
  }

  // Checks to see if this ship's firing cooldown
  // is over
  bool cooldownOver(unsigned long curTs) {
    return (curTs - lastTs >= clDwn);
  }
  
} PlayerShip, EnemyShip;

// Player-controlled triangle "ship"
const uint16_t PLAYER_SHOT_COOLDOWN = 250;  // Cooldown for player's shots
const uint16_t PLAYER_INV_COOLDOWN = 1500;  // Time player is invincible after taking damage
PlayerShip _playerShip(20, 32, 10, 4, PLAYER_SHOT_COOLDOWN, 3, PLAYER);
const uint8_t MAX_PLAYER_SHOTS = 7;   // max of 7 on screen
PlayerShot *_playerShots;

// Booleans representing various player states
bool _playerShield, _playerShooting, _playerInv, _playerVisible;
unsigned long _playerInvStartTs;

// Enemy ships
const uint16_t ENEMY_SHOT_COOLDOWN = 950;   // Cooldown for enemy's shots
const uint16_t ENEMY_SPAWN_COOLDOWN = 6000; // Time between enemy spawns (initially)
const uint8_t MAX_ENEMIES = 8;  // max of 8 enemies on screen
const uint8_t MAX_ENEMY_SHOTS = 24;  // max of 24 enemy shots on screen
EnemyShip *_enemyShips;
EnemyShot *_enemyShots;

unsigned long _enemySpawnTs;  // Last time an enemy spawned

LED _zeroHealth(0,ZERO_HEALTH_LED_PIN);
LED _oneHealth(0, ONE_HEALTH_LED_PIN);
LED _twoHealth(0, TWO_HEALTH_LED_PIN);

//DIFFICULTY _gameDiff;     // Game's difficulty
GAMESTATE _gameState;       // Game's state (menu, playing, game over)
bool _showingCoolGameOver;  // Used to keep track of when game over screen should be static
bool _retrySelected;        // Used to keep track of player's choice in game over screen
bool _showingCoolMenu;      // Used to keep track of when menu screen should be static
uint8_t _lastRcrdHlth = 3;  // Used to keep track of how much health the player last had
bool _vibrating;     // Used to keep track of if the vibromotor is vibrating or not
uint8_t _vibrationStrength;  // Used to keep track of the vibration strength
uint16_t _vibrationDuration;  // Used to keep track of how long the motor should vibrate
unsigned long _motorStartTs;  // Used to keep track of when the motor began vibrating

unsigned long _gameStartTs;  // Used to keep track of when the game starts

void setup() {
  Serial.begin(9600);

  initializeOledAndShowStartupScreen();

  pinMode(SHOOT_BUTTON_INPUT_PIN, INPUT);
  pinMode(SHIELD_BUTTON_INPUT_PIN, INPUT);

  pinMode(ZERO_HEALTH_LED_PIN, OUTPUT);
  pinMode(ONE_HEALTH_LED_PIN, OUTPUT);
  pinMode(TWO_HEALTH_LED_PIN, OUTPUT);

  pinMode(VIBROMOTOR_OUTPUT_PIN, OUTPUT);

  _playerShots = new PlayerShot[MAX_PLAYER_SHOTS];
  _enemyShips = new EnemyShip[MAX_ENEMIES];
  _enemyShots = new EnemyShot[MAX_ENEMY_SHOTS];

  // Set game state
  _gameState = MENU;

  // Initialize (reset) game
  resetGame();
}

void loop() {
  _display.clearDisplay();

  if(_gameState == MENU) {
    //Serial.println("Menu");
    gameMenu();
  } else if(_gameState == GAME) {
    //Serial.println("Playing Game");
    playingGame();
  } else {//if(_gameState == GAMEOVER) {
    //Serial.println("Game Over");
    gameOver();
  }
  
  _display.display();
}

// Resets the game
void resetGame() {
  // Set game states
  _showingCoolGameOver = true;
  _retrySelected = true;
  _showingCoolMenu = true;
  
  
  // Set player states
  _playerShooting = false;
  _playerInv = false;
  _playerVisible = true;
  _playerInvStartTs = 0;
  _playerShip.setLoc(20, 32);
  _playerShip.hlth = 3;
  _playerShip.lastTs = millis();

  _lastRcrdHlth = 3;

  _vibrating = false;
  _vibrationDuration = 0;
  
  _zeroHealth.resetBrightness();
  _oneHealth.resetBrightness();
  _twoHealth.resetBrightness();

  _gameStartTs = millis();
  

  // Get current TS to use as initial enemy spawn TS
  _enemySpawnTs = millis();

  // Initialize new (INVALID) player shots
  for(int i = 0; i < MAX_PLAYER_SHOTS; i++) {
    _playerShots[i].valid = false;
  }

  // Initialize new (INVALID) player shots
  for(int i = 0; i < MAX_ENEMIES; i++) {
    _enemyShips[i].side = INVALID;
  }

  // Initialize new (INVALID) player shots
  for(int i = 0; i < MAX_ENEMY_SHOTS; i++) {
    _enemyShots[i].valid = false;
  }
}

void gameMenu() {
  //Serial.println("Game Menu");
  
  bool anyButtonPressed = (digitalRead(SHOOT_BUTTON_INPUT_PIN) == HIGH) ||
                          (digitalRead(SHIELD_BUTTON_INPUT_PIN) == HIGH);
  //Serial.print("anyButtonPressed = ");
  //Serial.println(anyButtonPressed);

  // If the player pressed any of the two buttons
  if(anyButtonPressed) {
    //Serial.println("A button was pressed!");
    resetGame();
    _gameState = GAME;
    delay(500);
  } else {
    displayGameMenuScreen();
  }
}

// Used to keep track of when to show the
// "Press any button to play!" text
uint8_t _numLoops = 0;
bool _showingAnyButton = true;

void displayGameMenuScreen() {
  //Serial.println("Game Menu");
  if(_showingCoolMenu) {
    coolMenuScreen();
  } else {
    printGameName(20, 0);
    if(_showingAnyButton) {
      printAnyButton(19, 40);
    }
  }

  if(_numLoops <= 10) {
    _showingAnyButton = true;
  } else {
    _showingAnyButton = false;
  }

  if(_numLoops == 20) {
    _numLoops = 0;
  } else {
    _numLoops++;
  }
}

// Prints the name of the game at the given coords
void printGameName(uint8_t x, uint8_t y) {
  _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  _display.setTextSize(2);
  printTriangle(x, y);
  printWord(x-4, y+20, "McShooty");
}

// Made this because the "i" in "Triangle"
// had more space around it than it should
// and it was annoying me
void printTriangle(uint8_t x, uint8_t y) {
  printWord(x, y, "Tr");
  printBigThinLetter(x+22, y, 'i');
  _display.print("ang");
  printBigThinLetter(x+66, y, 'l');
  _display.print("e");
}

// Made this for the same reason I made
// "printTriangle"
void printAnyButton(uint8_t x, uint8_t y) {
  _display.setTextSize(1);
  printWord(x, y, "Press");
  printWord(x+33, y, "any");
  printWord(x+54, y, "button");
  printWord(x+26, y+10, "to");
  printWord(x+41, y+10, "p");
  printThinLetter(x+46, y+10 ,'l');
  _display.print("ay");
  printWord(x+62, y+10, "!");
}

// Used to show the cool menu screen
void coolMenuScreen() {
  delay(2000);
  _display.clearDisplay();
  _display.setTextSize(2);
  _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  playCoolMenuScreen();
  _showingCoolMenu = false;
}

// Plays the gool menu screen animation
void playCoolMenuScreen() {
  // Fade game title
  _display.clearDisplay();
  _display.display();
  delay(1000);
  for(int i = 15; i > 2; i--) {
    _display.clearDisplay();
    printGameName(20, 12);
    for(int x = 15; x < SCREEN_WIDTH - 15; x++) {
      for(int y = 11; y < 53; y++) {
        if((3 * x + y) % i != 0) {
          _display.drawPixel(x, y, SSD1306_BLACK);
        }
      }
    }
    _display.display();
    delay(50);
  }

  for(int i = 2; i < 10; i++) {
    _display.clearDisplay();
    printGameName(20, 12);
    for(int x = 15; x < SCREEN_WIDTH - 15; x++) {
      for(int y = 11; y < 53; y++) {
        if((3 * x + y) % i == 0) {
          _display.drawPixel(x, y, SSD1306_BLACK);
        }
      }
    }
    _display.display();
    delay(50);
  }
  printGameName(20, 12);
  _display.display();
  delay(1000);

  // Game title reflection effect
  for(int i = 0; i < 140; i += 12) {
    _display.clearDisplay();
    printGameName(20, 12);
    _display.drawLine(i+23, 11, i+11, 53, SSD1306_BLACK);
    _display.drawLine(i+22, 11, i+10, 53, SSD1306_BLACK);
    _display.drawLine(i+21, 11, i+9, 53, SSD1306_BLACK);
    _display.drawLine(i+20, 11, i+8, 53, SSD1306_BLACK);
    _display.drawLine(i+19, 11, i+7, 53, SSD1306_BLACK);
    _display.drawLine(i+18, 11, i+6, 53, SSD1306_BLACK);
    _display.drawLine(i+17, 11, i+5, 53, SSD1306_BLACK);
    _display.drawLine(i+16, 11, i+4, 53, SSD1306_BLACK);
    _display.drawLine(i+15, 11, i+3, 53, SSD1306_BLACK);
    _display.drawLine(i+14, 11, i+2, 53, SSD1306_BLACK);
    _display.drawLine(i+13, 11, i+1, 53, SSD1306_BLACK);
    _display.drawLine(i+12, 11, i, 53, SSD1306_BLACK);
    
    _display.drawLine(i+6, 11, i-6, 53, SSD1306_BLACK);
    _display.drawLine(i+5, 11, i-7, 53, SSD1306_BLACK);
    _display.drawLine(i+4, 11, i-8, 53, SSD1306_BLACK);
    _display.drawLine(i+3, 11, i-9, 53, SSD1306_BLACK);
    _display.display();
  }

  delay(1000);

  // Shift game title to top of screen
  for(int i = 12; i >= 0; i--) {
    _display.clearDisplay();
    printGameName(20, i);
    _display.display();
    delay(50);
  }
}

void gameOver() {
  gameOverChoice();

  bool playerHasChosen = (digitalRead(SHOOT_BUTTON_INPUT_PIN) == HIGH);

  // If the player has made their choice...
  if(playerHasChosen) {
    // Play the game again, otherwise go back to the menu
    if(_retrySelected) {
      //Serial.println("Going to Game");
      _gameState = GAME;
    } else {
      //Serial.println("Going to Menu");
      _gameState = MENU;
    }
    resetGame();
    delay(1000);
  } else {
    displayGameOverScreen();
  }
}

// Shows the game over screen
void displayGameOverScreen() {
  if(_showingCoolGameOver) {
    coolGameOver();
  } else {
    printGameOver();
    printRetryAndQuit();
  }
}

// Plays the cool game over animation
void coolGameOver() {
  delay(2000);
  _display.clearDisplay();
  _display.setTextSize(2);
  _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  playCoolGameOver();
  _display.setTextSize(1);
  printWord(49, 40, "Retry");
  _display.display();
  delay(500);
  printQuit(54,50);
  _display.display();
  delay(500);
  _showingCoolGameOver = false;
  _retrySelected = true;
}

// Plays the "GAME OVER" portion of the
// cool game over animation
void playCoolGameOver() {
  //Serial.println("playCoolGameOver() called");
  _display.setCursor(10, 20);
  delay(500);
  printCharacter('G');
  printCharacter('A');
  printCharacter('M');
  printCharacter('E');
  _display.setCursor(70, 20);
  printCharacter('O');
  printCharacter('V');
  printCharacter('E');
  printCharacter('R');
  delay(900);
}

// Prints the given character on the OLED display,
// then displays it on the screen
void printCharacter(char c) {
  //Serial.print("Printing ");
  //Serial.println(c);
  _display.print(c);
  _display.display();
  delay(100);
}

// Prints the words "GAME OVER" on the screen
// all at once
void printGameOver() {
  _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  _display.setTextSize(2);
  printWord(10, 20, "GAME");
  printWord(70, 20, "OVER");
}

// Prints "Retry" on top of "Quit"
void printRetryAndQuit() {
  if(_retrySelected) {
    _display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    _display.drawLine(48, 39, 78, 39, SSD1306_WHITE);
    _display.drawLine(48, 40, 48, 47, SSD1306_WHITE);
    _display.drawLine(48, 48, 78, 48, SSD1306_WHITE);
  } else {
    _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  }
  
  _display.setTextSize(1);
  printWord(49, 40, "Retry");

  if(_retrySelected) {
    _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  } else {
    _display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    _display.drawLine(53, 49, 74, 49, SSD1306_WHITE);
    _display.drawLine(53, 50, 53, 57, SSD1306_WHITE);
  }
  
  printQuit(54, 50);
}

// Prints "Quit" to the specified coordinates
void printQuit(uint8_t x, uint8_t y) {
  printWord(x, y, "Qu");
  printThinLetter(x+11, y, 'i');
  _display.print("t");
}

// Prints whole, or parts of, words to the specified
// coordinates (exceptions are words containing the
// letters 'i' and/or 'l')
void printWord(uint8_t x, uint8_t y, char* wordToPrint) {
  _display.setCursor(x, y);
  _display.print(wordToPrint);
}

// Prints thin letters (like 'l' or 'i') with proper
// spacing around them
void printThinLetter(uint8_t x, uint8_t y, char c) {
  _display.setCursor(x, y);
  _display.print(c);
  _display.setCursor(x+5, y);
}

// Same as printThinLetter except used for size 2 font
void printBigThinLetter(uint8_t x, uint8_t y, char c) {
  _display.setCursor(x, y);
  _display.print(c);
  _display.setCursor(x+10, y);
}

// Keeps track of which choice the player has selected
void gameOverChoice() {
  _analogJoystick.read();
  uint16_t upDownVal = _analogJoystick.getUpDownVal();
  int8_t stickDir = map(upDownVal, 0, _analogJoystick.getMaxAnalogValue() + 1, -3, 4);
  //Serial.print(upDownVal);
  //Serial.print(",");
  //Serial.println(stickDir);
  if(stickDir > 0) {
   _retrySelected = true;
  } else if(stickDir < 0) {
   _retrySelected = false;
  }
  Serial.print("_retrySelected = ");
  Serial.println(_retrySelected);
}

void playingGame() {
  // Move all valid shots
  updateShots();

  // Check if any shots OOB
  checkShotsOOB();

  // Perform player-specific operations
  playerOperations();

  // Perform enemy-specific operations
  enemyOperations();

  if(_gameState == GAMEOVER) {
    delay(500);
    digitalWrite(VIBROMOTOR_OUTPUT_PIN, LOW);
    delay(500);
    _twoHealth.resetBrightness();
    _oneHealth.resetBrightness();
    _zeroHealth.resetBrightness();
    _twoHealth.lightLED();
    _oneHealth.lightLED();
    _zeroHealth.lightLED();
  }
}

// Performs all player-based operations during the game
void playerOperations() {
  // If the player isn't invincible...
  if(!_playerInv) {
    // Check if player collides with enemy shots
    checkPlayerCollisions();

    // Check if shielding
    _playerShield = (digitalRead(SHIELD_BUTTON_INPUT_PIN) == HIGH);

    // Check if shooting
    _playerShooting = (digitalRead(SHOOT_BUTTON_INPUT_PIN) == HIGH);
  }

  // Move the player's ship
  movePlayerShip();

  // If the player should be visible, draw the
  // player's ship
  if(_playerVisible) {
    drawShip(_playerShip);
  }

  // If the player's shielding, draw the player's
  // shield
  if(_playerShield) {
    drawShield();
    setVibromotor(5, 50);
  }

  // If the player's shooting, make new shots
  if(_playerShooting) {
    playerShoot();
    setVibromotor(3, 70);
  }

  // Draw the player's shots
  drawShots();

  // If the player should be invincible, do stuff
  // related to that
  if(_playerInv) {
    playerInvincibility();
  }

  updateHealthLEDs(_playerShip.hlth);
  updateVibromotor();

  _twoHealth.lightLED();
  _oneHealth.lightLED();
  _zeroHealth.lightLED();
}

void updateHealthLEDs(uint8_t health) {
  if(health == 0) {
    //Serial.println("health is zero");
    _oneHealth.resetBrightness();
    _twoHealth.resetBrightness();
    
    _zeroHealth.updateBrightness();
  }
  if(health <= 1) {
    //Serial.println("health is one");
    _oneHealth.updateBrightness();
  }
  if(health <= 2) {
    //Serial.println("health is two");
    _twoHealth.updateBrightness();
  }

  /*
  Serial.print("LED Brightness: ");
  Serial.print(_twoHealth._brightness);
  Serial.print(",");
  Serial.print(_oneHealth._brightness);
  Serial.print(",");
  Serial.println(_zeroHealth._brightness);
  */
}

// Updates the vibromotor
void updateVibromotor() {
  if(millis() - _motorStartTs >= _vibrationDuration) {
    _vibrating = false;
  }
  
  if(_vibrating) {
    analogWrite(VIBROMOTOR_OUTPUT_PIN, _vibrationStrength);
    //Serial.println("Vibrating");
  } else {
    digitalWrite(VIBROMOTOR_OUTPUT_PIN, LOW);
    //Serial.println("Not vibrating");
  }
}

// Stuff related to the player being invincible
// (makes the player flash on screen)
void playerInvincibility() {
  unsigned long curTs = millis();
  if(curTs - _playerInvStartTs >= PLAYER_INV_COOLDOWN) {
    _playerInv = false;
    _playerVisible = true;
  } else {
    _playerVisible = !_playerVisible;
  }
}

// Performs all enemy-based operations during the game
void enemyOperations() {
  // Get the current timestamp
  unsigned long curTs = millis();

  // Check if an enemy should spawn
  checkEnemySpawn(curTs);

  // Check which enemies are shooting
  checkEnemiesShooting(curTs);

  // Check if any enemies are being hit
  checkAllEnemyCollisions();

  // Draw all of the enemies
  drawEnemies();
}

// Checks if an enemy is supposed to spawn, and
// spawns one if they are
void checkEnemySpawn(unsigned long curTs) {
  unsigned long curDiff = curTs - _enemySpawnTs;
  unsigned long cooldownDiff = (ENEMY_SPAWN_COOLDOWN - ((curTs - _gameStartTs) / 10));
  if(cooldownDiff > ENEMY_SPAWN_COOLDOWN) {
    cooldownDiff = 0;
  }
  if(curDiff >= cooldownDiff) {
    for(int i = 0; i < MAX_ENEMIES; i++) {
      if(_enemyShips[i].side == INVALID) {
        spawnEnemy(&_enemyShips[i]);
        _enemySpawnTs = curTs;
        return;
      }
    }
  }
}

const uint8_t ENEMY_WIDTH = 5;    // Half the height of each enemy
const uint8_t ENEMY_HEIGHT = 7;   // The width of each enemy

// Spawns an enemy
void spawnEnemy(EnemyShip *enemy) {
  //Serial.println("Spawning enemy!");
  int8_t xPos = (random() % (64 - ENEMY_WIDTH)) + 64;
  int8_t yPos = (random() % 64);
  enemy->x = xPos;
  enemy->y = yPos;
  enemy->wdth = ENEMY_WIDTH;
  enemy->hlfHght = ENEMY_HEIGHT;
  enemy->clDwn = ENEMY_SHOT_COOLDOWN + ((random() % 200) - 100);
  //Serial.print("Cooldown = ");
  //Serial.println(enemy->clDwn);
  enemy->hlth = 1;
  enemy->side = ENEMY;
}

// Draws all valid enemies
void drawEnemies() {
  for(int i = 0; i < MAX_ENEMIES; i++) {
    if(_enemyShips[i].side != INVALID) {
      drawShip(_enemyShips[i]);
    }
  }
}

// Checks which enemies should be shooting
void checkEnemiesShooting(unsigned long curTs) {
  for(int i = 0; i < MAX_ENEMIES; i++) {
    if(_enemyShips[i].side != INVALID && _enemyShips[i].cooldownOver(curTs)) {
      enemyShoot(&_enemyShips[i]);
      _enemyShips[i].lastTs = curTs;
      //Serial.print("Enemy ");
      //Serial.print(i);
      //Serial.println(" is shooting!");
    }
  }
}

// Makes a new shot in front of the given enemy
void enemyShoot(EnemyShip *enemy) {
  for(int i = 0; i < MAX_ENEMY_SHOTS; i++) {
    if(!_enemyShots[i].valid) {
      _enemyShots[i] = {enemy->x, enemy->y, 3, 2, 0, ENEMY, true};
      return;
    }
  }
}

// Checks if any enemies are being hit
void checkAllEnemyCollisions() {
  for(int i = 0; i < MAX_ENEMIES; i++) {
    if(_enemyShips[i].side != INVALID) {
      checkEnemyCollisions(&_enemyShips[i]);
    }
  }
}

// Checks if the given enemy is being hit
bool checkEnemyCollisions(EnemyShip *ship) {
  for(int i = 0; i < MAX_PLAYER_SHOTS; i++) {
    if(ship->shotCollides(&_playerShots[i])) {
      //Serial.println("Shot collides with ship");
      ship->side = INVALID;
      _playerShots[i].makeInvalid();
    }
  }
}

// Move the player's ship in the direction they hold the joystick
void movePlayerShip() {
  // Read analog joystick to control player's ship
  _analogJoystick.read();
  int upDownVal = _analogJoystick.getUpDownVal();
  int leftRightVal = _analogJoystick.getLeftRightVal();

  int multi = 2;

  // Player will move slower with their shield up
  if(_playerShield) {
    multi = 1;
  }
  
  // Translate joystick movement into amount of pixels to move
  int yMovementPixels = -multi * map(upDownVal, 0, _analogJoystick.getMaxAnalogValue() + 1, -1, 2);
  int xMovementPixels = multi * map(leftRightVal, 0, _analogJoystick.getMaxAnalogValue() + 1, -1, 2);

  // Move the player
  _playerShip.updateLoc(xMovementPixels, yMovementPixels);

  moveBackToBounds();
}

// Move the player back into the bounds of the screen if they
// try to move outside it
void moveBackToBounds() {
  if(_playerShip.x >= SCREEN_WIDTH) {
    _playerShip.x = SCREEN_WIDTH;
  } else if(_playerShip.x - _playerShip.wdth <= 0) {
    _playerShip.x = _playerShip.wdth;
  }
  if(_playerShip.y + _playerShip.hlfHght >= SCREEN_HEIGHT) {
    _playerShip.y = SCREEN_HEIGHT - _playerShip.hlfHght;
  } else if(_playerShip.y - _playerShip.hlfHght <= 0) {
    _playerShip.y = _playerShip.hlfHght;
  }
}

// Check if the player is touching any enemy shots or enemies
void checkPlayerCollisions() {
  //Serial.println("Checking player collisions");
  if(_playerShield) {
    for(int i = 0; i < MAX_ENEMY_SHOTS; i++) {
      if((_playerShip.wdth / 2) + 6 >= distance(_enemyShots[i].x, _enemyShots[i].y,
                                               _playerShip.x - (_playerShip.wdth/2),
                                               _playerShip.y)) {
        _enemyShots[i].valid = false;
      }
    }
  } else {
    for(int i = 0; i < MAX_ENEMY_SHOTS; i++) {
      if(_playerShip.shotCollides(&_enemyShots[i])) {
        //Serial.println("Shot collides with player");
        //Serial.print("Player health = ");
        //Serial.println(_playerShip.hlth);

        _playerInvStartTs = millis();
        _playerInv = true;

        setVibromotor(400, 200);
      }
    }
  }

  for(int i = 0; i < MAX_ENEMIES; i++) {
    if(shipsCollide(_playerShip, _enemyShips[i])) {
      _playerInvStartTs = millis();
      _playerInv = true;
      _playerShip.hlth -= 1;
    }
  }

  if(playerDead()) {
    _gameState = GAMEOVER;
  }
}

// Set the strength of the vibromotor and how long
// it vibrates
void setVibromotor(uint16_t duration, uint8_t strength) {
  _vibrating = true;
  _vibrationDuration = duration;
  _vibrationStrength = strength;
  _motorStartTs = millis();
}

// Checks to see if two ships are touching
bool shipsCollide(PlayerShip player, EnemyShip enemy) {
  return ((player.x > enemy.x) && (player.x - player.wdth < enemy.x + enemy.wdth)) &&
         ((player.y + player.hlfHght > enemy.y - enemy.hlfHght) && (player.y - player.hlfHght < enemy.y + enemy.hlfHght));
}

// Checks to see if the player has run out
// of health
bool playerDead() {
  return _playerShip.hlth <= 0;
}

// Checks the distance between two points
float distance(int x1, int y1, int x2, int y2){
  return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

// Checks to see if any shots (player's and
// enemies') are OOB
void checkShotsOOB() {
  for(int i = 0; i < MAX_PLAYER_SHOTS; i++) {
    if(_playerShots[i].checkOOB()) {
      _playerShots[i].valid = false;
    }
  }

  for(int i = 0; i < MAX_ENEMY_SHOTS; i++) {
    if(_enemyShots[i].checkOOB()) {
      _enemyShots[i].valid = false;
    }
  }
}

// Creates a shot at the player's location
void playerShoot() {
  unsigned long curTs = millis();
  if(_playerShip.cooldownOver(curTs) && !_playerShield) {
    for(int i = 0; i < MAX_PLAYER_SHOTS; i++) {
      if(!_playerShots[i].valid) {
        _playerShots[i] = {_playerShip.x, _playerShip.y, 5, 3, 0, PLAYER, true};
        _playerShip.lastTs = curTs;
        //Serial.println("Shooting!");
        return;
      }
    }
  }
}

// Updates all valid shots
void updateShots() {
  for(int i = 0; i < MAX_PLAYER_SHOTS; i++) {
    if(_playerShots[i].valid) {
      _playerShots[i].updateLoc();
    }
  }

  for(int i = 0; i < MAX_ENEMY_SHOTS; i++) {
    if(_enemyShots[i].valid) {
      _enemyShots[i].updateLoc();
    }
  }
}

// Draws the given ship
void drawShip(ship_t ship) {
  _display.fillTriangle(ship.x, ship.y,
             ship.x - ship.side * ship.wdth, ship.y + ship.hlfHght,
             ship.x - ship.side * ship.wdth, ship.y - ship.hlfHght,
             SSD1306_WHITE);
}

// Draws all valid shots
void drawShots() {
  for(int i = 0; i < MAX_PLAYER_SHOTS; i++) {
    if(_playerShots[i].valid) {
      //Serial.print(_playerShots[i].x);
      //Serial.print(",");
      //Serial.println(_playerShots[i].y);
      _display.drawLine(_playerShots[i].x, _playerShots[i].y, _playerShots[i].x + _playerShots[i].len, _playerShots[i].y, SSD1306_WHITE);
    }
  }

  for(int i = 0; i < MAX_ENEMY_SHOTS; i++) {
    if(_enemyShots[i].valid) {
      _display.drawLine(_enemyShots[i].x, _enemyShots[i].y, _enemyShots[i].x + _enemyShots[i].len, _enemyShots[i].y, SSD1306_WHITE);
    }
  }
}

// Draws the player's shield
void drawShield() {
  _display.drawCircle(_playerShip.x - (_playerShip.wdth / 2), _playerShip.y, (_playerShip.wdth / 2) + 6, SSD1306_WHITE);
}

// Initializes the OLED screen
void initializeOledAndShowStartupScreen(){
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Clear the screen
  _display.clearDisplay();
}
