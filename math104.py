import serial
import sys
import matplotlib.pyplot as plt
import matplotlib.animation as animation


COM = "/dev/cu.usbmodem41127101"
BAUD = 38400
FFT_SIZE = 1024
SAMPLE_HZ = 9000.0

frequencies = [i * (SAMPLE_HZ / FFT_SIZE)  for i in range(0, FFT_SIZE)]


ser = serial.Serial(COM, BAUD, timeout = .1)

fig = plt.figure()
ax = fig.add_subplot(1,1,1)

#ax.set(xlabel='Frequency (Hz)', ylabel='Amplitude',title='FFT Plot')
#ax.grid()

def getData(i):
	if ser.in_waiting:
		rawInput = str(ser.readline().decode().strip('\r\n'))#Capture serial output as a decoded string
		mags = [float(rawNum) for rawNum in rawInput.split()]
		print(mags)

		ax.clear()
		ax.plot(frequencies[1:], mags[1:])


ani = animation.FuncAnimation(fig, getData, interval=1)
plt.show()
