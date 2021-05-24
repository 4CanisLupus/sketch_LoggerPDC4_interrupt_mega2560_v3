//Logger PDC4 on SD card for
//arduino mega 2560
//need TODO write config.txt, another method for reading PDC4
#include <SPI.h>
#include <SD.h>
#include <Wire.h> 
#include <iarduino_RTC.h>
#include <avr/sleep.h>
//CONSTANTS

//control voltage for begin (!!!! need to check the number of pin)
#define CONTROLVOL_PIN 24
// pin for correct initialazing
#define INI_PIN 13
// pin for catching impulse (!!!! need to check the number of pin)
#define IMP_PIN 25
//interrupting pin
#define INTER_PIN 3
//sizes of arrays
#define BUFSIZE 50
#define BITNUM 9
#define CHANNELNUM 5 //5 ???

//  For RTC  DS1302
//  #define pin_SW_SDA 3                         // Назначение любого вывода Arduino для работы в качестве линии SDA программной шины I2C.
//  #define pin_SW_SCL 9                         // Назначение любого вывода Arduino для работы в качестве линии SCL программной шины I2C.
// DS2302
// RST pin 42
// CLK pin 43
// DAT = 40 
// For SD-cart
// CLK PIN 52
// D0 PIN 50
// DI PIN 51
// CS PIN 53
//VARIABLES

//for time scheme setting
// pin DS2302 RST, CLK, DAT
 iarduino_RTC time(RTC_DS1302, 42, 43, 40); 
 // seconds, minutes, hours, days of month, months, years, days of week (the first and the last one can be omitted)
 int seCu,miNu,hoUr,dayM,monT,yeaR,dayW;
 // delay for timer. use in start
 uint32_t timEr;
 // change this to match your SD shield or module;
// MKRZero SD: SDCARD_SS_PIN
const int chipSelect = 53;
//number of channels
int channelNum;
//number of using in code bits
int bitNum;
// counter bits
int counterBit;
//counter channes
int counterChannel;
// word array [bits]
int dataW[BITNUM];
//words (channel)
int dataS[CHANNELNUM];
//  string buffer for sending DATA on SD card
char sdBuffer[BUFSIZE];
// string buffer for saving CONFIG parameters on SD card
char sdConfig[BUFSIZE];
//counter the all impulses
int counterAllBit;
// CONTROL PIN flag
bool controlVoltageFlag;
//all impulses
//int impSentence;

// or struct of sentence

  //time of record
  //iarduino_RTC time;
  //channel 1 serviceData;
  //channel 2 currentSpeed;
  //channel 3 currentDirection;
  //channel 4 temperature;
  //channel 5 conductivity;
  //channel 6 pressure;
// 
  //№ RCM (there are next numbers of RCM: 98, 100, 121,124,215,181,183,184,634,642,647, 1329 (1329=132))
  int numRCM;
  // data file name on SD card
  char*  fileName;
  char nameBuffer[BUFSIZE];
  
void setup() {
Serial.begin(9600);
time.begin();

  //initialization
 pinMode(CONTROLVOL_PIN, INPUT);//parallel to INTER_PIN
 pinMode(IMP_PIN, INPUT);
 pinMode(INI_PIN, INPUT); //if jumper between 13pin and GND connected
 pinMode(INTER_PIN,INPUT_PULLUP); // interrupt from CONTROL PIN
 bitNum=BITNUM+1;
 channelNum=6;//CHANNELNUM+1;//+1???
 counterBit=0;
 counterChannel=0;
 counterAllBit=0; 
 timEr=0;//in miliseconds 
 numRCM=0; //0 flag of error
 controlVoltageFlag=false;
 //impSentence=channelNum*bitNum+1;//for clear synchro impulse ???
 fileName="datalog.txt";
 
//initialize SD card
Serial.println("Working without card .");
 while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
//    Serial.print("Initializing SD card...");
//   // see if the card is present and can be initialized:
//  if (!SD.begin(chipSelect)) {
//    Serial.println("Card failed, or not present");
//    // don't do anything more:
//    while (1);
//  }
//  Serial.println("card initialized.");


  //for manual load params
  if (digitalRead(INI_PIN)==LOW){
    choosingStartMethod();
  }
   
}

//TODO  delete from sentence synchroimpulse using quantity of channels
void loop() {
  //
   attachInterrupt(digitalPinToInterrupt(INTER_PIN), EventListener, LOW);
   set_sleep_mode(SLEEP_MODE_PWR_DOWN);
//   Serial.print(" sleeping point ");
   sleep_mode();
   // need to have control voltage to begin process (!!!! need to check HIGH or LOW)
//  if (digitalRead(CONTROLVOL_PIN)==LOW){
//     //for debagging
//    //Serial.print("  控制销 checking point ");
//   
//     readSentence(counterBit,counterChannel);
//    
//     //TODO another way to count pulse
//    // readSentence2(counterBit,counterChannel);
//   }
//  else {
//    //set the zero to all units, if no signal 
//    int i,j=0;
//    counterBit=0;
//     counterChannel=0;
//        // clear the date -????
//        for (i=0; i<(bitNum+1);i++)
//        dataW[i]=0;
//        for (j=0; j<(channelNum);j++)//+1
//        dataS[j]=0;
//  } 
}
//interrupt handler
void EventListener (){
     
        while (digitalRead(CONTROLVOL_PIN)==LOW){
           //controlVoltageFlag=true;
           readSentence(counterBit,counterChannel);
           //controlVoltageFlag=false;
        }   
    //set the zero to all units, after///
    int i,j=0;
     counterBit=0;
     counterChannel=0;
        // clear the date -????
        for (i=0; i<(bitNum+1);i++)
        dataW[i]=0;
        for (j=0; j<(channelNum);j++)//+1
        dataS[j]=0;
    
}
 // reading of sequence of impulses 
 // time in microsec
void readSentence(int num, int nwords){

  long duration;//us (!!!! need to check HIGH or LOW)
  duration = pulseIn(IMP_PIN, LOW);
  //imitation duration of 1~=82307
  //imitation duration of 0~=27775
    if((duration>26000)&&(duration<29000)){
      dataW[num]=1;
    //for debugging
    //Serial.print(dataW[num]);
    //Serial.print(" count ");
    //Serial.print(counterBit);
      counterBit++;
      counterAllBit++;
       if (counterBit>(bitNum-1)){
         Convert2String(dataW, sizeof(dataW), nwords);
         //for debugging
         // Serial.print(" dataW ");
        // Serial.print(dataW[num]);
        // Serial.print(" \n ");
         counterBit=0;
         delay(250);
      }
    }
    else if ((duration>82000)&&(duration<85000)){
     dataW[num]=0;
    //for debugging
    //Serial.print(dataW[num]);
//    Serial.print(" count ");
//    Serial.print(counterBit);
     counterBit++;
     counterAllBit++;
      if (counterBit>(bitNum-1)){
       Convert2String(dataW, sizeof(dataW), nwords);
     //for debugging
     //Serial.print(" dataW ");
     //Serial.print(dataW[num]);
     //Serial.print(" \n ");
       counterBit=0;
       delay(250);
      }
    }
}

 //make the decimecal number 
void Convert2String(int *data, int arrSize, int words){
 double sum=0;
 int j=0;
 arrSize=round(arrSize/sizeof(int));
  for (int i=arrSize;i>-1;i--){
     sum = sum + (pow(2,i) * data[j]);
    j++;
 }
  //preodering sentence
  dataS[words]=round(sum);
  
   //for debugging
  //Serial.print(time.gettime(" \n d-m-Y, H:i:s "));
  //Serial.print(words);
  // Serial.print (" \t");
  //Serial.print(dataS[words]);

  //use temp variable for set 0 (костыль!)???
  words++;
  counterChannel=words;
  //for debugging
//   Serial.print ("N ");
//  Serial.print(channelNum);
  
  //need to check!! ???
   if(words>(CHANNELNUM)){ 
        makeSentence(counterChannel);     
    }
}

void makeSentence(int count){ //count ???
 
    //sentence title
    strcat(sdBuffer, "#");
   strcat(sdBuffer, time.gettime("Y-m-d, H:i:s"));
   char s_word[8];
   
  // complicate the data 
  for (int i=0; i<channelNum;i++){//???
    strcat(sdBuffer,",");
    itoa(dataS[i],s_word,10);
    strcat(sdBuffer,s_word);
    }
   strcat(sdBuffer,"\r\n");  
   
  //for debugging
  //Serial.print(" \n");
  //Serial.print(" result ");
  Serial.print(sdBuffer);
  
  //to SD
  // commented out for debugging !!!!!!!!!!
  //writeToSD();
  //clear buffer
  for (int t=0; t<BUFSIZE; t++){
    sdBuffer[t]=0;
  }
    //loop for counter of channels
    counterChannel=0; 
    counterAllBit=0;
}

void writeToSD(){
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open(fileName, FILE_WRITE);
  //for debagging
  //Serial.println(fileName);
 
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.print(sdBuffer);
    dataFile.close(); 
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening file on SDcard");
  }
}

//choose the load method (no need for that????) 
void choosingStartMethod(){
  char buf[3];
   Serial.println("If you need to correct the parameters ENTER any key. For use default setting wait for 10 sec ");
 
  Serial.setTimeout(10000); //time limit
  int x=Serial.readBytes(buf,3); 
    if(x>0)initializingParams();
    else  Serial.println(time.gettime("Y-m-d, H:i:s"));
    
}

//manual initial procedure
void initializingParams(){
    //entering numbers of RCM, chanells,date and time
   do{
      numRCM=loadNumRCM();
     }  while((numRCM<=97)||(numRCM>=648));
      Serial.println(numRCM);
      
     //entering numbers of channels
   do{
        channelNum=loadNumChan();
     }while((channelNum<2)||(channelNum>=7));
       Serial.println(channelNum);
              
      // print date for checking
     Serial.println(time.gettime("Y-m-d, H:i:s"));
     // correct time
     //if need to set time manually 
     // time.settime(0,53,16,16,03,21,2);  // 0  сек,  мин,  час, число , месяц, 20... год, день недели
     timeCorrect();
            seCu=0; // default
            dayW=1; // default
           
     //making title of file
     Serial.println(" The data will be saved in the next file: ");
     makeTitle();
     
}
void makeTitle(){
//makeing the file name in the next format:
  // Date+RCM№.txt (8.3) 
    char tempNumRCM[8];
    
  strcat(nameBuffer, time.gettime("md"));
  strcat(nameBuffer,"N");
  itoa(numRCM,tempNumRCM,10);
  strcat(nameBuffer,tempNumRCM);
  strcat(nameBuffer,".txt");
  fileName=nameBuffer;
  //title of file 
  Serial.println(fileName);  
}

int loadNumRCM(){
  Serial.println("Please, enter right RCM number: ");
  return readNum();
 
}
int loadNumChan(){
    Serial.println("Please, enter right quantity of channels: ");
  return readNum();

}

void timeCorrect(){
  Serial.println("Are you need to correct current time(Y/N)?");
  char res=0;
  
  while (Serial.available()==0){
  }
    delay(100);
 
    while (Serial.available()){
        res=Serial.read();
        if((res=='y')||(res=='Y')){
          
          Serial.println(" Enter the year 20 ...(for example- 21 ):"); 
          do{
           yeaR=readNum();
            }  while((yeaR<=20)||(yeaR>=30));
            Serial.println(yeaR);  
          Serial.println(" Enter the month  ...(for example- 3 ):"); 
          do{
           monT=readNum();
            }  while((monT<=0)||(monT>=13)); 
             Serial.println(monT);   
          Serial.println(" Enter the day of month  ...(for example- 15 ):");    
          do{
           dayM=readNum();
            }  while((dayM<=0)||(dayM>=32));   
             Serial.println(dayM); 
          Serial.println(" Enter the hour  ...(for example- 15 ):");    
          do{
           hoUr=readNum();
            }  while((hoUr<0)||(hoUr>=25));     
             Serial.println(hoUr); 
           Serial.println(" Enter the minutes  ...(for example- 30 ):");    
          do{
           miNu=readNum();
            }  while((miNu<0)||(miNu>=61));    
             Serial.println(miNu);  
             //correcting the time
             time.settime(seCu,miNu,hoUr,dayM,monT,yeaR,dayW); 
              Serial.println(time.gettime("Y-m-d, H:i:s"));
        }
        else {Serial.println(" No time correct ");
          break;
          }
       }
     
  Serial.flush();
}

//auxiliary procedure for reading numbers from serial
int readNum(){
   int result=0;
   char resu;
  while (Serial.available()==0){
  }
    delay(100);
 
    while (Serial.available()){
        resu=Serial.read();
        if((resu!='\r')&&(resu!='\n'))
        result=result*10+(resu-'0');
    }
      return result;  
   Serial.println(result); 
  Serial.flush();
}
