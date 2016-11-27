#include "SOMO.h"
#include "Clock.h"
#include "chronodot.h"

int temperature;
bool isSleeping =false;

static const int panel = D2;
static const int rotary  = D6;
static const int spinning = D7;
static const int pullchain = A0;
static const int to_Moteino = D5;
static const int from_Moteino = A2;
static const int steps[4] = {50,150,250,350};

bool chainState = false;
byte panelMax = 220;
byte valIndex = 0;

bool state = 1;
bool oldstate = 1;
int count = 0;
bool flag = false;
long since = 0;

chronodot rtc;
bool isDots = false;

Clock master;   //hours,minutes,temperature
Clock timer;    //hours,minutes,seconds
Clock alarm;    //hours,minutes,seconds
Clock chrono;   //hours,minutes,seconds
Clock calendar; //day, month, year

bool isSettingTime = false;
bool isInputting = false;
bool timerRunning = false;
bool alarmSet = false;
byte secondsTick;
String summary;
String alarmContents = "";
String tomString = "";
String todString = "";

SOMO sound;
int masterVolume = 0;

void setup(){
    RGB.control(true);  //take control of photon's onboard LED
    RGB.color(0,0,0);   //turn off LED
    pinMode(panel,OUTPUT);
    masterVolume = 15;
    analogWrite(panel,map(masterVolume,0,30,0,panelMax));
    sound.send(sound.volume,0,masterVolume);
    pinMode(rotary,INPUT);
    pinMode(spinning,INPUT);
    pinMode(pullchain,INPUT);
    pinMode(to_Moteino,OUTPUT);
    digitalWrite(to_Moteino,LOW);
    pinMode(from_Moteino,INPUT);

    Wire.begin();
    Serial.begin(115200);

    isDots = master.toggleDots(isDots);//turn dots on.
    Serial.println("HELLO!");
    chainState = digitalRead(pullchain);

    //sync RTC time and calendar with Internet time and calendar
    Time.zone(-5); //eastern time
    delay(100);
    rtc.writeClock(0x00,Time.hour(),Time.minute(),Time.second());
    calendar.set(Time.month(),Time.day(),Time.year()-2000);

    Particle.subscribe("hook-response/darksky_webhook", gotWeatherData, MY_DEVICES);

    Particle.function("clock",webClockSet);
    Particle.function("alarm",webAlarmSet);
    Particle.function("dial",webDial);
    Particle.function("sound",webSound);
    Particle.function("light",webLight);

    Particle.variable("alarmStatus",alarmContents);
    Particle.variable("today", todString);
    Particle.variable("tomorrow", tomString);

    Particle.publish("adafruit_webhook", "Setup complete", 60, PRIVATE);
    Particle.publish("darksky_webhook");
}

void loop(){
    rtc.getTime(0,&master);
    if (rtc.alarmStatus()==1){
        String msg = "Flag " + String(alarm.first) + ':' + String(alarm.second)+ ':' + String(alarm.third);
        Particle.publish("adafruit_webhook", msg, 60, PRIVATE);
        if (alarmSet){
            Particle.publish("adafruit_webhook", "playing alarm", 60, PRIVATE);
            dialCommand(1);
            isSleeping = false;
            master.displayAlt();
            delay(30000); //turn on lights 30 secs after alarm starts playing
            toggleLight(1);
        }
        Serial.print("alarm register:");
        Serial.print(rtc.alarmStatus());
        Serial.print(",");
        Serial.println(rtc.alarmStatus(),BIN);
         // clear status register
        Wire.beginTransmission(0x68);
        Wire.write(0x0F);
        Wire.write(0); //clears alarm1 flag
        Wire.endTransmission();
        Serial.print("alarm register cleared:");
        Serial.print(rtc.alarmStatus());
        Serial.print(",");
        Serial.println(rtc.alarmStatus(),BIN);
    }
    if (master.second%10==0 && master.third==0){ //update temperature every 10 minutes
        Particle.publish("darksky_webhook");
        delay(1000);
    }
    if (!master.isAfternoon && master.first==4 && master.second==0 && master.third==0){ //turn on clock at 4 a.m.
        isSleeping = false;
    }
    if (isSettingTime){
        for (int i = 0; i<6;i++ ){
            timer.digits[i] = 10;
            timer.display(timer.digits[i],i+1);
        }
        while (1){
            int newDigit = readDial();
            if (newDigit==-2){ //timeout
                isSettingTime = false;
                break;
            }
            else if (newDigit==-1){ //chain pulled
                for (int i=0; i<6;i++){
                    if (timer.digits[i]==10){
                        timer.digits[i] = 0;
                    }
                }
                timer.set();
                isSettingTime = false;
                timerRunning = true;
                rtc.getTime(0,&master);
                secondsTick = master.third;
                break;
            }
            else{   //update digits microwave input style
                for (int i = 1; i<6;i++ ){
                    timer.display(timer.digits[i],i);
                }
                timer.display(newDigit,6);
            }
        }
    }
    else if (timerRunning==1){
        if (secondsTick!=master.third){//second has passed
            timer.display();
            secondsTick = master.third;
            if (timer.third==0){
                timer.third = 60;
                if (timer.second==0){
                    timer.second = 60;
                    if (timer.first==0){
                        sound.send(sound.play);
                        timerRunning = false;
                    }
                    else{
                        timer.first--;
                    }
                }
                else{
                    timer.second--;
                }
            }
            else{
                timer.third--;
            }
        }
    }
    else if (!isSleeping && digitalRead(from_Moteino)) {
        // master.display(master.third%10);
        master.displayAlt();
    }
    else{
        master.clearDisplay();
    }
    if (digitalRead(rotary)==0){
        dialCommand(readDial());
        count = 0;
    }
    if (digitalRead(pullchain)!=chainState){
        dialCommand(8);
        delay(100);
        chainState = !chainState;
    }
}

int readDial(){
    count = 0;
    int startTime = millis();
    int timeout = 10000;
    while(millis()-startTime <timeout){
        if (digitalRead(pullchain)!=chainState){
            delay(100);
            chainState = !chainState;
            isInputting = false;
            return -1;
        }
        oldstate = state;
        state = digitalRead(rotary);
        if(state==1 && oldstate==0){    //if the MCU reads that the phone contact just went from closed to open then the dial is in the process of spinning
            count++;
            Serial.print("count:");
            Serial.println(count);
            since = millis();
            flag = true;                //create a flag so later on we can check to see if sufficient time has passed to confirm that the dial is done spinning
            delay(100);
        }
        else if (millis()-since>150 & flag){ // on my dial there is 120 millisecond delay between contacts, if it's been longer than 150, the dial we know the dial is done spinning
            if (count==10){ // 0 is the tenth spot on the rotary dial so if we counted 10, 0 was the digit actually dialed
              count=0;
            }
            flag = false;
            return count;
        }
    }
    return -2;
}

int webClockSet(String command) {
    char  usableCommand[8];
    command.toCharArray(usableCommand, 8);
    byte tempHour = (usableCommand[0]-48)*10 + usableCommand[1]-48;
    byte tempMin = (usableCommand[3]-48)*10 + usableCommand[4]-48;
    rtc.writeClock(0x00,tempHour, tempMin,Time.second());
    return 1;
}

int webAlarmSet(String command) {
    char  usableCommand[8];
    command.toCharArray(usableCommand, 8);
    byte tempHour = (usableCommand[0]-48)*10 + usableCommand[1]-48;
    byte tempMin = (usableCommand[3]-48)*10 + usableCommand[4]-48;
    rtc.writeClock(0x07,tempHour, tempMin,0);
    alarmContentUpdate();
    alarm.display();
    delay(2500);
    return 2;
}

int webDial(String fromWeb){
    char  usableCommand[8];
    fromWeb.toCharArray(usableCommand, 8);
    dialCommand(usableCommand[0]-48);
    return usableCommand[0]-48;
}

int webSound(String fromWeb){
    char  usableCommand[8];
    fromWeb.toCharArray(usableCommand, 8);
    sound.send(sound.track,1,usableCommand[0]-48);
    return 0;
}

int webLight(String fromWeb){
    char  usableCommand[8];
    fromWeb.toCharArray(usableCommand, 8);
    toggleLight(usableCommand[0]-48);
    return 0;
}

void toggleLight(int light){
    digitalWrite(to_Moteino,HIGH);
    delay(50+100*light);
    digitalWrite(to_Moteino,LOW);
}

void gotWeatherData(const char *name, const char *data) {
    String str = String(data);
    char strBuffer[200] = "";
    str.toCharArray(strBuffer, 200);
    Particle.publish("adafruit_webhook", strBuffer, 60, PRIVATE);

    temperature     = atoi(strtok(strBuffer, "~"));
    summary         = strtok(NULL, "~");
    todString       = strtok(NULL, "~");
    todString       = todString + '\n' + strtok(NULL, "~");
    todString       = todString + " - " + strtok(NULL, "~");
    tomString       = strtok(NULL, "~");
    tomString       = tomString + '\n' + strtok(NULL, "~");
    tomString       = tomString + " - " + strtok(NULL, "~");

    master.aux = temperature;
}

void dialCommand(byte dialed){
    switch (dialed){
        case 0:
            isDots = master.toggleDots(isDots);
            calendar.display();
            delay(3000);
            isDots = master.toggleDots(isDots);
            break;
        case 1:
            sound.send(sound.track,1,1);
            break;
        case 2:
            sound.send(sound.stop);
            alarmSet = false;
            alarmContentUpdate();
            break;
        case 3:
            sound.send(sound.stop);
            alarmSet = true;
            alarmContentUpdate();
            alarm.display();
            delay(1000);
            break;
        case 4:
            alarmSet = false;
            alarmContentUpdate();
            break;
        case 5:
            volumeChange(-5);
            break;
        case 6:
            volumeChange(5);
            break;
        case 7:
            sound.send(sound.next);
            delay(100);
            sound.send(sound.next);
            break;
        case 8:
            isSleeping = !isSleeping;
            break;
        case 9:
            isSettingTime = true; break;
    }
}

void alarmContentUpdate(){
    rtc.getTime(0x07,&alarm);
    String status = (alarmSet==1)?"Alarm is ON":"Alarm is OFF" ;
    String timeOfDay = (alarm.isAfternoon==1)? " pm ":" am ";
    alarmContents = String(alarm.first) + ':' + String(alarm.second) + timeOfDay + status;
    Particle.publish("adafruit_webhook", alarmContents, 60, PRIVATE);
}

void volumeChange(int change){
    int tempVol = masterVolume;
    masterVolume += change;
    if (masterVolume>30){
        masterVolume = 30;
    }
    if(masterVolume<0){
        masterVolume = 0;
    }
    //gradually increase/decrease volume to new volume
    for (int i = tempVol; i!=masterVolume ; i+= (masterVolume-tempVol)/abs(masterVolume-tempVol)){
      sound.send(sound.volume,0,i);
      analogWrite(panel,map(i,0,30,0,panelMax));
      delay(100);
    }
    sound.send(sound.volume,0,masterVolume);
    analogWrite(panel,map(masterVolume,0,30,0,panelMax));
}
