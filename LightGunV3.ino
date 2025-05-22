/* IR Lightgun Arduino code by Tim Peeters 2018-19
 *  
 * Made with the 'Building an Arduino-based laser game tutorial'
 * by Duane O'Brien at https://www.ibm.com/developerworks/opensource/tutorials/os-arduino1/
 * 
 * V3.0 
 */
 
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// OLED display TWI address
#define OLED_ADDR   0x3C

// reset pin not used on 4-pin OLED module
Adafruit_SSD1306 display(-1);  // -1 = no reset pin


//Game Setup variables.
int maxPlayers = 10; //max players in game.
const int playerID = 1; //player number of this instance. (MUST BE BETWEEN 1 and 10!)
bool useSound = true;

int maxLives = 3;
int currentLives;


//Gun variables-------------|
bool isShooting = false;
int shotDelayTime = 100;

//Bullet Count
int currentBullets = 0;
int maxBullets = 100;


//Hardware variables
const int buttonPin = 6;
const int ledPin = 9;

const int speakerPin = 8;
const int laserPin = 2;
const int lcdClockPin = A5;
const int lcdSerialPin = A4;

const int sendPin = 3;
const int receiverPin = 5;

//The following variables declare the pulse lengths of the IR Code being send.
const int startBit   = 2000;   // This pulse sets the threshold for a transmission start bit
const int endBit     = 3000;   // This pulse sets the threshold for an end bit
const int one        = 1000;   // This pulse sets the threshold for a transmission that 
                               // represents a 1
const int zero       = 400;    // This pulse sets the threshold for a transmission that 
                               // represents a 0
                               
const int waitTime = 300;      // The amount of time to wait between pulses

//The following array holds the decoded IR data.
int ret[2];

bool hasDied = false;

void setup() {
  
  //Input Sensors
  pinMode(buttonPin, INPUT);

  //Output Sensors
  pinMode(ledPin, OUTPUT);
  pinMode(speakerPin, OUTPUT);
  pinMode(laserPin, OUTPUT);

  pinMode(lcdClockPin, OUTPUT);
  pinMode(lcdSerialPin, OUTPUT);
  
  //IR Sensors
  pinMode(sendPin, OUTPUT);
  pinMode(receiverPin, INPUT);
  
  //Serial Debug (for feedback)
  Serial.begin(9600);

  
  //Startup Sound
  for (int i = 1;i < 4;i++) {
    digitalWrite(ledPin, HIGH);
    playTone(1000/i, 100);
    digitalWrite(ledPin, LOW);
  }
  
  Serial.println("Gun Setup");
  
  //Game Setup
  currentLives = maxLives;
  currentBullets = maxBullets;
  Serial.println(currentBullets);

  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(WHITE);  
  display.setCursor(10,10);
  display.print("Player: " + String(playerID));
  display.setCursor(10,40);
  display.print("Lives: " + String(currentLives) + " / " + String(maxLives));
  display.setCursor(10,55);
  display.print("Bullets: " + String(currentBullets) + " / " + String(maxBullets));
  display.display();

  digitalWrite(laserPin, HIGH);
}

void loop() {

  if(hasDied == false){
    SenseIR();
    Trigger();
  }

  if(ret[0] != -1 && ret[0] != 0){
    playTone(1000, 50);
    Serial.print("Player: ");
    Serial.print(ret[0]);
    Serial.print(" Message: ");
    Serial.println(ret[1]);

    DoDamage();
  }

  
  if(ret[0] == -1){
    //Serial.println("ERROR: Unknown Signal");
    //playTone(3000, 300);
  }
}

void DoDamage(){
  if(ret[0] != playerID|| ret[0] != -1){
    if(currentLives > 1){
      currentLives--;

      digitalWrite(ledPin, HIGH);
      delay(100);
      digitalWrite(ledPin, LOW);
      
      for(int i = 0; i < 4; i++){
        playTone(10000 - i*100, 100/i);
      }  

      display.clearDisplay();
    
      display.setTextSize(1);
      display.setTextColor(WHITE);  
      display.setCursor(10,10);
      display.print("Player: " + String(playerID));
      display.setCursor(10,40);
      display.print("Lives: " + String(currentLives) + " / " + String(maxLives));
      display.setCursor(10,55);
      display.print("Bullets: " + String(currentBullets) + " / " + String(maxBullets));
      display.display();
    }
    else{
      GameOver();
      hasDied = true;
    }
  }
}

void GameOver(){
  if(hasDied == false){
      display.clearDisplay();
    
      display.setTextSize(2);
      display.setTextColor(WHITE);  
      display.setCursor(10,20);
      display.print("GAME OVER");
      display.display();

      digitalWrite(ledPin, HIGH);
      digitalWrite(laserPin, LOW);
      playTone(10000, 2500);
  }
}

void Trigger(){
  int buttonState = digitalRead(buttonPin);
  
  if(buttonState == 1 && isShooting == false){
    isShooting = true;
    
    if(currentBullets > 0){
      fireShot();
      Serial.println(currentBullets);
      
    }
    else{
      Serial.println("Out of bullets.");
    }
  }
  if(buttonState == LOW){
    isShooting = false;
  }
}

void fireShot(){
    currentBullets--;
    
    
    
    //Send IR data
    encodeIRpulse(playerID, 1);

    
    //enable the laser and led.
    digitalWrite(ledPin, HIGH);
    digitalWrite(laserPin, LOW);
    delay(200);
    digitalWrite(ledPin, LOW);
    digitalWrite(laserPin, HIGH);

    //Shooting sound
    playTone(4500, 300);
    playTone(4000, 200);
    
    //Update LCD Display
    display.clearDisplay();
    
    display.setTextSize(1);
    display.setTextColor(WHITE);  
    display.setCursor(10,10);
    display.print("Player: " + String(playerID));
    display.setCursor(10,40);
    display.print("Lives: " + String(currentLives) + " / " + String(maxLives));
    display.setCursor(10,55);
    display.print("Bullets: " + String(currentBullets) + " / " + String(maxBullets));
    display.display();

    
}

//The function for playing a single tone with a certain duration.
void playTone(int tone, int duration) {
  if(useSound == true){
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);
  }
 }
}

//IR Led receiver function.
void SenseIR(){
  
  //We will be sending a 8bit signal with IR, the first 4 bits define the sender of the signal. 
  //The other 4 bits define the message of the signal.
  int sender[4];
  //int message[4];
  int endPulse;

  
  if (pulseIn(receiverPin, LOW, 10000) < startBit) {
    ret[0] = -1;
    return;
  }

  //Assignment of pulses to bit array------||
  
  //sender bits
  sender[0]   = pulseIn(receiverPin, LOW);
  sender[1]   = pulseIn(receiverPin, LOW);
  sender[2]   = pulseIn(receiverPin, LOW);
  sender[3]   = pulseIn(receiverPin, LOW);

  //message bits
  //message[0]  = pulseIn(receiverPin, LOW);
  //message[1]  = pulseIn(receiverPin, LOW);
  //message[2]  = pulseIn(receiverPin, LOW);
  //message[3]  = pulseIn(receiverPin, LOW);

  endPulse = pulseIn(receiverPin, LOW);
  //---------------------------------------||

  
  for(int i = 0; i <= 3; i++){
    Serial.println(sender[i]);
    //Serial.println(message[i]);
  }
  

  //sender bit (player ID) decoding
  for(int i = 0; i <= 3; i++){
    //Serial.println(sender[i]);
    if(sender[i] > one && sender[i] < startBit) {
      sender[i] = 1;
    } 
    else if (sender[i] > zero && sender[i]) {
      sender[i] = 0;
    } 
    else 
    {
      //When the data is not a one or a zero, it is an unknown signal.
      ret[0] = -1;
      return;
    }
  }

  ret[0]=convert(sender);
  //Serial.println(ret[0]);

  /*
  //message bit decoding
  for(int i=0;i<=3;i++) {
    //Serial.println(message[i]);
    if(message[i] > one) {
      message[i] = 1;
    } else if (message[i] > zero) {
      message[i] = 0;
    } else {
      // Since the data is neither zero or one, we have an error
      Serial.println("unknown action");
      ret[0] = -1;
      return;
    }
  }
  ret[1]=convert(message);
  //Serial.println(ret[1]);
  */
  
  return;
  

}

void encodeIRpulse(int player, int message){
  int encoded[4];

  for (int i=0; i<4; i++) {
    encoded[i] = player>>i & B1;   //encode data as '1' or '0'
    
  }

  /*
  for (int i=4; i<8; i++) {
    encoded[i] = player>>i & B1;
  }
  */
  
  //Start of data stream.
  dataPulse(sendPin, startBit);
  digitalWrite(sendPin, HIGH);
  delayMicroseconds(waitTime);
   
  //Sending of data.
  for (int i=3; i>=0; i--) {
    if (encoded[i] == 0) {
     dataPulse(sendPin, zero);
    } else {
     dataPulse(sendPin, one);
    }
    
    digitalWrite(sendPin, HIGH);
    delayMicroseconds(waitTime);
  } 

  //End of data stream.
  dataPulse(sendPin, endBit);
}

void dataPulse(int pin, int pulseTime){
  for(int i = 0; i <= pulseTime/26; i++) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(13);
    digitalWrite(pin, LOW);
    delayMicroseconds(13);
  }
}

int convert(int bits[]) {
  int result = 0;
  int seed   = 1;
  for(int i=3;i>=0;i--) {
    if(bits[i] == 1) {
      result += seed;
    }
    seed = seed * 2;
  }
  
  return result;
}
 
 
