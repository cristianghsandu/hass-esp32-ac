import machine
import utime


class PushButton:
    ''' Uses D6 pin(GPIO12) by default '''

    def __init__(self, single_press_action, long_press_action, double_press_action, gpio_pin=12):
        self.button = machine.Pin(12, machine.Pin.IN, machine.Pin.PULL_UP)

        self.prev_state = -1
        self.start_press = -1
        self.end_press = -1
        self.interval = -1
        self.wait_for_single = False

        self.single_press_action = single_press_action
        self.long_press_action = long_press_action
        self.double_press_action = double_press_action

    def check_button(self):
        first = self.button.value()
        utime.sleep_ms(10)
        second = self.button.value()
        if first and not second:
            # print('Button pressed!')
            return 1
        elif not first and second:
            # print('Button released!')
            return 0
        return -1

    def loop(self):
        state = self.check_button()

        if state == -1 and self.wait_for_single:
            # Button not pressed
            now = utime.ticks_ms()
            since_last_press = (now - self.end_press)
            if self.end_press > 0 and since_last_press >= 300:
                print('short press')
                self.single_press_action()
                self.wait_for_single = False
        elif state == 1:
            if self.prev_state != state:
                self.start_press = utime.ticks_ms()
                # when was the last button end?
                self.interval = self.start_press - self.end_press
        elif state == 0:
            if self.prev_state != state:
                self.end_press = utime.ticks_ms()

                duration = self.end_press - self.start_press
                # print('Pressed for ', duration, ' ms')
                
                if duration >= 300:
                    print('long press')
                    self.long_press_action()
                else:
                    # print('interval ', self.interval)
                    if self.interval <= 300:
                        print('double press')
                        self.double_press_action()
                        self.wait_for_single = False
                    else:
                        self.wait_for_single = True

        self.prev_state = state

