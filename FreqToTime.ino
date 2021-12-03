// Simple DAC sine wave test on Teensy 3.1

#define ARM_MATH_CM4
#define TWOPI 3.14159 * 2
#define ANGULARMOM TWOPI / (1000000.0)

#include <arm_math.h>

float phase = 0.0;

elapsedMicros usec = 0;

float freqVect[] = {440.0, 554.0, 659.0, 880.0, 1108.0, 1318.0}; 
float magsVect[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};


float evaluateAtT(float freqs[], float mags[], int dim, float t) {
  float returnVal = 0;
  float magTracker = 0;

  for (int i = 0; i < dim; i++) {
    returnVal += mags[i] * sin(freqs[i] * ANGULARMOM * t);
    magTracker += mags[i] * mags[i];
  }

  float magRoot = 0;
  arm_sqrt_f32(magTracker, &magRoot);

  return returnVal / magRoot;
}


void setup() {
  analogWriteResolution(12);
  usec = 0;
}

void loop() {

  float val = evaluateAtT(freqVect, magsVect, 6, usec);
  
  analogWrite(A14, (int)(val * 512.0 + 2048.0));

}
