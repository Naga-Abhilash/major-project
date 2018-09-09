#include <Thread.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <stddef.h>
#include <Servo.h>

#include "SoftwareSerial.h"
//#include <dht.h>

SoftwareSerial esp(10,11);// RX, TX//6,7

String data;

String server = "192.168.4.2"; // www.example.com

String uri = "/phpfun/upload.php";// our example is /esppost.php
float greenPin = A0;
//dht sensor;

String temp ,hum,soil;
String temp1 ,hum1,soil1;
String slot,plant,soilmoisture,humidity,ldr,disturbance,duration;

#define char_length 30

Servo myservo;
int pos = 65;

#define EN        8  
//Direction pin
#define X_DIR     5 
#define Y_DIR     6
#define Z_DIR     7
//Step pin
#define X_STP     2
#define Y_STP     3 
#define Z_STP     4
#define MotorPin 26
#define MicroSwitchZ 37
#define MicroSwitchY 38
#define MicroSwitchX 39
#define MoistureSensor A8
#define ServoPin 24

#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

tmElements_t tm;
tmElements_t prevtm;
int prevtmsecond;
int prevtmminute;
int prevtmhour;
int prevtmday;
int prevtmmonth;
//int previousTime = RTC.getHours();

int par=0;
int grid_loc[10];
int seed_loc[10]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int endflag=0;
int endflagmoisture=0;
char seed_type[10]={'A','A','A','A','A','A','A','A','A','A'};
char plants[10][15]={"potato","onion"};
int delayTime=25; 
int stps;
int currx=0;
int curry=0;
int coor[11][2]={{0,0},{1,1},{1,-1},{4,1},{4,-1},{6,1},{6,-1},{8,1},{8,-1},{10,1},{10,-1}};


Thread myThread = Thread();
Thread mainThread = Thread();
Thread regularMoistureThread = Thread();
Thread EspThread = Thread();

int inp;
int inp2=1;
int inp3=1;
float threshold = 100;
float moisture_level[10]={threshold,threshold,threshold,threshold,threshold,threshold,threshold,threshold,threshold,threshold};
const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

tmElements_t tm_set;

class Node
{
  public:
    
    int data=0;
    Node *right;
    Node *left;

    Node()
    {
    
    }
};

class Scheduler
{
  public :
    int len = 0;
    Node *head = NULL;
    Node *temp = NULL;
    Node *temp1 = NULL; 
  
  int insert(int index, int inputData)
  {
    if(index > (len+1))
      return 0;
    else
    {
      if(index == 1)
      {
        Node *newNode = new Node();
        
        newNode->data = inputData;
        if(head != NULL)
          newNode->right = head;

        head = newNode;
        Node *temp2 = head;
        Node *temp3 = NULL;
        len = len+1;
      }
      else
      {
        temp = head;  
        for(int i=0;i<index-2;i++)
        {
          temp = temp->right;
        }
        temp1 = temp->right;
        Node *newNode = new Node();
        
        newNode->data = inputData;
        newNode->left = temp;
        temp->right = newNode;
        if(temp1 != NULL)
        {
          newNode->right = temp1;
          temp1->left = newNode;
        }
        len = len+1;
      }
      return 1;
    }
  }
  void displaydata(int len)
  {
    Node *temp=head;
    if(head == NULL)
      Serial.println("empty!");
     else
     {
        for(int i=0;i<len;i++)
        {
          Serial.print(temp->data);
          Serial.print(" ");
          temp=temp->right;
        }
     }
  }

  int getdata(int index)
  {
    if(index > len)
      return 0;
    else
    {
      Node *temp=head;
      for(int i=0;i<index-1;i++)
      {
        temp = temp->right;
      }
      return temp->data;
    }
  }
  
  int del(int index)
  {
    char buff[30];
    memset(buff, '\0', strlen(buff));
    if(index >= (len+1))
      return 0;
    else
    {
      if(index == 1)
      {
        Node *temp=head;
        head = temp->right;
        //sprintf(buff, "the deleted data is : %d", temp->data);
        //Serial.println(buff);
        delete temp;
        len=len-1;
      }
      else
      { 
        Node *temp=head;
        Node *temp1=head;
        for(int i=0;i<index-2;i++)
        {
          temp = temp->right;
        }
        temp1 = temp->right;
        temp->right = temp1->right;
        //sprintf(buff, "the deleted data is : %d", temp1->data);
        //Serial.println(buff);
        delete temp1;
        len=len-1;
      }
      return 1;
    }
  }
  
};

Scheduler sc;

void step(boolean dirx, byte dirPinx, byte stepperPinx, int stepsx)
{
  digitalWrite(dirPinx, dirx);
  //digitalWrite(dirPiny, diry);
 // delay(100);
  for (int i = 0; i < stps; i++)
  {
    digitalWrite(stepperPinx, HIGH);
    //digitalWrite(stepperPiny, HIGH);
    delayMicroseconds(delayTime); 
    digitalWrite(stepperPinx, LOW);
    //digitalWrite(stepperPiny, LOW);
    delayMicroseconds(delayTime); 
  }
}
void step1(boolean diry,byte dirPiny,byte stepperPiny, int stepsy)
{
 // digitalWrite(dirPinx, dirx);
  digitalWrite(dirPiny, diry);
 // delay(100);
  for (int i = 0; i < stps; i++)
  {
   // digitalWrite(stepperPinx, HIGH);
    digitalWrite(stepperPiny, HIGH);
    delayMicroseconds(delayTime); 
    //digitalWrite(stepperPinx, LOW);
    digitalWrite(stepperPiny, LOW);
    delayMicroseconds(delayTime); 
  }
}

void step2(boolean dirz,byte dirPinz,byte stepperPinz, int stepsz)
{
 // digitalWrite(dirPinx, dirx);
  digitalWrite(dirPinz, dirz);
 // delay(100);
  for (int i = 0; i < stps; i++)
  {
   // digitalWrite(stepperPinx, HIGH);
    digitalWrite(stepperPinz, HIGH);
    delayMicroseconds(delayTime); 
    //digitalWrite(stepperPinx, LOW);
    digitalWrite(stepperPinz, LOW);
    delayMicroseconds(delayTime); 
  }
}
void movexy(int x,int y)
{
  int newx=x-currx;
  int newy=y-curry;
  
  if(newx>0)
  {
    for(int i=0;i<newx;i++)
    {
      stps=13000;
      step(true, X_DIR, X_STP, stps);//forward anti
      //delay(500);
    }
  }
  else if(newx<0)
  {
    newx=-newx;
    for(int i=0;i<newx;i++)
    {
      stps=13000;
      step(false, X_DIR, X_STP, stps);//forward anti
      //delay(500);
    }
    
  }
  if(newy>0)
  {
    for(int i=0;i<newy;i++)
    {
      stps=5050;
      step1(false,Y_DIR,Y_STP,stps);//backward clock
      //delay(500);
    }
    
  }
  else if(newy<0)
  {
    newy=-newy;
    for(int i=0;i<newy;i++)
    {
      stps=5050;
      step1(true,Y_DIR,Y_STP,stps);//backward clock
      //delay(500);
    }
  }
  
  currx=x;
  curry=y;
  
}

void moveZ(bool dir)
{
  for(int i=0;i<3;i++){stps=14000;
  step2(dir,Z_DIR,Z_STP,stps);}//forward anti
  delay(500);
}

void plantSeed(int loc1)
{
  
  if(loc1 == 1)
  {
    Serial.println("seeding1");
    for (pos = 65; pos >= 45; pos -= 1) 
    { // goes from 180 degrees to 0 degrees
    myservo.write(pos);
    Serial.println(pos);// tell servo to go to position in variable 'pos'
    delay(25);
    }
    for (pos = 45; pos <= 65; pos += 1) 
    { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);
    Serial.println(pos);// tell servo to go to position in variable 'pos'
    delay(25);                    // waits 15ms for the servo to reach the position
    }
    
  
  }
  else if(loc1 == 2)
  {
    Serial.println("seeding2");
    for (pos = 65; pos <= 85; pos += 1) 
    { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos); 
    Serial.println(pos);// tell servo to go to position in variable 'pos'
    delay(25);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 85; pos >= 65; pos -= 1) 
  { // goes from 180 degrees to 0 degrees
    myservo.write(pos);
    Serial.println(pos);// tell servo to go to position in variable 'pos'
    delay(25);
   }
   
  }
  
}

void motor_start(int duration)
{
  digitalWrite(MotorPin,LOW);
  delay(duration);
  digitalWrite(MotorPin,HIGH);
  delay(duration);
}

float checkMoisture()
{
  float moistureSensorRead = analogRead(A8);
  delay(1000);
  return moistureSensorRead;
}

void moistureSensingRoutine(int loc[])
{
  moveToxy(0);
  for(int i=0;i<10;i++)
  {
    if(loc[i]!=-1)
    {
      moveToxy(i+1);
      motor_start(2000);
      moveZ(true);
      moisture_level[i]=checkMoisture();
      Serial.print("moisture : ");
      Serial.println(moisture_level[i]);
      moveZ(false);
      if(moisture_level[i]<threshold)
        motor_start(2000);  
    }
  }
  moveToxy(0);
}

void main_callback()
{
  //Serial.println("RTC ");
  while(sc.len>0)
  {
    Serial.println("hi");
    switch(sc.getdata(1))
    {
      case 1 : seeding_start(grid_loc);
                sc.del(1);
                RTC.read(prevtm);
                prevtmsecond = prevtm.Second;
                prevtmminute = prevtm.Minute;
                prevtmhour = prevtm.Hour;
                prevtmday = prevtm.Day;
                endflag = 0;
                break;
                delay(500);
      case 2 : moistureSensingRoutine(seed_loc);
                sc.del(1);
                RTC.read(prevtm);
                prevtmsecond = prevtm.Second;
                prevtmminute = prevtm.Minute;
                prevtmhour = prevtm.Hour;
                prevtmday = prevtm.Day;
                endflagmoisture = 0;
                delay(500);
                break;
      default : break; 
    }
  }
  
}

void moveToxy(int ch)
{
  int chx, chy;
  chx=coor[ch][0];
  chy=coor[ch][1];
  movexy(chx,chy);
}




void seeding_start(int loc[])
{
  //Serial.print("yo1");
  moveToxy(0);
  for(int i=0;i<10;i++)
  {
    if(loc[i]!=0)
    {
      moveToxy(i+1);
      motor_start(2000);
      moveZ(true);
      moisture_level[i]=checkMoisture();
      Serial.print("moisture seeding : ");
      Serial.println(moisture_level[i]);
      moveZ(false);
      plantSeed(loc[i]);
      if(moisture_level[i]<threshold)
        motor_start(2000);  
    }
  }
  moveToxy(0);
}

void httppost () {

esp.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");//start a TCP connection.
Serial.println(".....");

if( esp.find("OK")) {

Serial.println("TCP connection ready");

} delay(1000);

String postRequest =

"POST " + uri + " HTTP/1.0\r\n" +

"Host: " + server + "\r\n" +

"Accept: *" + "/" + "*\r\n" +

"Content-Length: " + data.length() + "\r\n" +

"Content-Type: application/x-www-form-urlencoded\r\n" +

"\r\n" + data;

String sendCmd = "AT+CIPSEND=";//determine the number of caracters to be sent.

esp.print(sendCmd);

esp.println(postRequest.length() );

delay(500);

if(esp.find(">")) { Serial.println("Sending.."); esp.print(postRequest);

if( esp.find("SEND OK")) { Serial.println("Packet sent");

while (esp.available()) {

String tmpResp = esp.readString();

Serial.println(tmpResp);

}

// close the connection

esp.println("AT+CIPCLOSE");

}

}}

void Esp_callback()
{
  for(int i=0;i<10;i++)
  {
  slot = String(i+1);

  if(seed_loc[i]!=0 && seed_loc[i]!=-1)
  plant = String(plants[seed_loc[i]-1]);
  else
  plant = String("no plant");

  soilmoisture=String(moisture_level[i]);
  

 humidity=String(55);
 ldr=String(40);
 disturbance=String(30);
 duration=String(2);

  data = "slot=" + slot + "&plant=" + plant + "&soilmoisture=" + soilmoisture + "&humidity=" + humidity + "&ldr=" + ldr + "&disturbance=" + disturbance + "&duration=" + duration;// data sent must be under this form //name1=value1&name2=value2.

  httppost();

  delay(1000);
  }
}

void bluetooth_callback()
{
  //Serial.println("RTC ");
  if(endflag == 0)
  {
    while(Serial3.available()>0)
    {
      seed_type[par]=Serial3.read();
      if((seed_type[par] != '0') && (seed_type[par] != 'A'))
        grid_loc[par]=seed_type[par]-'0';
      else
        grid_loc[par]=0;
      
      if(seed_type[par] == 'A')
        seed_loc[par]=-1;
      else if(seed_type[par] != '0')
        seed_loc[par]=seed_type[par]-'0';
      Serial.print(par);
      Serial.print(" ");
      Serial.println(seed_type[par]);
      par++;
      if(par == 10)
      {
        par = 0;
        Serial.print("yo");
        endflag=1;
      }
    }
  }
  else if(endflag == 1)
  {
    if(sc.len == 0)
      sc.insert(1,1);
    else if(sc.len > 0)
    {
      for(int i=0;i<=(sc.len);i++)
      {
        if(sc.getdata(i+1) != 1)
        {
          sc.insert(i+2,1);
          break;
        }
      }
    }
  }
}


void reg_moisture_callback()
{
  /*if (RTC.read(tm)) { 
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.print('/');
    Serial.print(tm.Month);
    Serial.print('/');
    Serial.print(tm.Day);
    Serial.print(' ');
    Serial.print(tm.Hour);
    Serial.print(':');
    Serial.print(tm.Minute);
    Serial.print(':');
    Serial.print(tm.Second);
    Serial.println(); */
    RTC.read(tm);
  long yearToMonth;
  long monthToDay;
  long dayToHour;
  long hourToMin;
  long minToSec;
  long sec;
  
  if((tm.Day - prevtm.Day) >= 0)
  {
    dayToHour = (tm.Day - prevtmday)*24;
    Serial.print("dayToHour : ");
    Serial.println(dayToHour);
    hourToMin = ((tm.Hour - prevtmhour) + (dayToHour))*60;
    Serial.print("hourToMin : ");
    Serial.println(hourToMin);
    minToSec = ((tm.Minute - prevtmminute) + (hourToMin))*60;
    sec = tm.Second + (minToSec);
    
    Serial.print("tm.Second : ");
    Serial.println(tm.Second);
    Serial.print("prevtmsecond : ");
    Serial.println(prevtmsecond);
    if(endflagmoisture == 0)
    {
      if((sec - prevtmsecond) > 60)
      {
        endflagmoisture=1;
        Serial.print("sc.getdata(1) : ");
        sc.insert(sc.len+1,2);
        Serial.print("sc.getdata(1) : ");
        Serial.println(sc.getdata(1));
      }
    }
  }
   // }
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm_set.Hour = Hour;
  tm_set.Minute = Min;
  tm_set.Second = Sec;
  return true;
}

bool getDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  tm_set.Day = Day;
  tm_set.Month = monthIndex + 1;
  tm_set.Year = CalendarYrToTm(Year);
  return true;
}

void setRTCTime()
{
  bool parse=false;
  bool config=false;

  // get the date and time the compiler was run
  if (getDate(__DATE__) && getTime(__TIME__)) {
    parse = true;
    // and configure the RTC with this info
    if (RTC.write(tm_set)) {
      config = true;
    }
  }

  Serial.begin(9600);
  while (!Serial) ; // wait for Arduino Serial Monitor
  delay(200);
  if (parse && config) {
    Serial.print("DS1307 configured Time=");
    Serial.print(__TIME__);
    Serial.print(", Date=");
    Serial.println(__DATE__);
  } else if (parse) {
    Serial.println("DS1307 Communication Error :-{");
    Serial.println("Please check your circuitry");
  } else {
    Serial.print("Could not parse info from the compiler, Time=\"");
    Serial.print(__TIME__);
    Serial.print("\", Date=\"");
    Serial.print(__DATE__);
    Serial.println("\"");
  }
}

void reset() {

esp.println("AT+RST");

delay(1000);

if(esp.find("OK") ) Serial.println("Module Reset");
Serial.println(".....");
}



void setup() {
  // put your setup code here, to run once:
  pinMode(MotorPin,OUTPUT);
  digitalWrite(MotorPin,HIGH);
  pinMode(MoistureSensor,INPUT);
  Serial.begin(9600);
  Serial3.begin(9600);
  #ifdef AVR
  Wire.begin();
  #else
  Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
  #endif
  setRTCTime();
  myservo.attach(ServoPin);
    myservo.write(pos);
    for (pos = 45; pos <= 85; pos += 1) 
    { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo.write(pos); 
      Serial.println(pos);// tell servo to go to position in variable 'pos'
      delay(25);                       // waits 15ms for the servo to reach the position
    }
    for (pos = 85; pos >= 45; pos -= 1) { // goes from 180 degrees to 0 degrees
      myservo.write(pos);     
      Serial.println(pos);// tell servo to go to position in variable 'pos'
      delay(25);                      // waits 15ms for the servo to reach the position
    }
    pos=65;
    myservo.write(pos);
  Serial.println("RTC ");
  RTC.read(prevtm);
  Serial.print("prevtmsecond : ");
  
  prevtmsecond = prevtm.Second;
  Serial.println(prevtmsecond);
  prevtmminute = prevtm.Minute;
  prevtmhour = prevtm.Hour;
  prevtmday = prevtm.Day;
  delay(2000);
  pinMode(X_DIR, OUTPUT); pinMode(X_STP, OUTPUT);
  pinMode(Y_DIR, OUTPUT); pinMode(Y_STP, OUTPUT);
  pinMode(Z_DIR, OUTPUT); pinMode(Z_STP, OUTPUT);

  pinMode(EN, OUTPUT);
  digitalWrite(EN, LOW);
  myThread.onRun(bluetooth_callback);
  myThread.setInterval(5);

  mainThread.onRun(main_callback);
  mainThread.setInterval(5);
  
  regularMoistureThread.onRun(reg_moisture_callback);
  regularMoistureThread.setInterval(5);

  EspThread.onRun(Esp_callback);
  EspThread.setInterval(5);
  
  sc=Scheduler();
  

  if (RTC.read(tm)) {
    Serial.print("Ok, Time = ");
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.println();
  }

  esp.begin(9600);

  Serial.begin(9600);

  reset();
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("RTC ");
  if(mainThread.shouldRun())
    mainThread.run();
    
  if(myThread.shouldRun())
    myThread.run();

  if(regularMoistureThread.shouldRun())
    regularMoistureThread.run();

   if(EspThread.shouldRun())
    EspThread.run();

}
