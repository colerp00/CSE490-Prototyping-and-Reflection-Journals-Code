let serial; // the Serial object
let serialOptions = { baudRate: 115200  };
let bluQueue = [];
let redQueue = [];
let xPos = 0;

function setup() {
  createCanvas(750, 420);

  // Setup Web Serial using serial.js
  serial = new Serial();
  serial.on(SerialEvents.CONNECTION_OPENED, onSerialConnectionOpened);
  serial.on(SerialEvents.CONNECTION_CLOSED, onSerialConnectionClosed);
  serial.on(SerialEvents.DATA_RECEIVED, onSerialDataReceived);
  serial.on(SerialEvents.ERROR_OCCURRED, onSerialErrorOccurred);

  // If we have previously approved ports, attempt to connect with them
  serial.autoConnectAndOpenPreviouslyApprovedPort(serialOptions);

  // Add in a lil <p> element to provide messages. This is optional
  pHtmlMsg = createP("Click anywhere on this page to open the serial connection dialog");

  background(50);
}

function draw() {
  
  while(bluQueue.length > 0){
    // Grab the least recent value of queue (first in first out)
    // JavaScript is not multithreaded, so we need not lock the queue
    // before reading/modifying.
    let bluVal = bluQueue.shift();
    let yPixelPos = height - bluVal * height;

    // Spruce up the color a bit by dynamically setting the line
    // color based on the current sensor value
    let redVal = redQueue.shift();
    let curColor = getCurColor(redVal, bluVal);
    stroke(curColor); //set the color
    line(xPos, height, xPos, yPixelPos);

    xPos++;
  }

  if(xPos >= width){
    xPos = 0;
    background(50);
  }
}

function getCurColor(colorFrac, frac) {
  let colorVal = colorFrac * 765;
  let redVal = 0;
  let grnVal = 0;
  let bluVal = 0;
  if(colorVal < 255) {
    redVal = Math.round((255 - colorVal) * 2 * frac);
    grnVal = Math.round((colorVal) * 2 * frac);
  } else if(colorVal < 510) {
    grnVal = Math.round((510 - colorVal) * 2 * frac);
    bluVal = Math.round((colorVal - 255) * 2 * frac);
  } else {
    bluVal = Math.round((765 - colorVal) * 2 * frac);
    redVal = Math.round((colorVal - 510) * 2 * frac);
  }
  //console.log("rgb(" + redVal + "," + grnVal + "," + bluVal + ")");
  return "rgb(" + redVal + "," + grnVal + "," + bluVal + ")";
}

function mouseClicked() {
  if (!serial.isOpen()) {
    serial.connectAndOpen(null, serialOptions);
  }
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
}

/**
 * Callback function by serial.js when web serial connection is closed
 * 
 * @param {} eventSender 
 */
function onSerialConnectionClosed(eventSender) {
  console.log("onSerialConnectionClosed");
  pHtmlMsg.html("onSerialConnectionClosed");
}

/**
 * Callback function serial.js when new web serial data is received
 * 
 * @param {*} eventSender 
 * @param {String} newData new data received over serial
 */
function onSerialDataReceived(eventSender, newData) {
  console.log("onSerialDataReceived", newData);
  pHtmlMsg.html("onSerialDataReceived: " + newData);

  // JavaScript is not multithreaded, so we need not lock the queue
  // before pushing new elements
  let data = split(newData, ",");
  bluQueue.push(parseFloat(data[0]));
  redQueue.push(parseFloat(data[1]));
}

/**
 * Called automatically by the browser through p5.js when mouse clicked
 */
function mouseClicked() {
  if (!serial.isOpen()) {
    serial.connectAndOpen(null, serialOptions);
  }
}