#!/usr/bin/python2.7

import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BCM)

def INT0():
  global bitCount 
  bitCount += 1 
  global flagDone
  flagDone = 0
  global weigand_counter
  weigand_counter = WEIGAND_WAIT_TIME


def INT1():
  global databits
  global bitCount
  databits[bitCount] = 1
  bitCount += 1
  global flagDone 
  flagDone = 0
  global weigand_counter
  weigand_counter = WEIGAND_WAIT_TIME  


# main function
def main():
  GPIO.setmode(GPIO.BCM)
  
  GPIO.setup(20,GPIO.IN, pull_up_down=GPIO.PUD_UP)
  GPIO.setup(21,GPIO.IN, pull_up_down=GPIO.PUD_UP)
  

  MAX_BITS=100                #  max number of bits 
  WEIGAND_WAIT_TIME=3000      # time to wait for another weigand pulse.  

  databits=bytearray(MAX_BITS)    # stores all of the data bits
  bitCount=0;              # number of bits currently captured
  flagDone=0;              # goes low when data is currently being captured
  weigand_counter=0;       # countdown until we assume there are no more bits

  facilityCode=0;        #  decoded facility code
  cardCode=0;            #  decoded card code 


#try:
  GPIO.wait_for_edge(20, GPIO.FALLING)
  GPIO.wait_for_edge(21, GPIO.FALLING)
  print "\nFalling detected"
#except KeyboardInterrupt:
#  GPIO.cleanup()
#GPIO.cleanup()

while True:
  print "Waiting for scan to happen"

if __name__=="__main__":
    main()


