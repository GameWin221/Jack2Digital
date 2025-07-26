import scipy.io.wavfile as wavf
import numpy as np
import matplotlib.pyplot as plt
import serial
import time

SAMPLE_COUNT = 32
BYTES_TO_READ = SAMPLE_COUNT*2
SAMPLE_TIME = 20
SAMPLE_RATE = 16000

def transform(v: int) -> int:
    return v

if __name__ == "__main__":
    ser = serial.Serial("COM7", 500000, xonxoff=False, rtscts=False, timeout=0.1, dsrdtr=False)

    plt.ion()
    fig = plt.figure()
    ax = fig.add_subplot(111)
    line1, = ax.plot([], [], 'b-')
    
    ser.reset_input_buffer()
    
    prevt = time.time() - 0.01
    
    data = []
    #while True:
    for _ in range(SAMPLE_RATE // 32 * SAMPLE_TIME):
        #if len(data) > 32 * BYTES_TO_READ:
        #   data = data[32:]
        
        raw = ser.read(BYTES_TO_READ)

        if (len(raw) == BYTES_TO_READ):
            nums = []
            for i in range(0, BYTES_TO_READ, 2):
                nums.append(int.from_bytes(raw[i:i+2], 'little'))
                
            a = sum(nums) // len(nums)
            for i in range(len(nums)):
                nums[i] -= a
            #if(nums.count(1) != 32):
            #    print(nums)
            data.extend(nums)
            
            if (len(nums) != SAMPLE_COUNT):
                print("ERROR, len=", len(nums))
        else:
            print('empty')
    
        now = time.time()
        #print(f"{SAMPLE_COUNT/(now-prevt):.0f} samples/s")
        prevt = now
        
        #xpoints = [i for i in range(len(data))]

        #line1.set_xdata(xpoints)
        #line1.set_ydata(data)
        #plt.xlim(xpoints[0], xpoints[-1])
        #plt.ylim(min(data), max(data))
        #
        #fig.canvas.draw()
        #fig.canvas.flush_events()
    
    ser.close()
    
    #plt.pause(2.0)
    print(f"file created with {len(data)}samples at {SAMPLE_RATE}hz")
    fs = SAMPLE_RATE#44100 
    wavf.write('out.wav', fs, np.array(data))