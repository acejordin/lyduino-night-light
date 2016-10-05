/*
  lyduino-night-light

  A sketch for Arduino Yun for a dual configurable night light.

  https://github.com/acejordin/lyduino-night-light

 */


#include <Process.h>
#include <LiquidCrystal.h>
#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>

BridgeServer server;
Process date;                 // process used to get the date
int hours, minutes, seconds;  // for the results
int lastSecond = -1;          // need an impossible value for comparison
String timeStr;
String ampm;

int redPin = 9;
int greenPin = 10;
int bluePin = 11;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 13, 5, 4, 3, 2);

//Night light time
//Night Start Time (Military time)
int nightStartHour = 19;
int nightStartMinute = 55;
int nightStart = nightStartHour * 60 + nightStartMinute;
//Night End Time (Military time)
int nightEndHour = 20;
int nightEndMinute = 30;
int nightEnd = nightEndHour * 60 + nightEndMinute;

//Day light time
//Day Start Time (Military time)
int dayStartHour = 6;
int dayStartMinute = 30;
int dayStart = dayStartHour * 60 + dayStartMinute;
//Day End Time (Military time)
int dayEndHour = 8;
int dayEndMinute = 00;
int dayEnd = dayEndHour * 60 + dayEndMinute;

void setup() {
  pinMode(8, OUTPUT);
  //digitalWrite(8, LOW);
  Bridge.begin();
  //digitalWrite(8, HIGH);        // initialize Bridge
  SerialUSB.begin(9600);    // initialize serial

  //init RBG LED
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  //setColor(0, 128, 0); //Green
  //setColor(255, 30, 0); //orange
  //setColor(0x4B, 0x0, 0x82); // indigo

  //while (!Serial);              // wait for Serial Monitor to open
  SerialUSB.println("Time Check");  // Title of sketch

  // run an initial date process. Should return:
  // hh:mm:ss :
  if (!date.running()) {
    date.begin("date");
    date.addParameter("+%T");
    date.run();
  }

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  timeStr = "";
  ampm = "AM";

  if (lastSecond != seconds) { // if a second has passed
    if(hours > 12){ 
      hours -= 12;
      ampm = "PM";
    }
    
    // print the time:
    if (hours <= 9) {
      timeStr += "0";
    }
    
    timeStr += String(hours) + ":";
    if (minutes <= 9) {
      timeStr += "0";
    }
    
    timeStr += String(minutes) + ":";
    if (seconds <= 9) {
      //SerialUSB.print("0");  // adjust for 0-9
      timeStr += "0";
    }
    
    timeStr += String(seconds);
    timeStr += " " + ampm;

    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    // print the time
    lcd.print(timeStr);

    // restart the date process:
    if (!date.running()) {
      date.begin("date");
      date.addParameter("+%T");
      date.run();
    }
  }

  //if there's a result from the date process, parse it:
  while (date.available() > 0) {
    // get the result of the date process (should be hh:mm:ss):
    String timeString = date.readString();

    // find the colons:
    int firstColon = timeString.indexOf(":");
    int secondColon = timeString.lastIndexOf(":");

    // get the substrings for hour, minute second:
    String hourString = timeString.substring(0, firstColon);
    String minString = timeString.substring(firstColon + 1, secondColon);
    String secString = timeString.substring(secondColon + 1);

    // convert to ints,saving the previous second:
    hours = hourString.toInt();
    minutes = minString.toInt();
    lastSecond = seconds;          // save to do a time comparison
    seconds = secString.toInt();

    int currentMin = hours * 60 + minutes;
    
    //check if night light is on
    if(currentMin >= nightStart && currentMin < nightEnd) { 
      //Turn night light on!
      setColor(255, 30, 0); //orange      
    }
    else {
      //Turn night light off!
      setColor(0, 0, 0); //off
    }

    //check if day light is on
    if(currentMin >= dayStart && currentMin < dayEnd) {
      //Turn day light on!
      digitalWrite(8, HIGH);
    }
    else {
      //Turn day light off!
      digitalWrite(8, LOW);
    }
  }

  BridgeClient client = server.accept();

  if (client) {
    process(client);
    client.stop();
  }

  delay(50);
}

void process(BridgeClient client) {
  String command = client.readStringUntil('/');

  if (command == "digital") {
    digitalCommand(client);
  }
  if (command == "analog") {
    analogCommand(client);
  }
  if (command == "mode") {
    modeCommand(client);
  }
}

void digitalCommand(BridgeClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/digital/13/1"
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
  } else {
    value = digitalRead(pin);
  }

  // Send feedback to client
  client.print(F("Pin D"));
  client.print(pin);
  client.print(F(" set to "));
  client.println(value);

  // Update datastore key with the current pin value
  String key = "D";
  key += pin;
  Bridge.put(key, String(value));
}

void analogCommand(BridgeClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/analog/5/120"
  if (client.read() == '/') {
    // Read value and execute command
    value = client.parseInt();
    analogWrite(pin, value);

    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" set to analog "));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "D";
    key += pin;
    Bridge.put(key, String(value));
  } else {
    // Read analog pin
    value = analogRead(pin);

    // Send feedback to client
    client.print(F("Pin A"));
    client.print(pin);
    client.print(F(" reads analog "));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "A";
    key += pin;
    Bridge.put(key, String(value));
  }
}

void modeCommand(BridgeClient client) {
  int pin;

  // Read pin number
  pin = client.parseInt();

  // If the next character is not a '/' we have a malformed URL
  if (client.read() != '/') {
    client.println(F("error"));
    return;
  }

  String mode = client.readStringUntil('\r');

  if (mode == "input") {
    pinMode(pin, INPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as INPUT!"));
    return;
  }

  if (mode == "output") {
    pinMode(pin, OUTPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as OUTPUT!"));
    return;
  }

  client.print(F("error: invalid mode "));
  client.print(mode);
}

void setColor(int red, int green, int blue) {
  #ifdef COMMON_ANODE
  reg = 255 - red;
  green = 255 - green;
  blue = 255 - blue;
  #endif
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

