from machine import Pin
from button import PushButton

def main():
    # Key codes gotten from AnalysIR
    keys = \
    {
        'tv': {
            'on': [ 2581,-917,405,-875,427,-448,405,-469,384,-917,832,-469,405,-469,405,-448,384,-469,384,-469,384,-469,405,-469,405,-469,384,-469,832,-469,384,-896,853,-469,384,-896,853,-84309,2581,-917,405,-896,405,-469,363,-512,341,-917,832,-512,341,-512,341,-512,341,-512,341,-512,341,-512,341,-512,341,-512,341,-491,811,-512,341,-917,853,-491,341,-917,853 ]
        }
    }

    ir_pin = Pin(14, Pin.OUT)

    single_press_action = lambda : ir_pin.value(1) if ir_pin.value() == 0 else ir_pin.value(0)
    long_press_action = lambda : print('long')
    double_press_action = lambda : print('double')

    button = PushButton(single_press_action, long_press_action, double_press_action)


    while True:
        button.loop()
