// Math 104 Extra Credit Project
// Max Reisner (mreisner@stanford.edu)


#define ARM_MATH_CM4
#define ARM_MATH_CM4
#include <arm_math.h>

int SAMPLE_RATE_HZ = 10000;             // Sample rate of the audio in hertz.
float SPECTRUM_MIN_DB = 30.0;          // Audio intensity (in decibels) that maps to low LED brightness.
float SPECTRUM_MAX_DB = 60.0;          // Audio intensity (in decibels) that maps to high LED brightness.
// Useful for turning the LED display on and off with commands from the serial port.
const int FFT_SIZE = 1024;              // Size of the FFT.  Realistically can only be at most 256
// without running out of memory for buffers and other state.
const int AUDIO_INPUT_PIN = 16;        // Input ADC pin for audio data.
const int ANALOG_READ_RESOLUTION = 10; // Bits of resolution for the ADC.
const int ANALOG_READ_AVERAGING = 16;  // Number of samples to average with each ADC reading.
const int POWER_LED_PIN = 13;          // Output pin for power LED (pin 13 to use Teensy 3.0's onboard LED).

IntervalTimer samplingTimer;
float samples[FFT_SIZE * 2];
float magnitudes[FFT_SIZE];
int sampleCounter = 0;

void setup() {
  // Set up serial port.
  Serial.begin(38400);

  // Set up ADC and audio input.
  pinMode(AUDIO_INPUT_PIN, INPUT);
  analogReadResolution(ANALOG_READ_RESOLUTION);
  analogReadAveraging(ANALOG_READ_AVERAGING);

  // Turn on the power indicator LED.
  pinMode(POWER_LED_PIN, OUTPUT);
  digitalWrite(POWER_LED_PIN, HIGH);

  // Begin sampling audio
  samplingBegin();
}

void loop() {
  // Calculate FFT if a full sample is available.
  if (samplingIsDone()) {
    // Run FFT on sample data.
    arm_cfft_radix4_instance_f32 fft_inst;
    arm_cfft_radix4_init_f32(&fft_inst, FFT_SIZE, 0, 1);
    arm_cfft_radix4_f32(&fft_inst, samples);
    // Calculate magnitude of complex numbers output by the FFT.
    //arm_cmplx_mag_f32(samples, magnitudes, FFT_SIZE);

    for (int i = 0; i < FFT_SIZE; i++) {
      //Serial.print(i * (float(SAMPLE_RATE_HZ) / float(FFT_SIZE)));
      //Serial.print("");
      Serial.print(abs(samples[i*2]));
      Serial.print(' ');
    }

    Serial.println();

    // Restart audio sampling.
    samplingBegin();
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
