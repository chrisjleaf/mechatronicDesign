#ifndef _STATE_H_
#define _STATE_H_

extern Localizer localizer;
extern State targState;

typedef struct {
  float l; 
  float r; 
} MotorSpeeds; 
extern MotorSpeeds *speeds;
extern float AGain;

void startState(void);
void doStall(void);
void goForwardBy(float dist);
void turnLeft90(void);
void doMotion(void);
int isMotionComplete(void);
#endif //_STATE_H_
