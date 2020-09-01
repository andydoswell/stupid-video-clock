/* Another Stpid Arduino Clock

    (c) A.G.Doswell 16th Aug 2020

    full desciption on the webiste http:/www.andydoz.blogspot.com

*/

int clockInt = INT0; // interrupt pin from oscillator
volatile long timerSeconds; // floating point timerSeconds
volatile int mins; // minutes
unsigned int oldMins; // previous minutes used in monitor timer
int truncatedSecs; //integer timerSeconds
int oldTruncatedSecs; //old integer timerSeconds - used to initiate transmit
int loopCounter; // counts the loops
unsigned int soundLevel; // level of sound used to triger monitor power
int soundRaw; // raw output of ADC
const int calPin = 6; // pull this pin low to enable calibration mode.
const int monitorPwrPin = 10; // output pin controlling power to monitor.
const int soundThreshold = 5; // level at which the monitor power is switched on
const int monitorTime = 2; // minimum number of minutes the monitor is on
const int runTimer = 9; // pin to enable or reset timer, controlled by Arduino 2.
volatile unsigned int calMins;

void setup() {
  attachInterrupt(clockInt, clockCounter, RISING); // start interrupt running
  Serial.begin (9600); //start serial interface
  pinMode (calPin, INPUT_PULLUP);
  pinMode (monitorPwrPin, OUTPUT);
  pinMode (runTimer, INPUT_PULLUP);
  digitalWrite (monitorPwrPin, HIGH);
}

void loop() {

  if (!digitalRead(runTimer)) {
    timerSeconds = 0;
    digitalWrite(monitorPwrPin, HIGH);
  }
 if (timerSeconds >= 60000) { // reset timerSeconds, but preserve the "remainder" and increment minutes
    timerSeconds -= 60000;
    mins++;
    calMins++;
    if (mins >= 60) {
      mins = 0;
    }
  }
  soundRaw = analogRead (A1); // reads the sound level
  // "Software" rectifier. Changes the negative readings to positive.
  if (soundRaw <= 512) {
    soundRaw = map(soundRaw, 0, 512, 512, 0);
  }
  if (soundRaw >= 513) {
    soundRaw = soundRaw - 512;
  }
  soundLevel += soundRaw;// increments the soundLevel
  //averages the sound level, and if it's greater than the threshold, power the monitor up, and start the monitor timer
  if (loopCounter >= 200) {
    soundLevel = soundLevel / loopCounter;
    if (soundLevel >= soundThreshold) {
      digitalWrite (monitorPwrPin, HIGH);
      oldMins = mins;
    }
    loopCounter = 0;
    soundLevel = 0;
  }

  if (mins >= oldMins + monitorTime) {// if the monitor timer has expired, turn the monitor off
    digitalWrite (monitorPwrPin, LOW);
    mins = 0;
    oldMins = 0;
  }

  truncatedSecs = int (timerSeconds/1000); // truncate timerSeconds, we don't want to transmit floats
  if (oldTruncatedSecs != truncatedSecs) { // transmit when timerSeconds updated
    if (!digitalRead (calPin)) { // if the cal pin is low, transmit the calibration message, if not, transmit normal.
      transmitCal ();
    }
    else {
      transmit();
    }
    oldTruncatedSecs = truncatedSecs;
  }
  loopCounter++;// increment the loop counter
}

void clockCounter()        // Called by interrupt, driven by the crystal oscillator
{
  timerSeconds += 35.7415453259; // this number is the number of seconds multiplied by 1000 each cycle of our ocsillator increments the clock. (x1000 increases accuracy)
  return;
}

void transmit () { // transmit time to other Arduino
  Serial.write(truncatedSecs);
}

void transmitCal () { // transmits human readable text for calibration purposes.
  Serial.println ();
  Serial.print (calMins);
  Serial.print(":");
  Serial.print(timerSeconds);
}
