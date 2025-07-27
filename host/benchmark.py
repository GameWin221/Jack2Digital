import numpy as np
import serial
import time

SAMPLE_COUNT = 32
BYTES_TO_READ = SAMPLE_COUNT*2
SAMPLE_TIME = 20
SAMPLE_RATE = 16000

if __name__ == "__main__":
    ser = serial.Serial("COM7", 500000, xonxoff=False, rtscts=False, timeout=0.1, dsrdtr=False)
    ser.reset_input_buffer()
    
    prevt = time.time() - 0.01
    
    nums = np.zeros(SAMPLE_COUNT, dtype=np.int16)
    
    while True:
        raw = ser.read(BYTES_TO_READ)
        
        if (len(raw) == BYTES_TO_READ):
            for i in range(0, BYTES_TO_READ, 2):
                nums[i//2] = int.from_bytes(raw[i:i+2], 'little')
                
            a = sum(nums) // len(nums)
            nums -= a

            if (len(nums) != SAMPLE_COUNT):
                print("ERROR, len=", len(nums))
        else:
            print('empty')
    
        now = time.time()
        print(f"{SAMPLE_COUNT/(now-prevt):.0f} samples/s")
        prevt = now
        
    ser.close()