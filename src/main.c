#include "main.h"
#include "misc.h"
#include <stdio.h>
#include "math.h"
#include "state.h"

typedef enum { NOP, FORWARD, LEFT } opType;

typedef struct {
  opType command; 
  float  argument; 
  int    delay;  
} operation; 

operation Commands[] = { 
  { NOP,      0.f,   1000 },
  { FORWARD, 22.f,  1000 },  // Right 
  { LEFT,     5.f,   1000 },
  { FORWARD, 20.f,   1000 },  // Up
  { LEFT,     5.f,   1000 },  
  { FORWARD, 24.f,   1000 },  // Left
  { LEFT,     0.f,   3000 },
  { FORWARD, 7.0f,    500 },  // Down
  { LEFT,     0.f,   2000 },
  { FORWARD, 12.f,   1000 },  // Right
  { NOP,      0.f,   1000 },
};

const int numCommands = sizeof(Commands)/sizeof(operation); 
int currentCommandIndex = 0; 
int timeout = 0; 
volatile int running = 0;
volatile int ready   = 0;
int motionDone = 0; 

int calibrateColor = 0; 
int sendMap = 0;
ColorSensors colorSensors; 
Accel accel; 
Motors motors; 

Pid pid; 
PID_Gains angleGains  = { 35.f, 0.00f, 0.0f },
          distGains   = { 40.f, 0.00f, 0.0f },
          bearGains   = { -20.f, 0.00f, 0.0f },
          motorGains  = { 100.f, 0.00f, 50.0f };

static void init(void);
static void loop(void);

void doUpdateState(void){
  __disable_irq();
  
  localizer->cacheState(localizer);
  
  USART_puts("[");
  USART_putFloat(localizer->state->x);
  USART_puts(", ");
  USART_putFloat(localizer->state->y);
  USART_puts(", ");
  USART_putFloat(localizer->state->theta);
  USART_puts("]\n");
  
  __enable_irq();
}

long time;
void start(void){
  running = 1;
  USART_puts("Start!\n");
  colorSensors->startColor(NONE);
}

void halt(void){
  running = 0;
  colorSensors->halt();
}

void doLog();

void doColorCalibrate(void){
  colorSensors->calibrateColors(colorSensors); 
}

void startCommand(int index){
  switch(Commands[index].command){
    case FORWARD:
      goForwardBy(Commands[index].argument);
      break;
    case LEFT:
      turnLeft90();
      break;
    case NOP:
    default: 
      doStall();
      break;
  }
}
void endCommand(int index){
  delay(Commands[index].delay);
}

int main(void) {
  initSysTick(); 
  delay_blocking(500); // Give the hardware time to warm up on cold start
  init();
  delay(1000); 
  startCommand(currentCommandIndex);
  //start();
  do {
    doUpdateState();
    if(calibrateColor){
      doColorCalibrate();
      calibrateColor = 0; 
    }
    if( (isMotionComplete() && !motionDone ) ){
      endCommand(currentCommandIndex++);
      motionDone = 1; 
      if (currentCommandIndex < numCommands){ 
        startCommand(currentCommandIndex);
        motionDone = 0;
      }
    }
    if(running){
      loop();
    }
    delay(500);
  } while (1);
}

void setCalibrateColor(void){
  calibrateColor = 1; 
}

static void init() {
  initLEDs();
  init_USART(); 
  createGrid();
  colorSensors = createColorSensors(); 
  accel   = initAccel(); 
  motors  = createMotors();
  delay_blocking(500);
  localizer = createLocalizer(motors, accel);
  startState();
  ready = 1; 
  USART_puts("Init finished\n");
}

void doLog(void){
  USART_putInt(motors->getLeftCount());
  USART_puts("\t");
  USART_putInt(motors->getRightCount());
  USART_puts("\n");

  USART_puts("x, y: ");
  USART_putFloat(localizer->state->x);
  USART_puts("\t");
  USART_putFloat(localizer->state->y);
  USART_puts("\n");
   
  USART_puts("Angle: ");
  USART_putFloat(localizer->state->theta);
  USART_puts("\n");
  USART_puts("Vel: ");
  USART_putFloat(localizer->state->vel);
  USART_puts("\n");
}

void loop(void) {
  static int i = 1; 
  //doLog();

  if(i % 30){
    //sendGuesses();
  }

  if(i++ & 0x1)
    enableLEDs(BLUE);
  else 
    disableLEDs(BLUE);
}

void tick_loop(void){
  static int loopCount = 0;
  localizer->update(localizer);
  motors->updateOffset(motors, localizer->_state->theta);
  if(running){
    doMotion(); 
  /*
  if(loopCount == 0){
    localizer->update(localizer);
    pid->loop(pid, targState, localizer->_state);
    loopCount = 10; 
  }
  motors->doMotorPID(motors); 
  loopCount--;
  */
  }
  else{
    motors->setOffset(motors, PWM_MIN_L, PWM_MIN_R);
    motors->setSpeeds(motors, 0,0);
  }
}
