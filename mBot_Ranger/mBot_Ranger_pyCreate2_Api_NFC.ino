#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <MeAuriga.h>
#include <PN532_SWHSU.h>
#include <PN532_HSU.h>
#include <PN532.h>

//Encoder Motor
MeEncoderOnBoard Encoder_1(SLOT1);
MeEncoderOnBoard Encoder_2(SLOT2);
MeUltrasonicSensor ultrasonic_10(10);
MeLineFollower linefollower_9(9);
MeRGBLed rgbled_0(0, 12);
Me7SegmentDisplay seg7_6(8);

MeSerial SWSerial(PORT_6);

//PN532_SWHSU pn532swhsu(SWSerial);
//PN532 nfc_reader(pn532swhsu);

PN532_HSU pn532hsu(Serial2);
PN532 nfc_reader(pn532hsu);

long currentTime = 0;
long lastTime = 0;
long prev_tagTime = 0;
long curr_tagTime = 0;
double angle_rad = PI/180.0;
double angle_deg = 180.0/PI;
void EncoderStop();
double rotation;
double degreesPerInch;
double rpm;
void Initialize();
double turnDegreeL;
double turnDegreeR;
void LeftSquare();
void RightSquare();
void Reverse(double inches, double speed);
void forward(double inches, double speed);
void TurnLeft(double degrees, double speed);
void TurnRight(double degrees, double speed);
float factor = 0.996;
float factor1 = 0.950; //for right motor forward motion correction
float factor2 = 0.950; //for right motor backward motion correction
bool RxFlag;
bool LineFlag;
bool ForwardFlag;
bool ReverseFlag;
bool RightFlag;
bool LeftFlag;
long KeepAliveTime = 0;
short low_speed = 0;
short high_speed = 0;
int velocity = 0;
short low_distance = 0;
short high_distance = 0;
short distance = 0;
short low_radius = 0;
short high_radius = 0;
short radius = 0;
short low_angle = 0;
short high_angle = 0;
short angle = 0;
unsigned short Obstacle_ID = 0;
unsigned char OBSTACLE;
uint8_t tag_read_success;
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; // Buffer to store the returned UID
uint8_t uidLength;                       // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
union input_data 
{
   byte command[8];
   byte indat[8];
}inp;

void isr_process_encoder1(void)
{
      if(digitalRead(Encoder_1.getPortB()) == 0){
            Encoder_1.pulsePosMinus();
      }else{
            Encoder_1.pulsePosPlus();
      }
}

void isr_process_encoder2(void)
{
      if(digitalRead(Encoder_2.getPortB()) == 0){
            Encoder_2.pulsePosMinus();
      }else{
            Encoder_2.pulsePosPlus();
      }
}

void _delay(float seconds){
    long endTime = millis() + seconds * 1000;
    while(millis() < endTime)_loop();
}

void move(int direction, int speed)
{
      int leftSpeed = 0;
      int rightSpeed = 0;
      
      if(direction == 1){
            leftSpeed = -speed*factor1;
            rightSpeed = speed;
      }else if(direction == 2){
            leftSpeed = speed*factor2;
            rightSpeed = -speed;
      }else if(direction == 3){
            leftSpeed = -speed;
            rightSpeed = -speed;
      }else if(direction == 4){
            leftSpeed = speed;
            rightSpeed = speed;
      }
      Encoder_1.setTarPWM(leftSpeed);
      Encoder_2.setTarPWM(rightSpeed);
}

void moveDegrees(int direction,long degrees, int speed_temp)
{
      speed_temp = abs(speed_temp);
      
      if(direction == 1)
      {
            Encoder_1.move(-degrees*factor,(float)speed_temp);
            Encoder_2.move(degrees,(float)speed_temp);
      }
      else if(direction == 2)
      {
            Encoder_1.move(degrees,(float)speed_temp);
            Encoder_2.move(-degrees,(float)speed_temp);
      }
      else if(direction == 3)
      {
            Encoder_1.move(-degrees,(float)speed_temp);
            Encoder_2.move(-degrees,(float)speed_temp);
      }
      else if(direction == 4)
      {
            Encoder_1.move(degrees,(float)speed_temp);
            Encoder_2.move(degrees,(float)speed_temp);
      }
}

void EncoderStop(void)
{
    _delay(0.2);
    while(!((((Encoder_1.getCurrentSpeed())==(0))) && (((Encoder_2.getCurrentSpeed())==(0)))))
    {
        _loop();
    }
}

void Reverse(double inches, double speed = 150)
{
    rotation = (degreesPerInch) * (inches);
    moveDegrees(2,rotation,speed);
    EncoderStop();
}

void Reverse_NonStop(double speed = 150)
{
    move(2,speed);
}

void Forward(double inches, double speed = 150)
{
    rotation = (degreesPerInch) * (inches);
    moveDegrees(1,rotation,speed);
    EncoderStop();
}

void Forward_NonStop(double speed = 150)
{
    move(1,speed);
}

void Initialize(void)
{
    rpm = 150;
    degreesPerInch = 70.73;
    turnDegreeL = 5.450; //5.7;
    turnDegreeR = 5.450; //5.7;
}

void LeftSquare(void)
{
    for(int __i__=0;__i__<4;++__i__)
    {
        Forward(20,rpm);
        _delay(0.5);
        TurnLeft(90,rpm);
    }
}

void RightSquare(void)
{
    for(int __i__=0;__i__<4;++__i__)
    {
        Forward(20,rpm);
        _delay(0.5);
        TurnRight(90,rpm);
    }
}

void LineFollowing(void)
{
  if(linefollower_9.readSensors() == 3.000000){
    move(1,50/100.0*255);
  }

  if(linefollower_9.readSensors() == 2.000000){
    Encoder_1.setTarPWM(-1*50/100.0*255);
    Encoder_2.setTarPWM(0/100.0*255);
  }

  if(linefollower_9.readSensors() == 1.000000){
    Encoder_1.setTarPWM(-1*0/100.0*255);
    Encoder_2.setTarPWM(50/100.0*255);
  }

  if(linefollower_9.readSensors() == 0.000000){
    move(2,35/100.0*255);
  }
}

void TurnRight(double degrees, double speed = 150)
{
  rotation = (degrees) * (turnDegreeR);
  Encoder_1.move(rotation,abs(speed));
  Encoder_2.move(rotation,abs(speed));
  EncoderStop();
}

void TurnRight_NonStop(double speed = 150)
{
  move(4,speed);
}

void TurnLeft(double degrees, double speed = 150)
{
  rotation = (degrees) * (turnDegreeL);
  Encoder_1.move((rotation) * (-1),abs(speed));
  Encoder_2.move((rotation) * (-1),abs(speed));
  EncoderStop();
}

void TurnLeft_NonStop(double speed = 150)
{
  move(3,speed);
}

void StopMoving(void)
{
    Encoder_1.setTarPWM(0);
    Encoder_2.setTarPWM(0);
    EncoderStop();   
}

void Obstacle_Avoidance(void)
{
    TurnRight(90,rpm);
    Forward(16,rpm);
    TurnLeft(90,rpm);
    Forward(32,rpm);
    TurnLeft(90,rpm);
    Forward(16,rpm);
    TurnRight(45,rpm);
    StopMoving();
}

void _loop(){
    Encoder_1.loop();
    Encoder_2.loop();
}

void setup(){
    
    Serial.begin(115200);
    SWSerial.begin(115200);
    
    attachInterrupt(Encoder_1.getIntNum(), isr_process_encoder1, RISING);
    Encoder_1.setPulse(9);
    Encoder_1.setRatio(39.267);
    Encoder_1.setPosPid(1.8,0,1.2);
    Encoder_1.setSpeedPid(0.18,0,0);
    attachInterrupt(Encoder_2.getIntNum(), isr_process_encoder2, RISING);
    Encoder_2.setPulse(9);
    Encoder_2.setRatio(39.267);
    Encoder_2.setPosPid(1.8,0,1.2);
    Encoder_2.setSpeedPid(0.18,0,0);
    //Set Pwm 8KHz
    TCCR1A = _BV(WGM10);
    TCCR1B = _BV(CS11) | _BV(WGM12);
    TCCR2A = _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(CS21);
    
    rgbled_0.setpin(44);
    RxFlag = false;
    LineFlag = false; 
    ForwardFlag = false;
    ReverseFlag = false;
    RightFlag = false;
    LeftFlag = false;
    KeepAliveTime = 0;
    OBSTACLE = 0;
    Obstacle_ID = 0;
    Initialize();
    
    nfc_reader.begin();
    uint32_t versiondata = nfc_reader.getFirmwareVersion();
    
    if (! versiondata) {
      Serial.print("Didn't find PN53x board");
      while (1); // halt
    }

    // Got version data, print it out!
    Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
    Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
    Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
        
    nfc_reader.setPassiveActivationRetries(0xFF);
    nfc_reader.SAMConfig();
   
    /* 
    if (nfc_reader.tagPresent())
    {
      rgbled_0.setColor(0,101,255,0);
      rgbled_0.show();
      _delay(2);
      rgbled_0.setColor(0,0,0,0);
      rgbled_0.show();

      Serial.println("Found NFC Tag!");
      NfcTag tag = nfc_reader.read();
      Serial.println(tag.getTagType());
      Serial.print("UID: ");
      Serial.println(tag.getUidString());
    }
    else
    {
      Serial.println("No NFC Tag Present!"); 
    }
    */    
    
    rgbled_0.setColor(0,101,255,40);
    rgbled_0.show();
    _delay(2);
    rgbled_0.setColor(0,0,0,0);
    rgbled_0.show();

    lastTime = millis()/1000;
    prev_tagTime = millis();
    curr_tagTime = prev_tagTime;
    Serial.println("Robot Test Start at relative time 0 sec !");    
}

void loop(){

    bool KeepAliveFlag = false;
    int display_currentTime_no_overflow = 0;
    static int prev_display_currentTime_no_overflow = 0;
           
    currentTime = millis()/1000 - lastTime;
      
    if((currentTime % 5) == 0){
      display_currentTime_no_overflow = currentTime % 10000;
      if (display_currentTime_no_overflow != prev_display_currentTime_no_overflow) {
        seg7_6.display((long)(display_currentTime_no_overflow));
        prev_display_currentTime_no_overflow = display_currentTime_no_overflow;        
      }
    }

    if((currentTime % 10) == 0){
      if (KeepAliveTime != currentTime){
        KeepAliveTime = currentTime;
        KeepAliveFlag = true;        
      }
    }

    if(KeepAliveFlag == true){
      SWSerial.println("Link Alive");
      KeepAliveFlag = false;
    }

    curr_tagTime = millis();
    if ((curr_tagTime - prev_tagTime) >= 40) {
      prev_tagTime = curr_tagTime;
      //StopMoving();
      tag_read_success = 0;
      tag_read_success = nfc_reader.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength, 25);
      if (tag_read_success) {

        StopMoving();
        SWSerial.print("Tag ID");
        for (uint8_t i=0; i < uidLength; i++) 
        {
          SWSerial.print(" 0x");SWSerial.print(uid[i], HEX); 
        }
        SWSerial.print(" at time ");
        SWSerial.println(currentTime);
        Forward(4,rpm);
        //_delay(0.2);
      }
    }
    
    if(ultrasonic_10.distanceCm() < 20.32){
      OBSTACLE = 1;
      
      //Obstacle_ID = Obstacle_ID + 1;
      //if (Obstacle_ID == 11) {
      //  Obstacle_ID = 1;
      //}
      currentTime = millis()/1000 - lastTime;
      SWSerial.print("Obstacle detected");
      //SWSerial.print(Obstacle_ID);
      SWSerial.print(" at time ");
      SWSerial.println(currentTime);
      StopMoving();
      _delay(0.2);

      for(int i=0; i < 10; i++) { //Delay for 10 seconds
        _delay(1);

        currentTime = millis()/1000 - lastTime;
        
        if((currentTime % 5) == 0){
          display_currentTime_no_overflow = currentTime % 10000;
          seg7_6.display((long)(display_currentTime_no_overflow));
        }        

        if((currentTime % 10) == 0){
          Serial2.println("Link Alive");
        }
      }
    }

    if(OBSTACLE == 1) {
      if(ultrasonic_10.distanceCm() < 20.32){
        Obstacle_Avoidance();
      }
      OBSTACLE = 0;      
    }
    
    if(SWSerial.available())
    {
      int index = 0;

      for (int i=0; i < 8; i++){
        inp.indat[i] = 0x00;
      }
   
      while (SWSerial.available() > 0){
            inp.indat[index] = SWSerial.read();
            index ++;
            if (index == 8) {
              break;
            }
      }
      
      while (SWSerial.available() > 0){
        SWSerial.read();
        _delay(0.0015);
      }
      
      RxFlag = true;
    }

    if (RxFlag == true) { //Uplink Command is received from Test Station
      switch(inp.command[0]){

        case(0xC9): low_speed = (inp.command[2] & 0x00FF); //Drive Distance command 201
                    high_speed = (inp.command[1] & 0x00FF) << 8;
                    velocity = low_speed | high_speed;
                    
                    if ((velocity > 255) || (velocity <= 0)) {
                        velocity = rpm;
                    }
                    
                    low_distance = (inp.command[4] & 0x00FF); 
                    high_distance = (inp.command[3] & 0x00FF) << 8;
                    distance = low_distance | high_distance;
                    
                    if (distance > 0) {
                        Forward(distance, velocity);
                    }
                    else {
                        Reverse(-1.0*distance, velocity);
                    }
                    ForwardFlag = false;
                    ReverseFlag = false;
                    RightFlag = false;
                    LeftFlag = false;                      
                    LineFlag = false;
                    SWSerial.print("AcK RX ");
                    SWSerial.print(inp.command[0],HEX);
                    break;

        case(0xCA): low_speed = (inp.command[2] & 0x00FF); //Turn Angle command 202
                    high_speed = (inp.command[1] & 0x00FF) << 8;
                    velocity = low_speed | high_speed;
                    
                    if ((velocity > 255) || (velocity <= 0)) {
                        velocity = rpm;
                    }
                    
                    low_angle = (inp.command[4] & 0x00FF); 
                    high_angle = (inp.command[3] & 0x00FF) << 8;
                    angle = low_angle | high_angle;
                    
                    if (angle > 0) { //CCW rotation correpsonds tp positive rotation angle
                        TurnLeft(angle,velocity);
                    }
                    else { //CW rotation correpsonds to negative rotation angle
                        angle = abs(angle);
                        TurnRight(angle,velocity);
                    }
                    ForwardFlag = false;
                    ReverseFlag = false;
                    RightFlag = false;
                    LeftFlag = false;                      
                    LineFlag = false;
                    SWSerial.print("AcK RX ");
                    SWSerial.print(inp.command[0],HEX);
                    break;
    
        case(0x89): low_speed = (inp.command[2] & 0x00FF); //Drive command 137
                    high_speed = (inp.command[1] & 0x00FF) << 8;
                    velocity = low_speed | high_speed;
                    
                    if (velocity > 255) {
                        velocity = rpm;
                    }
                    else if (velocity < -255) {
                        velocity = -rpm;
                    }
                    
                    low_radius = (inp.command[4] & 0x00FF);
                    high_radius = (inp.command[3] & 0x00FF) << 8;
                    radius = low_radius | high_radius;
        
                    if (radius == 0x8000){ //Drive straight
                      if (velocity > 0) { //Move forward 
                        Forward_NonStop(velocity);
                        ForwardFlag = true;
                        ReverseFlag = false;
                        RightFlag = false;
                        LeftFlag = false;                      
                        LineFlag = false;
                      }
                      else if (velocity < 0) { //Move backwards
                        velocity = abs(velocity);
                        Reverse_NonStop(velocity);
                        ForwardFlag = false;
                        ReverseFlag = true;
                        RightFlag = false;
                        LeftFlag = false;                                            
                        LineFlag = false;                        
                      }
                      else { //Stop Moving as velocity = 0
                        StopMoving();
                        ForwardFlag = false;
                        ReverseFlag = false;
                        RightFlag = false;
                        LeftFlag = false;                                            
                        LineFlag = false;
                      }
                    }
                    else if (radius == 0xFFFF) { //Turn in place clockwise
                        velocity = abs(velocity);
                        TurnRight_NonStop(velocity);
                        ForwardFlag = false;
                        ReverseFlag = false;
                        RightFlag = true;
                        LeftFlag = false;                                            
                        LineFlag = false;
                    }
                    else if (radius == 0x0001) { //Turn in place counter-clockwise
                        velocity = abs(velocity);
                        TurnLeft_NonStop(velocity);
                        ForwardFlag = false;
                        ReverseFlag = false;
                        RightFlag = false;
                        LeftFlag = true;                                            
                        LineFlag = false;
                    }
                    
                    SWSerial.print("AcK RX ");
                    SWSerial.print(inp.command[0],HEX);
                    break;

        case(0xCB): LineFlag = true; //Line Following command 203
                    ForwardFlag = false;
                    ReverseFlag = false;
                    RightFlag = false;
                    LeftFlag = false;                                            
                    SWSerial.print("AcK RX ");
                    SWSerial.print(inp.command[0],HEX);
                    break;
                           
        default:    break;
      }
      for (int i=0; i < 8; i++){
        inp.indat[i] = 0x00;
      }      
      RxFlag = false;
    }

    if (ForwardFlag == true){
      Forward_NonStop(velocity);
    }
    
    if (ReverseFlag == true){
      Reverse_NonStop(velocity);
    }
    
    if (RightFlag == true){
      TurnRight_NonStop(velocity);
    }
    
    if (LeftFlag == true){
      TurnLeft_NonStop(velocity);
    }
    
    if (LineFlag == true){
      LineFollowing();
    }
       
    _loop();
}
