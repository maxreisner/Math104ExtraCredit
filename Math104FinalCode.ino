
#define ARM_MATH_CM4
#include <arm_math.h>

#define TWOPI 3.14159 * 2
#define ANGULARMOM TWOPI / (1000000.0)

int SAMPLE_RATE_HZ = 1000;
const int FFT_SIZE = 1024;   

const int AUDIO_INPUT_PIN = 16;        // Input ADC pin for audio data.
const int ANALOG_READ_RESOLUTION = 10; // Bits of resolution for the ADC.
const int ANALOG_READ_AVERAGING = 16;  // Number of samples to average with each ADC reading.
const int POWER_LED_PIN = 13;          // Output pin for power LED (pin 13 to use Teensy 3.0's onboard LED).

IntervalTimer samplingTimer;
float samples[FFT_SIZE * 2];
float magnitudes[FFT_SIZE];
int sampleCounter = 0;

float frequencies[FFT_SIZE / 2];
elapsedMicros usec = 0;

float playbackVals[10000];
int playbackCounter = 0;
//const float storeSampleRatio = 1024.0 / 100.0;

void setup() {
  Serial.begin(38400);


  // Set up ADC and audio input.
  pinMode(AUDIO_INPUT_PIN, INPUT);
  analogReadResolution(ANALOG_READ_RESOLUTION);
  analogReadAveraging(ANALOG_READ_AVERAGING);

  // Turn on the power indicator LED.
  pinMode(POWER_LED_PIN, OUTPUT);
  digitalWrite(POWER_LED_PIN, HIGH);

  pinMode(8, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);

  analogWriteResolution(12);
  for (int i = 0; i < FFT_SIZE / 2; i++) {
    frequencies[i] = float(i) * (float(SAMPLE_RATE_HZ) / float(FFT_SIZE));
  }
  
  usec = 0;

}

void loop() {

  if (digitalRead(8) == 0) {
    sampleCounter = 0;
    playbackCounter = 0;
    samplingBegin();
    Serial.println("Sampling...");
    
    while (digitalRead(8) == 0) {
      
      if (samplingIsDone()) {
        // Run FFT on sample data.
        arm_cfft_radix4_instance_f32 fft_inst;
        arm_cfft_radix4_init_f32(&fft_inst, FFT_SIZE, 0, 1);
        arm_cfft_radix4_f32(&fft_inst, samples);
        // Calculate magnitude of complex numbers output by the FFT.
        //arm_cmplx_mag_f32(samples, magnitudes, FFT_SIZE);
    
        /*for (int i = 0; i < FFT_SIZE; i++) {
          //Serial.print(i * (float(SAMPLE_RATE_HZ) / float(FFT_SIZE)));
          //Serial.print("");
          Serial.print(abs(samples[i*2]));
          Serial.print(' ');
        }*/
    
        //Serial.println();
        if (playbackCounter < 10000) {
          for (int i = 0; i < FFT_SIZE; i++) {
            playbackVals[playbackCounter + i] =  evaluateAtT(frequencies, samples, FFT_SIZE / 2, float(i) * (1000000.0 / SAMPLE_RATE_HZ)); 
          }
        }
        else {
          Serial.println("Samples Full!");
        }
        
        playbackCounter += FFT_SIZE;
        Serial.println(playbackCounter);
    
        // Restart audio sampling.
        samplingBegin();
      }
    }

    Serial.println("Done sampling");
  }
  if (digitalRead(7) == 0) {

    Serial.println("Playing back!");
    int i = 0;
    
    while (i < playbackCounter) {
      analogWrite(A14, (int) (playbackVals[i] * 512.0 + 2048.0));
      delayMicroseconds(int(1000000.0 / SAMPLE_RATE_HZ ));
      i++;
    }

    Serial.println("Done playing");
  }
}





////////////////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

// Compute the average magnitude of a target frequency window vs. all other frequencies.
void windowMean(float* magnitudes, int lowBin, int highBin, float* windowMean, float* otherMean) {
  *windowMean = 0;
  *otherMean = 0;
  // Notice the first magnitude bin is skipped because it represents the
  // average power of the signal.
  for (int i = 1; i < FFT_SIZE / 2; ++i) {
    if (i >= lowBin && i <= highBin) {
      *windowMean += magnitudes[i];
    }
    else {
      *otherMean += magnitudes[i];
    }
  }
  *windowMean /= (highBin - lowBin) + 1;
  *otherMean /= (FFT_SIZE / 2 - (highBin - lowBin));
}

void samplingCallback() {
  // Read from the ADC and store the sample data
  samples[sampleCounter] = (float32_t)analogRead(AUDIO_INPUT_PIN);
  // Complex FFT functions require a coefficient for the imaginary part of the input.
  // Since we only have real data, set this coefficient to zero.
  samples[sampleCounter + 1] = 0.0;
  // Update sample buffer position and stop after the buffer is filled
  sampleCounter += 2;
  if (sampleCounter >= FFT_SIZE * 2) {
    samplingTimer.end();
  }
}

void samplingBegin() {
  // Reset sample buffer position and start callback at necessary rate.
  sampleCounter = 0;
  samplingTimer.begin(samplingCallback, 1000000 / SAMPLE_RATE_HZ);
}

boolean samplingIsDone() {
  return sampleCounter >= FFT_SIZE * 2;
}

float evaluateAtT(float freqs[], float mags[], int dim, float t) {
  float returnVal = 0;
  float magTracker = 0;

  for (int i = 1; i < 450; i++) {
    returnVal += mags[i*2] * sin(freqs[i] * ANGULARMOM * t);
    magTracker += mags[i*2] * mags[i*2];
  }

  float magRoot = 0;
  arm_sqrt_f32(magTracker, &magRoot);

  return returnVal / magRoot;
}
