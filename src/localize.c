#include "localize.h"
#include "math.h"
#include "usart.h"

#define s1x -4
#define s1y -4

#define s2x -4
#define s2y  0

#define s3x -4
#define s3y  4

#define s4x  4
#define s4y -4

#define s5x  4
#define s5y  0

#define s6x  4
#define s6y  4

#define CLEAN_ANGLE(X) ( atan2f(cos(Y

static float compliFilter = 0.f;						//More implies more weight to accel
static struct localizer _storage; 

typedef struct enc{
  long L; 
  long R; 
} enc_t, *Enc;
static enc_t _enc;
static state_t _statePriv;
static state_t _state;


// Derive update, currently uses encoders to get dS and Accel to get theta
static void update(Localizer self){
  static int iters = 0; 
  iters++;
  float dSL, dSR, dS, dTheta;									//Updates from encoders
  
  int newL = self->m->getLeftCount(),						//Get encoder ticks
      newR = self->m->getRightCount();
  
  // Encoder differences										//Inverse Kinematics
  dSL = newL - self->enc->L; 
  dSR = newR - self->enc->R; 

  // Translate to position updates
  dS      =  ENC_TO_D((dSL + dSR) / 2.f);  
  dTheta  = fixAngle((ENC_TO_D(dSR - dSL)) / WHEEL_BASE_WIDTH);

  // Apply Rw = Rw + dRw
  self->_state->vel = dS;

  self->_state->x += dS * cosf(self->_state->theta + dTheta/2);  
  self->_state->y += dS * sinf(self->_state->theta + dTheta/2);  

  if (iters & 0x10){
    iters = 0; 
    // Get accel angle and do complimentary filter
    // This is a delay as accelerometer is much slower
    self->_state->theta = compliFilter*self->acc->getAngle() + 
                    (1-compliFilter)*(self->_state->theta + dTheta);
  } else { 
    self->_state->theta = (self->_state->theta + dTheta);
  }
  
  self->_state->theta = fixAngle(self->_state->theta);
  self->enc->L = newL;
  self->enc->R = newR;
  // TODO Update the Transforms here??
}

static void cacheState(Localizer self){
  self->state->x      = self->_state->x;
  self->state->y      = self->_state->y;
  self->state->theta  = self->_state->theta;
  self->state->vel    = self->_state->vel;
}

static void restart(Localizer self){
  self->_state->x       = 0.f;
  self->_state->y       = 0.f;
  self->_state->theta   = 0.f;//self->acc->getAngle();
  self->_state->vel     = 0.0f;

  self->cacheState(self);
}

sensorPos findSensorLocations(Localizer self){
	float xRobot,yRobot,tRobot;
	
	sensorPos senPositions;
	
	xRobot = self->_state->x;
	yRobot = self->_state->y;
	tRobot = self->_state->theta;
	
	sensorPos.s1.row = (xRobot + s1x*cosf(tRobot) - s1y*sinf(tRobot))/4 + 1;
	sensorPos.s1.col = (yRobot + s1x*sinf(tRobot) + s1y*cosf(tRobot))/4 + 1;	
	
	sensorPos.s2.row = (xRobot + s2x*cosf(tRobot) - s2y*sinf(tRobot))/4 + 1;
	sensorPos.s2.col = (yRobot + s2x*sinf(tRobot) + s2y*cosf(tRobot))/4 + 1;	
	
	sensorPos.s3.row = (xRobot + s3x*cosf(tRobot) - s3y*sinf(tRobot))/4 + 1;
	sensorPos.s3.col = (yRobot + s3y*sinf(tRobot) + s3y*cosf(tRobot))/4 + 1;	
	
	sensorPos.s4.row = (xRobot + s4x*cosf(tRobot) - s4y*sinf(tRobot))/4 + 1;
	sensorPos.s4.col = (yRobot + s4x*sinf(tRobot) + s4y*cosf(tRobot))/4 + 1;	
	
	sensorPos.s5.row = (xRobot + s5x*cosf(tRobot) - s5y*sinf(tRobot))/4 + 1;
	sensorPos.s5.col = (yRobot + s5x*sinf(tRobot) + s5y*cosf(tRobot))/4 + 1;	
	
	sensorPos.s6.row = (xRobot + s6x*cosf(tRobot) - s6y*sinf(tRobot))/4 + 1;
	sensorPos.s6.col = (yRobot + s6x*sinf(tRobot) + s6y*cosf(tRobot))/4 + 1;	
	
}

Localizer createLocalizer(Motors m, Accel acc){
  Localizer l = &_storage;

  // Public state outside ISR
  l->state = &_state;
  // Private state inside ISR
  l->_state = &_statePriv;

  l->enc    = &_enc;
  l->enc->L = m->getLeftCount(); 
  l->enc->R = m->getRightCount();

  l->m    = m;
  l->acc  = acc;

  l->update     = update; 
  l->restart    = restart; 
  l->cacheState = cacheState;
  
  // Initialize state
  l->restart(l);

  return l;
}

