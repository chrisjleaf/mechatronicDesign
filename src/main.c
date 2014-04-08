#include "main.h"
#include "misc.h"
#include <stdio.h>
#include "math.h"

volatile int running = 0;

ColorSensors colorSensors; 
Accel accel; 
Motors motors; 

Localizer localizer;

state_t _targState = {10.f,10.f,PI/2,0.f}; 
State targState;

Pid pid; 
PID_Gains angleGains  = { 0.0f, 0.0f, 0.0f },
          distGains   = { 0.01f, 0.0f, 0.0f },
          bearGains   = { 0.0f, 0.0f, 0.0f };
static void init(void);
static void loop(void);

void start(void){
  localizer->restart(localizer);
  running = 1;
}
void halt(void){
  running = 0;
  motors->setSpeeds(motors, 0,0);
}

int main(void) {
  delay(500); // Give the hardware time to warm up on cold start
  init();
  start();
  do {
    if(running)
      loop();
  } while (1);
}

static void init() {
  targState = &_targState;
  init_USART(); 
  initLEDs();
  colorSensors = createColorSensors(); 
  accel   = initAccel(); 
  motors  = createMotors(); 
  delay(500);
  localizer = createLocalizer(motors, accel);
  pid = createPID(distGains, bearGains,angleGains, motors); 
  initSysTick(); 
  //USART_puts("Init finished \r\n");
}

int doColor(Color c){
  colorSensors->measureColor(colorSensors,c); 
  while(colorSensors->done < COLOR_SENSOR_ITERS); 
  volatile uint16_t* res = colorSensors->getResult(); 
  return res[0];
}

void doLog(void){
  USART_putInt(motors->getLeftCount());
  USART_puts("\t");
  USART_putInt(motors->getRightCount());
  USART_puts("\n\r");

  USART_puts("x, y: ");
  USART_putFloat(localizer->state->x);
  USART_puts("\t");
  USART_putFloat(localizer->state->y);
  USART_puts("\n\r");
   
  USART_puts("Angle: ");
  USART_putFloat(localizer->state->theta);
  USART_puts("\n\r");
  USART_puts("Vel: ");
  USART_putFloat(localizer->state->vel);
  USART_puts("\n\r");
}

void doColors(void){
  int n,r,g,b,min;
  n = doColor(NONE);
  r = doColor(RED)   - n;
  g = doColor(GREEN) - n;
  b = doColor(BLUE)  - n;

  colorSensors->guessColor(r,g,b);

  
  USART_putInt(n);
  USART_puts("\t");
  USART_putInt(r);
  USART_puts("\t");
  USART_putInt(g);
  USART_puts("\t");
  USART_putInt(b);
  USART_puts("\n\r");
  
}

/*void doPID(void){
  pid->loop(pid, targState, localizer->state);
}*/

void doCalibrateColors(){
  colorSensors->calibrateColor(colorSensors, NONE);
  colorSensors->calibrateColor(colorSensors, RED);
  colorSensors->calibrateColor(colorSensors, GREEN);
  colorSensors->calibrateColor(colorSensors, BLUE);
}

void doUpdateState(void){
  __disable_irq();
  localizer->cacheState(localizer);
  __enable_irq();
}

void loop(void) {
  static int i = 0; 
  //doUpdateState();
  //doCalibrateColors();
  //doColors();
  doLog();
  delay(200);
  if(i++ & 0x1)
    enableLEDs(BLUE);
  else 
    disableLEDs(BLUE);
}

void tick_loop(void){
  localizer->update(localizer);
  pid->loop(pid, targState, localizer->state);
}
