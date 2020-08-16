/* Another Stpid Arduino Clock

    (c) A.G.Doswell 16th Aug 2020

    full desciption on the webiste http:/www.andydoz.blogspot.com

*/


#include <TVout.h>
#include <pollserial.h>
#include <fontALL.h>
int secs, oldsecs, x, y, x1, x2, x3, y1, y2, y3, radius;
int mins = 59;
int hours = 23;
const int hoursPin = 2;
const int minsPin = 3;
const int setPin = 1;
int mode ; // true = clock false = pong
int modeTime ; // no. of seconds each mode is displayed for
int modeSecs; // elapsed second since last mode change
boolean pongFlag = true ; // true if last mode is pong
const int batLength = 14;
const int batWidth = 1;
const int maxVelocityY = 6;
TVout TV;
unsigned char xPong, yPong;
pollserial pserial;
const int rightBatX = 125;
const int leftBatX = 2;
int rightBatY = 0;
int leftBatY = 0;
char ballVolX = 3;
char ballVolY = 2;
unsigned char ballX = 0;
unsigned char ballY = 0;
float R1 = 10000;
float logR2, R2, temp;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
unsigned int thermRaw;
const int chimePin = 4;
int chimeSecs;
int chimeCount;
boolean chimeFlag;
const int runTimer = 10;
int zOff = 150;
int xOff = 0;
int yOff = 0;
int cSize = 50;
int view_plane = 64;
float angle = PI / 60;
float cube3d[8][3] = {
  {xOff - cSize, yOff + cSize, zOff - cSize},
  {xOff + cSize, yOff + cSize, zOff - cSize},
  {xOff - cSize, yOff - cSize, zOff - cSize},
  {xOff + cSize, yOff - cSize, zOff - cSize},
  {xOff - cSize, yOff + cSize, zOff + cSize},
  {xOff + cSize, yOff + cSize, zOff + cSize},
  {xOff - cSize, yOff - cSize, zOff + cSize},
  {xOff + cSize, yOff - cSize, zOff + cSize}
};
unsigned char cube2d[8][2];

void setup()  {
  TV.begin(PAL, 128, 96);
  TV.set_hbi_hook(pserial.begin(9600));
  TV.clear_screen();
  TV.select_font(font8x8);
  pinMode (hoursPin, INPUT_PULLUP);
  pinMode (minsPin, INPUT_PULLUP);
  pinMode (setPin, INPUT_PULLUP);
  pinMode (runTimer, OUTPUT);
  clockSet();

}
void loop() {
  if (pserial.available()) {
    secs = ((char)pserial.read());
  }

  if (oldsecs != secs) {
    oldsecs = secs;
    modeSecs++;
    if (!secs) {
      mins++;
      if (mins == 60) {
        mins = 0;
        hours ++;
      }
      if (hours == 24) {
        hours = 0;
      }
    }
    if (mode == 0) {
      if (pongFlag) {
        drawClockFace();
        pongFlag = false;
      }
      drawHands();
      displayTime();
    }
  }
  if (mode == 1) {
    pongFlag = true;
    TV.clear_screen();
    pong ();
  }
  if (modeSecs >= modeTime) {
    mode++;
    getTemp();
    if (mode == 3) {
      mode = 0;
    }
    modeSecs = 0;
    modeTime = random (30, 180);
    TV.clear_screen();
  }

  if (mode == 2) {
    int rsteps = random (10, 60);
    switch (random(6)) {
      case 0:
        for (int i = 0; i < rsteps; i++) {
          zrotate(angle);
          printcube();
        }
        break;
      case 1:
        for (int i = 0; i < rsteps; i++) {
          zrotate(2 * PI - angle);
          printcube();
        }
        break;
      case 2:
        for (int i = 0; i < rsteps; i++) {
          xrotate(angle);
          printcube();
        }
        break;
      case 3:
        for (int i = 0; i < rsteps; i++) {
          xrotate(2 * PI - angle);
          printcube();
        }
        break;
      case 4:
        for (int i = 0; i < rsteps; i++) {
          yrotate(angle);
          printcube();
        }
        break;
      case 5:
        for (int i = 0; i < rsteps; i++) {
          yrotate(2 * PI - angle);
          printcube();
        }
        break;
    }

  }
  if (mins == 59 && secs == 58 && !chimeFlag ) {
    mode = 0;
    modeSecs = 0;
    TV.clear_screen();
    drawClockFace();
  }
  check_chime ();
}

void drawClock () {
  if (pongFlag) {
    drawClockFace();
    pongFlag = false;
  }
  drawHands();
  displayTime();
}

void displayTime() {
  TV.set_cursor (0, 0);
  if (hours < 10) {
    TV.print("0");
  }
  TV.print(hours);
  TV.print(":");
  if (mins < 10) {
    TV.print("0");
  }
  TV.print(mins);
  TV.set_cursor (0, 60);
  TV.print(temp, 0);
  TV.print(" C");
}

void drawHands () {
  TV.draw_circle(x, y, radius - 5, BLACK, BLACK);
  float angle = secs * 6 ;
  angle = (angle / 57.29577951) ; //Degs to rads
  x3 = (x + (sin(angle) * (radius - 6)));
  y3 = (y - (cos(angle) * (radius - 6)));
  TV.draw_line(x, y, x3, y3, WHITE);
  angle = mins * 6 ;
  angle = (angle / 57.29577951) ;
  x3 = (x + (sin(angle) * (radius - 10)));
  y3 = (y - (cos(angle) * (radius - 10)));
  TV.draw_line(x, y, x3, y3, WHITE);
  angle = hours * 30 + int((mins / 12) * 6 )   ;
  angle = (angle / 57.29577951) ;
  x3 = (x + (sin(angle) * (radius - 17)));
  y3 = (y - (cos(angle) * (radius - 17)));
  TV.draw_line(x, y, x3, y3, WHITE);
}

void drawClockFace () {
  x = (TV.hres() / 2) + 25;
  y = (TV.vres() / 2) - 10;
  radius = (TV.vres() / 2) - 10 ;
  //clock face
  TV.draw_circle(x, y, radius, WHITE);
  //hour ticks
  for ( int z = 0; z < 360; z = z + 30 ) {
    float angle = z ;
    angle = (angle / 57.29577951) ; //deg to rad
    x2 = (x + (sin(angle) * radius));
    y2 = (y - (cos(angle) * radius));
    x3 = (x + (sin(angle) * (radius - 5)));
    y3 = (y - (cos(angle) * (radius - 5)));
    TV.draw_line(x2, y2, x3, y3, WHITE);
  }
  TV.set_cursor(62, 86);
  TV.print("Doz");
}
void clockSet () {

  TV.set_cursor (40, 75);
  TV.print("Set Time");
  TV.set_cursor (50, 84);
  if (hours < 10) {
    TV.print("0");
  }
  TV.print(hours);
  TV.print(":");
  if (mins < 10) {
    TV.print("0");
  }
  TV.print(mins);

  if (!digitalRead(hoursPin)) {
    hours++;
    if (hours == 24) {
      hours = 0;
    }
    TV.delay (500);
  }
  if (!digitalRead(minsPin)) {
    mins++;
    if (mins == 60) {
      mins = 0;
    }
    TV.delay (500);
  }
  if (!digitalRead (setPin)) { //! in here before final compile
    TV.clear_screen ();
    digitalWrite(runTimer, LOW); // stop the timer running on Arduino 1
    TV.delay (100);
    digitalWrite(runTimer, HIGH);
    return;
  }
  clockSet();
}
void drawPongScreen() {
  //draw right bat
  rightBatY = ballY - (batLength / 2) - 1 ; //+ (random(-1, +1));
  if (rightBatY > TV.vres() - batLength) {
    rightBatY = TV.vres() - batLength;
  }
  if (rightBatY < 1) {
    rightBatY = 1;
  }
  xPong = rightBatX;
  for (int i = 0; i < batWidth; i++) {
    TV.draw_line(xPong + i, rightBatY, xPong + i, rightBatY + batLength, 1);
  }

  // draw left bat
  leftBatY = rightBatY ;
  if (leftBatY > TV.vres() - batLength) {
    leftBatY = TV.vres() - batLength;
  }
  if (leftBatY < 1) {
    leftBatY = 1;
  }
  xPong = leftBatX;

  for (int i = 0; i < batWidth; i++) {
    TV.draw_line(xPong + i, leftBatY, xPong + i, leftBatY + batLength, 1);
  }
  // print time as "score"
  TV.print(37, 2, hours);
  TV.set_cursor (75, 2);
  if (mins < 10) {
    TV.print("0");
  }
  TV.print(mins);
  //plot ball
  TV.set_pixel(ballX, ballY, 2);
}

void drawBox() {
  //TV.clear_screen();
  for (int i = 1; i < TV.vres() - 4; i += 6) {
    TV.draw_line(TV.hres() / 2, i, TV.hres() / 2, i + 3, 1);
  }
  // had to make box a bit smaller to fit tv
  TV.draw_line(0, 0, 0, 95, 1 ); // left
  TV.draw_line(0, 0, 127, 0, 1 ); // top
  TV.draw_line(127, 0, 127, 95, 1 ); // right
  TV.draw_line(0, 95, 127, 95, 1 ); // bottom
}

void pong () {
  drawBox ();
  ballX += ballVolX;
  ballY += ballVolY;

  // change direction if ball hits top or bottom
  if (ballY <= 1 || ballY >= TV.vres() - 1)
  { ballVolY = -ballVolY;
    //TV.delay (100);
  }

  // detect if the ball hit the bat on the left
  if (ballVolX < 1) {
    if (ballX <= (batWidth + 1) && ballY >=  leftBatY && ballY <= (leftBatY + batLength)) {
      ballVolX = -ballVolX;
      ballVolY += random (-2, 2) * ((ballY - leftBatY) - (batLength / 2)) / (batLength / 2);
      //TV.delay(100);
    }
  }
  // detect if the ball hit the bat on the right
  if (ballVolX > 0) {
    if (ballX >= (TV.hres() - batWidth - 1) && ballY >=  rightBatY && ballY <= (rightBatY + batLength)) {
      ballVolX = -ballVolX;
      ballVolY += random (-2, 2) * ((ballY - leftBatY) - (batLength / 2)) / (batLength / 2);
      //TV.delay(100);
    }
  }
  // limit ball speed
  if (ballVolY > maxVelocityY) ballVolY = maxVelocityY;
  if (ballVolY < -maxVelocityY) ballVolY = -maxVelocityY;

  drawPongScreen();

  TV.delay_frame(1);
  //if (++frame == 60) frame = 0;
}

void getTemp () {
  thermRaw = 0;
  for (int i = 0; i <= 99; i++) {
    thermRaw = thermRaw + analogRead (A4);
  }
  thermRaw = thermRaw / 100;
  R2 = R1 * (1023.0 / (float)thermRaw - 1.0);
  logR2 = log(R2);
  temp = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  temp = temp - 273.15;

}

void check_chime () {
  if (!mins && !secs && !chimeFlag ) {
    chimeCount = hours;
    if (!hours) {
      chimeCount = 12;
    }
    if (hours > 12) {
      chimeCount = hours - 12;
    }
  }
  if (chimeCount && !chimeFlag) {
    chime ();
  }
  if (chimeCount && (chimeSecs + 1 == secs) && chimeFlag) {
    chime ();
  }
  if (!chimeCount && (chimeSecs + 1 == secs)) {
    chimeFlag = false;
  }
}

void chime () {
  digitalWrite(chimePin, HIGH);
  delay (20);
  digitalWrite(chimePin, LOW);
  chimeCount --;
  chimeSecs = secs;
  chimeFlag = true;
}

void cube() {

}

void printcube() {
  //calculate 2d points
  for (byte i = 0; i < 8; i++) {
    cube2d[i][0] = (unsigned char)((cube3d[i][0] * view_plane / cube3d[i][2]) + (TV.hres() / 2));
    cube2d[i][1] = (unsigned char)((cube3d[i][1] * view_plane / cube3d[i][2]) + (TV.vres() / 2));
  }
  TV.delay_frame(1);
  TV.clear_screen();
  draw_cube();
}

void zrotate(float q) {
  float tx, ty, temp;
  for (byte i = 0; i < 8; i++) {
    tx = cube3d[i][0] - xOff;
    ty = cube3d[i][1] - yOff;
    temp = tx * cos(q) - ty * sin(q);
    ty = tx * sin(q) + ty * cos(q);
    tx = temp;
    cube3d[i][0] = tx + xOff;
    cube3d[i][1] = ty + yOff;
  }
}

void yrotate(float q) {
  float tx, tz, temp;
  for (byte i = 0; i < 8; i++) {
    tx = cube3d[i][0] - xOff;
    tz = cube3d[i][2] - zOff;
    temp = tz * cos(q) - tx * sin(q);
    tx = tz * sin(q) + tx * cos(q);
    tz = temp;
    cube3d[i][0] = tx + xOff;
    cube3d[i][2] = tz + zOff;
  }
}

void xrotate(float q) {
  float ty, tz, temp;
  for (byte i = 0; i < 8; i++) {
    ty = cube3d[i][1] - yOff;
    tz = cube3d[i][2] - zOff;
    temp = ty * cos(q) - tz * sin(q);
    tz = ty * sin(q) + tz * cos(q);
    ty = temp;
    cube3d[i][1] = ty + yOff;
    cube3d[i][2] = tz + zOff;
  }
}

void draw_cube() {
  TV.draw_line(cube2d[0][0], cube2d[0][1], cube2d[1][0], cube2d[1][1], WHITE);
  TV.draw_line(cube2d[0][0], cube2d[0][1], cube2d[2][0], cube2d[2][1], WHITE);
  TV.draw_line(cube2d[0][0], cube2d[0][1], cube2d[4][0], cube2d[4][1], WHITE);
  TV.draw_line(cube2d[1][0], cube2d[1][1], cube2d[5][0], cube2d[5][1], WHITE);
  TV.draw_line(cube2d[1][0], cube2d[1][1], cube2d[3][0], cube2d[3][1], WHITE);
  TV.draw_line(cube2d[2][0], cube2d[2][1], cube2d[6][0], cube2d[6][1], WHITE);
  TV.draw_line(cube2d[2][0], cube2d[2][1], cube2d[3][0], cube2d[3][1], WHITE);
  TV.draw_line(cube2d[4][0], cube2d[4][1], cube2d[6][0], cube2d[6][1], WHITE);
  TV.draw_line(cube2d[4][0], cube2d[4][1], cube2d[5][0], cube2d[5][1], WHITE);
  TV.draw_line(cube2d[7][0], cube2d[7][1], cube2d[6][0], cube2d[6][1], WHITE);
  TV.draw_line(cube2d[7][0], cube2d[7][1], cube2d[3][0], cube2d[3][1], WHITE);
  TV.draw_line(cube2d[7][0], cube2d[7][1], cube2d[5][0], cube2d[5][1], WHITE);
  displayTime ();
  check_chime();
}
