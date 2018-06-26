//#include <LiquidCrystal_I2C.h>
#include<avr/wdt.h>//NEW
#include <LiquidCrystal.h>
//LiquidCrystal_I2C  lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // 0x3F(sometimes 0x27) is the I2C bus address for an unmodified backpack pcf8574
const int rs = 6, en = 9, d4 = 10, d5 = 11, d6 = 12, d7 =13;//rs = 11, en = 10, d4 = 9, d5 = 6, d6 = 3, d7 =2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
#include <avr/sleep.h>


#include "RTClib.h"
RTC_DS1307 rtc;

#include <Functions.h>
//#include <sleep.h>


bool checkProgram=false;
int testHours=-1;
int minCheck=0;
int testStartHour=0;
bool readUser=false;
int secondLoopAround=0;
//long previousMillis=0;

//DateTime now = rtc.now(); //get the current date-time
void setup()                         
{  
  wdt_disable();
  Serial.begin(9600);              // Initiates the serial to do the monitoring 
  delayTimeFunc(1000);//protection for rtc...
  Wire.begin();
  rtc.begin();
  lcd.begin (16,2); // for 16 x 2 LCD module
  resetDone=false;
//  lcd.setBacklightPin(3,POSITIVE);
//  lcd.setBacklight(HIGH);
  
  //now = rtc.now(); //get the current date-time
  if(EEPROM.read(10)==255){
    EEPROM.write(10,0);
  }
  if(EEPROM.read(11)==255){
    EEPROM.write(11,0);
  }
  if(EEPROM.read(12)==255){
    EEPROM.write(12,-1);
  }
  
  if(EEPROM.read(14)==255){
    EEPROM.write(14,0);
  }
  optimumAngleSet  = EEPROM.read(14);
  
  if(EEPROM.read(15)==255){
    EEPROM.write(15,0);
  }
  if(EEPROM.read(16)==255){//startHours
    EEPROM.write(16,0);
  }
  
  if(EEPROM.read(17)==255){//endHours
    EEPROM.write(17,0);
  }
  
  /*if(EEPROM.read(18)==255){//endHours
    EEPROM.write(18,0);
  }*/

  if(EEPROM.read(19)==255){//designType
    EEPROM.write(19,'A');
  }

  designType = EEPROM.read(19);

  
  if(EEPROM.read(20)==255){//currentHour
    EEPROM.write(20,0);
  }
  
 
  pinMode(RELAY_IN_ONE, OUTPUT);    
  pinMode(RELAY_IN_TWO, OUTPUT);    
  pinMode(RELAY_IN_THREE, OUTPUT); 
  pinMode(RELAY_IN_FOUR, OUTPUT); 
  Stop(1);
  Stop(2);


String user_input;   

//Serial.println("Starting process of actuator calibration and getting timings");  
dailyActuatorOrientation = 'R'; //this value needs to come in from the app.. F stands for forward           DEFAULT
seasonalActuatorOrientation = 'R'; //this value needs to come in from the app.. R stands for reverse        DEFAULT
setseasonalAxisBT = 'F';//this value needs to come in from the app... T stands for true
setdailyAxisBT = 'F';//this value needs to come in from the app... T stands for true

/*EEPROM.write(0,'A');

EEPROM.write(1,'O');

EEPROM.write(4,'A');

EEPROM.write(5,'T');



EEPROM.write(2,17);

EEPROM.write(3,17);

EEPROM.write(6,18);

EEPROM.write(7,19);*/

byte eepromArray[10];
for(int i=0;i<10;i++){
 eepromArray[i] = EEPROM.read(i);
}

//Check for chars A and O in the first two indexes. AO stands for Actuator One and AT stands for actuator two
/*todo
store actuator orientations in the eeprom.....
*/
if(eepromArray[0]=='A' && eepromArray[1] == 'O' && eepromArray[4] == 'A' && eepromArray[5] == 'T'){ 
 // Serial.println("getting getActuatorOneSpeedFromEEPROM");

 
  actuatorOneForwardRuntime  = eepromArray[2];
  actuatorOneReverseRuntime  = eepromArray[3];
  actuatorTwoForwardRuntime  = eepromArray[6];   
  actuatorTwoReverseRuntime  = eepromArray[7];


  if(255 != EEPROM.read(12)){
    seasonalActuatorOrientation = EEPROM.read(12);
  } 
  
  if(255 != EEPROM.read(13)){
    dailyActuatorOrientation   = EEPROM.read(13);
  }
  
  /*needs to happen for sesonal as well....*/
   setDelayTimingsArray(actuatorOneForwardRuntime,actuatorOneReverseRuntime);        
  
   haveDailyTimings=true;
   haveSeasonalTimings=true;
   
   actuatorTwoForwardRuntimeConst = actuatorTwoForwardRuntime;
   actuatorTwoReverseRuntimeConst = actuatorTwoReverseRuntime;

    
 /*  actuatorTwoForwardRuntime = 1000*((actuatorTwoForwardRuntime*2.0)/47);//this is timing for per week...
   actuatorTwoReverseRuntime = 1000*((actuatorTwoReverseRuntime*2.0)/47);*/
   

}

clearLCDScreen(); 
delayTimeFunc(200);
  
// begin returns a boolean that can be used to detect setup problems.
wdt_enable(WDTO_8S);//NEW

}

uint32_t old_ts;


void loop() 
{            
  
    
  
    DateTime now = rtc.now();
    delayTimeFunc(200);
    
    uint32_t ts = now.unixtime();
  
    
  if (old_ts == 0 || old_ts != ts) {
    old_ts = ts;
    
    if(now.hour()<24){
      hour = now.hour();
    }
    minute = now.minute(); 
    day = now.day();//7,14,21,28
    month = now.month();
    year = now.year();
    second = now.second();       
    
    if(year<2018 || year > 2100){
      rtcChecker++;
    }
  
    if(rtcChecker>20000){
     // Serial.println("RTC out. Re-setting system to optimum angle and going into sleep mode");
      //lcd.clear();
      clearLCDScreen();    
      lcd.print("RTC out-Sleeping"); 
      rtcChecker=0;
      stopatOptimumAngle();
      sleep_enable();
      set_sleep_mode(SLEEP_MODE_PWR_DOWN);
      sleep_cpu();
      
    }
   /* Serial.print("Hour: ");
    Serial.println(hour);
    
    Serial.print(" Min: ");
    Serial.println(minute);

    Serial.print(" Day: ");
    Serial.println(day);
    
    Serial.print(" Month: ");
    Serial.println(month);*/
    
    // lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(hour);
    lcd.print(":");
    lcd.print(minute);
    lcd.print(":");
    lcd.print(second);   
    
    lcd.setCursor(0,1);
    lcd.print(month);
    lcd.print("/");
    lcd.print(day);
    lcd.print("/");
    lcd.print(year);   

    lcd.setCursor(10,1);
    lcd.print("DN:"); 
    lcd.print(getDayOfYear(month,day,year));
    //lcd.clear(); 

    delayTimeFunc(500);

    
    
  }
  

  
  if(Serial.available()){
  
      
  char c = Serial.read();
  
  /* int monthT=0;//T for temp..
      int dayT=0;
      int yearT=0;
      int hourT=0;
      int minuteT=0;
      int secondsT=0;*/
      
  delayTimeFunc(100);  
  if(configInProgress){
    c=' ';  
  }
 ////X,Z,Q,W,E,U,Y,S,V,N,G,h
    if(c=='h'){
      
            
      String dateDivision;          
      String inputRawDate;         
      
      inputRawDate = Serial.readString();      
      Serial.println(inputRawDate);
      dateDivision = inputRawDate[0];
      dateDivision += inputRawDate[1];      
      month=dateDivision.toInt();
           
       //inputRawDate[] = Serial.readString();
      dateDivision = inputRawDate[3];
      dateDivision += inputRawDate[4];      
      day=dateDivision.toInt();
      

      dateDivision = inputRawDate[6];
      dateDivision +=inputRawDate[7];
      dateDivision +=inputRawDate[8];
      dateDivision +=inputRawDate[9];      
      year = dateDivision.toInt();     
      
      dateDivision =inputRawDate[11];
      dateDivision +=inputRawDate[12];      
      hour=dateDivision.toInt();
      
      dateDivision=inputRawDate[14];
      dateDivision+=inputRawDate[15];
      minute=dateDivision.toInt();

      dateDivision=inputRawDate[17];
      dateDivision+=inputRawDate[18];
      
      int seconds = dateDivision.toInt();
      // January 21, 2014 at 3am you would call:
      // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
      rtc.adjust(DateTime(year, month, day, hour, minute, seconds));
      Serial.println("-------");
      Serial.print(month);
      Serial.print("/");
      Serial.print(day);
      Serial.print("/");
      Serial.print(year);
      Serial.println("/");

      Serial.print(hour);
      Serial.print(":");
      Serial.print(minute);
      Serial.print(":");
      Serial.println(seconds);
      Serial.println("-------");
      
      
      //delayTimeFunc(500);
      
    }    
    if(c=='X'){//this is for calibration     
    //  Serial.println("Calibrating Daily Actuator");
       clearLCDScreen();       
       lcd.print("Calibrating..."); 
       setdailyAxisBT = 'T';//T for true of course
       configInProgress=true;     
    }
    else if(c=='Z'){//this is for calibration
       setseasonalAxisBT='t';
       clearLCDScreen();       
       lcd.print("Calibrating...");
       configInProgress=true; 
    }    
    else if(c=='Q'){
       seasonalActuatorOrientation = 'R';
       EEPROM.write(12,seasonalActuatorOrientation);        
       setseasonalAxis=true;
       clearLCDScreen();       
       lcd.print("Changing");        
       lcd.setCursor(0,1);
       lcd.print("Orientation");
       configInProgress=true; 
    }
    else if(c=='W'){
       seasonalActuatorOrientation = 'F';
       EEPROM.write(12,seasonalActuatorOrientation);
       setseasonalAxis=true;  
       clearLCDScreen();
       lcd.print("Changing");        
       lcd.setCursor(0,1);
       lcd.print("Orientation"); 
       configInProgress=true;  
    }
    else if(c=='E'){    
      //Serial.println("here");//TEST PLEASE REMOVE          
       dailyActuatorOrientation = 'R';
       EEPROM.write(13,dailyActuatorOrientation);
       setDailyAxis=true;
       forceReset=true;
       clearLCDScreen();
       lcd.print("Changing");        
       lcd.setCursor(0,1);
       lcd.print("Orientation");
       configInProgress=true;  
       if(hour<=10){
        EEPROM.write(16,0);//this is important because it needs to actuate to 10 AM position, in case calibration was run after it had attempted to move it to 10 at 7am...
       }      
    }
    else if(c=='U'){
       dailyActuatorOrientation = 'F';
       EEPROM.write(13,dailyActuatorOrientation);
       setDailyAxis=true;
       forceReset=true;
       clearLCDScreen();
       lcd.print("Changing");        
       lcd.setCursor(0,1);
       lcd.print("Orientation"); 
       configInProgress=true;
    }
    else if(c=='Y'){ 
       stopatOptimumAngle();          
       EEPROM.write(14,1);
       configInProgress=true;
    }
    else if(c=='S'){
       optimumAngleSet=false;
       EEPROM.write(14,0);   
    }
    else if(c=='V'){
       designType='A';
       EEPROM.write(19,designType);   
    }
    
    else if(c=='N'){
       designType='S';
       Serial.println("Changing Design Type to S");
       EEPROM.write(19,designType);   
    }
    else if(c=='G'){
      Serial.println("<");//START
      Serial.print("Optimum Angle -  ");
      if(optimumAngleSet){
        Serial.println("SET");
      }
      else{
        Serial.println("UNSET");
      }

      Serial.print("Design Type: ");
      Serial.println(designType);
      Serial.println("Daily Actuator");
      Serial.print("ForwardRuntime  -  ");
      Serial.println(actuatorOneForwardRuntime);
      Serial.print("Reverse Runtime -  ");
      Serial.println(actuatorOneReverseRuntime);
      Serial.print("Orientation -  ");
      Serial.println(dailyActuatorOrientation);

      Serial.println("_______________________");
      Serial.println("Seasonal Actuator");
      Serial.print("ForwardRuntime  -  ");
      Serial.println(actuatorTwoForwardRuntimeConst);
      Serial.print("Reverse Runtime -  ");
      Serial.println(actuatorTwoReverseRuntimeConst);
      Serial.print("Orientation -  ");
      Serial.println(seasonalActuatorOrientation);      

      Serial.print("Current System Time - ");
      Serial.print(hour);
      Serial.print(":");
      Serial.print(minute);
      Serial.print(":");
      Serial.print(now.second());   
      
      Serial.print("  ");
      Serial.print(month);
      Serial.print("/");
      Serial.print(day);
      Serial.print("/");
      Serial.println(year);   

      Serial.println("Forward Delay Times:");
      for(int i=0;i<22;i++){
        Serial.println(delayTimesForwardActuatorOne[i]);
      }
      
      Serial.println("Reverse Delay Times:");
      for(int i=0;i<22;i++){
        Serial.println(delayTimesReverseActuatorOne[i]);
      }
      Serial.print("totalDelay: ");      
      Serial.println(totalDealyTemp);      

      Serial.print("currentBlock: ");      
      Serial.println(currentBlockTemp);      

      Serial.println(">");//END
      Serial.flush();
      
     
    }
    
  }
 //request values of setseasonalAxisBT and setdailyAxisBT from slave arduino constantly
    if(setseasonalAxisBT=='t'){
     // Serial.println("setting seasonal");
      setseasonalAxis = true;    
      setseasonalActuatorTimings(actuatorTwoForwardRuntime,actuatorTwoReverseRuntime,now.month());
      
      setseasonalAxisBT='F';//reset to false
      actuatorTwoForwardRuntimeConst = actuatorTwoForwardRuntime;
      actuatorTwoReverseRuntimeConst = actuatorTwoReverseRuntime;
     // actuatorTwoForwardRuntime = 1000*((actuatorTwoForwardRuntime*0.84)/182);//1000*((actuatorTwoForwardRuntime*2.0)/47);//this has to be refactored.... (1000*((15*1.8076)/47))/7
                                     
     // actuatorTwoReverseRuntime = 1000*((actuatorTwoReverseRuntime*0.84)/182); //1000*((actuatorTwoReverseRuntime*2.0)/47);
      Serial.println("actuatorTwoForwardRuntime in mains: ");
      Serial.println(actuatorTwoForwardRuntime);
      Serial.println("actuatorTwoReverseRuntime in mains: ");
      Serial.println(actuatorTwoReverseRuntime);      
      haveSeasonalTimings=true;
      
    }
    if(setdailyAxisBT=='T'){        
       //Serial.println("setdailyAxisBT set from app");
       EEPROM.write(8,-1);//so that it enters the doDailyMovement function...
       setdailyActuatorTimings(actuatorOneForwardRuntime,actuatorOneReverseRuntime);
       if(hour<=10){
        EEPROM.write(16,0);//this is important because it needs to actuate to 10 AM position, in case calibration was run after it had attempted to move it to 10 at 7am...
       }
       Serial.print("EEPROM.read(16): ");
       Serial.println(EEPROM.read(16));
       setdailyAxisBT='F';//reset to false
       haveDailyTimings=true;
       setDailyAxis=true;
       forceReset=true;
    }                     
//either manually of after reaching the time. but if reset, it needs to check if it has already moved for the current hour    


if(haveSeasonalTimings && setseasonalAxis){      
          doSeasonalMovement(year,month,day,hour,seasonalActuatorOrientation);  
          
}

//i2c_eeprom_write_byte(0x57,0,0);
//******************************************************************************************daily***********************************************************************************************
/*if you ever wonder in the future why not do a ++ instead for currentBlock, then think about what will happen if the lights go out(++ reference will be lost). 
 this number has to be hard set here...*/
/*Serial.println(haveDailyTimings);//TEST PLEASE REMOVE
 Serial.println(hour);//TEST PLEASE REMOVE
 Serial.println(month);//TEST PLEASE REMOVE*/
if(haveDailyTimings && hour<24 && month<=12){
    if(hour>=7 && hour<8){//6885
        bool timeCheck=false;
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }   
        timeCheck=false;//so the check happens a 3rd time...     
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }
        
       if(timeCheck){
              
              currentBlock=0;//no block before this...       
              startHours=true;
              
              if(minute>=30){        
                currentBlock=1;                
              } 
               if(!optimumAngleSet && EEPROM.read(8) != currentBlock || setDailyAxis ){
                setDailyAxis=true;       
                doDailyMovement(currentBlock,dailyActuatorOrientation,hour,minute);  
                EEPROM.write(20,hour);            
              }
              if(EEPROM.read(9)){

                
                 EEPROM.write(9,0);//resetting seasonal        
              }               
              
       }
    }  

    else if(hour>=8 && hour<9){      
        bool timeCheck=false;
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }   
        timeCheck=false;//so the check happens a 3rd time...     
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }
        
       if(timeCheck){
            currentBlock=2;
            startHours=true;
            if(minute>=30){
              currentBlock=3;               
            } 
             if(!optimumAngleSet && EEPROM.read(8) != currentBlock || setDailyAxis){
              setDailyAxis=true;
              doDailyMovement(currentBlock,dailyActuatorOrientation,hour,minute);            
              EEPROM.write(20,hour);  
            }
            if(EEPROM.read(9)){
                              
                
                
               EEPROM.write(9,0);//resetting seasonal        
            }
            
       }
    }
    else if(hour>=9 && hour<10){      
        bool timeCheck=false;
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }   
        timeCheck=false;//so the check happens a 3rd time...     
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }
        
       if(timeCheck){
            currentBlock=4;
            startHours=true;
            if(minute>=30){
              currentBlock=5;
              startHours=true;        
            } 
             if(!optimumAngleSet && EEPROM.read(8) != currentBlock || setDailyAxis ){
              setDailyAxis=true;
              doDailyMovement(currentBlock,dailyActuatorOrientation,hour,minute);            
              EEPROM.write(20,hour);  
            }
            if(EEPROM.read(9)){
                              
                
                
               EEPROM.write(9,0);//resetting seasonal        
            }      
            
       }
    }
  else if(hour>=10 && hour<11){      
         bool timeCheck=false;
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }   
        timeCheck=false;//so the check happens a 3rd time...     
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }
       
       if(timeCheck){
        
              currentBlock=6;     
              startHours=true;//END OF START HOUR HERE...
              
              if(minute>=30){
                startHours=false;
                currentBlock=7;
              } 
              if(!optimumAngleSet && EEPROM.read(8) != currentBlock || setDailyAxis ){
                setDailyAxis=true;
                doDailyMovement(currentBlock,dailyActuatorOrientation,hour,minute);            
                EEPROM.write(20,hour);  
              }  
              if(!EEPROM.read(15)){
                seasonalAxisDailyMovement(hour);//seasonal axis should dip
                EEPROM.write(15,1);
                
              }
              if(EEPROM.read(9)){
                                
                
                
                 EEPROM.write(9,0);//resetting seasonal        
              }    
              
       }
    }
    else if(hour>=11 && hour<12){     
           bool timeCheck=false;
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }   
        timeCheck=false;//so the check happens a 3rd time...     
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }
       
       if(timeCheck){
            currentBlock=8;
            if(minute>=30){
              currentBlock=9;
            }       
             if(!optimumAngleSet && EEPROM.read(8) != currentBlock || setDailyAxis ){
              setDailyAxis=true;
              doDailyMovement(currentBlock,dailyActuatorOrientation,hour,minute);            
              EEPROM.write(20,hour);  
            }
            if(!EEPROM.read(15)){
              seasonalAxisDailyMovement(hour);
              EEPROM.write(15,1);
              
            }
            if(EEPROM.read(9)){
                              
                
                
               EEPROM.write(9,0);//resetting seasonal        
            }    
            
       }
    }
    else if(hour>=12 && hour<13){  
        bool timeCheck=false;
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }   
        timeCheck=false;//so the check happens a 3rd time...     
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }
       
       if(timeCheck){       
          currentBlock=10;
          if(minute>=30){
            currentBlock=11;
          } 
           if(!optimumAngleSet && EEPROM.read(8) != currentBlock || setDailyAxis){
            setDailyAxis=true;
            doDailyMovement(currentBlock,dailyActuatorOrientation,hour,minute);            
            EEPROM.write(20,hour);  
          }
          if(!EEPROM.read(15)){
            seasonalAxisDailyMovement(hour);
            EEPROM.write(15,1);
            
          }
          if(EEPROM.read(9)){
                            
                
                
             EEPROM.write(9,0);//resetting seasonal        
          }       
          
       }
    }
    else if(hour>=13 && hour<14){        
        bool timeCheck=false;
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }   
        timeCheck=false;//so the check happens a 3rd time...     
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }
       
       if(timeCheck){
           currentBlock=12;
            if(minute>=30){
              currentBlock=13;
            } 
             if(!optimumAngleSet && EEPROM.read(8) != currentBlock || setDailyAxis){
              setDailyAxis=true;
              doDailyMovement(currentBlock,dailyActuatorOrientation,hour,minute);            
              EEPROM.write(20,hour);  
            }
            if(!EEPROM.read(15)){
              seasonalAxisDailyMovement(hour);
              EEPROM.write(15,1);
              
            }
            if(EEPROM.read(9)){
                              
                
                
               EEPROM.write(9,0);//resetting seasonal        
            }      
            
       }
    }
    else if(hour>=14 && hour<15){     
        bool timeCheck=false;
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }   
        timeCheck=false;//so the check happens a 3rd time...     
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }
       
       if(timeCheck){
           currentBlock=14;
            if(minute>=30){
              currentBlock=15;
            } 
            if(!optimumAngleSet && EEPROM.read(8) != currentBlock || setDailyAxis){
              setDailyAxis=true;
              doDailyMovement(currentBlock,dailyActuatorOrientation,hour,minute);            
              EEPROM.write(20,hour);   
            }
            if(!EEPROM.read(15)){
              seasonalAxisDailyMovement(hour);
              EEPROM.write(15,1);
              
            }
            if(EEPROM.read(9)){
                              
                
                
               EEPROM.write(9,0);//resetting seasonal        
            }      
           
       }
    }
  else if(hour>=15 && hour<16){   
        bool timeCheck=false;
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }   
        timeCheck=false;//so the check happens a 3rd time...     
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }
       
       if(timeCheck){
            currentBlock=16;
            if(minute>=30){
              currentBlock=17;
            } 
            if(!optimumAngleSet && EEPROM.read(8) != currentBlock || setDailyAxis){
              setDailyAxis=true;
              doDailyMovement(currentBlock,dailyActuatorOrientation,hour,minute);            
              EEPROM.write(20,hour);
              
            }
            if(!EEPROM.read(15)){
              seasonalAxisDailyMovement(hour);
              EEPROM.write(15,1);
              
            }
            if(EEPROM.read(9)){
                              
                
                
               EEPROM.write(9,0);//resetting seasonal        
            }      
           
       }
    }
    else if(hour>=16 && hour<17){            
        bool timeCheck=false;
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }   
        
        timeCheck=false;//so the check happens a 3rd time...     
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }
        
       if(timeCheck){
               currentBlock=18;
                if(minute>=30){
                  currentBlock=19;
                  endHours=true;
                } 
                if(!optimumAngleSet && EEPROM.read(8) != currentBlock || setDailyAxis){
                  setDailyAxis=true;
                  doDailyMovement(currentBlock,dailyActuatorOrientation,hour,minute);            
                  EEPROM.write(20,hour);
                  
                }
                if(EEPROM.read(15)){
                  seasonalAxisDailyMovement(hour);
                  EEPROM.write(15,0);
                  
                }            
                if(EEPROM.read(9)){
                                  
                
                
                   EEPROM.write(9,0);//resetting seasonal        
                }  
                
       }
    }
    /*
      whats happening here is we need an indicator that the last hours actuation is done. if its not done, then it needs to be done, but just once between the hours of 5 and 8.
      because it if does it again, it will see that expectedPreviousBlockValue != actualPreviousBlockValue, at which point it will reset the actuator at every hour after 5.
      it needs to attempt to actuate it between this hour if it hasnt because imagine we turn on the system after 6pm. it will go to the start position and do nothing if we
      dont put hour>=17 && hour<20. if we put hour>=17 && hour<18, then nothing can happend between 18 and 20. that can confuse people. if we dont check EEPROM.read(8) != 10,
      then as mentioned earlier,expectedPreviousBlockValue != actualPreviousBlockValue, at which point it will reset the actuator at every hour after 5. 
    */
    else if(hour>=17 && hour<20 ){
        bool timeCheck=false;
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }   
        timeCheck=false;//so the check happens a 3rd time...     
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }
        
       if(timeCheck){                 
                if(hour==17){
                  currentBlock=20;//so it doesnt reset it back to 20...    
                }
                
                endHours=true;    
                  
                if(hour==17 && minute>=30){
                  currentBlock=21;                
                  endHours=true;
                } 
                
                if(hour>=18){
                  currentBlock=21;                
                  endHours=false;
                } 
                if(!optimumAngleSet && EEPROM.read(8) != currentBlock  || setDailyAxis ){
                  /*just check whether this hour block was already done. it could have been done between 5-7 PM. the if statement above is a littel different from the ones above. We have added
                   && EEPROM.read(8) != 21 as an extra measure because this block runs between hour 17 to hour 20...thats 3 hours. so it keeps changing current block back and forth from 20
                   to 21, because of which, at every hour, the actuator resets becasue the condition expectedPreviousBlockValue != actualPreviousBlockValue turns out to be true.....so now,
                   we dont run the function doDailyMovement after 5:30 PM.....17 is 5pm and at minute 30, last movement will happen.        
                   */        
                  setDailyAxis=true;
                  doDailyMovement(currentBlock,dailyActuatorOrientation,hour,minute);            
                  EEPROM.write(20,hour);
                  
                }
                if(EEPROM.read(15)){
                  seasonalAxisDailyMovement(hour);
                  EEPROM.write(15,0);
                  
                }
                if(EEPROM.read(9)){               
                   EEPROM.write(9,0);//resetting seasonal        
                }
                
       }   
    }    
    else if(hour>=20){//reset at 8 pm...         
        bool timeCheck=false;
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }   
        timeCheck=false;//so the check happens a 3rd time...     
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }
        
       if(timeCheck){
            currentBlock=22;
            endHours=false;
            
            if(!optimumAngleSet && EEPROM.read(8) != currentBlock || setDailyAxis){  
              setDailyAxis=true;      
              doDailyMovement(currentBlock,dailyActuatorOrientation,hour,minute);//same here. we dont want it to keep doing it for hours after 20. just once if it already hasnt
                
            } 
            if(EEPROM.read(15)){
              seasonalAxisDailyMovement(hour);
              EEPROM.write(15,0);
              
            }
            if(EEPROM.read(9)){
                              
                
                
               EEPROM.write(9,0);//resetting seasonal        
            }
            if(hour==23 && EEPROM.read(20) != 23){              
              Serial.println(EEPROM.read(20));
              
              EEPROM.write(20,hour);
            }
            
       }        
    }
    else if(hour>=0 && hour<7){
     //  if(EEPROM.read(15))
       
       //EEPROM.write(9,0);//indicating that seasonal actuator should now move
       bool timeCheck=false;
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }   
        timeCheck=false;//so the check happens a 3rd time...     
        delayTimeFunc(3000);
        if(now.hour()==hour){
          timeCheck=true;
        }
        
      
       if(timeCheck && EEPROM.read(20)==23){//this is important because rtc returns 0 randomly...sometimes thrice. make sure hour before was actually 11 before running this block.
           if(EEPROM.read(15)){
            seasonalAxisDailyMovement(hour);
            EEPROM.write(15,0);//resetting seasonaldaily      
            EEPROM.write(20,hour);
           } 
           if(EEPROM.read(16)){
             EEPROM.write(16,0);//resetting daily        
           }
           /*if(EEPROM.read(18)){
             EEPROM.write(18,0);//resetting `last hour        
           }*/
            
           //*********************************************************************seasonal********************************************************************
            //setseasonalAxis
           
            if(haveSeasonalTimings && !EEPROM.read(9)){                    
              doSeasonalMovement(year,month,day,hour,seasonalActuatorOrientation);             
            }
       }
    }
    
}

if(checkProgram){

  if(minCheck==2){
    minCheck=0;
    minute=0;
  }

}  
    wdt_reset();
}
/*
bug fixes:
-check whether panel has reset before actuating 
-actuate every 30 mins instead of an hour
-actuate seasonal everyday 
-get diagostics about which hours the actuation happened and which were missed

just have a set button for optimum angle and display a message when set is pressed...

going half values of array wont work for optimum angle. just divide the total time in half and use that...

dec 21st - june 21st = reverse-o forward
reverse 0, forward 2.4 secs
june 22nd - dec 21st = reverse-o forward


test

test 2
*/


