let pHtmlMsg;
let pHtmlMsgPlayerStats;
let pHtmlMsgRoom;
let pHtmlMsgGameState;


let serialOptions = { baudRate: 230400 };
let serial;
let lastSentSerial = "";

const DEBUG = true;

// The canvas width and height
const canvasWidth = 750;
const canvasHeight = 480;

// offscreen buffer
let offscreenGfxBuffer;

// Sound classifier
let classifier;
// Options for the SpeechCommands18w model
const soundOptions = { probabilityThreshold: 0.9 };
// Top result
let topSoundResult;
// The previous result
let prevSoundResult = "";
// Results array for classifier
let results = [];


// States of the game
const states = {
  WAITING:  "waiting",
  GAME:     "game"
}

const halfDoorWidth = 60;
const wallWidth = 30;

// States the player can be in
const playStates = {
  CUTSCENE: "cutscene",  // Player cannot do anything during a "cutscene"
  EXPLORE: "exploring",
  PUZZLE: "puzzle"
}

const exploreStates = {
  // While exploring, the player could be walking, standing still, attacking,
  // or blocking
  WALK:   "walk",
  STILL:  "still",
  ATTACK: "attack",
  BLOCK:  "block"
}

const directions = {
  UP:     "up",
  DOWN:   "down",
  LEFT:   "left",
  RIGHT:  "right"
}

// Game object
let game = {
  state: states.WAITING,    // The game's current state
}

// Player object
let player = {
  state: playStates.CUTSCENE,    // The player's current state
  expState: exploreStates.STILL, // The player's state if they are exploring
  dir: directions.DOWN,          // The direction the player is facing
  x: canvasWidth / 2,            // The location of the player along the x-axis
  y: canvasHeight / 2,           // The location of the player along the y-axis
  xSpd: 0,                       // The player's speed along the x-axis
  ySpd: 0                        // The player's speed along the y-axis
}

let playerMaxSpeed = 4;

// Stuff for the console
const consoleSide = 60;
const puzzleConsole = {x1: (canvasWidth - consoleSide) / 2,
                       x2: (canvasWidth + consoleSide) / 2,
                       y1: ((canvasHeight - consoleSide) / 2),
                       y2: ((canvasHeight + consoleSide) / 2)};

// Stuff for OLED screen
const oledRoomSide = 14;
const oledWallWidth = 2;
const oledScreenWidth = 128;
const oledScreenHeight = 64;

let gameRooms = [];
let puzzleAnswer = [];
let roomsMap = new Map();

let lastDoorTraversed = "";
let curDoorTraversed = "";

let correctGuesses = 0;
let correctSoundGuesses = 0;
const numRooms = 5;

let puzzleText = "How did you get here?";

function setup() {
  // Create the canvas
  createCanvas(canvasWidth, canvasHeight);

  // Setup Web Serial using serial.js
  serial = new Serial();
  serial.on(SerialEvents.CONNECTION_OPENED, onSerialConnectionOpened);
  serial.on(SerialEvents.CONNECTION_CLOSED, onSerialConnectionClosed);
  serial.on(SerialEvents.DATA_RECEIVED, onSerialDataReceived);
  serial.on(SerialEvents.ERROR_OCCURRED, onSerialErrorOccurred);

  if (DEBUG) {
    // Add in <p> elements to provide debug messages
    pHtmlMsg = createP("Click anywhere on this page to open the serial connection dialog and begin!");

    pHtmlMsgPlayerStats = createP("Player Stats");

    pHtmlMsgRoom = createP("Room");

    pHtmlMsgGameState = createP("Game State");
  }

  let curX = oledScreenWidth / 2;
  let curY = oledScreenHeight / 2;

  let colors = [[200,0,0], [0,200,0], [0,0,200], [200,200,0], [200,0,200],
                [0,200,200], [200,200,200], [200,130,0], [130,200,0],
                [200,0,130], [130,0,200], [0,200,130], [0,130,200]];

  // Get random directions for the answer
  for (let i = 0; i < numRooms; i++) {
    let r = getRandomInt(4);
    let c = random(colors);
    colors = removeFromArray(c, colors);
    if (r == 0) {
      if (i != numRooms - 1) { puzzleAnswer[i] = directions.UP; }
    } else if (r == 1) {
      if (i != numRooms - 1) { puzzleAnswer[i] = directions.DOWN; }
    } else if (r == 2) {
      if (i != numRooms - 1) { puzzleAnswer[i] = directions.LEFT; }
    } else {
      if (i != numRooms - 1) { puzzleAnswer[i] = directions.RIGHT; }
    }
    gameRooms[i] = {id: "room " + (i + 1), x: curX, y: curY};
    roomsMap.set("room " + (i + 1), makeRoom(c));

    if (r == 0) {
      curY -= oledRoomSide - oledWallWidth;
    } else if (r == 1) {
      curY += oledRoomSide - oledWallWidth;
    } else if (r == 2) {
      curX -= oledRoomSide - oledWallWidth;
    } else {
      curX += oledRoomSide - oledWallWidth;
    }

    if (DEBUG) {
      console.log(gameRooms[i]);
    }
  }
  roomsMap.delete("room " + numRooms);
  roomsMap.set("room " + numRooms, makeRoom([200, 0, 200])); // Final Room
  
  offscreenGfxBuffer = createGraphics(width, height);
  swapRoomBuffer();
}

function draw() {
  updateStates();

  switch (game.state) {
    case states.WAITING:
      drawWaiting();
      break;
    case states.GAME:
      drawGame();
      break;
  }

  if (DEBUG) {
    pHtmlMsgPlayerStats.html("Player: [" + player.x + ", " + player.y + "], " + player.state);
    pHtmlMsgRoom.html("Room: " + gameRooms[correctGuesses].id);
    pHtmlMsgGameState.html("Game State: " + game.state);
  }
}

function removeFromArray(c, colors) {
  for (let i = 0; i < colors.length; i++) {
    let curColor = colors[i];
    if (c[0] == curColor[0] && c[1] == curColor[1] && c[2] == curColor[2]) {
      colors.splice(i,1);
    }
  }
  return colors;
}

function getRandomInt(max) {
  return Math.floor(Math.random() * max);
}

function updateStates() {
  if (player.state == playStates.EXPLORE && player.expState == exploreStates.WALK) {
    // Update the player's coordinates
    setPlayerCoords(player.x + player.xSpd, player.y + player.ySpd);
    moveBackToBounds();
  }
  
  if (lastDoorTraversed != "") {
    if (DEBUG) {
      console.log(lastDoorTraversed);
    }
    
    lastDoorTraversed = "";
    swapRoomBuffer();
  }

  if (playerNearConsole() && player.expState == exploreStates.ATTACK) {
    player.state = playStates.PUZZLE;
  }

  if (player.state == playStates.PUZZLE) {
    if (classifier == undefined) {
      // Load SpeechCommands18w sound classifier model
      classifier = ml5.soundClassifier('SpeechCommands18w', soundOptions, soundModelReady);
    }
      
    puzzleOperations();

    if (correctSoundGuesses == puzzleAnswer.length) {
      // Player wins!
      puzzleText = puzzleText + "\nYOU WIN!";
      drawPuzzleScreen();
      player.state = playStates.CUTSCENE;
    }
  }

  if (player.state == playStates.PUZZLE && player.expState == exploreStates.BLOCK) {
    classifier = undefined;

    // Let player move again
    player.state = playStates.EXPLORE;
    player.expState = exploreStates.STILL;
    swapRoomBuffer();
  }
}

function refreshScreen() {
  drawRoom();
  drawPlayer();
}

function drawWaiting() {
  // Draw the waiting screen
}

function drawMenu() {
  // Draw the menu screen
  game.state = states.GAME;
  player.state = playStates.EXPLORE;
  player.expStates = exploreStates.STILL;
}

function drawGame() {
  // Draw stuff for the game
  switch (player.state) {
    case playStates.PUZZLE:
      drawPuzzleScreen();
      break;
    case playStates.EXPLORE:
      drawDungeon();
      break;
    case playStates.CUTSCENE:
      // Cutscene stuff (nothing)
      break;
  }
}

function drawPuzzleScreen() {
  refreshScreen();
  drawPuzzleBackground();
  printPuzzleText(puzzleText, width / 2, height / 2, CENTER,
                  roomsMap.get("room " + correctGuesses).wallColor());
}

function printPuzzleText(pText, x, y, alignment, tColor) {
  fill(tColor);
  textSize(20);
  textAlign(alignment);
  text(pText, x, y);
}

function puzzleOperations() {
  // Only aknowledge results that sound like directions
  if (topSoundResult != undefined && isADirection(topSoundResult.label)) {
    if (topSoundResult.label == puzzleAnswer[correctSoundGuesses]) {
      if (topSoundResult.confidence >= 0.98) {
        if (correctSoundGuesses == 0) {
          puzzleText = topSoundResult.label;
        } else {
          puzzleText = puzzleText + ", " + topSoundResult.label;
        }
        correctSoundGuesses++;
      } else {
        if (DEBUG) {
          console.log("Hmm... you're close but I can't be sure you said it correctly.");
        }
      }
    } else if (topSoundResult.confidence >= 0.98) {
      puzzleText = topSoundResult.label + " is wrong, try again.";
      correctSoundGuesses = 0;
    }

    topSoundResult = undefined;
  }
}

function isADirection(soundLabel) {
  return soundLabel == directions.UP || soundLabel == directions.DOWN ||
         soundLabel == directions.LEFT || soundLabel == directions.RIGHT;
}

function drawPuzzleBackground() {
  fill(255);
  rect(100, 100, width - 200, height - 200);
}

function drawDungeon() {
  if (playerThroughDoor()) {
    if (correctGuesses < puzzleAnswer.length && lastDoorTraversed == puzzleAnswer[correctGuesses]) {
      if (DEBUG) {
        console.log("guessed correctly");
      }
      correctGuesses++;
    } else {
      if (DEBUG) {
        console.log("guessed wrong");
      }
      correctGuesses = 0;
    }
    serialWriteRoomData(correctGuesses);
  }

  refreshScreen();
  if (correctGuesses == puzzleAnswer.length) {
    drawPuzzleConsole();
  }
}

function drawRoom() {
  image(offscreenGfxBuffer, 0, 0);
}

function swapRoomBuffer() {
  // Get the room
  let room = roomsMap.get("room " + (correctGuesses + 1));

  // Draw floor
  offscreenGfxBuffer.background(room.floorColor());
  offscreenGfxBuffer.fill(room.wallColor());
  drawOuterWall();

  // Draw doorway(s)
  offscreenGfxBuffer.fill(room.floorColor());
  drawTopDoorway();
  drawBottomDoorway();
  drawLeftDoorway();
  drawRightDoorway();
}

function drawOuterWall() {
  offscreenGfxBuffer.noStroke();
  offscreenGfxBuffer.rect(0, 0, width, wallWidth);
  offscreenGfxBuffer.rect(0, 0, wallWidth, height);
  offscreenGfxBuffer.rect(width - wallWidth, 0, wallWidth, height);
  offscreenGfxBuffer.rect(0, height - wallWidth, width, wallWidth);
}

function drawTopDoorway() {
  offscreenGfxBuffer.noStroke();
  offscreenGfxBuffer.rect((width/2) - halfDoorWidth, 0, 2*halfDoorWidth, wallWidth);
}

function drawBottomDoorway() {
  offscreenGfxBuffer.noStroke();
  offscreenGfxBuffer.rect((width/2) - halfDoorWidth, height - wallWidth, 2*halfDoorWidth, wallWidth);
}

function drawLeftDoorway() {
  offscreenGfxBuffer.noStroke();
  offscreenGfxBuffer.rect(0, (height/2) - halfDoorWidth, wallWidth, 2*halfDoorWidth);
}

function drawRightDoorway() {
  offscreenGfxBuffer.noStroke();
  offscreenGfxBuffer.rect(width - wallWidth, (height/2) - halfDoorWidth, wallWidth, 2*halfDoorWidth);
}

function playerThroughDoor() {
  if (playerThroughTopDoor()) {
    lastDoorTraversed = directions.UP;
    return changeRoom(player.x, canvasHeight - wallWidth);
  } else if (playerThroughBottomDoor()) {
    lastDoorTraversed = directions.DOWN;
    return changeRoom(player.x, wallWidth);
  } else if (playerThroughLeftDoor()) {
    lastDoorTraversed = directions.LEFT;
    return changeRoom(canvasWidth - wallWidth, player.y);
  } else if (playerThroughRightDoor()) {
    lastDoorTraversed = directions.RIGHT;
    return changeRoom(wallWidth, player.y)
  }
  return false;
}

function changeRoom(x, y) {
  setPlayerCoords(x, y);
  return true;
}

function playerThroughTopDoor() {
  return player.y == 0 &&
         player.x >= (canvasWidth/2) - halfDoorWidth &&
         player.x <= (canvasWidth/2) + halfDoorWidth;
}

function playerThroughBottomDoor() {
  return player.y == canvasHeight &&
         player.x >= (canvasWidth/2) - halfDoorWidth &&
         player.x <= (canvasWidth/2) + halfDoorWidth;
}

function playerThroughLeftDoor() {
  return player.x == 0 &&
         player.y >= (canvasHeight/2) - halfDoorWidth &&
         player.y <= (canvasHeight/2) + halfDoorWidth;
}

function playerThroughRightDoor() {
  return player.x == canvasWidth &&
         player.y >= (canvasHeight/2) - halfDoorWidth &&
         player.y <= (canvasHeight/2) + halfDoorWidth;
}

function drawPuzzleConsole() {
  offscreenGfxBuffer.stroke(150)
  offscreenGfxBuffer.fill(100);
  offscreenGfxBuffer.rect(puzzleConsole.x1, puzzleConsole.y1, consoleSide, consoleSide, 10);
}

function drawPlayer() {  
  // Set the color of the player
  switch (player.expState) {
    case exploreStates.ATTACK:
      fill(255, 0, 0);
      break;
    case exploreStates.BLOCK:
      fill(0, 0, 255);
      break;
    case exploreStates.WALK:
      fill(0, 255, 0);
      break;
    case exploreStates.STILL:
      fill(150, 0, 150);
      break;
  }

  let plyrHlfW = 8;
  let plyrHlfH = 16;

  // draw the player
  switch (player.dir) {
    case directions.DOWN:
      triangle(player.x, player.y - plyrHlfH,
               player.x - plyrHlfW, player.y + plyrHlfH,
               player.x + plyrHlfW, player.y + plyrHlfH);
      break;
    case directions.UP:
      triangle(player.x, player.y + plyrHlfH,
               player.x - plyrHlfW, player.y - plyrHlfH,
               player.x + plyrHlfW, player.y - plyrHlfH);
      break;
    case directions.LEFT:
      triangle(player.x - plyrHlfH, player.y,
               player.x + plyrHlfH, player.y - plyrHlfW,
               player.x + plyrHlfH, player.y + plyrHlfW);
      break;
    case directions.RIGHT:
      triangle(player.x + plyrHlfH, player.y,
               player.x - plyrHlfH, player.y - plyrHlfW,
               player.x - plyrHlfH, player.y + plyrHlfW);
      break;
  }
}

function setPlayerCoords(x, y) {
  player.x = x;
  player.y = y;
}

function moveBackToBounds() {
  // Move the player back inside the bounds of the screen
  if (playerInDoorway()) {
    if (player.x < 0) {
      player.x = 0;
    } else if (player.x > canvasWidth) {
      player.x = canvasWidth;
    }
    if (player.y < 0) {
      player.y = 0;
    } else if (player.y > canvasHeight) {
      player.y = canvasHeight;
    }
  } else if (playerInConsole()) {
    moveOutOfConsole(puzzleConsole.x1, puzzleConsole.x2, puzzleConsole.y1, puzzleConsole.y2);
  } else {
    if (player.x < wallWidth) {
      player.x = wallWidth;
    } else if (player.x > canvasWidth - wallWidth) {
      player.x = canvasWidth - wallWidth;
    }
    if (player.y < wallWidth) {
      player.y = wallWidth;
    } else if (player.y > canvasHeight - wallWidth) {
      player.y = canvasHeight - wallWidth;
    }
  }
}

function moveOutOfConsole(x1, x2, y1, y2) {
  switch (player.dir) {
    case directions.UP:
      setPlayerCoords(player.x, y1);
      break;
    case directions.DOWN:
      setPlayerCoords(player.x, y2);
      break;
    case directions.LEFT:
      setPlayerCoords(x2, player.y);
      break;
    case directions.RIGHT:
      setPlayerCoords(x1, player.y);
      break;
  }
}

function playerInDoorway() {
  return playerInRightDoorway() || playerInLeftDoorway() ||
         playerInTopDoorway() || playerInBottomDoorway();
}

function playerInTopDoorway() {
  return playerInProximity((canvasWidth/2) - halfDoorWidth,
                           (canvasWidth/2) + halfDoorWidth,
                           -10, wallWidth);
}

function playerInBottomDoorway() {
  return playerInProximity((canvasWidth/2) - halfDoorWidth,
                           (canvasWidth/2) + halfDoorWidth,
                           canvasHeight - wallWidth, canvasHeight + 10);
}

function playerInLeftDoorway() {
  return playerInProximity(-10, wallWidth,
                           (canvasHeight/2) - halfDoorWidth,
                           (canvasHeight/2) + halfDoorWidth);
}

function playerInRightDoorway() {
  return playerInProximity(canvasWidth - wallWidth, canvasWidth + 10,
                           (canvasHeight/2) - halfDoorWidth,
                           (canvasHeight/2) + halfDoorWidth);
}

function playerNearConsole() {
  if (correctGuesses == numRooms - 1) {
    return playerInProximity(((canvasWidth - consoleSide) / 2) - 20, 
                             ((canvasWidth + consoleSide) / 2) + 20,
                             ((canvasHeight - consoleSide) / 2) - 20,
                             ((canvasHeight + consoleSide) / 2) + 20);
  }
  return false;
}

function playerInConsole() {
  if (correctGuesses == numRooms - 1) {
    return playerInProximity(puzzleConsole.x1 + 1, puzzleConsole.x2 - 1,
                             puzzleConsole.y1 + 1, puzzleConsole.y2 - 1);
  }
  return false;
}

function playerInProximity(minX, maxX, minY, maxY) {
  return player.x <= maxX && player.x >= minX &&
         player.y <= maxY && player.y >= minY;
}

/**
 * Callback function by serial.js when there is an error on web serial
 * 
 * @param {} eventSender 
 */
 function onSerialErrorOccurred(eventSender, error) {
  console.log("onSerialErrorOccurred", error);
  
  if (DEBUG) {
    pHtmlMsg.html(error);
  }

  resetGame();
}

/**
 * Callback function by serial.js when web serial connection is opened
 * 
 * @param {} eventSender 
 */
function onSerialConnectionOpened(eventSender) {
  console.log("onSerialConnectionOpened");

  if (DEBUG) {
    pHtmlMsg.html("Serial connection opened successfully");
  }

  game.state = states.GAME;
  player.state = playStates.EXPLORE;
  player.expStates = exploreStates.STILL;
}

/**
 * Callback function by serial.js when web serial connection is closed
 * 
 * @param {} eventSender 
 */
function onSerialConnectionClosed(eventSender) {
  console.log("onSerialConnectionClosed");

  if (DEBUG) {
    pHtmlMsg.html("onSerialConnectionClosed");
  }

  game.state = states.WAITING;
  player.state = playStates.CUTSCENE;
  resetGame();
}

/**
 * Callback function serial.js when new web serial data is received
 * 
 * @param {*} eventSender 
 * @param {String} newData new data received over serial
 */
function onSerialDataReceived(eventSender, newData) {
  if (!newData.startsWith("#")) {
    console.log("onSerialDataReceived", newData);

    if (DEBUG) {
      pHtmlMsg.html("onSerialDataReceived: " + newData);
    }

    let data = split(newData, ",");
    let xSpd = parseFloat(data[0]);
    let ySpd = -parseFloat(data[1]);

    // Get the player's state
    if (data[2] != "") {
      player.expState = data[2];
    } else if (xSpd == 0 && ySpd == 0) {
      player.expState = exploreStates.STILL;
    } else {
      player.expState = exploreStates.WALK;
    }
    // Set the player's speed
    player.xSpd = Math.round(xSpd * playerMaxSpeed);
    player.ySpd = Math.round(ySpd * playerMaxSpeed);

    // Determine the player's direction
    if (xSpd != 0 || ySpd != 0) {
      if (Math.abs(ySpd) >= Math.abs(xSpd)) {
        if (ySpd < 0) {
          player.dir = directions.DOWN;
        } else if (ySpd > 0) {
          player.dir = directions.UP;
        }
      } else {
        if (xSpd < 0) {
          player.dir = directions.LEFT;
        } else if (xSpd > 0) {
          player.dir = directions.RIGHT;
        }
      }
    }
  } else {
    if (DEBUG) {
      pHtmlMsgArduino.html("Arduino received: " + newData);
    }
  }
}

/**
 * Called automatically by the browser through p5.js when mouse clicked
 */
function mouseClicked() {
  if (game.state == states.WAITING && !serial.isOpen()) {
    serial.connectAndOpen(null, serialOptions);
  }
}

async function serialWriteRoomData(curRoom) {
  if (serial.isOpen()) {
    let room = gameRooms[curRoom];
    
    let strData = "cur: " + room.id + "," + curRoom + ";" + room.x + "," + room.y;
    serial.writeLine(strData);
    console.log(strData);
  }
}

function resetGame() {
  // Game object
  game.state = states.WAITING;

  // Player object
  player.state = playStates.CUTSCENE;
  player.expState = exploreStates.STILL;
  player.dir = directions.DOWN;
  player.x = canvasWidth / 2;
  player.y = canvasHeight / 2;
  player.xSpd = 0;
  player.ySpd = 0;
}

function makeRoom(desiredColor) {
  return {red: desiredColor[0],
          blu: desiredColor[1],
          grn: desiredColor[2],
          floorColor: function() {
            return color(this.red, this.blu, this.grn);
          },
          wallColor: function() {
            return color((this.red / 2), (this.blu / 2), (this.grn / 2));
          }
         };
}

function soundModelReady() {
  // classify sound if doing correct puzzle
  classifier.classify(gotSoundResult);
  console.log("Sound model ready")
}

function gotSoundResult(error, result) {
  if (player.state == playStates.PUZZLE) {
    console.log("here");
    if (error) {
      console.log(error);
      return;
    }

    if (DEBUG) {
      // log the result
      console.log("result: " + result[0].label + ", confidence: " + result[0].confidence);
    }
    topSoundResult = result[0];
  }
}
