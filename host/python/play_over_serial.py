import numpy as np
import serial
import time
import pyaudio
from multiprocessing import Process, Pipe

SAMPLE_COUNT = 32
BYTES_TO_READ = SAMPLE_COUNT*2
SAMPLE_RATE = 16000

def audio_player_loop(pipe):
    p = pyaudio.PyAudio()
    stream = p.open(format=pyaudio.paInt16, channels=1, rate=SAMPLE_RATE, output=True, frames_per_buffer=SAMPLE_COUNT)
    
    while True:  
        data = pipe.recv() # blocks until received
        stream.write(data, SAMPLE_COUNT)

if __name__ == "__main__":
    ser = serial.Serial("COM7", 1152000, xonxoff=False, rtscts=False, timeout=0.1, dsrdtr=False)

    parentp, childp = Pipe()  
    audio_process = Process(target=audio_player_loop, args=(childp, ))
    audio_process.start()
    
    ser.reset_input_buffer()
    
    nums = np.zeros(SAMPLE_COUNT, dtype=np.int16)
    
    prevt = time.time()
    
    while True:   
        raw = ser.read(BYTES_TO_READ)
        
        if (len(raw) == BYTES_TO_READ):
            for i in range(0, BYTES_TO_READ, 2):
                nums[i//2] = raw[i] | (raw[i+1] << 8)
                
            a = sum(nums) // len(nums)
            nums -= a # Center the waveform
            nums *= 80 # Increase volume
            
            parentp.send(nums)
        else:
            print('empty')
    
        now = time.time()
        #print(f"{SAMPLE_COUNT/(max(now-prevt, 0.000001)):.0f} samples/s")
        prevt = now
    
    ser.close()
    stream.close()
    audio_process.join()