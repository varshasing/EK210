#include "RTClib.h"
#include "Adafruit_APDS9960.h"
RTC_DS3231 rtc;
//create the APDS9960 object
Adafruit_APDS9960 apds;
//the pin that the interrupt is attached to
#define INT_PIN 3
#define BEGINNING_NIGHT 11
#define END_NIGHT 8
//char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
bool proxSensed, resetProx;
int startTime, endTime, timeElapsed, startDim;
bool lightOn, beginDim;
bool alreadyDetected;
bool brightnessOffed;
bool lowed;
void setup() {
  lowed = false;
  brightnessOffed = false;
  lightOn = false;
  proxSensed = false;
  resetProx = false;
  beginDim = false;
  alreadyDetected = false;
  Serial.begin(57600);
  pinMode(INT_PIN, INPUT_PULLUP);

  if (!apds.begin()) {
    Serial.println("failed to initialize device! Please check your wiring.");
  } else Serial.println("Device initialized!");

  //enable proximity mode
  apds.enableProximity(true);

  //enable color sensing mode
  apds.enableColor(true);


  //set the interrupt threshold to fire when proximity reading goes above 175
  apds.setProximityInterruptThreshold(0, 1);

  //enable the proximity interrupt
  apds.enableProximityInterrupt();


#ifndef ESP8266
  while (!Serial)
    ;  // wait for serial port to connect. Needed for native USB
#endif

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(9, OUTPUT);
}
void loop() {
  // put your main code here, to run repeatedly:
  DateTime now = rtc.now();
  int currentHour = now.hour();
  int currentMinute = now.minute();
  int currentSecond = now.second();
  uint16_t r, g, b, c;
  //bool interrupted;
  int distance = apds.readProximity();

  //setHigh();
  //wait for color data to be ready
  while (!apds.colorDataReady()) {
    delay(5);
  }
  apds.getColorData(&r, &g, &b, &c);
  /*
  Serial.print("red: ");
  Serial.print(r);

  Serial.print(" green: ");
  Serial.print(g);

  Serial.print(" blue: ");
  Serial.print(b);

  Serial.print(" clear: ");
  Serial.println(c);
  Serial.println();
  //digitalWrite(LED_BUILTIN, LOW);
  //Serial.println(currentHour);
  */
  /*
    if (currentHour > 20 || currentHour < 7) {
      analogWrite(5, 0);
      analogWrite(6, 0);
      analogWrite(9, 0);
      digitalWrite(LED_BUILTIN, LOW);
    } else {
      digitalWrite(LED_BUILTIN, HIGH);
      //Serial.println("Yep");
    }
    */
  //interrupted = digitalRead(INT_PIN);
  //Serial.println(interrupted);
  //Serial.println(c);




  if (c < 150) {
    if (brightnessOffed) {
      setHigh();
      brightnessOffed = false;
    }
    if ((currentHour > BEGINNING_NIGHT || currentHour < END_NIGHT) && lightOn && !lowed) {
      //Serial.println("Changing");
      if (!beginDim) {
        Serial.println("Not GOOD");
        beginDim = true;
        startDim = (int)now.unixtime();
      }
    } else {
      beginDim = false;
    }

    //Serial.println(now.unixtime() - startDim);
    if (beginDim && ((int)now.unixtime() - startDim) <= 125) {
      analogWrite(6, 150 - (((int)now.unixtime() - startDim)));
      Serial.println((int)now.unixtime() - startDim);
    }
    if (distance < 2) {
      proxSensed = false;
      alreadyDetected = false;
      timeElapsed = 0;
    }
    if (!alreadyDetected && proxSensed && (distance > 1)) {
      endTime = (int)now.unixtime();
      Serial.print("End: ");
      Serial.println(endTime);
      timeElapsed = endTime - startTime;
      Serial.println("Time elapsed: ");
      Serial.println(timeElapsed);
      if ((timeElapsed > 1) && !alreadyDetected) {
        //timeElapsed = 0;
        Serial.println("TRYING");
        alreadyDetected = true;
        if (lightOn) {
          ledOff();
          lightOn = false;
        } else {
          if (currentHour > BEGINNING_NIGHT || currentHour < END_NIGHT) {
            setLow();  //Low blue light
            lightOn = true;
            lowed = true;
          } else {
            setHigh();  //High blue light
            lightOn = true;
          }
        }
        //Serial.println("REGISTERED");
      }
    }
    if ((distance > 1) && !alreadyDetected && !proxSensed) {
      //Serial.println("This ain't it");
      //clear the interrupt
      //apds.clearInterrupt();
      // if (!proxSensed) {
      startTime = (int)now.unixtime();
      Serial.print("Start: ");
      Serial.println(startTime);
      proxSensed = true;
      //Serial.println("Prox detected");
      // }
    }
  } else {
    brightnessOffed = true;
    ledOff();
    Serial.print("Light level: ");
    Serial.println(c);
  }
  if (resetProx) {
    resetProx = false;
    proxSensed = false;
  }


  //Serial.println(c);
  /*
  //int currentDimStage = 0;
  int startDim;
  if ((currentHour > BEGINNING_NIGHT || currentHour < END_NIGHT) && lightOn) {
    //Serial.println("Changing");
    if (!beginDim) {
      Serial.println("Not GOOD");
      beginDim = true;
      startDim = (int)now.unixtime();
    }
  } else {
    beginDim = false;
  }
 
  //Serial.println(now.unixtime() - startDim);
  if (beginDim && ((int)now.unixtime() - startDim) <= 125) {
    analogWrite(6, 150 - (((int)now.unixtime() - startDim)));
    Serial.println((int)now.unixtime() - startDim);
  }
  
  if (c < 200) {
    if (proxSensed && (distance < 2)) {
      proxSensed = false;
      endTime = (int)now.unixtime();
      Serial.print("End: ");
      Serial.println(endTime);
      timeElapsed = endTime - startTime;
      Serial.println("Time elapsed: ");
      Serial.println(timeElapsed);
      if (timeElapsed > 0) {
        timeElapsed = 0;
        if (lightOn) {
          ledOff();
          lightOn = false;
        } else {
          if (currentHour > BEGINNING_NIGHT || currentHour < END_NIGHT) {
            setLow();  //Low blue light
            lightOn = true;
          } else {
            setHigh();  //High blue light
            lightOn = true;
          }
        }
        //Serial.println("REGISTERED");
      }
    }
    if ((distance > 1)) {
      //Serial.println("This ain't it");
      //clear the interrupt
      apds.clearInterrupt();
      if (!proxSensed) {
        startTime = (int)now.unixtime();
        Serial.print("Start: ");
        Serial.println(startTime);
        proxSensed = true;
        //Serial.println("Prox detected");
      }
    }
  } else {
    ledOff();
    Serial.print("Light level: ");
    Serial.println(c);
  }
  if (resetProx) {
    resetProx = false;
    proxSensed = false;
  }

  //Serial.println(proxSensed);
  /*
  if (!digitalRead(INT_PIN)) {
    Serial.println(apds.readProximity());
    //clear the interrupt
    apds.clearInterrupt();
  }
  */
}

void setLow() {
  //150 150 50
  analogWrite(5, 150);
  analogWrite(9, 150);
  analogWrite(6, 25);
}

void setHigh() {
  //150 150 150
  analogWrite(5, 150);
  analogWrite(9, 150);
  analogWrite(6, 150);
}

void ledOff() {
  analogWrite(5, 0);
  analogWrite(6, 0);
  analogWrite(9, 0);
}
