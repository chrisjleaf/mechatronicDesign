#include "colorSensor.h"
#include "localize.h"
#include "map.h"
#include "Arduino.h"

#define averagingFactor 4


#define RED_PIN     9
#define GREEN_PIN   10
#define BLUE_PIN    11
#define SENSOR_PIN  A0

typedef struct centroid{ 
  float r; 
  float g; 
  float b;
} 
centroid_t;

void guessColor(confidences_t c, int r, int g, int b);

static struct centroid edge    = {101.0463f,243.8729f,280.8654f};
static struct centroid metal   = {1172.3f,1740.8f,1944.7f};
static struct centroid yellow  = {2274.5f,621.7f,1933.0f};

//static struct centroid white   = { 2239.8f,    2219.5f,   2158.0f    };
//static struct centroid *centroids[4]; //Why buddy!!! Why???

static confidences_t colorConfidence; //Stores updates for color confidence values

confidences_t doColor(void){
  int red = 0, green = 0, blue = 0, ambient = 0;
  int i;

  delay(10);
  for(i = 0; i < averagingFactor; i++){
    ambient += analogRead(SENSOR_PIN);
    delay(10);
  }

  digitalWrite(RED_PIN,HIGH);
  delay(5);
  for(i = 0; i < averagingFactor; i++){
    red += analogRead(SENSOR_PIN);
    delay(10);
  }
  digitalWrite(RED_PIN,LOW);
  red -= ambient;

  digitalWrite(GREEN_PIN,HIGH);
  delay(5);
  for(i = 0; i < averagingFactor; i++){
    green += analogRead(SENSOR_PIN);
    delay(10);
  }
  digitalWrite(GREEN_PIN,LOW);
  green -= ambient;

  digitalWrite(BLUE_PIN,HIGH);
  delay(5);
  for(i = 0; i < averagingFactor; i++){
    blue += analogRead(SENSOR_PIN);
    delay(10);
  }
  digitalWrite(BLUE_PIN,LOW);
  blue -= ambient;

  guessColor(colorConfidence,red,green,blue);

  return colorConfidence;
}

float calcCentDiff(int r, int g, int b, centroid_t cent){
  float score =  (cent.r - r)*(cent.r - r) 
              +  (cent.g - g)*(cent.g - g) 
              +  (cent.b - b)*(cent.b - b);
  return score;
}

void guessColor(confidences_t c, int r, int g, int b){

  c.boundary = calcCentDiff(r,g,b,edge); 
  c.metal    = calcCentDiff(r,g,b,metal); 
  c.yellow   = calcCentDiff(r,g,b,yellow);
}

void calibrateForEachColorCentroid(struct centroid colorCentroid){
  int i,j;
  int red=0,
  green =0,
  blue =0,
  ambient =0;

  for(j=0;j<100;j++){
    delay(50);
    for(i = 0; i < 4; i++){
      ambient += analogRead(SENSOR_PIN);
      delay(10);
    }

    digitalWrite(RED_PIN,HIGH);
    delay(50);
    for(i = 0; i < 4; i++){
      red += analogRead(SENSOR_PIN);
      delay(10);
    }
    digitalWrite(RED_PIN,LOW);
    red -= ambient;

    digitalWrite(GREEN_PIN,HIGH);
    delay(50);
    for(i = 0; i < 4; i++){
      green += analogRead(SENSOR_PIN);
      delay(10);
    }
    digitalWrite(GREEN_PIN,LOW);
    green -= ambient;

    digitalWrite(BLUE_PIN,HIGH);
    delay(50);
    for(i = 0; i < 4; i++){
      blue += analogRead(SENSOR_PIN);
      delay(10);
    }
    digitalWrite(BLUE_PIN,LOW);
    blue -= ambient;
  }
  colorCentroid.r = red/100.f;
  colorCentroid.g = green/100.f;
  colorCentroid.b = blue/100.f;
}

