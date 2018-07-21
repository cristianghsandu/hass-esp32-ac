from machine import Pin, PWM
import time

class IrSender:
    ''' Class for sending IR signals, usese GPIO 14 on ESP32 by default '''
    def __init__(self, pwm_pin=14):
        self.pin = Pin(pwm_pin)
        # 38kHz: common freq for IR
        self.pwm = PWM(self.pin, freq=38000, duty=0)

    def send(self, pulses):
        for p in pulses:
            # LED on
            if p > 0:
                self.pwm.duty(1023)
            else:
                self.pwm.duty(0)
            
            time.sleep_us(abs(p))
        
        self.pwm.duty(0)