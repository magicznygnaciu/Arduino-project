#include <SimpleTimer.h>

#include <TM1638.h>

char* CODE = "00000000";
int COUNT = 8;
boolean isSegmentChosen;

int encoderPinA = 2;
int encoderPinB = 3;
int encoderButton = 4;
int segmentNumber = 7;


//rortary param
int encoderPinALastState = LOW;
int encoderPinACurrentState = LOW;

enum programStates{
  INITIALIZE = 0,
  EDIT = 1,
  INDIRECT = 2,
  COUNTDOWN = 3,
  BOOM = 4
};
enum buttonStates{
  NO_ACTION = 0,
  CLICK = 1,
  LONG_CLICK = 2,
  RIGHT = 3,
  LEFT = 4
};

//program variables
int currentState;

//timer variables
TM1638 module(8,9,7);
SimpleTimer timer;
int number =100;
int timeInterval = 100;
int timerId;

//button variables
long buttonTimer = 0;
long longPressTime = 600;

boolean buttonActive = false;
boolean longPressActive = false;

//display params
int blinkTime = 500   ;
boolean blinkActive = false;
int blinkTimer;

//boooooom params
long timeToExplosion;
long codeToTimeExplosion;



void setup()
{
  configureEncoder();
  currentState = INITIALIZE;
  buttonActive = false;
  Serial.begin(9600);
}

void loop()
{
  action();
  //timer.run();


}
void initialize();
void edit();
void indirect();
void countdown();
void boom();


void action(){
  switch (currentState){
    case INITIALIZE:
      initf();
      break;
    case EDIT:
      edit();
      break;
    case INDIRECT:
      indirect();
      break;
    case COUNTDOWN:
      countdown();
      break;
    case BOOM:
      boom();
      break;
  }
}

void initf(){
    displayFromCode();
    if (encoderButtonClicked() == LONG_CLICK){
      currentState ++;
      isSegmentChosen = false; //ustawienie domyślnego trybu edycji, wybór liczby
      module.clearDisplay();
      
    }
}

void edit(){
  segmentBlink();
  //module.setDisplayToString(CODE);
  switch (encoderButtonClicked()){
    case CLICK:
      isSegmentChosen = !isSegmentChosen;
      break;
    
    case LONG_CLICK:
      currentState = INDIRECT;
      //Serial.print(currentState);
      break;

    case RIGHT:
      if(!isSegmentChosen){
        segmentNumber++;
        if(segmentNumber > 7){
          segmentNumber = 8 - COUNT;
        }
      }else{
        if (CODE[7 - segmentNumber] >= '9'){
          CODE[7 - segmentNumber] = '0';
        } else {
          CODE[7 - segmentNumber]+=1;
        }
      }
     
    
     break;
    case LEFT:
     if(!isSegmentChosen){
        if (segmentNumber > 8 - COUNT){
          segmentNumber--;
        } else {
          segmentNumber = 7;
        }
      }else{
        if (CODE[7-segmentNumber] <= '0'){
          CODE[7-segmentNumber] = '9';
        } else {
          CODE[7-segmentNumber]-=1;
        }      
      }
      
      break;
  }
  
}
void indirect(){
  displayFromCode();
  int action = encoderButtonClicked(); 
  if(action ==LONG_CLICK){
    currentState = EDIT;
  }else if(action == CLICK){
    timeToExplosion = getTimeExplosion();
    timerId =timer.setInterval(timeInterval, decreaseTimer);
    currentState = COUNTDOWN;
  }
}
void countdown(){
  timer.run();
  module.setDisplayToDecNumber(timeToExplosion,0,false);
  Serial.print(timeInterval);
  Serial.print("\n");
  switch(encoderButtonClicked()){
    case CLICK:
      timer.toggle(timerId);
      break;
    case RIGHT:
      timer.deleteTimer(timerId);
      //maxSpeed1();
      timerId = timer.setInterval(timeInterval,maxSpeed1);
      
    break;
    case LEFT:
      timer.deleteTimer(timerId);
      timerId = timer.setInterval(timeInterval, decreaseTimer);
    break;
      
  }
  
  if(timeToExplosion==0){
    timer.disable(timerId);
    currentState=BOOM;
  }
  
}

void maxSpeed1(){
  long MaxSpeed = timeToExplosion/600*COUNT;
  long fastStep = MaxSpeed/10;
  long timeToMinus = MaxSpeed - fastStep;
  long one = 1;
  timeToExplosion -= one + timeToMinus;
}

/*void maxSpeed2(){ //gdy schodzi do 0 lub <0 to error
  if(timeToExplosion==0){ //nie wiem gdzie dać przejscie do trybu boom 
    currentState=BOOM;
  }
  if(timeInterval == 0 && timeToExplosion >= 19999){
    timeToExplosion-=19999;
  }
  if(timeInterval == 0 && timeToExplosion<19999){
    timeToExplosion-=19;
  }
  if(timeInterval == 0 && timeToExplosion<1999){
    timeToExplosion--;
  }
  
}*/
void boom(){
  Serial.print("BOOOOOM");
  Serial.print("\n");
}

void decreaseTimer(){
  if (timeToExplosion > 0){
    timeToExplosion-= 1;
  }
}
void configureEncoder(){
  pinMode (encoderPinA, INPUT);
  pinMode (encoderPinB, INPUT);
  pinMode (encoderButton, INPUT);
  
}

void segmentBlink(){
  for(int i = 7; i >= 8 - COUNT ; i--){
    if(i != segmentNumber){
      module.setDisplayDigit(CODE[8 - i - 1],i,false);
    }else{
      int segmentTime = (millis()-blinkTimer)%1000; //bez modułu błąd po 30 sekundach - do segmentTime dodaje się 65000
       if(blinkActive == false){
          blinkActive = true;
          blinkTimer = millis();
      } else if(segmentTime < blinkTime){
          module.setDisplayDigit(CODE[8 - i - 1],i,false);
      } else if(segmentTime<blinkTime*2){
          module.clearDisplayDigit(segmentNumber, false);
      } else if(segmentTime> blinkTime*2){
          blinkActive = false;
      }

    }
   /* Serial.print("Segment ");
    Serial.print(i);
    Serial.print(" "); 
    Serial.print(millis()-blinkTimer);      
    Serial.print("\n");*/
  
  }
 
}
int encoderButtonClicked(){
  if (digitalRead(encoderButton) == LOW) {

    if (buttonActive == false) {

      buttonActive = true;
      buttonTimer = millis();

    }

    if ((millis() - buttonTimer > longPressTime) && (longPressActive == false)) {

      longPressActive = true;


    }

  } else {
  //krencenie pencak
   
    if (buttonActive == true) {
      buttonActive = false;
    
      if (longPressActive == true) {

        longPressActive = false;
        return LONG_CLICK;        
      } else {
        return CLICK;
      }

    } else {
     encoderPinACurrentState = digitalRead(encoderPinA);
     if((encoderPinALastState == LOW) && (encoderPinACurrentState == HIGH)){
       encoderPinALastState = encoderPinACurrentState;
       if(digitalRead(encoderPinB) == LOW){
        return RIGHT;
       }else{
         return LEFT;
       }
     }
     encoderPinALastState = encoderPinACurrentState;
    }

  }
  

     
  return 0;
  
}

void displayFromCode(){
    for(int i = 7; i >= 8 - COUNT ; i--){
      module.setDisplayDigit(CODE[8 - i - 1],i,false);
    }
}
long getTimeExplosion(){
  long sum = 0;
  long multiplier = 1;
  for(int i=0;i < COUNT;i++){
    sum += multiplier*(CODE[i] - '0');
    Serial.print("CODE:");
    Serial.print(multiplier);
    Serial.print(sum);
    Serial.print("\n");
    multiplier *= 10;
  }
 
  return sum;
}
