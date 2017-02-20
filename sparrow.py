#!/usr/bin/python2.7

import RPi.GPIO as GPIO


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
  
  GPIO.setup(38,GPIO.IN)
  GPIO.setup(40,GPIO.IN)
  

  MAX_BITS=100                #  max number of bits 
  WEIGAND_WAIT_TIME=3000      # time to wait for another weigand pulse.  

  databits=bytearray(MAX_BITS)    # stores all of the data bits
  bitCount=0;              # number of bits currently captured
  flagDone=0;              # goes low when data is currently being captured
  weigand_counter=0;       # countdown until we assume there are no more bits

  facilityCode=0;        #  decoded facility code
  cardCode=0;            #  decoded card code 

if __name__=="__main__":
    main()


