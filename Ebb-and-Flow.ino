
#include <Bounce2.h>
#include <Wire.h>
#include <avr/wdt.h>
#include <LiquidCrystal_I2C.h>
#include <Metro.h>

#define I2C_ADDR      0x27 // I2C address of PCF8574A
#define BACKLIGHT_PIN 3
#define En_pin        2
#define Rw_pin        1
#define Rs_pin        0
#define D4_pin        4
#define D5_pin        5
#define D6_pin        6
#define D7_pin        7

#define low1_pin        9
#define low2_pin        10
#define high1_pin        7
#define high2_pin        8

#define out1PinNumber        5
#define out2PinNumber        6


LiquidCrystal_I2C twilcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin, BACKLIGHT_PIN, POSITIVE);

boolean state = 1;//START OF DRAINING/.
long debounceDelay = 1000;    // the debounce time; increase if the output flickers

Metro countdownTimer = Metro(60000);//1 min timer
Metro displayTimer = Metro(1000);//1 sec 

int EbbFlowIntervalMin = 15; //15min
int CurrentStatusEbbFlowInterval = 0;

Bounce low1PinBouncer = Bounce();
Bounce low2PinBouncer = Bounce();
Bounce high1PinBouncer = Bounce();
Bounce high2PinBouncer =  Bounce();

int low1;
int low2;
int high1;
int high2;

void setup() {

  //input
  pinMode(low1_pin, INPUT);
  low1PinBouncer.attach(low1_pin);
  low1PinBouncer.interval(debounceDelay);
 
  pinMode(low2_pin, INPUT);
  low2PinBouncer.attach(low2_pin);
  low2PinBouncer.interval(debounceDelay);
  
  pinMode(high1_pin, INPUT);
  high1PinBouncer.attach(high1_pin);
  high1PinBouncer.interval(debounceDelay);
  
  pinMode(high2_pin, INPUT);
  high2PinBouncer.attach(high2_pin);
  high2PinBouncer.interval(debounceDelay);

  pinMode(out1PinNumber, OUTPUT);
  pinMode(out2PinNumber, OUTPUT);

  twilcd.begin(20,4);
  twilcd.clear();
  
  watchdogSetup();
}


void loop() {
  
  //15 min mode switch
  if (countdownTimer.check() == 1)
  {
     CurrentStatusEbbFlowInterval += 1;
    
    if (CurrentStatusEbbFlowInterval >= EbbFlowIntervalMin)
    {
      state = !state;
      CurrentStatusEbbFlowInterval = 0;
    }
  }
  
  CheckStatus();

  //update disaply every second
  if (displayTimer.check() == 1)
  {
    Display();
  }

  wdt_reset();
}


void CheckStatus()
{
   low1PinBouncer.update();
   low2PinBouncer.update();
   high1PinBouncer.update();   
   high2PinBouncer.update();
    
   low1 = low1PinBouncer.read();
   low2 = low2PinBouncer.read();
  high1 = high1PinBouncer.read();
  high2 = high2PinBouncer.read();

  if (state)
  {
    digitalWrite(out1PinNumber, 1);
    digitalWrite(out2PinNumber, !(low1 | low2));
  }
  else
  {
    digitalWrite(out2PinNumber, 1);
    digitalWrite(out1PinNumber, (high1 | high2));
  }
}

void Display()
{
    char  DrainingText[20] = "Drain Mode";
    char  FillingText[20] = "Fill Mode";

    twilcd.setCursor(0, 0);
    
    if (state)
    {
        twilcd.print(DrainingText);
    }
    else
    {
      twilcd.print(FillingText);
    }
    
    char Line2[20] = "Min into Phase:";
    char temp[2];
    itoa(CurrentStatusEbbFlowInterval, temp, 10);
    strcat(Line2, temp);  
    twilcd.setCursor(0, 1);
    twilcd.print(Line2);


    char high1switch[1];
    char high2switch[1];
    char Line3[20] = "High Level ";
    itoa(high1, high1switch, 10);
    itoa(high2, high2switch, 10);
    strcat(Line3, high1switch);  
    strcat (Line3, (const char *)'/');
    strcat (Line3, high2switch);
    twilcd.setCursor(0, 2);  
    twilcd.print(Line3);

    char low1switch[1];
    char low2switch[1];
    char Line4[20] = "Low Level ";
    itoa(low1, low1switch, 10);
    itoa(low2, low2switch, 10);
    strcat(Line4, low1switch);  
    strcat (Line4, (const char *)'/');
    strcat (Line4, low2switch);
    twilcd.setCursor(0, 3); 
    twilcd.print(Line3);
}

void watchdogSetup(void)
{
  cli();  // disable all interrupts
  wdt_reset(); // reset the WDT timer
  /*
   WDTCSR configuration:
   WDIE = 1: Interrupt Enable
   WDE = 1 :Reset Enable
   WDP3 = 0 :For 2000ms Time-out
   WDP2 = 1 :For 2000ms Time-out
   WDP1 = 1 :For 2000ms Time-out
   WDP0 = 1 :For 2000ms Time-out
  */
  // Enter Watchdog Configuration mode:
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  // Set Watchdog settings:
   WDTCSR = (0<<WDIE) | (1<<WDE) | (0<<WDP3) | (1<<WDP2) | (1<<WDP1) | (1<<WDP0);
  sei();
}




