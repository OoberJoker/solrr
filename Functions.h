#include <Wire.h>
//#include <Helios.h>

//#include <Sodaq_DS3231.h>
#include <EEPROM.h>
//#include <BH1750.h>
//#include <math.h>
//#define PI 3.1415926
//#define ZENITH -.83
//sq(130)+sq(340)=132500
//sqrt(132500-2*130*340*cos(170)) = 468

//sqrt(132500-2*130*340*cos(168)) = 213

//sqrt(132500-2*130*340*cos(166)) = 213

//char weekDay[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

//testing
int hour=25;//25 because if it starts at 0, the 0 hour block executes before it gets to the checkprogram function...
int minute=0;
int day=0;
int year=0;
int second=0;

//long halfHourInMillis = 1800000;
int month=1;//test.....
long resetTime=50000;//50 secs
// Helios helios;
int delayTimesReverseActuatorOne[22];
int delayTimesForwardActuatorOne[22];

/*int delayTimesReverseActuatorTwo[11];
int delayTimesForwardActuatorTwo[11];*/



int actuatorTwoSpeed = 0;
int actuatorTwoDelayTime = 0;
bool setseasonalAxis = false;//the app will set this variable...
bool setDailyAxis=false;
int actuatorTwoDelayTimePerDay=0;
bool haveDailyTimings=false;
bool haveSeasonalTimings=false;

bool resetDone=false;
int readCurrentHour=0;
int currentHour=0;
int nextHour=0;

//int panelIndexCountValue = 3;
int delayTime = 1000;
//int previousBlockValue=0;
//BH1750 lightMeter(0x23);
char dailyActuatorOrientation;
char seasonalActuatorOrientation;
char setseasonalAxisBT;
char setdailyAxisBT;
//char manualCalibrationSignalDailyActuator;
//char manualCalibrationSignalSeasonalActuator;
//long totalActuationTime=0;
int actuatorOneForwardRuntime=0;
int actuatorOneReverseRuntime=0;
int actuatorTwoForwardRuntime=0;
int actuatorTwoReverseRuntime=0;

int actuatorTwoForwardRuntimeConst=0;
int actuatorTwoReverseRuntimeConst=0;
bool configInProgress=false;
int currentBlock=0;

bool forceReset=false;
bool optimumAngleSet=false;
int rtcChecker=0;
int correctionFactor=0;//yet to come
int designType='A';//A for array and S for stackup....
bool endHours=false;
bool startHours=false;

int totalDealyTemp = 0;
int currentBlockTemp = 0;
float slaveDeviceTimeCountDelay=80.0;
//int weightToRPMAdjustementTimeGeneral=10;
double adjustementFactor = 0.16;


//MOTOR 1 DAILY
#define RELAY_IN_ONE 4
#define RELAY_IN_TWO 5

//MOTOR 2 SEASONAL
#define RELAY_IN_THREE 7
#define RELAY_IN_FOUR 8


void clearLCDScreen(){
	lcd.setCursor(0,0);
	lcd.print("                ");		
	lcd.setCursor(0,1);
	lcd.print("                ");	
	lcd.setCursor(0,0);
}
long delayTimeFunc(long delayTime){  //NEW
//int tenPercentOfDelayTime=delayTime*0.10;
int resetCounter=0;


  for(long i=0;i<delayTime;i++){     
      delay(1);   
	  if(resetCounter==1000){
		  resetCounter=0;
		  wdt_reset();
	  }
	  resetCounter++;
      
  }
}


void lcdInit(){
//	const int rs = 6, en = 9, d4 = 10, d5 = 11, d6 = 12, d7 =13;//rs = 11, en = 10, d4 = 9, d5 = 6, d6 = 3, d7 =2;
	//LiquidCrystal lcdObject = new LiquidCrystal(rs, en, d4, d5, d6, d7);//this will call the right init...
	//lcd.init(1, 6, 255, 9, 10, 11, 12, 13, 0, 0, 0, 0); 
	//actual function....init(1, rs, 255, enable, d0, d1, d2, d3, 0, 0, 0, 0);
	lcd.init(1,rs,255,en, d4, d5, d6, d7, 0, 0, 0, 0);
	lcd.begin(16,2);
	lcd.setCursor(0,0);
}

void sendEndChar(){
	
	clearLCDScreen();	
	lcd.print("Transmitting...");
	Serial.println("");
	Serial.println("%");
	delayTimeFunc(500);
	
}

void Stop(int motorNumber)
{
 // Serial.println("Stop");
  
  clearLCDScreen();    
  lcd.print("Stop"); 
  
  if(motorNumber==1){ 
     digitalWrite(RELAY_IN_ONE,LOW);
	 digitalWrite(RELAY_IN_TWO,LOW);
  }
  else{
     digitalWrite(RELAY_IN_THREE,LOW);
	 digitalWrite(RELAY_IN_FOUR,LOW);
  }
  delayTimeFunc(2000);
}



void Forward(int motorNumber)
{
  //Serial.println("Forward");
  
  clearLCDScreen();    
  lcd.print("Forwarding");

  if(motorNumber==1){     
	 digitalWrite(RELAY_IN_ONE,HIGH);	
	 digitalWrite(RELAY_IN_TWO,LOW);        
  }
  else{
     digitalWrite(RELAY_IN_THREE,HIGH);
     digitalWrite(RELAY_IN_FOUR,LOW);        
  }
}

void Reverse(int motorNumber)
{
 // Serial.println("Reverse");
  
  clearLCDScreen();
  lcd.print("Reversing");
  if(motorNumber==1){     
     digitalWrite(RELAY_IN_ONE,LOW);
	 digitalWrite(RELAY_IN_TWO,HIGH);        
  }
  else{
     digitalWrite(RELAY_IN_THREE,LOW);
	 digitalWrite(RELAY_IN_FOUR,HIGH);        
    
  }
}





void motionMethodName(int methodNumber,int actuatorName,bool flip){//methodName 1 = Forward , 2 = Reverse. actuatorName 1 = daily, 2 = seasonal
  if(!flip){
    if(methodNumber==1){
      Forward(actuatorName);
    }
    else{
      Reverse(actuatorName);
    }
  }
  else{
    if(methodNumber==1){
      Reverse(actuatorName);
    }
    else{
      Forward(actuatorName);
    }
    
  }
}  

/*void Calibrate(int panelIndexCount,int indexDelay,int methodNumber,int actuatorName){
    //calibration mode...
          uint16_t lux[panelIndexCount];
          uint16_t highestLux=0;
          int highestLuxIndex=0;
      for(int index=0;index<panelIndexCount;index++){
        lux[index] = lightMeter.readLightLevel();
			delayTimeFunc(100);
			if(index>0){//because we want to get the 4 lux values but index the panels only 3 times...
				motionMethodName(methodNumber,actuatorName,false);
				delayTimeFunc(indexDelay);
				Stop(actuatorName);
				delayTimeFunc(500);
			}
      }
                                             
          for(int t=0;t<panelIndexCount;t++){//because we have 4 lux values but indexed only 3 times...
			  motionMethodName(methodNumber,actuatorName,true);
			  delayTimeFunc(indexDelay);
			  Stop(actuatorName);
			  delayTimeFunc(500);       
          }

          highestLux = lux[0];
           for(int t=0;t<panelIndexCount;t++){
            Serial.print("Lux Value: ");
            Serial.println(lux[t]);
              if(lux[t]>highestLux){
                highestLux=lux[t];
                highestLuxIndex=t;
              }             
           }

            Serial.print("Highest Lux Value: ");
            Serial.println(highestLuxIndex);
          for(int r=0;r<=highestLuxIndex;r++){
              motionMethodName(methodNumber,actuatorName,false);
              delayTimeFunc(indexDelay);           
              Stop(actuatorName);
          }
     
}*/

/*
replace exact forward and backward times
make sure the right index times are being used from delayTimesReverseActuatorOne array for forward and backward movements
its running 1 hour ahead
*/



void getRequestToDevice(int &actuatorRuntime){
	char temp[2];
    int counter=0;
    int data=0;
	
	Wire.requestFrom(9,2);//get actuator time                  	  
	while(Wire.available()){        
	temp[counter] = Wire.read();            
	counter++;
	}
	counter=0;	
	  
	sscanf(temp, "%d", &data);	
	//if(data>0){//TEST PLEASE REMOVE
		actuatorRuntime = data;
	//}
	/*Serial.println("actuatorRuntime: ");
	Serial.println(actuatorRuntime);*/
	
	delayTimeFunc(100);	
}


void getActuatorRunTime(int forwardsPin,int backwardsPin,int &forwardRuntime,int &backwardRuntime){
const int forwards = forwardsPin;//pin 4 and 7 
const int backwards = backwardsPin;//pin 5 and 8
int resetValue=0;      

int motorNumber=0;

	if(forwardsPin==4){
		motorNumber=1;
	}
	else{
		motorNumber=2;
	} 
	
	pinMode(forwards, OUTPUT);//set relay as an output
	pinMode(backwards, OUTPUT);
	
	//Stop(motorNumber);
	
	Reverse(motorNumber);//go full BACK so we can then measure the forward runtime...
	delayTimeFunc(resetTime);	
	Stop(motorNumber);
	
	getRequestToDevice(resetValue);
	
	
    clearLCDScreen();    
    lcd.print("Reset done");
    delayTimeFunc(1000);
	
	
	
    clearLCDScreen();    
    lcd.print("Forwarding");
	Forward(motorNumber);//start forward motion      
	delayTimeFunc(resetTime);
	Stop(motorNumber);	
	
	getRequestToDevice(forwardRuntime);	
	
	
	
	delayTimeFunc(2000);
	
		
	
	
    clearLCDScreen();    
    lcd.print("Reversing");
	Reverse(motorNumber);//Activate the relay the other direction, they must be different to move the motor	 
	delayTimeFunc(resetTime);   
	Stop(motorNumber);
	
	getRequestToDevice(backwardRuntime);	
	
	
	
	delayTimeFunc(2000);
}



void getDailyActuatorRuntime(int &forwardRuntime,int &backwardRuntime){ 
  	getActuatorRunTime(RELAY_IN_ONE,RELAY_IN_TWO,forwardRuntime,backwardRuntime); 
	
}


void getSeasonalActuatorRuntime(int &forwardRuntime,int &backwardRuntime){
	getActuatorRunTime(RELAY_IN_THREE,RELAY_IN_FOUR,forwardRuntime,backwardRuntime); 
	
}


/*bool checkLeapYear(int year){
	if (year % 4 == 0)
    {
        if (year % 100 == 0)
        {
            if (year % 400 == 0){               
				return true;				
			}
            else{                
				return false;
			}
        }
        else{             
				return true;
		}
    }
    else{        
		return false;
	}
	
}*/

int getDayOfYear(int month,int dayOfMonth,int year){

    int daysinaMonth[12];
	daysinaMonth[0] = 31;		
	daysinaMonth[1] = 28;	
	daysinaMonth[2] = 31;	
	daysinaMonth[3] = 30;	
	daysinaMonth[4] = 31;	
	daysinaMonth[5] = 30;
	daysinaMonth[6] = 31;
	daysinaMonth[7] = 31;	
	daysinaMonth[8] = 30;
	daysinaMonth[9] = 31;
	daysinaMonth[10] = 30;
	daysinaMonth[11] = 31;
	
	
	int temp=0;
	for(int i=0;i<month-1;i++){
		temp += daysinaMonth[i];

	}	
		return temp+dayOfMonth;
	

}


void seasonalAxisDailyMovement(int hour){
	float additionalDelayReverse = ((slaveDeviceTimeCountDelay*actuatorTwoReverseRuntime)/1000);
	float additionalDelayForward = ((slaveDeviceTimeCountDelay*actuatorTwoForwardRuntime)/1000);
	
	float actuatorTwoReverseRuntimeCalculated = actuatorTwoReverseRuntime+additionalDelayReverse;	
	float actuatorTwoForwardRuntimeCalculated = actuatorTwoForwardRuntime+additionalDelayForward;
	
	/*Serial.print("actuatorTwoReverseRuntimeCalculated: ");
	Serial.println(actuatorTwoReverseRuntimeCalculated);
	
	Serial.print("actuatorTwoForwardRuntimeCalculated: ");
	Serial.println(actuatorTwoForwardRuntimeCalculated);
	
	Serial.print("actuatorTwoReverseRuntime: ");
	Serial.println(actuatorTwoReverseRuntime);
	
	
	Serial.print("actuatorTwoForwardRuntime: ");
	Serial.println(actuatorTwoForwardRuntime);
	
	Serial.print("additionalDelayReverse: ");
	Serial.println(additionalDelayReverse);*/
	
	
	
	if(seasonalActuatorOrientation=='R'){	
	/*Serial.print("EEPROM.read(15): ");
	Serial.println(EEPROM.read(15));
	
	
		Serial.print("hour");
		Serial.println(hour);
		
		Serial.print("------");
		Serial.println(hour);*/
	
			if(hour>=10 && hour<16){
				int adjustementDelay  = 1000*(actuatorTwoForwardRuntimeCalculated*adjustementFactor);
				Forward(2);
				delayTimeFunc(adjustementDelay);//so it actuates 16%
				Stop(2);	
				
				
				/*Serial.print("Reverse Dominant - Seasonal Axis Daily Movement Forward. Actuation Time:");//--phone test statement
				
				Serial.println(adjustementDelay);//	--phone test statement
				Serial.print("hour: ");
				Serial.println(hour);*/
			}
			else{
				int adjustementDelay = 1000*(actuatorTwoReverseRuntimeCalculated*adjustementFactor);
				Reverse(2);
				delayTimeFunc(adjustementDelay);
				Stop(2);
				
	
				/*Serial.print("Reverse Dominant - Seasonal Axis Daily Movement Reverse. Actuation Time:");//--phone test statement
				
				Serial.println(adjustementDelay);//--phone test statement
				Serial.print("hour: ");
				Serial.println(hour);*/
			}		
	}
	else{
		    if(hour>=10 && hour<16){
				int adjustementDelay = 1000*(actuatorTwoReverseRuntimeCalculated*adjustementFactor);
				Reverse(2);
				delayTimeFunc(adjustementDelay);
				Stop(2);				
				
	
				/*Serial.print("Forward Dominant - Seasonal Axis Daily Movement Reverse. Actuation Time:");//--phone test statement				
				Serial.println(adjustementFactor);//--phone test statement*/
			}
			else{
				int adjustementDelay = 1000*(actuatorTwoForwardRuntimeCalculated*adjustementFactor);
				Forward(2);
				delayTimeFunc(adjustementDelay);
				Stop(2);
				
	
				/*Serial.print("Forward Dominant - Seasonal Axis Daily Movement Forward. Actuation Time:");//--phone test statement				
				Serial.println(adjustementFactor);//--phone test statement*/
			}
		
	}
	lcdInit();
	delayTimeFunc(500);//Reverse Dominant - Seasonal Axis Daily Movement Reverse. Actuation Time:638

}



void doSeasonalMovement(int year,int month,int day,int hour,char actuatorOrientation){

	
	int dayNumberofYear = getDayOfYear(month,day,year);	
	
	float additionalDelayReverse = ((slaveDeviceTimeCountDelay*actuatorTwoReverseRuntime)/1000);
	float additionalDelayForward = ((slaveDeviceTimeCountDelay*actuatorTwoForwardRuntime)/1000);
	
	float actuatorTwoDelayTimePerDayReverse = (1000*((actuatorTwoReverseRuntime+additionalDelayReverse)/182));
	float actuatorTwoDelayTimePerDayForward = (1000*((actuatorTwoForwardRuntime+additionalDelayForward)/182));

	float actuatorTwoReverseRuntimeCalculated = actuatorTwoReverseRuntime+additionalDelayReverse;
	
	float actuatorTwoForwardRuntimeCalculated = actuatorTwoForwardRuntime+additionalDelayReverse;
	 
	bool reverse=false;
	bool forward=false;
	long totalActuationTime=0;    
	if(actuatorOrientation=='R'){
		reverse=true;
	}
	else if(actuatorOrientation=='F'){
		forward=true;
	}
	
	
    if(setseasonalAxis){
		  int decemberOffSet=10;	
    
	   						
          
          
          if(month<=6 && dayNumberofYear<=172){//June 21st...  months 1-6     
				if(reverse){
				  Forward(2);
				  delayTimeFunc(resetTime);
				  Stop(2);		  
				}
				else if(forward){
				  Reverse(2);
				  delayTimeFunc(resetTime);
				  Stop(2);		  
				}//RESET...		

			          
	/*we need this offset because the reverse actuation of the system begins from december 21st. So if the system is ever calibrated after december 21st,
	it needs to consider the 9 days of december...				*/
                        if(reverse){							
								totalActuationTime = ((dayNumberofYear+decemberOffSet)*actuatorTwoDelayTimePerDayReverse);//+9 for counting december days...21st to 31st							
								Reverse(2);
								delayTimeFunc(totalActuationTime);
								Stop(2);							
						}
						else if(forward){								
								totalActuationTime = ((dayNumberofYear+decemberOffSet)*actuatorTwoDelayTimePerDayForward);								
								Forward(2);
								delayTimeFunc(totalActuationTime);
								Stop(2);								
							
						}
         }//24912
         else{         //dayMultiplier order is reversed from above because we are counting backwards from december to whatever month it currently is...
		 //months 7-12				
						
					if(month==12){								
							if(dayNumberofYear>355){
								if(reverse){
								  Forward(2);
								  delayTimeFunc(resetTime);
								  Stop(2);		  
								}
								else if(forward){
								  Reverse(2);
								  delayTimeFunc(resetTime);
								  Stop(2);		  
								}//RESET...
								
								dayNumberofYear = decemberOffSet-(365-dayNumberofYear);	//so could be 1,2,3...to 9...	
								if(reverse){									
										totalActuationTime = (dayNumberofYear*actuatorTwoDelayTimePerDayReverse);
										Reverse(2);
										delayTimeFunc(totalActuationTime);
										Stop(2);	
								}
							
								else if(forward){																	
										totalActuationTime = (dayNumberofYear*actuatorTwoDelayTimePerDayForward);								
										Forward(2);
										delayTimeFunc(totalActuationTime);
										Stop(2);																			
								}
								
							}
							else{
									if(reverse){
									  Reverse(2);
									  delayTimeFunc(resetTime);
									  Stop(2);		  
									}
									else if(forward){
									  Forward(2);
									  delayTimeFunc(resetTime);
									  Stop(2);		  
									}//RESET...
									
								if(reverse){
											totalActuationTime = (dayNumberofYear*actuatorTwoDelayTimePerDayForward);								
											Forward(2);
											delayTimeFunc(totalActuationTime);
											Stop(2);	
								}
								else if(forward){									
											totalActuationTime = (dayNumberofYear*actuatorTwoDelayTimePerDayReverse);
											Reverse(2);
											delayTimeFunc(totalActuationTime);
											Stop(2);											
								}
							}						
						
					}	
					else{	//months 6/22,7,8,9,10,11...				
							if(reverse){
							  Reverse(2);
							  delayTimeFunc(resetTime);
							  Stop(2);		  
							}
							else if(forward){
							  Forward(2);
							  delayTimeFunc(resetTime);
							  Stop(2);		  
							}//RESET...
							
                        if(reverse){							
								totalActuationTime = (dayNumberofYear*actuatorTwoDelayTimePerDayForward);								
								Forward(2);
								delayTimeFunc(totalActuationTime);
								Stop(2);	
						}
						else if(forward){							
								totalActuationTime = (dayNumberofYear*actuatorTwoDelayTimePerDayReverse);
								Reverse(2);
								delayTimeFunc(totalActuationTime);
								Stop(2);								
						}
					}
            
            
          }
                        
                        setseasonalAxis = false;					  
						configInProgress=false;											
						EEPROM.write(15,0);
						sendEndChar();    
    }
    else{  
	
		bool actuated = EEPROM.read(9);
        
	
		actuatorTwoDelayTimePerDayReverse = 1000*((actuatorTwoReverseRuntimeCalculated*0.88)/182);
		actuatorTwoDelayTimePerDayForward = 1000*((actuatorTwoForwardRuntimeCalculated*0.88)/182);
	
	//was 0.84 before. Made it 0.88 so its able to compensate for weight and in case the 16% movement forwards and backwards messes up, meaning over actuates.
	
		if(!actuated){			
            if(month<=6 && dayNumberofYear<=172){        //june 21st...              
             
					if(reverse){		
						  Reverse(2);
						  delayTimeFunc(actuatorTwoDelayTimePerDayReverse/*+adjustementDelay*/);//adjustementFactor
						  Stop(2); 													
					}
					else if(forward){				
						  Forward(2);
						  delayTimeFunc(actuatorTwoDelayTimePerDayForward/*+adjustementDelay*/);
						  Stop(2);						  
					}        
                  EEPROM.write(9,1);//indicating that we have actuated for this day.                     
            }
            else{               
					if(month==12){							
							if(dayNumberofYear>355){
								if(reverse){				
										totalActuationTime = actuatorTwoDelayTimePerDayReverse/*+adjustementDelay*/;
										Reverse(2);
										delayTimeFunc(totalActuationTime);
										Stop(2);
								}
							
								else if(forward){																
										totalActuationTime = actuatorTwoDelayTimePerDayForward/*+adjustementDelay*/;								
										Forward(2);
										delayTimeFunc(totalActuationTime);
										Stop(2);										
								}
								
							}
							else{
								if(reverse){
									
											totalActuationTime = actuatorTwoDelayTimePerDayForward/*+adjustementDelay*/ ;								
											Forward(2);
											delayTimeFunc(totalActuationTime);
											Stop(2);									
								}
								else if(forward){									
										
											totalActuationTime = actuatorTwoDelayTimePerDayReverse/*+adjustementDelay*/;
											Reverse(2);
											delayTimeFunc(totalActuationTime);
											Stop(2);												
								}
							}							
							
						
					 EEPROM.write(9,1);//indicating that we have actuated for this day. 		
						
					}	
					else{//months 7,8,9,10,11
						 if(reverse){
							 // int adjustementDelay = 1000*(actuatorTwoForwardRuntime*adjustementFactor);
							  Forward(2);
							  delayTimeFunc(actuatorTwoDelayTimePerDayForward/*+adjustementDelay*/);
							  Stop(2);
						 }
						 else if(forward){
							 // int adjustementDelay = 1000*(actuatorTwoReverseRuntime*adjustementFactor);
							  Reverse(2);
							  delayTimeFunc(actuatorTwoDelayTimePerDayReverse/*+adjustementDelay*/);
							  Stop(2); 	
							
						 }    
					}					 
					EEPROM.write(9,1);//indicating that we have actuated for this day.
                 
            }   
		}
	
			
			//Serial.println("END OF SEASONAL ACTUATOR MOVEMENT");
    }
	lcdInit();
   
}



void setDelayTimingsArray(int &actuatorOneForwardRuntime,int &actuatorOneReverseRuntime){
	//why recompute when its the same value that needs to be set?....
	float additionalDelayReverse = ((slaveDeviceTimeCountDelay*actuatorOneReverseRuntime)/1000);
	float additionalDelayForward = ((slaveDeviceTimeCountDelay*actuatorOneForwardRuntime)/1000);
	
	
	float actuatorOneReverseRuntimeCalculated = actuatorOneReverseRuntime+additionalDelayReverse;	
	float actuatorOneForwardRuntimeCalculated = actuatorOneForwardRuntime+additionalDelayForward;
	
	
	
	
	delayTimesForwardActuatorOne[0] = (1000*((actuatorOneForwardRuntimeCalculated*2.45)/38))/2; //7-8
    delayTimesForwardActuatorOne[1] = delayTimesForwardActuatorOne[0]; //7-8
    
    delayTimesForwardActuatorOne[2] = (1000*((actuatorOneForwardRuntimeCalculated*2.54)/38))/2; //8-9   
    delayTimesForwardActuatorOne[3] = delayTimesForwardActuatorOne[2]; //8-9
    
    delayTimesForwardActuatorOne[4] = (1000*((actuatorOneForwardRuntimeCalculated*2.6)/38))/2; //9-10    //was 2.4
    delayTimesForwardActuatorOne[5] =   delayTimesForwardActuatorOne[4]; //9-10

    
    delayTimesForwardActuatorOne[6] = (1000*((actuatorOneForwardRuntimeCalculated*3.7)/38))/2; //10-11 //was 3.5
    delayTimesForwardActuatorOne[7] =  delayTimesForwardActuatorOne[6]; //10-11
    
    delayTimesForwardActuatorOne[8] = (1000*((actuatorOneForwardRuntimeCalculated*4.5)/38))/2; //11-12    
    delayTimesForwardActuatorOne[9] =   delayTimesForwardActuatorOne[8]; //11-12

    
    delayTimesForwardActuatorOne[10] = (1000*((actuatorOneForwardRuntimeCalculated*4.6)/38))/2; //12-1
    delayTimesForwardActuatorOne[11] = delayTimesForwardActuatorOne[10]; //12-1
    
    delayTimesForwardActuatorOne[12] = (1000*((actuatorOneForwardRuntimeCalculated*4.6)/38))/2; //1-2
    delayTimesForwardActuatorOne[13] = delayTimesForwardActuatorOne[12]; //1-2
    
    delayTimesForwardActuatorOne[14] = (1000*((actuatorOneForwardRuntimeCalculated*4.5)/38))/2; //2-3
    delayTimesForwardActuatorOne[15] = delayTimesForwardActuatorOne[14]; //2-3
    
    delayTimesForwardActuatorOne[16] = (1000*((actuatorOneForwardRuntimeCalculated*3.4)/38))/2; //3-4    
    delayTimesForwardActuatorOne[17] = delayTimesForwardActuatorOne[16]; //3-4
    
    delayTimesForwardActuatorOne[18] = (1000*((actuatorOneForwardRuntimeCalculated*3.5)/38))/2; //4-5    
    delayTimesForwardActuatorOne[19] = delayTimesForwardActuatorOne[18]; //4-5
    
    delayTimesForwardActuatorOne[20] = (1000*((actuatorOneForwardRuntimeCalculated*2.45)/38))/2;//5-6    
    delayTimesForwardActuatorOne[21] = delayTimesForwardActuatorOne[20];//5-6
    //Serial.println("Storing Forward time for daily actuator");    
	/*--------------------------------------------------------------------------------------------------------------------------*/
	delayTimesReverseActuatorOne[0] = (1000*((actuatorOneReverseRuntimeCalculated*2.45)/38))/2; //7-8
    delayTimesReverseActuatorOne[1] = delayTimesReverseActuatorOne[0]; //7-8
    
    delayTimesReverseActuatorOne[2] = (1000*((actuatorOneReverseRuntimeCalculated*2.54)/38))/2; //8-9    
    delayTimesReverseActuatorOne[3] = delayTimesReverseActuatorOne[2]; //8-9
    
    delayTimesReverseActuatorOne[4] = (1000*((actuatorOneReverseRuntimeCalculated*2.4)/38))/2; //9-10    
    delayTimesReverseActuatorOne[5] = delayTimesReverseActuatorOne[4]; //9-10

    
    delayTimesReverseActuatorOne[6] = (1000*((actuatorOneReverseRuntimeCalculated*3.5)/38))/2; //10-11
    delayTimesReverseActuatorOne[7] = delayTimesReverseActuatorOne[6]; //10-11
    
    delayTimesReverseActuatorOne[8] = (1000*((actuatorOneReverseRuntimeCalculated*4.5)/38))/2; //11-12    
    delayTimesReverseActuatorOne[9] = delayTimesReverseActuatorOne[8]; //11-12

    
    delayTimesReverseActuatorOne[10] = (1000*((actuatorOneReverseRuntimeCalculated*4.6)/38))/2; //12-1
    delayTimesReverseActuatorOne[11] = delayTimesReverseActuatorOne[10]; //12-1
    
    delayTimesReverseActuatorOne[12] = (1000*((actuatorOneReverseRuntimeCalculated*4.6)/38))/2; //1-2
    delayTimesReverseActuatorOne[13] = delayTimesReverseActuatorOne[12]; //1-2
    
    delayTimesReverseActuatorOne[14] = (1000*((actuatorOneReverseRuntimeCalculated*4.5)/38))/2; //2-3
    delayTimesReverseActuatorOne[15] = delayTimesReverseActuatorOne[14]; //2-3
    
    delayTimesReverseActuatorOne[16] = (1000*((actuatorOneReverseRuntimeCalculated*3.4)/38))/2; //3-4    
    delayTimesReverseActuatorOne[17] = delayTimesReverseActuatorOne[16]; //3-4
    
    delayTimesReverseActuatorOne[18] = (1000*((actuatorOneReverseRuntimeCalculated*3.5)/38))/2; //4-5    
    delayTimesReverseActuatorOne[19] = delayTimesReverseActuatorOne[18]; //4-5
    
    delayTimesReverseActuatorOne[20] = (1000*((actuatorOneReverseRuntimeCalculated*2.45)/38))/2;//5-6    
    delayTimesReverseActuatorOne[21] = delayTimesReverseActuatorOne[20];//5-6
	
}


void setdailyActuatorTimings(int &actuatorOneForwardRuntime,int &actuatorOneReverseRuntime){	   
	  EEPROM.write(8,11);//this means it will now reset the actuator to a 0 position and then re-actuate it to the current position according to whatever time it is  
      getDailyActuatorRuntime(actuatorOneForwardRuntime,actuatorOneReverseRuntime);//Daily Actuator. Param input needs to come in from the app.   
	  
      //actuatorOneForwardRuntime += 1;
     // actuatorOneReverseRuntime += 1;
      EEPROM.write(0, 'A');    
      EEPROM.write(1, 'O');  
      EEPROM.write(2, actuatorOneForwardRuntime);    
      EEPROM.write(3, actuatorOneReverseRuntime);
      delayTimeFunc(10);
      if(dailyActuatorOrientation=='R'){
        Forward(1);    
        delayTimeFunc(resetTime);
        Stop(1);
		
	
      }
      else{
        Reverse(1);
        delayTimeFunc(resetTime);
        Stop(1);
		
	
      }
	  
	  setDelayTimingsArray(actuatorOneForwardRuntime,actuatorOneReverseRuntime);
  
    //Serial.println("Storing Reverse time for daily actuator");
	  
	configInProgress=false;
	 
	
}


void setseasonalActuatorTimings(int &actuatorTwoForwardRuntime,int &actuatorTwoReverseRuntime,int month){
	  //Serial.println("Writing seasonal Actuator time to EEPROM"); 
	  EEPROM.write(9,0);/*we are writing 0 to this index because we calibrated the actuator and reset it to either a fully extended or fully retracted position.
      When this happens, the program loses reference to its position as it was before. Imagine it stopped at 1 PM then started at 4 PM. We at 4 PM calibrated it to get the timings,
      at which point the actuator will be either fully extended or fully retracted. we need to have the doSeasonalMovement start from index 0 to actually make the 
      actuator stop at the proper position....     
      */ 
	  getSeasonalActuatorRuntime(actuatorTwoForwardRuntime,actuatorTwoReverseRuntime);//Seasonal Actuator. Param input needs to come in from the app.
      //actuatorTwoForwardRuntime += 1;
     // actuatorTwoReverseRuntime += 1;
	/* actuatorTwoForwardRuntime = actuatorTwoForwardRuntime+((slaveDeviceTimeCountDelay*actuatorTwoForwardRuntime)/1000);
	 actuatorTwoReverseRuntime = actuatorTwoReverseRuntime+((slaveDeviceTimeCountDelay*actuatorTwoReverseRuntime)/1000);*/
	
	 
      EEPROM.write(4, 'A');    
      EEPROM.write(5, 'T');   
      EEPROM.write(6, actuatorTwoForwardRuntime);   
      EEPROM.write(7, actuatorTwoReverseRuntime);

    if(seasonalActuatorOrientation=='R'){
	  if(month<=6){
		  /*Serial.print("Month: ");
		  Serial.println(month);		  
		  Serial.println("resetting seasonal actuator");*/
		  clearLCDScreen();    
		  lcd.print("resetting ");
		  Forward(2);
		  delayTimeFunc(resetTime);//resetting		  
		  Stop(2);
		  
	  }
	  else{
		  /*Serial.print("Month: ");
		  Serial.print(month);
		  Serial.println("resetting seasonal actuator");*/
		  clearLCDScreen();    
		  lcd.print("resetting ");
		  Reverse(2);
		  delayTimeFunc(resetTime);//resetting
		  Stop(2);
		  
	  }
	}
	else{
	  if(month<=6){
		  /*Serial.print("Month: ");
		  Serial.println(month);		  
		  Serial.println("resetting seasonal actuator");*/
		  clearLCDScreen();   
		  lcd.print("resetting ");
		  Reverse(2);
		  delayTimeFunc(resetTime);//resetting		  
		  Stop(2);
	  }
	  else{
		  /*Serial.print("Month: ");
		  Serial.print(month);
		  Serial.println("resetting seasonal actuator");*/
		  clearLCDScreen();    
		  lcd.print("resetting ");
		  Forward(2);
		  delayTimeFunc(resetTime);//resetting
		  Stop(2);
	  }
		
	}
	
	
	configInProgress=false;
	
}









/*dont poll the function every hour
it needs to progress every hour according to the block that its currently in
blocks will be 0-10, so a total of 11 blocks from 7am-6pm.
it needs to sit at the last position between 7pm-8pm. 
at 8pm, it needs to reset to the 0 position depending on the orientation
at 7am, it needs to be at 0 position depending on the orientation
if it stalled and starts later, then it needs to reset and actuate to the current blocks value
*/

void doDailyMovement(int currentBlockValue,char actuatorOrientation,int hour,int minute){
	
	int totalDelay=0;	
	int actualPreviousBlockValue=EEPROM.read(8);	//9
	delayTimeFunc(5);
	int expectedPreviousBlockValue=currentBlockValue-1;//-1
	bool forceResetFlag=false;
	int goBackToBlock = 9;/*by negating this value from a current block, it will set it to some previos block, depending on the value of the current block
						we need this so that we can choose for example blocks of 12,12:30, and 1, which have the max movement, because they have the highest time...*/
	

	if(setDailyAxis){
		
				if(forceReset){
						totalDelay=0;					
						if(actuatorOrientation=='R'){	
							Forward(1);
							delayTimeFunc(resetTime); 
							Stop(1);//reset
								for(int i=0;i<=currentBlockValue;i++){
									totalDelay+= delayTimesReverseActuatorOne[i];
								}
							
							Reverse(1);						
							delayTimeFunc(totalDelay);
							Stop(1);								
							totalDealyTemp = totalDelay;
							currentBlockTemp = currentBlockValue;
							forceReset=false;
							forceResetFlag=true;
							if(hour>=16 && hour<20){//could also use currentBlockValue....
								endHours=true;
							}
						}
						else{
							Reverse(1);
							delayTimeFunc(resetTime); 
							Stop(1);//reset
								for(int i=0;i<=currentBlockValue;i++){
									totalDelay+= delayTimesReverseActuatorOne[i];
								}
							
							Forward(1);						
							delayTimeFunc(totalDelay);
							Stop(1);								
							totalDealyTemp = totalDelay;
							currentBlockTemp = currentBlockValue;
							forceReset=false;
							forceResetFlag=true;
							if(hour>=16 && hour<20){//could also use currentBlockValue....
								endHours=true;
							}
							
						}
						
				}
				if(currentBlockValue==0){				
					if(actuatorOrientation=='R'){					
							Forward(1);
							delayTimeFunc(resetTime);
							Stop(1);//reset
							
							
							delayTimeFunc(2000);
							Reverse(1);
							delayTimeFunc(delayTimesReverseActuatorOne[0]);
							Stop(1);
							EEPROM.write(16,0);//reset to 0							
							expectedPreviousBlockValue= actualPreviousBlockValue;							
					}
					else{
						
							Reverse(1);
							delayTimeFunc(resetTime);
							Stop(1);//reset
							
							
							delayTimeFunc(2000);
							Forward(1);
							delayTimeFunc(delayTimesReverseActuatorOne[0]);
							Stop(1);
							EEPROM.write(16,0);//reset to 0						
							expectedPreviousBlockValue= actualPreviousBlockValue;						
						
					}
				}
				
				if(startHours && designType=='A'){
					//Serial.print("totalDealy NOW: ");
					//Serial.print(totalDelay);
					totalDelay=0;
					if(actuatorOrientation=='R'){		
						
						if(!EEPROM.read(16)){
							int startBlock=1;
							if(currentBlockValue>1){
								startBlock=currentBlockValue+1;
							}
							
							for(int i=startBlock;i<7;i++){//7 because 10:00 AM is in index 6...starting from 1 because we have already indexed 0 at 7AM.indexing 7:30-9 
								totalDelay+= delayTimesReverseActuatorOne[i];
							}
							Reverse(1);
							delayTimeFunc(totalDelay);
							Stop(1);
							EEPROM.write(16,1);
						}						
				
					}
					else{
						
						if(!EEPROM.read(16)){
							int startBlock=1;
							if(currentBlockValue>1){
								startBlock=currentBlockValue+1;
							}
							
							for(int i=startBlock;i<7;i++){//7 because 10:00 AM is in index 6...starting from 1 because we have already indexed 0 at 7AM.indexing 7:30-9 
								totalDelay+= delayTimesReverseActuatorOne[i];
							}
							Forward(1);
							delayTimeFunc(totalDelay);
							Stop(1);
							EEPROM.write(16,1);
						}			
						
					}
				}
			    if(currentBlockValue==22){
						if(actuatorOrientation=='R'){		
							Forward(1);
							delayTimeFunc(resetTime);
							Stop(1);					
						}
						else{
							Reverse(1);
							delayTimeFunc(resetTime);
							Stop(1);						
						}
				}

				else {
					
					if(expectedPreviousBlockValue != actualPreviousBlockValue && !forceResetFlag){
						totalDelay=0;
						if(actuatorOrientation=='R'){		
							
							Forward(1);
							delayTimeFunc(resetTime);
							Stop(1);//reset
							for(int i=0;i<=currentBlockValue;i++){
								totalDelay+= delayTimesReverseActuatorOne[i];
							}				
												
							Reverse(1);
							delayTimeFunc(totalDelay);
							Stop(1);			
							
							totalDealyTemp = totalDelay;
							currentBlockTemp = currentBlockValue;						
						}
						else{							
							Reverse(1);
							delayTimeFunc(resetTime);
							Stop(1);//reset
							for(int i=0;i<=currentBlockValue;i++){
								totalDelay+= delayTimesReverseActuatorOne[i];
							}				
												
							Forward(1);
							delayTimeFunc(totalDelay);
							Stop(1);			
							
							totalDealyTemp = totalDelay;
							currentBlockTemp = currentBlockValue;		
							
						}

					}
					else{
						
						if(designType=='A' && endHours){
							totalDelay=0;
							if(actuatorOrientation=='R'){
									if(forceResetFlag){
										
										for(int i=19;i<=currentBlockValue;i++){//19 is 4:30 PM...so start from there. 
											totalDelay += delayTimesForwardActuatorOne[i-goBackToBlock];//goBackToBlock will actuate it for timings of 12,12:30 and 1 PM..
										}
											Forward(1);
											delayTimeFunc(totalDelay);
											Stop(1);
									
									
									}
									else{
										totalDelay = delayTimesForwardActuatorOne[currentBlockValue-goBackToBlock];//goBackToBlock will actuate it for timings of 12,12:30 and 1 PM..							
										Forward(1);
										delayTimeFunc(totalDelay);
										Stop(1);
									}								
							}
							else{
									if(forceResetFlag){
										
										for(int i=19;i<=currentBlockValue;i++){//19 is 4:30 PM...so start from there. goBackToBlock will actuate it for timings of 12,12:30 and 1 PM..
											totalDelay += delayTimesForwardActuatorOne[i-goBackToBlock];//goBackToBlock will actuate it for timings of 12,12:30 and 1 PM..
										}
											Reverse(1);
											delayTimeFunc(totalDelay);
											Stop(1);
									
									
									}
									else{
										totalDelay = delayTimesForwardActuatorOne[currentBlockValue-goBackToBlock];//goBackToBlock will actuate it for timings of 12,12:30 and 1 PM..
										Reverse(1);
										delayTimeFunc(totalDelay);
										Stop(1);
									}							
								
							}
							
							
							
						}
						else{
							if(hour<18  && !forceResetFlag){
								if(actuatorOrientation=='R'){
											if(designType=='A'){
												if( currentBlockValue>=7){
													totalDelay=0;
													totalDelay = delayTimesReverseActuatorOne[currentBlockValue];
													Reverse(1);
													delayTimeFunc(totalDelay);
													Stop(1);									
												}
											}
											else{
												if(currentBlock>0){
														totalDelay=0;
														totalDelay = delayTimesReverseActuatorOne[currentBlockValue];
														Reverse(1);
														delayTimeFunc(totalDelay);
														Stop(1);													
												}
												
											}
								}
								else{
									
									if(designType=='A'){
												if( currentBlockValue>=7){
													totalDelay=0;
													totalDelay = delayTimesReverseActuatorOne[currentBlockValue];
													Forward(1);
													delayTimeFunc(totalDelay);
													Stop(1);													
												}
											}
											else{
												if(currentBlock>0){
														totalDelay=0;
														totalDelay = delayTimesReverseActuatorOne[currentBlockValue];
														Forward(1);
														delayTimeFunc(totalDelay);
														Stop(1);													
												}
												
											}
									
								}
								
								
								
							}
						}
					}
					
					
					
				}
				setDailyAxis=false;
				EEPROM.write(8, currentBlockValue);
				delayTimeFunc(5);									
				configInProgress=false;
				sendEndChar();
	}
	lcdInit();
	delayTimeFunc(500);

}

void stopatOptimumAngle(){
		int delayTime=0;
	   clearLCDScreen();
       lcd.print("Optimum Position");
	   float additionalDelayReverse = ((slaveDeviceTimeCountDelay*actuatorOneReverseRuntime)/1000);
	   float additionalDelayForward = ((slaveDeviceTimeCountDelay*actuatorOneForwardRuntime)/1000);
	
	  
		if(dailyActuatorOrientation=='R'){
			Forward(1);
			delayTimeFunc(resetTime);
			Stop(1);
			delayTimeFunc(2000);			
		
		    delayTime = (1000*((actuatorOneReverseRuntime+additionalDelayReverse)/2));
		
			Reverse(1);
			delayTimeFunc(delayTime);
			Stop(1);
			delayTime=0;
			delayTimeFunc(2000);
		}
		else if(dailyActuatorOrientation=='F'){
			Reverse(1);
			delayTimeFunc(resetTime);
			Stop(1);
			delayTimeFunc(2000);						
			delayTime = (1000*((actuatorOneReverseRuntime+additionalDelayReverse)/2));			
			Forward(1);
			delayTimeFunc(delayTime);
			Stop(1);
			delayTime=0;
			delayTimeFunc(2000);
			
		}
	optimumAngleSet=true;	
	configInProgress=false;
	sendEndChar();
	lcdInit();
	delayTimeFunc(500);
	
}

/*
digviraj.patil@virajeng.com
digvijay007
*/



/*Dual axis time based solar tracker(does not use a light sensor and is much more accurate as well as reliable to the given location). Price, which includes transportation,
installtion and calibration of system would be Rs. 200200/kW*

features:
moves during peak sun light hours...
resets after 8pm
resets to the right position if lights go out
calibrates actuators 

check crystals and capacitor for ds1307. looks like it may need a 12.5pF capacitor across the crystal of X1 and X2 and the crystal...
get the reset buttons right
add a 4.7k resistor for i2c
move the i2c ports else where and add an extra port as well
may be protect circuit for surges?



add an adjustement factor for both seasonal and daily movements. make it some % of the overall timing. should be able to get it from the phone.

get the bluetooth to set the system time:
try changing time on external rtc with bluetooth: 
-set the rtc to some random time. 
-then connect bt and try to set it from the app
then connect to main program and see if that works.



-there should be a check on every hour if the hour is actually the hour returned by the rtc
-only request the hour and minute every 30 mins...could millis until next read 

*/

