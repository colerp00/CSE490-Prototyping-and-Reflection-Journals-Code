let pHtmlMsg;
let pHtmlMsgPlayerStats;
let pHtmlMsgRoom;
let pHtmlMsgGameState;
let pHtmlMsgKeys;
let pHtmlMsgArduino;


let serialOptions = { baudRate: 230400 };
let serial;
let lastSentSerial = "";

const DEBUG = true;

// The canvas width and height
const canvasWidth = 750;
const canvasHeight = 480;

// offscreen buffer
let offscreenGfxBuffer;

let videoBuffer;

// Stuff for rooms
let visitedRooms = new Set();
let numVisited = 1;

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

// HandPose
let handpose;
const handOptions = {
  maxContinuousChecks: Infinity, 
  detectionConfidence: 0.9,
  scoreThreshold: 0.85
};
// Predictions for handPose
let predictions = [];
// The current hand pose
let curHandPose = null;
// hand pose model is initialized
let isHandPoseModelInitialized = false;

let numCorrectPoses = 0;

// Video for HandPose
let video;



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
  },
  AREA3: {
    ROOM1: "room 3-1",
    ROOM2: "room 3-2"
  }
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

const puzzleStates = {
  // Dictates the behavior of buttons when in certain puzzles
  P1: "puzzle 1",
  P2: "puzzle 2",
  P3: "puzzle 3"
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
  state: playStates.CUTSCENE,    // The player's current state
  expState: exploreStates.STILL, // The player's state if they are exploring
  puzState: puzzleStates.P1,     // The player's state if they are doing a puzzle
  dir: directions.DOWN,          // The direction the player is facing
  x: canvasWidth / 2,            // The location of the player along the x-axis
  y: canvasHeight / 2,           // The location of the player along the y-axis
  xSpd: 0,                       // The player's speed along the x-axis
  ySpd: 0                        // The player's speed along the y-axis
}

let playerMaxSpeed = 4;
let playerHasKey1 = false;
let playerHasKey2 = false;
let puzzle1Complete = false;
let puzzle2Complete = false;
let puzzle3Complete = false;

let areasMap = new Map();
areasMap.set("room 0-1", makeRoom(100, 0, 0, false, true, false, false));
areasMap.set("room 0-2", makeRoom(200, 50, 0, true, false, true, true));
areasMap.set("room 1-1", makeRoom(0, 0, 100, false, false, true, false));
areasMap.set("room 1-2", makeRoom(50, 0, 200, false, false, true, false));
areasMap.set("room 2-1", makeRoom(0, 100, 0, false, false, false, true));
areasMap.set("room 2-2", makeRoom(50, 200, 0, false, false, false, true));
areasMap.set("room 3-1", makeRoom(100, 0, 100, true, false, false, false));
areasMap.set("room 3-2", makeRoom(200, 0, 200, true, false, false, false));

// Stuff for the consoles
const consoleSide = 60;
// Console locations
const console1 = {x1: canvasWidth - 100 - consoleSide,
                  x2: canvasWidth - 100,
                  y1: ((canvasHeight - consoleSide) / 2),
                  y2: ((canvasHeight + consoleSide) / 2)};
const console2 = {x1: 100,
                  x2: 100 + consoleSide,
                  y1: ((canvasHeight - consoleSide) / 2),
                  y2: ((canvasHeight + consoleSide) / 2)};
const console3 = {x1: ((canvasWidth - consoleSide) / 2),
                  x2: ((canvasWidth + consoleSide) / 2),
                  y1: canvasHeight - 100 - consoleSide,
                  y2: canvasHeight - 100};

// Stuff for keys
const keyWidth = 20;
const keyHeight = 50
// Key locations
const key1 = {x1: canvasWidth - 100 - keyWidth,
              x2: canvasWidth - 100,
              y1: ((canvasHeight - keyHeight) / 2),
              y2: ((canvasHeight + keyHeight) / 2)};

const key2 = {x1: 100,
              x2: 100 + keyWidth,
              y1: ((canvasHeight - keyHeight) / 2),
              y2: ((canvasHeight + keyHeight) / 2)};

// Stuff for OLED screen
const OLEDroomSide = 16;
const OLEDroomHalf = 8;
const OLEDwallWidth = 2;


const puzzle1Answer = "right";
const puzzle2Answer = 5;

function setup() {
  // Create the canvas
  createCanvas(canvasWidth, canvasHeight);

  // Setup Web Serial using serial.js
  serial = new Serial();
  serial.on(SerialEvents.CONNECTION_OPENED, onSerialConnectionOpened);
  serial.on(SerialEvents.CONNECTION_CLOSED, onSerialConnectionClosed);
  serial.on(SerialEvents.DATA_RECEIVED, onSerialDataReceived);
  serial.on(SerialEvents.ERROR_OCCURRED, onSerialErrorOccurred);

  // Create video capture for HandPose and hide it
  video = createCapture(VIDEO);
  video.hide();

  // If we have previously approved ports, attempt to connect with them
  //serial.autoConnectAndOpenPreviouslyApprovedPort(serialOptions);

  if (DEBUG) {
    // Add in <p> elements to provide debug messages
    pHtmlMsg = createP("Click anywhere on this page to open the serial connection dialog and begin!");

    pHtmlMsgPlayerStats = createP("Player Stats");

    pHtmlMsgRoom = createP("Room");

    pHtmlMsgGameState = createP("Game State");

    pHtmlMsgArduino = createP("Arduino");

    pHtmlMsgKeys = createP("Keys");
  }
  
  offscreenGfxBuffer = createGraphics(width, height);
  swapRoomBuffer();
  videoBuffer = createGraphics(width, height);

  visitedRooms.add("room 0-1");
  playerHasKey1 = false;
  playerHasKey2 = false;
}

function draw() {
  updateStates();

  switch (game.state) {
    case states.WAITING:
      drawWaiting();
      break;
    case states.MENU:
      drawMenu();
      break;
    case states.GAME:
      drawGame();
      break;
    case states.GAMEOVER:
      drawGameOver();
      break;
  }

  pHtmlMsgPlayerStats.html("Player: [" + player.x + ", " + player.y + "], " + player.state);
  pHtmlMsgRoom.html("Room: " + game.area);
  pHtmlMsgGameState.html("Game State: " + game.state);
  pHtmlMsgKeys.html("Key 1: " + playerHasKey1 + ", Key 2: " + playerHasKey2);

  if (player.state == playStates.PUZZLE && player.puzState == puzzleStates.P2) {
    //image(video, 0, 0, width, height);

  }
}

function updateStates() {
  if (player.state == playStates.EXPLORE) {
    // Update the player's coordinates
    if (player.expState == exploreStates.WALK) {
      setPlayerCoords(player.x + player.xSpd, player.y + player.ySpd);
      moveBackToBounds();
    }
  }

  if (playerNearConsole() && player.expState == exploreStates.ATTACK) {
    player.state = playStates.PUZZLE;
    switch (game.area) {
      case areas.AREA1.ROOM1:
        player.puzState = puzzleStates.P1;
        break;
      case areas.AREA2.ROOM1:
        player.puzState = puzzleStates.P2;
        break;
      case areas.AREA3.ROOM1:
        player.puzState = puzzleStates.P3;
        break;
    }
  }

  if (player.state == playStates.PUZZLE) {
    if (player.puzState == puzzleStates.P1 && !puzzle1Complete) {
      if (classifier == undefined) {
        // Load SpeechCommands18w sound classifier model
        classifier = ml5.soundClassifier('SpeechCommands18w', soundOptions);
        classifier.classify(gotResult);
      }
      //puzzle1Complete = true;
      //areasMap.get(areas.AREA1.ROOM1).hasRightDoor = true;
      //areasMap.delete("room 1-1");
      //areasMap.set("room 1-1", makeRoom(0, 0, 100, false, false, true, true));
      puzzle1Operations();
    } else if (player.puzState == puzzleStates.P2 && !puzzle2Complete) {
      if (handpose == undefined) {
        //videoBuffer = createGraphics(width, height);
        // initialize handpose
        handpose = ml5.handpose(video, handOptions, handModelReady);
        handpose.on("predict", handResultReady);
      }
      //puzzle2Complete = true;
      //areasMap.get(areas.AREA2.ROOM1).hasLeftDoor = true;

      //areasMap.delete("room 2-1");
      //areasMap.set("room 2-1", makeRoom(0, 100, 0, false, false, true, true));
      puzzle2Operations();
    } else if (player.puzState == puzzleStates.P3 && !puzzle3Complete) {
      puzzle3Complete = true;
      areasMap.get(areas.AREA3.ROOM1).hasBtmDoor = true;
      //areasMap.delete("room 3-1");
      //areasMap.set("room 3-1", makeRoom(100, 0, 100, true, true, false, false));
      puzzle3Operations();
    } else {
      player.state = playStates.EXPLORE;
    }
  }

  if (player.state == playStates.PUZZLE && player.expState == exploreStates.BLOCK) {
    switch (player.puzState) {
      case puzzleStates.P1:
        classifier = undefined;
        break;
      case puzzleStates.P2:
        faceapi = undefined;
        break;
      case puzzleStates.P3:
        // Nothing yet
        // TODO: do something here?
        break;
    }

    // Let player move again
    player.state = playStates.EXPLORE;
    player.expState = exploreStates.STILL;
    swapRoomBuffer();
  }

  if (game.area == areas.AREA1.ROOM2) {
    playerHasKey1 = true;
  } else if (game.area == areas.AREA2.ROOM2) {
    playerHasKey2 = true;
  } else if (game.area == areas.AREA3.ROOM2) {
    if (playerInProximity(canvasWidth / 3, 2 * canvasWidth / 3,
                          canvasHeight / 3, 2 * canvasHeight / 3)) {
      // PLAYER WINS!
    }
  }

  if (playerHasKey1 && playerHasKey2 && !areasMap.get("room 0-2").hasBtmDoor) {
    areasMap.delete("room 0-2")
    areasMap.set("room 0-2", makeRoom(200, 50, 0, true, true, true, true));
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

function drawGameOver() {
  // Draw the game over screen
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
  switch (player.puzState) {
    case puzzleStates.P1:
      drawPuzzle1();
      break;
    case puzzleStates.P2:
      drawPuzzle2();
      break;
    case puzzleStates.P3:
      drawPuzzle3();
      break;
  }
}

function drawPuzzle1() {
  //fill(areasMap.get(areas.AREA1.ROOM1).wallColor());
  //textAlign(CENTER);
  //text("puzzle 1", width / 2, height / 2);
  printPuzzleText("puzzle 1", width / 2, height / 2, CENTER,
                  areasMap.get(areas.AREA1.ROOM1).wallColor());
}

function drawPuzzle2() {
  //fill(areasMap.get(areas.AREA2.ROOM1).wallColor());
  //textAlign(CENTER);
  //text("puzzle 2", width / 2, height / 2);
  printPuzzleText("puzzle 2", width / 2, height / 2, CENTER,
                  areasMap.get(areas.AREA2.ROOM1).wallColor());
}

function drawPuzzle3() {
  //fill(areasMap.get(areas.AREA3.ROOM1).wallColor());
  //textAlign(CENTER);
  //text("puzzle 3", width / 2, height / 2);
  printPuzzleText("puzzle 3", width / 2, height / 2, CENTER,
                  areasMap.get(areas.AREA3.ROOM1).wallColor());
}

function printPuzzleText(pText, x, y, alignment, tColor) {
  fill(tColor);
  textAlign(alignment);
  text(pText, x, y);
}

function puzzle1Operations() {
  if (topSoundResult != undefined && prevSoundResult.label != topSoundResult.label) {
    if (topSoundResult.label == puzzle1Answer) {
      if (topSoundResult.confidence >= 0.95) {
        puzzle1Complete = true;
        areasMap.get(areas.AREA1.ROOM1).hasRightDoor = true;
        swapRoomBuffer();
        refreshScreen();
        console.log("puzzle 1 complete!");
      } else {
        console.log("Hmm... you're close but I can't be sure you said it right.");
      }
    } else {
      console.log("Wrong answer, try again.")
    }
  }
}


function puzzle2Operations() {
  if (DEBUG) {
    //video.show();
    if (predictions) {
      console.log("Here!");
      image(video, 0, 0, width, height);
      //drawBox(detections);
      //drawLandmarks(detections);
      drawKeypoints();
    }
  }

  if (curHandPose && checkIfCorrectPose(curHandPose)) {
    puzzle2Complete = true;
    areasMap.get(areas.AREA2.ROOM1).hasLeftDoor = true;
    swapRoomBuffer();
    refreshScreen();
    console.log("puzzle 1 complete!");
  }
}

function puzzle3Operations() {
  // TODO: puzzle 3 operations
}

function drawPuzzleBackground() {
  //offscreenGfxBuffer.fill(255);
  //offscreenGfxBuffer.rect(100, canvasWidth - 100, 100, canvasHeight - 100);
  fill(255);
  rect(100, 100, width - 200, height - 200);
  //offscreenGfxBuffer.rect(100, 200, 100, 200);
}

function drawDungeon() {
  if (throughDoor()) {
    swapRoomBuffer();
    visitedRooms.add(game.area);
    console.log("drawDungeon");
    console.log("visitedNum: " + numVisited);
    console.log("visitedRooms.length: " + visitedRooms.size);
    if (visitedRooms.size > numVisited) {
      //console.log("visitedRooms.length > numVisited");
      numVisited++;
      serialWriteRoomData(game.area, true);

      if (game.area == areas.AREA1.ROOM2) {
        playerHasKey1 = true;
      }
      if (game.area == areas.AREA2.ROOM2) {
        playerHasKey2 = true;
      }
      
    } else {
      //console.log("visitedRooms.length == numVisited");
      serialWriteRoomData(game.area, false);
    }
  }
  if (game.area == areas.AREA1.ROOM1 && !puzzle1Complete) {
    drawPuzzle1Console();
  }
  if (game.area == areas.AREA2.ROOM1 && !puzzle2Complete) {
    drawPuzzle2Console();
  }
  if (game.area == areas.AREA3.ROOM1 && !puzzle3Complete) {
    drawPuzzle3Console();
  }

  refreshScreen();
}

function drawRoom() {
  image(offscreenGfxBuffer, 0, 0);
}

function swapRoomBuffer() {
  // Get the room
  let room = areasMap.get(game.area);

  // Draw floor
  offscreenGfxBuffer.background(room.floorColor());
  offscreenGfxBuffer.fill(room.wallColor());
  drawOuterWall();

  // Draw doorway(s)
  offscreenGfxBuffer.fill(room.floorColor());
  if (room.hasTopDoor) { drawTopDoorway(); }
  if (room.hasBtmDoor) { drawBottomDoorway(); }
  if (room.hasLeftDoor) { drawLeftDoorway(); }
  if (room.hasRightDoor) { drawRightDoorway(); }
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

function throughDoor() {
  switch (game.area) {
    case areas.AREA0.ROOM1:
      if (playerThroughBottomDoor()) {
        return changeRoom(areas.AREA0.ROOM2, player.x, wallWidth);
      }
      break;
    case areas.AREA0.ROOM2:
      if (playerThroughRightDoor()) {
        return changeRoom(areas.AREA1.ROOM1, wallWidth, player.y);
      } else if (playerThroughLeftDoor()) {
        return changeRoom(areas.AREA2.ROOM1, canvasWidth - wallWidth, player.y);
      } else if (playerThroughTopDoor()) {
        return changeRoom(areas.AREA0.ROOM1, player.x, canvasHeight - wallWidth);
      } else if (playerThroughBottomDoor() && playerHasKey1) {
        return changeRoom(areas.AREA3.ROOM1, player.x, wallWidth);
      }
      break;
    case areas.AREA1.ROOM1:
      if (playerThroughLeftDoor()) {
        return changeRoom(areas.AREA0.ROOM2, canvasWidth - wallWidth, player.y);
      } else if (playerThroughRightDoor()) {
        return changeRoom(areas.AREA1.ROOM2, wallWidth, player.y);
      }
      break;
    case areas.AREA1.ROOM2:
      if (playerThroughLeftDoor()) {
        return changeRoom(areas.AREA1.ROOM1, canvasWidth - wallWidth, player.y);
      }
      break;
    case areas.AREA2.ROOM1:
      if (playerThroughLeftDoor()) {
        return changeRoom(areas.AREA2.ROOM2, canvasWidth - wallWidth, player.y);
      } else if (playerThroughRightDoor()) {
        return changeRoom(areas.AREA0.ROOM2, wallWidth, player.y);
      }
      break;
    case areas.AREA2.ROOM2:
      if (playerThroughRightDoor()) {
        return changeRoom(areas.AREA2.ROOM1, wallWidth, player.y);
      }
      break;
    case areas.AREA3.ROOM1:
      if (playerThroughTopDoor()) {
        return changeRoom(areas.AREA0.ROOM2, player.x, canvasHeight - wallWidth);
      } else if (playerThroughBottomDoor() && playerHasKey2) {
        return changeRoom(areas.AREA3.ROOM2, player.x, wallWidth);
      }
      break;
    case areas.AREA3.ROOM2:
      if (playerThroughTopDoor()) {
        return changeRoom(areas.AREA3.ROOM1, player.x, canvasHeight - wallWidth);
      }
      break;
  }
  return false;
}

function changeRoom(newRoom, x, y) {
  game.area = newRoom;
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

function drawPuzzle1Console() {
  offscreenGfxBuffer.stroke(150)
  offscreenGfxBuffer.fill(100);
  offscreenGfxBuffer.rect(canvasWidth - 100 - consoleSide,
                          (canvasHeight - consoleSide) / 2,
                          consoleSide, consoleSide, 10);
}

function drawPuzzle2Console() {
  offscreenGfxBuffer.stroke(150)
  offscreenGfxBuffer.fill(100);
  offscreenGfxBuffer.rect(100, (canvasHeight - consoleSide) / 2,
                          consoleSide, consoleSide, 10);
}

function drawPuzzle3Console() {
  offscreenGfxBuffer.stroke(150)
  offscreenGfxBuffer.fill(100);
  offscreenGfxBuffer.rect((canvasWidth - consoleSide) / 2,
                          canvasHeight - 100 - consoleSide,
                          consoleSide, consoleSide, 10);
}

// TODO: make a proper drawPlayer function (don't just draw a triangle with
//       different colors)
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
  //debugBounds();

  let inConsole = playerInConsole();
  // Move the player back inside the bounds of the screen
  if (playerInDoorway()) {
    if (player.x < 0) {
      player.x = 0;
    } else if (player.x > canvasWidth) {
      player.x = canvasWidth;
    }
    if (player.y < 0) {
      console.log("player.y = 0");
      player.y = 0;
    } else if (player.y > canvasHeight) {
      console.log("player.y = canvasHeight");
      player.y = canvasHeight;
    }
  } else if (inConsole != 0) {
    if (inConsole == 1) {
      moveOutOfConsole(console1.x1, console1.x2, console1.y1, console1.y2);
    } else if (inConsole == 2) {
      moveOutOfConsole(console2.x1, console2.x2, console2.y1, console2.y2);
    } else if (inConsole == 3) {
      moveOutOfConsole(console3.x1, console3.x2, console3.y1, console3.y2);
    }
  } else { //if (!playerInConsole() && !playerInDoorway()) {
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

/*function debugBounds() {
  if (playerInDoorway() && !lastWayVal) {
    console.log("player in doorway");
    lastWayVal = true;
  } else if (!playerInDoorway() && lastWayVal) {
    console.log("player not in doorway");
    lastWayVal = false;
  }

  if (playerInConsole() && !lastConVal) {
    console.log("player in console");
    lastWayVal = true;
  } else if (!playerInConsole() && lastConVal) {
    console.log("player not in console");
    lastWayVal = false;
  }

  if (!playerInConsole() && !playerInDoorway() && !lastOthVal) {
    console.log("player not inside anything");
    lastOthVal = true;
  } else if (playerInConsole() && playerInDoorway() && lastOthVal) {
    console.log("player in something");
    lastOthVal = false;
  }
}*/

function playerInDoorway() {
  switch (game.area) {
    case areas.AREA0.ROOM1:
      return playerInBottomDoorway();
    case areas.AREA0.ROOM2:
      return playerInRightDoorway() ||
             playerInLeftDoorway() ||
             playerInTopDoorway() ||
             (playerInBottomDoorway() && playerHasKey1 && playerHasKey2);
    case areas.AREA1.ROOM1:
      return playerInLeftDoorway() || (playerInRightDoorway() && puzzle1Complete);
    case areas.AREA1.ROOM2:
      return playerInLeftDoorway();
    case areas.AREA2.ROOM1:
      return (playerInLeftDoorway() && puzzle2Complete) || playerInRightDoorway();
    case areas.AREA2.ROOM2:
      return playerInRightDoorway();
    case areas.AREA3.ROOM1:
      return playerInTopDoorway() || (playerInBottomDoorway() && puzzle3Complete);
    case areas.AREA3.ROOM2:
      return playerInTopDoorway();
  }
  return false;
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
  switch (game.area) {
    case areas.AREA1.ROOM1:
      return playerInProximity(canvasWidth - 120 - consoleSide, canvasWidth - 80,
                               ((canvasHeight - consoleSide) / 2) - 20,
                               ((canvasHeight + consoleSide) / 2) + 20) &&
                               !puzzle1Complete;
    case areas.AREA2.ROOM1:
      return playerInProximity(80, 120 + consoleSide,
                               ((canvasHeight - consoleSide) / 2) + 20,
                               ((canvasHeight + consoleSide) / 2) - 20) &&
                               !puzzle2Complete;
    case areas.AREA3.ROOM1:
      return playerInProximity(((canvasWidth - consoleSide) / 2) - 20,
                               ((canvasWidth + consoleSide) / 2) + 20,
                               canvasHeight - 120 - consoleSide, canvasHeight - 80) &&
                               !puzzle3Complete;
  }
  return false;
}

function playerInConsole() {
  switch (game.area) {
    case areas.AREA1.ROOM1:
      if (playerInProximity(console1.x1 + 1,
                            console1.x2 - 1,
                            console1.y1 + 1,
                            console1.y2 - 1) &&
          !puzzle1Complete) {
        return 1;
      }
      break;
    case areas.AREA2.ROOM1:
      if (playerInProximity(console2.x1 + 1,
                            console2.x2 - 1,
                            console2.y1 + 1,
                            console2.y2 - 1) &&
          !puzzle2Complete) {
        return 2;
      }
      break;
    case areas.AREA3.ROOM1:
      if (playerInProximity(console3.x1 + 1,
                            console3.x2 - 1,
                            console3.y1 + 1,
                            console3.y2 - 1) &&
          !puzzle3Complete) {
        return 3;
      }
      break;
  }
  return 0;
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
  pHtmlMsg.html(error);
  resetGame();
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
    pHtmlMsg.html("onSerialDataReceived: " + newData);

    let data = split(newData, ",");
    let xSpd = parseFloat(data[0]);// - 0.5;
    let ySpd = -parseFloat(data[1]);// + 0.5;

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

async function serialWriteRoomData(curArea, newArea) {
  if (serial.isOpen()) {
    /*//let strData = "";
    for (let room of visitedRooms) {
      strData = strData + room + ",";
    }
    strData = strData.substring(0, strData.length - 1);
    if (strData != lastSentSerial) {
      serial.writeLine(strData);
      lastSentSerial = strData;
    }*/
    let strData = "cur:" + curArea + "," + newArea + ";";
    let x = 0;
    let y = 0;
    switch (game.area) {
      case areas.AREA0.ROOM1:
        x = 64;
        y = OLEDroomHalf;
        break;
      case areas.AREA0.ROOM2:
        x = 64;
        y = OLEDroomSide + OLEDroomHalf - OLEDwallWidth;
        break;
      case areas.AREA1.ROOM1:
        x = 64 + OLEDroomSide - OLEDwallWidth;
        y = OLEDroomSide + OLEDroomHalf - OLEDwallWidth;
        break;
      case areas.AREA1.ROOM2:
        x = 64 + (2 * (OLEDroomSide - OLEDwallWidth));
        y = OLEDroomSide + OLEDroomHalf - OLEDwallWidth;
        break;
      case areas.AREA2.ROOM1:
        x = 64 - OLEDroomSide + OLEDwallWidth
        y = OLEDroomSide + OLEDroomHalf - OLEDwallWidth;
        break;
      case areas.AREA2.ROOM2:
        x = 64 - (2 * (OLEDroomSide - OLEDwallWidth));
        y = OLEDroomSide + OLEDroomHalf - OLEDwallWidth;
        break;
      case areas.AREA3.ROOM1:
        x = 64;
        y = (2 * OLEDroomSide) + OLEDroomHalf - (2 * OLEDwallWidth);
        break;
      case areas.AREA3.ROOM2:
        x = 64;
        y = (3 * OLEDroomSide) + OLEDroomHalf - (3 * OLEDwallWidth);
        break;
    }
    strData = strData + x + "," + y;
    serial.writeLine(strData);
  }
}

function resetGame() {
  // Game object
  game.state = states.WAITING;
  game.area = areas.AREA0.ROOM1;

  // Player object
  player.state = playStates.CUTSCENE;
  player.expState = exploreStates.STILL;
  player.puzState = puzzleStates.P1;
  player.dir = directions.DOWN;
  player.x = canvasWidth / 2;
  player.y = canvasHeight / 2;
  player.xSpd = 0;
  player.ySpd = 0;

  // Keys
  playerHasKey1 = false;
  playerHasKey2 = false;
  //playerHasKey1 = true;
  //playerHasKey2 = true;

  // Puzzles completed
  puzzle1Complete = false;
  puzzle2Complete = false;
  puzzle3Complete = false;

  areasMap.delete("room 0-2");
  areasMap.delete("room 1-1");
  areasMap.delete("room 2-1");
  areasMap.delete("room 3-1");
  areasMap.set("room 1-1", makeRoom(0, 0, 100, false, false, true, false));
  areasMap.set("room 2-1", makeRoom(0, 100, 0, false, false, false, true));
  areasMap.set("room 3-1", makeRoom(100, 0, 100, true, false, false, false));
}

function makeRoom(r, b, g, topDoor, btmDoor, leftDoor, rightDoor) {
  return {red: r,
          blu: b,
          grn: g,
          hasTopDoor: topDoor,
          hasBtmDoor: btmDoor,
          hasLeftDoor: leftDoor,
          hasRightDoor: rightDoor,
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
  if (player.state == playStates.PUZZLE && player.puzState == puzzleStates.P1) {
    classifier.classify(gotSoundResult);
    console.log("Sound model ready")
  }
}

function gotSoundResult(error, result) {
  if (error) {
    console.log(error);
    return;
  }
  // log the result
  console.log("result: " + result[0].label + ", confidence: " + result[0].confidence);
  topSoundResult = result[0];
}

// When the model is ready
function handModelReady() {
  console.log('Hand model ready!');
}

function handResultReady() {
  if (predictions && predictions.length > 0) {
    curHandPose = predictions[0];
  } else {
    curHandPose = null;
  }
}

function checkIfCorrectPose(currentPose) {
  // Wrist position
  let wristPos = currentPose.landmarks[0];
  
  // Thumb tip and base
  let thmbTipPos = currentPose.landmarks[4];
  let thmbBasePos = currentPose.landmarks[1];

  // Pointer tip and base
  let pntrTipPos = currentPose.landmarks[8];
  let pntrBasePos = currentPose.landmarks[5];

  // Middle tip and base
  let midlTipPos = currentPose.landmarks[12];
  let midlBasePos = currentPose.landmarks[9];

  // Ring tip and base
  let ringTipPos = currentPose.landmarks[16];
  let ringBasePos = currentPose.landmarks[13];

  // Pinky tip and base
  let pnkyTipPos = currentPose.landmarks[20];
  let pnkyBasePos = currentPose.landmarks[17];

  let flatHand = Math.abs((thmbTipPos[2] + thmbBasePos[2] + pntrTipPos[2] +
                          pntrBasePos[2] + midlTipPos[2] + midlBasePos[2] + 
                          ringTipPos[2] + ringBasePos[2] + pnkyTipPos[2] + 
                          pnkyBasePos[2] + wristPos[2]) / 11) <= 8;

  if (flatHand) {
    console.log("flat hand!");
  }
  //let thumbCorrect = 
}

// A function to draw ellipses over the detected keypoints
function drawKeypoints() {
  for (let i = 0; i < predictions.length; i += 1) {
    const prediction = predictions[i];
    for (let j = 0; j < prediction.landmarks.length; j += 1) {
      const keypoint = prediction.landmarks[j];
      videoBuffer.fill(0, 255, 0);
      videoBuffer.noStroke();
      videoBuffer.ellipse(keypoint[0], keypoint[1], 10, 10);
    }
  }
}
