/*
 * Copyright (c) 2015 seeed technology inc.
 * Website    : www.seeed.cc
 * Author     : Jiankai.li @2017/2/27
 * Modified Time: July 2015
 * Description: This is wireless control  light Master Side
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <string.h>
#include <IRSendRev.h>
#include <EEPROM.h>
#include <avr/wdt.h>
#include <TimerOne.h>
#include <SoftwareSerial.h>

#include <Wire.h>
#include <SeeedOLED.h>
#include <avr/pgmspace.h>
#include "logo.h"


#define debug   1

// the value of the data frame
// Head  Addr  Command  End
#define HEAD             0xAA
#define END              0x55

#define BoardCastAddress 0xff

#define Switch_PIN       2
#define ModeUP_PIN       7
#define ModeDown_PIN     8

const int pinRecv = 6;

#define LEDCommandNum    24
enum LEDstatus {
    led_on            = 0,
    led_off           = 1,
    led_down          = 2,
    led_up            = 3,
    led_red           = 4,
    led_green         = 5,
    led_blue          = 6,
    led_white         = 7,
    led_lightSalmon   = 8,
    led_orangeRed     = 9,
    led_orange        = 10,
    led_yellow        = 11,
    led_paleGreen     = 12,
    led_skyBlue       = 13,
    led_turquoise     = 14,
    led_royalBlue     = 15,
    led_steelBlue     = 16,
    led_purple        = 17,
    led_rebeccapurple = 18,
    led_orchid        = 19,
    led_flash         = 20,
    led_strobe        = 21,
    led_fade          = 22,
    led_smooth        = 23,
    
};

typedef enum LEDstatus ledstatus_t;
ledstatus_t eLEDStatus;


enum Workstatus {
    light_off           = 0,
    light_IRremote      = 1,
    light_write         = 2,
    light_waterflow     = 3,
    light_random        = 4,
    light_gesture       = 5,
    light_reserved      = 6,
    light_on            = 7,
    
};

typedef enum Workstatus Workstatus_t;
Workstatus_t eWorkstatus;
Workstatus_t last_eWorkstatus;


// // store the univerisal data
// unsigned char CONST_DATA[8] = {9,181,89,11,33,4,0,255};

// store the comand data
unsigned char COMMAND_DATA[24][2] = {
    {224,31},  // led_on
    {96,159},  // led_off
    {32,223},  // led_down
    {160,95},  // led_up
    {144,111}, // led_red
    {16,239},  // led_green
    {80,175},  // led_blue
    {208,47},  // led_white
    {176,79},  // led_lightSalmon
    {168,87},  // led_orangeRed
    {152,103}, // led_orange
    {136,119}, // led_yellow
    {48,207},  // led_paleGreen
    {40,215},  // led_skyBlue
    {24,231},  // led_turquoise
    {8,247},   // led_royalBlue
    {112,143}, // led_steelBlue
    {104,151}, // led_purple
    {88,167},  // led_rebeccapurple
    {72,183},  // led_orchid
    {240,15},  // led_flash
    {232,23},  // led_strobe
    {216,39},  // led_fade
    {200,55},  // led_smooth
};


SoftwareSerial mySerial(4,5); // RX, TX

uint8_t dtaRecv[20];
uint8_t dtaSend[4];
                                                             // ir send connect to D3
                                  
uint8_t Status,MyAddress,cmdStatus;                          // The command status, Address, and temp Command
                               
                                    
void setup()
{
    Serial.begin(9600);
    
    mySerial.begin(9600);                                    // This depend on the wireless module
    
    eWorkstatus = light_reserved;
    
    last_eWorkstatus = eWorkstatus;
    
    wdt_enable(WDTO_2S);
    Timer1.initialize(200000);                               // set a timer of length 1000000 microseconds 1 second
    Timer1.attachInterrupt( timerIsrFeedFog );               // attach the service routine here
    wdt_reset();
    pinMode(Switch_PIN, INPUT_PULLUP);
    pinMode(ModeUP_PIN,INPUT_PULLUP);
    pinMode(ModeDown_PIN,INPUT_PULLUP);
    dtaSend[0] = HEAD;
    dtaSend[1] = BoardCastAddress;
    dtaSend[3] = END;
    
    IR.Init(pinRecv);
    // init the oled
    Wire.begin();	
    SeeedOled.init();                                        //initialze SEEED OLED display
    SeeedOled.clearDisplay();                                // clear the screen and set start position to top left corner
    SeeedOled.deactivateScroll(); 
    SeeedOled.drawBitmap((unsigned char*) SeeedLogo,1024);   // 1024 = 128 Pixels * 64 Pixels / 8
    // SeeedOled.drawBitmap((unsigned char*) SeeedOFF,1024);   // 1024 = 128 Pixels * 64 Pixels / 8
    Serial.println("System init ");
    
}

void loop()
{
    
    IREvent(); 
    // Do the action here
    switch (eWorkstatus) {
       
    case light_off:
        break;
    case light_on:
        break;        
    case light_IRremote:  // Mode 1
        break;
    case light_write:     // Mode 2
        lightWrite((uint8_t)light_write,250);
        break;
    case light_waterflow: // Mode 3
        sendCommand(BoardCastAddress,led_on);
        lightWrite((uint8_t)light_waterflow,250);
        break;
    case light_random:
        sendCommand(BoardCastAddress,led_on);
        lightRandom(500);
        break;
    default:
        break;
    }
    
    // Update the display
    IfButtonClick();
    if (last_eWorkstatus != eWorkstatus) {
        displayStatus(eWorkstatus);
        last_eWorkstatus = eWorkstatus;
    }
}

// Send the data to the slave part
void sendData() {
    for (int j=0; j<4; j++) {
        mySerial.write(dtaSend[j]);
    }
}

// Send the command 
void sendCommand(uint8_t addr,uint8_t command) {
    dtaSend[1] = addr;
    dtaSend[2] = command;
    sendData();
}

void lightWrite(uint8_t flag, uint8_t delay) {
    static unsigned long currentMillis ;
    static unsigned long previousMillis = 0;
    uint8_t interval = delay;  
    for (int i=1; i<=78;) {
        currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;
            // Serial.println("================previousMillis==================");
            // Serial.println(previousMillis);
            if(flag == (uint8_t)light_write) {
                if(i>4) {
                    sendCommand(i-4,led_off);
                    sendCommand(i,led_on);
                } else {
                    sendCommand(i,led_on);
                }
                i++;
            } else if(flag == (uint8_t)light_waterflow) {
                sendCommand(i,led_fade);
                i++;
            }
        }
        if (IfButtonClick() || IREvent()) {
            Serial.println("Break the light write");
            break;
        }
    }
}

void lightRandom(uint16_t delay) {
    static unsigned long currentMillis ;
    static unsigned long previousMillis = 0;
    boolean flag = false;
    uint16_t interval = delay;  
    for (int i=1; i<=5;) {
        currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;
            for(uint8_t j=1; j<=75; j++) {
                sendCommand(j,random(4,23));
                // Serial.println("Change the light");
                if (IfButtonClick() || IREvent()) {
                    // Serial.println("Break the light write");
                    flag = true;
                    break;
                }
            }
            i++;
        }
        if (IfButtonClick() || IREvent() || flag) {
            Serial.println("Break the light write");
            break;
        }
    }
}

// This is the function to judge if the Mode control button is pressed.
uint8_t IfButtonClick() {
    static uint8_t lastButtonState =  HIGH;
    static uint8_t buttonState = HIGH;
    static uint8_t lastButtonState2 =  HIGH;
    static uint8_t buttonState2 = HIGH;
    
    static uint8_t lastButtonState3 =  HIGH;
    static uint8_t buttonState3 = HIGH;
    static unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
    static unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
    static boolean flag = false;
 
    uint8_t reading = digitalRead(ModeUP_PIN);

    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != buttonState) {
            buttonState = reading;
            if (buttonState == HIGH) {
                Serial.println("The Button up Pressed");
                uint8_t status = (uint8_t)eWorkstatus;
                if(++status >= (uint8_t)light_random ) {
                    eWorkstatus = light_random;
                } else {
                    eWorkstatus = (Workstatus_t)status;
                }
                return 1;
            }
        }
    }
    
    uint8_t reading2 = digitalRead(ModeDown_PIN);
    if (reading2 != lastButtonState2) {
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading2 != buttonState2) {
            buttonState2 = reading2;
            if (buttonState2 == HIGH) {
                Serial.println("The Button DOWN Pressed");
                uint8_t status = (uint8_t)eWorkstatus;
                if (status >= light_random) {
                    status = light_random;
                }
                if(--status <= (uint8_t)light_IRremote ) {
                    eWorkstatus = light_IRremote;
                } else {
                    eWorkstatus = (Workstatus_t)status;
                }
                return 2;
            }
        }
    }
    
    uint8_t reading3 = digitalRead(Switch_PIN);
    if (reading3 != lastButtonState3) {
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading3 != buttonState3) {
            buttonState3 = reading3;
            if (buttonState3 == HIGH) {
                Serial.println("The Button Switch Pressed");
                if(flag == true) {
                    flag = false;
                    eWorkstatus = light_off;
                    sendCommand(BoardCastAddress,led_off);
                    sendCommand(BoardCastAddress,led_off);
                } else {
                    flag = true;
                    eWorkstatus = light_on;
                    sendCommand(BoardCastAddress,led_on);
                    sendCommand(BoardCastAddress,led_on);
                }
                return 3;
            }
        }
    }
    lastButtonState3 = reading3;
    lastButtonState2 = reading2;
    lastButtonState = reading;
    return 0;
}

// This is the display function.
void displayStatus(Workstatus_t status) {
    switch(status) {
    case light_off:
        SeeedOled.clearDisplay();
        SeeedOled.deactivateScroll(); 
        SeeedOled.drawBitmap((unsigned char*) SeeedOFF,1024);   // 1024 = 128 Pixels * 64 Pixels / 8
        break;        
    case light_on:
        SeeedOled.clearDisplay();
        SeeedOled.deactivateScroll(); 
        SeeedOled.drawBitmap((unsigned char*) SeeedON,1024);   // 1024 = 128 Pixels * 64 Pixels / 8
        break;    
    case light_IRremote:
        SeeedOled.clearDisplay();
        SeeedOled.deactivateScroll(); 
        SeeedOled.drawBitmap((unsigned char*) mode1,1024);   // 1024 = 128 Pixels * 64 Pixels / 8
        SeeedOled.setTextXY(7,0);          //Set the cursor to Xth Page, Yth Column 
        SeeedOled.setPageMode();
        SeeedOled.putString("Controller mode"); //Print the String
        SeeedOled.setHorizontalScrollProperties(Scroll_Left,7,7,Scroll_5Frames); //Set Scrolling properties to Scroll Left
        SeeedOled.activateScroll();   
        break;
    case light_write:
        SeeedOled.clearDisplay();  
        SeeedOled.deactivateScroll(); 
        SeeedOled.drawBitmap((unsigned char*) mode2,1024);   // 1024 = 128 Pixels * 64 Pixels / 8
        
        SeeedOled.setTextXY(7,0);          //Set the cursor to Xth Page, Yth Column 
        SeeedOled.setPageMode();
        SeeedOled.putString("Write mode"); //Print the String
        SeeedOled.setHorizontalScrollProperties(Scroll_Left,7,7,Scroll_5Frames); //Set Scrolling properties to Scroll Left
        SeeedOled.activateScroll();   
        break;    
    case light_waterflow:
        SeeedOled.clearDisplay(); 
        SeeedOled.deactivateScroll(); 
        SeeedOled.drawBitmap((unsigned char*) mode3,1024);   // 1024 = 128 Pixels * 64 Pixels / 8
        SeeedOled.setTextXY(7,0);          //Set the cursor to Xth Page, Yth Column 
        SeeedOled.setPageMode();
        SeeedOled.putString("WaterFlow mode"); //Print the String
        SeeedOled.setHorizontalScrollProperties(Scroll_Left,7,7,Scroll_5Frames); //Set Scrolling properties to Scroll Left
        SeeedOled.activateScroll();   
        break;    
    case light_random:
        SeeedOled.clearDisplay(); 
        SeeedOled.deactivateScroll(); 
        SeeedOled.drawBitmap((unsigned char*) mode4,1024);   // 1024 = 128 Pixels * 64 Pixels / 8
        SeeedOled.setTextXY(7,0);          //Set the cursor to Xth Page, Yth Column 
        SeeedOled.setPageMode();
        SeeedOled.putString("Random mode"); //Print the String
        SeeedOled.setHorizontalScrollProperties(Scroll_Left,7,7,Scroll_5Frames); //Set Scrolling properties to Scroll Left
        SeeedOled.activateScroll();   
        break;    
    default:
        SeeedOled.clearDisplay();
        SeeedOled.deactivateScroll(); 
        SeeedOled.drawBitmap((unsigned char*) SeeedLogo,1024);   // 1024 = 128 Pixels * 64 Pixels / 8
        break;
    }
    
}


// This is used for wireless communication
// The Frame data is 
// Head  Address  command  End
// 0xAA  0xFF     0xFF     0x55


void serialEvent() {
    while (Serial.available()) { 
        mySerial.write(Serial.read());
    }    
}

// This is used for IR control all of the light

uint8_t IREvent() {
    if (IR.IsDta()) {
        IR.Recv(dtaRecv);
        #if debug
        for (int j=8; j<10;j++) {
             Serial.print(dtaRecv[j]);
        }
        Serial.println("==========Recive the IR=========");
        #endif
        for (int i=0; i<24;i++) {
            if (memcmp(COMMAND_DATA[i],&dtaRecv[8],2) == 0) {
                Serial.print("Recive the Command index is: ");
                Serial.println(i);
                sendCommand(BoardCastAddress,i);
                sendCommand(BoardCastAddress,i);
                i==1?eWorkstatus = light_reserved:eWorkstatus = light_IRremote;
                return 1;
            }
             
        }
    }
    return 0;
}

void timerIsrFeedFog()
{   
    wdt_reset();
}
