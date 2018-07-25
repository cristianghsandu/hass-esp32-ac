# IR Blaster
# Created at 2018-07-25 18:15:25.931533



import streams
from infrared import infrared

s=streams.serial()

#Ir Remotes Raw data can be found on various websites like https://www.remotecentral.com/index.html
#The IRSender.sendRaw method takes as input a list of times in microseconds where the first time is for the state 1 phase (IR LED firing) the second time is for state 0 (IR LED OFF) and so on.
SamsungON=[4500, 4500, 590, 1690, 590, 1690, 590, 1690, 590, 590, 590, 590, 590, 590, 590, 590, 590, 590, 590, 1690, 590, 1690, 590, 1690, 590, 590, 590, 590, 590, 590, 590, 590, 590, 590, 590, 590, 590, 1690, 590, 590, 590, 590, 590, 590, 590, 590, 590, 590, 590, 590, 590, 1690, 590, 590, 590, 1690, 590, 1690, 590, 1690, 590, 1690, 590, 1690, 590, 1690, 590, 4500, 4500, 590, 1690, 590, 1690, 590, 1690, 590, 590, 590, 590, 590, 590, 590, 590, 590, 590, 590, 1690, 590, 1690, 590, 1690, 590, 590, 590, 590, 590, 590, 590, 590]

#Create a IR packet sender passing it the pin where the IR LED is connected to specifying the PWM feature 
sender = infrared.IRSender(D6.PWM)                
i=0

while True:
    print("Sending Samsung TV ON/OFF", i)
    sender.sendRaw(SamsungON)
    sleep(2000)
    i+=1