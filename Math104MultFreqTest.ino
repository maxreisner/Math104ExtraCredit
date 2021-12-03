// Simple DAC sine wave test on Teensy 3.1

float phase = 0.0;
float twopi = 3.14159 * 2;
elapsedMicros usec = 0;
elapsedMillis msec = 0;



float freqList[] = {440.0, 554.0, 659.0}; 
float freqMags[] = {1.0, 1.0, 1.0};

int i = 0;

const int noteLength = 200;

float angularMom = twopi / (1000000.0);

void setup() {
  analogWriteResolution(12);
  usec = 0;
}

void loop() {

  float val = (sin(freqList[0] * angularMom * usec) + sin(2* freqList[0] * angularMom * usec) + + sin(freqList[1] * angularMom * usec) + sin(freqList[2] * angularMom * usec)) / 2.0 * 512.0 + 2048.0;
  analogWrite(A14, (int)val);

  /*if (msec > noteLength) {
    msec = msec - noteLength;

    i++;
    
    if (i > 7) {
      i = 0;
    }

    angularMom = freqList[i] * twopi / (1000000.0);    
  }*/
}
