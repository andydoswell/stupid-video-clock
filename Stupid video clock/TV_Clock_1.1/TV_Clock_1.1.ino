/* Another Stpid Arduino Clock

    (c) A.G.Doswell 16th Aug 2020

    full desciption on the webiste http:/www.andydoz.blogspot.com

*/

int clockInt = INT0; // interrupt pin from oscillator
volatile int seconds; // floating point timerSeconds
int mins; // minutes
unsigned int oldMins; // previous minutes used in monitor timer
int oldSeconds; //old seconds - used to initiate transmit
int loopCounter; // counts the loops
unsigned int soundLevel; // level of sound used to triger monitor power
int soundRaw; // raw output of ADC
const int calPin = 6; // pull this pin low to enable calibration mode.
const int monitorPwrPin = 10; // output pin controlling power to monitor.
const int soundThreshold = 5; // level at which the monitor power is switched on
const int monitorTime = 2; // minimum number of minutes the monitor is on
const int runTimer = 9; // pin to enable or reset timer, controlled by Arduino 2.
volatile unsigned int calMins;
float counter;
float calFactor = 28.061154497;// interrupt frequency in Hz 28.19161,28.3952160722, 28.0639385514 , 28.0622680791larger number, slower clock

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
    counter = 0;
    seconds = 0;
    digitalWrite(monitorPwrPin, HIGH);
  }

  if (counter >= calFactor) {
    counter = counter - calFactor;
    seconds ++;
    if (seconds >= 60) {
      seconds = 0;
      mins++;
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

  if (oldSeconds != seconds) { // transmit when timerSeconds updated
    if (!digitalRead (calPin)) { // if the cal pin is low, transmit the calibration message, if not, transmit normal.
      transmitCal ();
    }
    else {
      transmit();
    }
    oldSeconds = seconds;
  }
  loopCounter++;// increment the loop counter
  //counter ++;
  delay (10);
}

void clockCounter()        // Called by interrupt, driven by the crystal oscillator
{
  counter++;
  return;
}

void transmit () { // transmit time to other Arduino
  Serial.write(seconds);
  //Serial.print(counter,7);
  //Serial.print(" ");
  //Serial.println (seconds);
}

void transmitCal () { // transmits human readable text for calibration purposes.
  Serial.println ();
  Serial.print (calMins);
  Serial.print(":");
  Serial.print(seconds);
}
