let pHtmlMsg;
let pHtmlMsgPlayerStats;
let pHtmlMsgRoom;
let pHtmlMsgGameState;
let pHtmlMsgArduino;
let serialOptions = { baudRate: 115200 };
let serial;

let canvasWidth = 750;
let canvasHeight = 480;

let offscreenGfxBuffer;
let visitedRooms = new Set();
let lastSentSerial = "";

// Options for the SpeechCommands18w model, the default probabilityThreshold is 0
const options = { probabilityThreshold: 0.7 };
let classifier;// = ml5.soundClassifier('path/to/model.json', options, modelReady);

// States of the game
const states = {
  WAITING:  "waiting",
  MENU:     "menu",
  GAME:     "game",
  GAMEOVER: "gameover",
}

// Areas the player can go to
const areas = {
  AREA0: {
    ROOM1: "room 0-1",
    ROOM2: "room 0-2"
  },
  AREA1: {
    ROOM1: "room 1-1",
    ROOM2: "room 1-2"
  },
  AREA2: {
    ROOM1: "room 2-1",
    ROOM2: "room 2-2"
  }
}

// States the player can be in
const playStates = {
  CUTSCENE: "cutscene",  // Player cannot do anything during a "cutscene"
  EXPLORE: {
    // While exploring, the player could be walking, standing still, attacking,
    // or blocking
    WALK:   "walk",
    STILL:  "still",
    ATTACK: "attack",
    BLOCK:  "block"
  },
  PUZZLE: {
    // Dictates the behavior of buttons when in certain puzzles
    PUZZLE1: "puzzle 1",
    PUZZLE2: "puzzle 2"
  }
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
  area: areas.AREA0.ROOM1   // The area and room the player is in
}

// Player object
let player = {
  state: playStates.CUTSCENE,   // The player's current state
  dir: directions.DOWN,         // The direction the player is facing
  x: canvasWidth / 2,           // The location of the player along the x-axis
  y: canvasHeight / 2,          // The location of the player along the y-axis
  xSpd: 0,                      // The player's speed along the x-axis
  ySpd: 0                       // The player's speed along the y-axis
}

let playerMaxSpeed = 4;

function setup() {
  createCanvas(canvasWidth, canvasHeight);

  // Setup Web Serial using serial.js
  serial = new Serial();
  serial.on(SerialEvents.CONNECTION_OPENED, onSerialConnectionOpened);
  serial.on(SerialEvents.CONNECTION_CLOSED, onSerialConnectionClosed);
  serial.on(SerialEvents.DATA_RECEIVED, onSerialDataReceived);
  serial.on(SerialEvents.ERROR_OCCURRED, onSerialErrorOccurred);

  classifier = ml5.soundClassifier('SpeechCommands18w', options, modelReady);

  // If we have previously approved ports, attempt to connect with them
  //serial.autoConnectAndOpenPreviouslyApprovedPort(serialOptions);

  // Add in a lil <p> element to provide messages. This is optional
  pHtmlMsg = createP("Click anywhere on this page to open the serial  connection dialog and begin!");

  pHtmlMsgPlayerStats = createP("Player Stats");

  pHtmlMsgRoom = createP("Room");

  pHtmlMsgGameState = createP("Game State");

  pHtmlMsgArduino = createP("Arduino");
  
  offscreenGfxBuffer = createGraphics(width, height);
  swapRoom();
  //offscreenGfxBuffer.background(100); 
}

function draw() {
  switch (game.state) {
    case states.WAITING:
      drawWaiting();
      break;
    case states.MENU:
      drawMenu();
      break;
    case states.GAME:
      //updatePlayState();
      drawGame();
      break;
    case states.GAMEOVER:
      drawGameOver();
      break;
  }

  pHtmlMsgPlayerStats.html("Player: [" + player.x + ", " + player.y + "], " + player.state);
  pHtmlMsgRoom.html("Room: " + game.area);
  pHtmlMsgGameState.html("Game State: " + game.state);
}

function drawWaiting() {
  // Draw the waiting screen
}

function drawMenu() {
  // Draw the menu screen
  game.state = states.GAME;
  player.state = playStates.EXPLORE.STILL;
  console.log("drawMenu reached");
}

function drawGameOver() {
  // Draw the game over screen
}

function drawGame() {
  // Draw stuff for the game
  //clear();

  if (inDoorway()) {
    swapRoom();
    visitedRooms.add(game.area);
  }
  drawRoom();
  drawPlayer();

  serialWriteRoomData();
}

function swapRoom() {
  switch (game.area) {
    case areas.AREA0.ROOM1:
      offscreenGfxBuffer.background(100,0,0);
      break;
    case areas.AREA0.ROOM2:
      offscreenGfxBuffer.background(0,100,0);
      break;
    case areas.AREA1.ROOM1:
      offscreenGfxBuffer.background(0,0,100);
      break;
    case areas.AREA1.ROOM2:
      offscreenGfxBuffer.background(100,0,100);
      break;
    case areas.AREA2.ROOM1:
      offscreenGfxBuffer.background(100,100,0);
      break;
    case areas.AREA2.ROOM2:
      offscreenGfxBuffer.background(0,100,100);
      break;
  }
}

function inDoorway() {
  switch (game.area) {
    case areas.AREA0.ROOM1:
      if (player.y == canvasHeight) {
        game.area = areas.AREA0.ROOM2;
        setPlayerCoords(player.x , 20);
        return true;
      }
      break;
    case areas.AREA0.ROOM2:
      if (player.x == canvasWidth) {
        game.area = areas.AREA1.ROOM1;
        setPlayerCoords(20, player.y);
        return true;
      } else if (player.x == 0) {
        game.area = areas.AREA2.ROOM1;
        setPlayerCoords(canvasWidth - 20, player.y);
        return true;
      } else if (player.y == 0) {
        game.area = areas.AREA0.ROOM1;
        setPlayerCoords(player.x, canvasHeight - 20);
        return true;
      }
      break;
    case areas.AREA1.ROOM1:
      if (player.x == 0) {
        game.area = areas.AREA0.ROOM2;
        setPlayerCoords(canvasWidth - 20, player.y);
        return true;
      } else if (player.x == canvasWidth) {
        game.area = areas.AREA1.ROOM2;
        setPlayerCoords(20, player.y);
        return true;
      }
      break;
    case areas.AREA1.ROOM2:
      if (player.x == 0) {
        game.area = areas.AREA1.ROOM1;
        setPlayerCoords(canvasWidth - 20, player.y);
        return true;
      }
      break;
    case areas.AREA2.ROOM1:
      if (player.x == 0) {
        game.area = areas.AREA2.ROOM2;
        setPlayerCoords(canvasWidth - 20, player.y);
        return true;
      } else if (player.x == canvasWidth) {
        game.area = areas.AREA0.ROOM2;
        setPlayerCoords(20, player.y);
        return true;
      }
      break;
    case areas.AREA2.ROOM2:
      if (player.x == canvasWidth) {
        game.area = areas.AREA2.ROOM1;
        setPlayerCoords(20, player.y);
        return true;
      }
      break;
  }
  return false;
}

function drawRoom() {
  image(offscreenGfxBuffer, 0, 0);
}

// TODO: make a proper drawPlayer function (don't just draw a triangle with
//       different colors)
function drawPlayer() {  
  // Update the player's coordinates
  if (player.state == playStates.EXPLORE.WALK) {
    setPlayerCoords(player.x + player.xSpd, player.y + player.ySpd);
  }

  // Set the color of the player
  switch (player.state) {
    case playStates.EXPLORE.ATTACK:
      fill(255, 0, 0);
      break;
    case playStates.EXPLORE.BLOCK:
      fill(0, 0, 255);
      break;
    case playStates.EXPLORE.WALK:
      fill(0, 255, 0);
      break;
    case playStates.EXPLORE.STILL:
      fill(150, 0, 150);
      break;
  }

  let plyrHlfW = 5;
  let plyrHlfH = 10;

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

/**
 * Callback function by serial.js when there is an error on web serial
 * 
 * @param {} eventSender 
 */
 function onSerialErrorOccurred(eventSender, error) {
  console.log("onSerialErrorOccurred", error);
  pHtmlMsg.html(error);
}

/**
 * Callback function by serial.js when web serial connection is opened
 * 
 * @param {} eventSender 
 */
function onSerialConnectionOpened(eventSender) {
  console.log("onSerialConnectionOpened");
  pHtmlMsg.html("Serial connection opened successfully");

  game.state = states.MENU;
}

/**
 * Callback function by serial.js when web serial connection is closed
 * 
 * @param {} eventSender 
 */
function onSerialConnectionClosed(eventSender) {
  console.log("onSerialConnectionClosed");
  pHtmlMsg.html("onSerialConnectionClosed");

  // Pause the game (or close it maybe)
  game.state = states.WAITING;
  player.state = playStates.CUTSCENE;
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
    pHtmlMsg.html("onSerialDataReceived: " + newData);

    let data = split(newData, ",");
    let xSpd = parseFloat(data[0]) - 0.5;
    let ySpd = -parseFloat(data[1]) + 0.5;

      // Get the player's state
    if (data[2] != "") {
      player.state = data[2];
    } else if (xSpd == 0 && ySpd == 0) {
      player.state = playStates.EXPLORE.STILL;
    } else {
      player.state = playStates.EXPLORE.WALK;
    }
    // Set the player's speed
    player.xSpd = Math.round(xSpd * playerMaxSpeed);
    player.ySpd = Math.round(ySpd * playerMaxSpeed);

    // Move the player back inside the bounds of the screen
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
    pHtmlMsgArduino.html("Arduino received: " + newData);
  }
}

/**
 * Called automatically by the browser through p5.js when mouse clicked
 */
function mouseClicked() {
  switch (game.state) {
    case states.WAITING:
      if (!serial.isOpen()) {
        serial.connectAndOpen(null, serialOptions);
      }
      break;
    case states.MENU:
      // Do stuff for when the mouse is clicked in the menu
      break;
    case states.GAME:
      // Do stuff for when the mouse is clicked in the menu
      break;
    case states.GAMEOVER:
      // Do stuff for when the mouse is clicked in the menu 
      break;
  }
}

async function serialWriteRoomData() {
  if (serial.isOpen()) {
    let strData = "";
    for (let room of visitedRooms) {
      strData = strData + room + ",";
    }
    strData = strData.substring(0, strData.length - 1);
    if (strData != lastSentSerial) {
      serial.writeLine(strData);
      lastSentSerial = strData;
    }
  }
}

function modelReady() {
  // classify sound
  classifier.classify(gotResult);
}

function gotResult(error, result) {
  if (error) {
    console.log(error);
    return;
  }
  // log the result
  //console.log(result);
  console.log(result[0].label);
  
  serial.writeLine("#" + result[0].label);
}
