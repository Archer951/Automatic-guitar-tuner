 #include <Arduino.h>
#include <U8x8lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

int page = 1;
bool SUflag = false;
bool SlopeUp = false;

bool SDflag = false;
bool SlopeDown = false;

bool SBflag = false;
bool SlopeBack = false;

bool SSflag = false;
bool SlopeSelect = false;
bool checked = false;
int K;
int i;
int val;
int p; // menu stage

float vin = 0.0;
float vout = 0.0;
int value = 0;
int MaxPWM = 255;
float DesiredHz = 0;
float LowerLimit = 0;
float UpperLimit = 0;
int pomocnicza;
float e = 0;
float PosHzTol = 0;
float NegHzTol = 0;
//clipping indicator variables
boolean clipping = 0;

//data storage variables
byte newData = 0;
byte prevData = 0;
unsigned int time = 0;//keeps time and sends vales to store in timer[] occasionally
int timer[10];//storage for timing of events
int slope[10];//storage for slope of events
unsigned int totalTimer;//used to calculate period
unsigned int period;//storage for period of wave
byte index = 0;//current storage index
float frequency;//storage for frequency calculations
int maxSlope = 0;//used to calculate max slope as trigger point
int newSlope;//storage for incoming slope data

//variables for decided whether you have a match
byte noMatch = 0;//counts how many non-matches you've received to reset variables if it's been too long
byte slopeTol = 3;//slope tolerance- adjust this if you need
int timerTol = 10;//timer tolerance- adjust this if you need

//variables for amp detection
unsigned int ampTimer = 0;
byte maxAmp = 0;
byte checkMaxAmp;
byte ampThreshold = 10;//raise if you have a very noisy signal


void setup(void)
{
  Serial.begin(9600);
  pinMode(3, OUTPUT); // PWM
  pinMode(4, OUTPUT); // DIR
  pinMode(13,INPUT); // DOWN
  pinMode(11,INPUT); // UP
  pinMode(19, INPUT); // OK
  pinMode(A0,INPUT); // TUNER
  pinMode(A3,INPUT); // VOLTAGE
  digitalWrite(4,HIGH);
  u8x8.begin();
  u8x8.setContrast(255);
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_7x14B_1x2_f);
  u8x8.setInverseFont(1);
  u8x8.drawString(1,1,"Tune");
  u8x8.drawString(1,3,"Em");
  u8x8.drawString(1,5,"All");
  delay(300);
  u8x8.clear();
  i = 1;
  p = 1; 
//  value = analogRead(A3);
//  vout = (value * 5.0) / 1024.0;
//  vin = vout / (9880.0 / (109480.0));
//  if (vin<0.09) {
//   vin=0.0;//statement to quash undesired reading !
//  }
//  if(vin > 6.0)
//  {
//  MaxPWM = 255.0 - ((vin - 6.0)/vin*255.0);
//  }
//  else MaxPWM = 255; 
cli();
  ADCSRA = 0;
  ADCSRB = 0;
  
  ADMUX |= (1 << REFS0); //ustawienie napiecia referencyjnego
  ADMUX |= (1 << ADLAR); //left align the ADC value- so we can read highest 8 bits from ADCH register only
  //ADMUX |= (1 << MUX1) || (1<<MUX0);
  
  ADCSRA |= (1 << ADPS2) | (1 << ADPS0); //set ADC clock with 32 prescaler- 16mHz/32=500kHz
  ADCSRA |= (1 << ADATE); //enabble auto trigger
  ADCSRA |= (1 << ADIE); //enable interrupts when measurement complete
  ADCSRA |= (1 << ADEN); //enable ADC
  ADCSRA |= (1 << ADSC); //start ADC measurements
  
  sei();
   
  
}
void loop(){
  Serial.print(A0);
  Serial.print("    ");
  Serial.println(A3);

//--------------------------- MAIN -----------------------------------

  
  if (page == 1) // Main menu page
    {
      p = 1;
      val = 3;
      u8x8.setInverseFont(1);
      u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
      u8x8.drawString(0,0,"      MENU      ");
      u8x8.setInverseFont(0);
      u8x8.drawString(1,1,"One String");
      u8x8.drawString(1,2,"All Strings");
      u8x8.drawString(1,3,"Options");
      u8x8.drawString(0,i,">");
    }

//----------------------- ONE STRING ---------------------------------

    if (page == 10) // One String Page
    {
      p = 2;
      val = 7;
      u8x8.setInverseFont(1);
      u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
      u8x8.drawString(0,0,"  CHOSE STRING  ");
      u8x8.setInverseFont(0);
      u8x8.drawString(1,1,"E");
      u8x8.drawString(1,2,"A");
      u8x8.drawString(1,3,"D");
      u8x8.drawString(1,4,"G");
      u8x8.drawString(1,5,"B");
      u8x8.drawString(1,6,"E");
      u8x8.drawString(1,7,"Back");
      u8x8.drawString(0,i,">");
      pomocnicza = i;
      
    }
  if (page == 11) // All string -> Standard choice Page
    {
      p = 3;
      val = 4;
      u8x8.setInverseFont(1);
      u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
      u8x8.drawString(0,0," CHOSE TUNNING  ");
      u8x8.setInverseFont(0);
      u8x8.drawString(1,1,"Standard");
      u8x8.drawString(1,2,"Half Down");
      u8x8.drawString(1,3,"Step Down");
      u8x8.drawString(1,4,"Back");
      u8x8.drawString(0,i,">");
      checked = false;
    }
  // ------------------------- ALL STRINGS ---------------------------------
  
  if (page == 20) // All String Page
    { 
      p = 2;
      val = 3;
      u8x8.setInverseFont(1);
      u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
      u8x8.drawString(0,0," CHOSE TUNNING  ");
      u8x8.setInverseFont(0);
      u8x8.drawString(1,1,"Standard");
      u8x8.drawString(1,2,"Drop");
      u8x8.drawString(1,3,"Back");
      u8x8.drawString(0,i,">");
    }

    if (page == 21) // All string -> Standard choice Page
    {
      p = 3;
      val = 4;
      u8x8.setInverseFont(1);
      u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
      u8x8.drawString(0,0," CHOSE TUNNING  ");
      u8x8.setInverseFont(0);
      u8x8.drawString(1,1,"Standard");
      u8x8.drawString(1,2,"Half Down");
      u8x8.drawString(1,3,"Step Down");
      u8x8.drawString(1,4,"Back");
      u8x8.drawString(0,i,">");
    }
    if (page == 22) // All String -> Drop choice Page
    {
      p = 3;
      val = 4;
      u8x8.setInverseFont(1);
      u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
      u8x8.drawString(0,0,"   CHOSE DROP   ");
      u8x8.setInverseFont(0);
      u8x8.drawString(1,1,"Drop D");
      u8x8.drawString(1,2,"Drop C#");
      u8x8.drawString(1,3,"Drop C");
      u8x8.drawString(1,4,"Back");
      u8x8.drawString(0,i,">");
    }

// ------------------------- OPTIONS --------------------------------

    if (page == 30) // Options Page
    {
      p = 2;
      val = 2;
      //u8x8.clear();
      u8x8.setInverseFont(1);
      u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
      u8x8.drawString(0,0,"    OPTIONS     ");
      u8x8.setInverseFont(0);
      u8x8.drawString(1,1,"Check Voltage");
      u8x8.drawString(1,2,"Back");
      u8x8.drawString(0,i,">");
      checked = false;

    }

    if (page == 31) // Voltage Meter
    {
      p = 3;  val = 7;   i = 7;
      u8x8.setInverseFont(1);
      u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
      u8x8.drawString(0,0," MOTOR VOLTAGE  ");
      u8x8.setInverseFont(0);

      //attachInterrupt(A3,VoltageMeter,
//      if (checked == false) {
//      VoltageMeter();
//      }
      u8x8.drawString(0,2,"Voltage: ");
      u8x8.setCursor(9,2);
      u8x8.print(vin,2);
      u8x8.drawString(13,2,"V");
      u8x8.drawString(0,4,"MaxPWM: ");
      u8x8.setCursor(8,4);
      u8x8.print(MaxPWM);
      u8x8.drawString(1,7,"Back");
      u8x8.drawString(0,7,">");
    }


    if (page == 40) // TUNER V1
    {
      
      PosHzTol = (DesiredHz*1.0595 - DesiredHz)*0.05;
      NegHzTol = (DesiredHz - DesiredHz/1.0595)*0.05;
      p = 4;  val = 7;
      u8x8.setInverseFont(1);
      u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
      u8x8.drawString(0,0,"     TUNNING    ");
      u8x8.setInverseFont(0);
      u8x8.drawString(0,3,"Actual: ");
      u8x8.drawString(0,2,"Desired: ");
      u8x8.drawString(0,i,">"); 
      u8x8.drawString(1,7,"Back");
      u8x8.setCursor(9,2);
      u8x8.print(DesiredHz);
    if (checkMaxAmp>ampThreshold){
      
      
    frequency = 38462/float(period);//calculate frequency timer rate/period
    if (frequency >= LowerLimit && frequency <= UpperLimit){
      u8x8.setCursor(8,3);
      u8x8.print(frequency);
    e = frequency - DesiredHz; // większe od zera == za wysoko

    if (e > PosHzTol)
    {
      digitalWrite(4,HIGH);
      if (e < 10*PosHzTol)
      {
        analogWrite(3,80);
      }
      else
      analogWrite(3,120);
    }
    if (e < - NegHzTol)
    {
      digitalWrite(4,LOW);
      if (e > - 10*NegHzTol)
      {
          analogWrite(3,80);
      }
      else
      analogWrite(3,120);
    }
    if (e >= -NegHzTol && e <= PosHzTol)
    {
      
      u8x8.drawString(0,5,"ready");
      analogWrite(3,0);
      
      delay(1000);
      u8x8.clear();
      page = 10;
      i = 1;
      p = 2;
    }
    else {u8x8.drawString(0,5,"     ");}
    }
    
   
    
    
  }

}
 
    
    //------------------------- SELECTION -------------------------------
    
SelectOpt();
Upwards();
Downwards();
CalculateHz();

}

void Upwards()
{
  int up = digitalRead(11);
  if (up == 0)
    SlopeUp = false;
    
  if (up == 1 && SlopeUp == false)
    SUflag = true;
    
  if (SUflag == true)
    {
      SlopeUp = true;
      i= i - 1;
      u8x8.drawString(0,i+1," ");
      if(i<1)
      i = val;
      SUflag = false;
    }
}

void Downwards() 
{ 
  int down = digitalRead(13);
//  Serial.print(down);
//  Serial.print("    ");
//  Serial.println(SlopeDown);
  if (down == 0)
    SlopeDown = false;
    
  if (down == 1 && SlopeDown == false)
    SDflag = true;
    
  if (SDflag == true)
    {
      SlopeDown = true;
      i=i+1;
      u8x8.drawString(0,i-1," ");
      if(i>val)
      i = 1;
      SDflag = false;
    }
}

void SelectOpt() 
{ 
  int select = digitalRead(9);
//  Serial.print(select);
//  Serial.print("    ");
//  Serial.println(SlopeSelect);
  if (select == 0)
    SlopeSelect = false;
    
  if (select == 1 && SlopeSelect == false)
    SSflag = true;
    
  if (SSflag == true)
    {
      SlopeSelect = true;
      
      if (p == 1)
      {
        page = i*10;
        u8x8.clear();
        i = 1;
        SSflag = false;
      }
      if (p == 2 && i != val)
      {
        page = page+1;
        u8x8.clear();
        i = 1;
        SSflag = false;
      }
      if (p == 3 && i != val)
      {
        page = 40;
        u8x8.clear();
        i = 7;
        SSflag = false;
      }
      if (p == 2 && i == val)
      {
        u8x8.clear();
        page = 1;
        i = 1;
        SSflag = false;
      }
      if (p == 3 && i == val)
      {
        if(page>10 && page < 20)
        {
        u8x8.clear();
        page = 10;
        i = 1;
        SSflag = false;
        }
        if(page>20 && page < 30)
        {
        u8x8.clear();
        page = 20;
        i = 1;
        SSflag = false;
        }
        if(page>30 && page < 40)
        {
        u8x8.clear();
        page = 30;
        i = 1;
        SSflag = false;
        }
      }
//       if (p == 4 && i == 1)
//      {
//        digitalWrite(4,HIGH);
//        analogWrite(3,MaxPWM);
//        //delay(200);
//        SSflag = false;
//        }
//     
//      if (p == 4 && i == 2)
//      {
//        digitalWrite(4,LOW);
//        analogWrite(3,MaxPWM);
//        //delay(200);
//        SSflag = false;
//        }
//     
//      if (p == 4 && i == 3)
//      {
//        digitalWrite(4,LOW);
//        analogWrite(3,0);
//        //delay(200);
//        SSflag = false;
//        }  
      if (p == 4 && i == val)
      {
      u8x8.clear();
        page = 11;
        i = 1;
        SSflag = false;
        analogWrite(3,0);
      }
    }
}


void CalculateHz(){
  float StepHz;
float frequencies[3][6] = {{82.41, 110.00, 146.83, 196.00, 246.94, 329.63}, 
                          {77.78,  103.82,  138.58,  184.99,  233.07,  311.11}, 
                          {73.41,   97.99,  130.80,  174.60,  219.98,  293.65}};
if (page == 10 && i != val){
  DesiredHz = frequencies[0][i-1];
}
if (page == 11 && i != val){
DesiredHz = frequencies[i-1][pomocnicza - 1];
LowerLimit = DesiredHz / 1.1893;
UpperLimit = DesiredHz * 1.1893;
}


}
ISR(ADC_vect) {//when new ADC value ready

  
  
  prevData = newData;//store previous value
  newData = ADCH;//get value from A0
  if (prevData < 127 && newData >=127){//if increasing and crossing midpoint
    newSlope = newData - prevData;//calculate slope
    if (abs(newSlope-maxSlope)<slopeTol){//if slopes are ==
      //record new data and reset time
      slope[index] = newSlope;
      timer[index] = time;
      time = 0;
      if (index == 0){//new max slope just reset
        noMatch = 0;
        index++;//increment index
      }
      else if (abs(timer[0]-timer[index])<timerTol && abs(slope[0]-newSlope)<slopeTol){//if timer duration and slopes match
        //sum timer values
        totalTimer = 0;
        for (byte i=0;i<index;i++){
          totalTimer+=timer[i];
        }
        period = totalTimer;//set period
        //reset new zero index values to compare with
        timer[0] = timer[index];
        slope[0] = slope[index];
        index = 1;//set index to 1
        PORTB |= B00010000;//set pin 12 high
        noMatch = 0;
      }
      else{//crossing midpoint but not match
        index++;//increment index
        if (index > 9){
          reset();
        }
      }
    }
    else if (newSlope>maxSlope){//if new slope is much larger than max slope
      maxSlope = newSlope;
      time = 0;//reset clock
      noMatch = 0;
      index = 0;//reset index
    }
    else{//slope not steep enough
      noMatch++;//increment no match counter
      if (noMatch>9){
        reset();
      }
    }
  }
    
  if (newData == 0 || newData == 1023){//if clipping
    clipping = 1;//currently clipping
    Serial.println("clipping");
  }
  
  time++;//increment timer at rate of 38.5kHz
  
  ampTimer++;//increment amplitude timer
  if (abs(127-ADCH)>maxAmp){
    maxAmp = abs(127-ADCH);
  }
  if (ampTimer==1000){
    ampTimer = 0;
    checkMaxAmp = maxAmp;
    maxAmp = 0;
  }
  
}


//void VoltageMeter(){
//if (checked == false){
//value = analogRead(A0);
//  vout = (value * 5.0) / 1024.0;
//  vin = vout / (9880.0 / (109480.0));
//  if (vin<0.09) {
//   vin=0.0;//statement to quash undesired reading !
//  }
//  if(vin > 6.0)
//  {
//  MaxPWM = 255.0 - ((vin - 6.0)/vin*255.0);
//  }
//  else MaxPWM = 255;  
//checked = true;
//}
//}


void reset(){//clean out some variables
  index = 0;//reset index
  noMatch = 0;//reset match couner
  maxSlope = 0;//reset slope
}


void checkClipping(){//manage clipping indication
  if (clipping){//if currently clipping
    clipping = 0;
  }
}
