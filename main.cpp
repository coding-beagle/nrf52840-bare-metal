#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// GPIO addresses
#define GPIO_0_BASEADDRESS 0x50000000
#define GPIO_DRIVE_OFFSET 0x504
#define GPIO_PORT_OFFSET 0x300
#define GPIO_READ_REG_OFFSET 0x510
#define GPIO_CONF_OFFSET 0x700

// CLK SRC addresses
#define CLKCTRL_BASEADDRESS 0x40000000
#define LFCLKSRC_OFFSET 0x518
#define LFCLKSTART_OFFSET 0x008
#define RTC0_BASEADDRESS 0x4000B000
#define PRSCL_OFFSET 0x508
#define RTC0_STOP_OFFSET 0x004
#define RTC0_CLEAR_OFFSET 0x008
#define RTC0_COUNTERVAL_OFFSET 0x504

// pin name / pin number / port  
// these are meant for the Seeed Studio Xiao NRF52840 board
#define D0 2, 0
#define D1 3, 0  
#define D2 28, 0
#define D3 29, 0 
#define D4 4, 0  
#define D5 5, 0  
#define D6 11, 1 
#define D7 12, 1 
#define D8 13, 1 
#define D9 14, 1 
#define D10 15, 1 
#define LED_BUILTIN_BLUE 6,0 
#define LED_BUILTIN_GREEN 30,0 
#define LED_BUILTIN_RED 26,0 
#define BUTTON_BUILTIN D1

class DigitalOut{
  int pin, port;
  bool isOn = false;
  public:
    DigitalOut(int pinNumber, int portNumber): pin(pinNumber), port(portNumber){ 
      int* pinConfig = (int *)(GPIO_0_BASEADDRESS + GPIO_CONF_OFFSET+4*pin + GPIO_PORT_OFFSET * port);
      *pinConfig = 0x00000003;   //  Configure pin as output, disconnect input buffer, no pull, standard '0', standard '1', no pin sensing.
    }

    void write(int val){
        int* pinDrive = (int *)(GPIO_0_BASEADDRESS + GPIO_DRIVE_OFFSET + GPIO_PORT_OFFSET*port);
        if(val == 0){
          *pinDrive = (*pinDrive & ~(1 << pin)); // set correct bit via masking
          isOn = false;
        }
        else{
          *pinDrive = (*pinDrive | (1 << pin));
          isOn = true;
        }
    }

    DigitalOut &operator=(int val){
      write(val);
      return *this;
    }
    
    operator bool(){
      return isOn;
    }
    
};

class DigitalIn{
  int pin, port;
  enum pinMode{
    PULLUP, PULLDOWN
  };

  pinMode pinmode = PULLUP;

  public:
    DigitalIn(int pinNumber, int portNumber): pin(pinNumber), port(portNumber){
      int* pinConfig = (int *)(GPIO_0_BASEADDRESS + GPIO_CONF_OFFSET+4*pin + GPIO_PORT_OFFSET * port);
      *pinConfig = 0x0000000c;   //  Configure pin as input, connect input buffer, pull up, standard '0', standard '1', no pin sensing.
    }

  int read(){
    int* pinRead = (int *)(GPIO_0_BASEADDRESS + GPIO_READ_REG_OFFSET + GPIO_PORT_OFFSET*port);  

    int output = ~(*pinRead & (1 << pin)); // mask out every other bit, flip input (to match logic levels)
    output = (output >> pin) & 1;       // bitshift to LSB, mask out everything else
    return output;
  }

  void mode(pinMode pinmode){
    if(pinmode == PULLUP){
      int* pinConfig = (int *)(GPIO_0_BASEADDRESS + GPIO_CONF_OFFSET+4*pin + GPIO_PORT_OFFSET * port);
      *pinConfig = 0x0000000c;
    }
    if(pinmode == PULLDOWN){
      int* pinConfig = (int *)(GPIO_0_BASEADDRESS + GPIO_CONF_OFFSET+4*pin + GPIO_PORT_OFFSET * port);
      *pinConfig = 0x00000004;
    }
  }
  
  operator int(){
    return read();
  } 
};

class LFClkTimer{ // util class for turning on LFClkTimer, not intended to be user facing
  private:
    bool isOn = false;
  public:
    LFClkTimer(){
      int * CLKCONTROL = (int*)(CLKCTRL_BASEADDRESS + LFCLKSRC_OFFSET);
      *CLKCONTROL = 0; // 32.678 kHz RC Oscillator, Disable Bypass of LFCLK, Disable External LFCLK Source
      int * START_CLK = (int*)(CLKCTRL_BASEADDRESS + LFCLKSTART_OFFSET);
      *START_CLK = 1; // START THE CLOCK
      isOn;
    }
};

class MillisCounter{
  public:
    MillisCounter(float period=1000.0){  // period is in ms
      LFClkTimer LFCLKTimer;
      changePeriod(period);
      reset();
    }

    void changePeriod(float frequency){ // change the period of the timer by setting prescalar of RTC0
      if(frequency != 0){
        float f_prescalar = round(32768 / frequency)- 1;
        int prescalar = (int)(f_prescalar);
        prescalar = (prescalar & 0xFFF); // mask out 12 LSBs
        printf("Prescalar = %d\n", prescalar);
        int * setPrescalar = (int *)(RTC0_BASEADDRESS + PRSCL_OFFSET);
        *setPrescalar = (prescalar);
      }
    }

    void begin(){
      int* RTC0Start = (int *)(RTC0_BASEADDRESS); // base address is used for starting
      *RTC0Start = 1;
    }

    void end(){
      int* RTC0Stop = (int *)(RTC0_BASEADDRESS + RTC0_STOP_OFFSET);
      *RTC0Stop = 1;
    }

    void reset(){
      int* RTC0Clear = (int *)(RTC0_BASEADDRESS + RTC0_CLEAR_OFFSET);
      *RTC0Clear = 1;
    }

    int getVal(){
      int* RTC0CounterReg = (int*)(RTC0_BASEADDRESS + RTC0_COUNTERVAL_OFFSET);
      return *RTC0CounterReg;
    }

    operator int(){
      return getVal();
    }
};

// example usage
// this code utilises the builtin LEDs and the button on the Seeed Studio expansion board
int main(){
  DigitalOut BLUELED(LED_BUILTIN_BLUE);
  DigitalOut GREENLED(LED_BUILTIN_GREEN);

  MillisCounter MILLIS;   // millisCounter updates every 1 ms, similar to arduino Millis
  MILLIS.begin();         // user has to explicitly start millis timer

  int previousOffTime = 0, togglePeriod = 500; // in ms

  DigitalIn Button(BUTTON_BUILTIN);
  
  BLUELED = 1;
  int currentMillis = 0;

  while(1){
    
    currentMillis = MILLIS;

    if(Button == 1){
      GREENLED = 1;
    }
    else{GREENLED = 0;}
    
    if(currentMillis - previousOffTime >= togglePeriod){  // todo automate this syntax
      BLUELED = !BLUELED;
      previousOffTime = currentMillis;
    }

  }

  return 0;
}
