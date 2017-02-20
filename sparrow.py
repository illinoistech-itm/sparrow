#!/usr/bin/python2.7

import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BCM)

def printBits(facilityCode,cardCode):
      print "FC = " 
      print facilityCode
      print ", CC = "
      print "\n", cardCode


def INT0(channel):
  global bitCount 
  bitCount += 1 
  global flagDone
  flagDone = 0
  global weigand_counter
  weigand_counter = WEIGAND_WAIT_TIME


def INT1(channel):
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
  GPIO.add_event_detect(20, GPIO.RISING, callback=INT0, bouncetime=300)
  GPIO.add_event_detect(21, GPIO.RISING, callback=INT1, bouncetime=300)
  print "\n" , "Falling detected"
#except KeyboardInterrupt:
#  GPIO.cleanup()
#GPIO.cleanup()

  while True:
    print "Waiting for scan to happen"
  # This waits to make sure that there have been no more data pulses before processing data
    if not flagDone:
      if (weigand_counter -1 )== 0:
        flagDone = 1
   
    # if we have bits and we the weigand counter went out
    if bitCount > 0 and flagDone: 
      i = ''
 
      print "Read "
      print bitCount
      print " bits. "
 
      if bitCount == 35:
        # 35 bit HID Corporate 1000 format
        # facility code = bits 2 to 14
        for i in range(2,15): 
  	   facilityCode <<=1
           facilityCode |= databits[i]
      
        # card code = bits 15 to 34
        for i in range(14,35):
           cardCode <<=1
           cardCode |= databits[i]
      
        printBits(facilityCode,cardCode)
    
      elif bitCount == 26:
        # standard 26 bit format
        # facility code = bits 2 to 9
        for i in range(1,10):
           facilityCode <<=1
           facilityCode |= databits[i]
      
        # card code = bits 10 to 23
        for i in range(9,26):
           cardCode <<=1
           cardCode |= databits[i]
      
        printBits(facilityCode,cardCode)  
    
      else: 
       # you can add other formats if you want!
        print "\n","Unable to decode." 
    
       # cleanup and get ready for the next card
        bitCount = 0
        facilityCode = 0
        cardCode = 0
        for i in range(0,MAX_BITS+1): 
          databits[i] = 0

if __name__=="__main__":
    main()


