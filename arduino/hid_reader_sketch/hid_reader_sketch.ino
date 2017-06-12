/*
 * HID RFID Reader Wiegand Interface for Arduino
 * Originally by  Daniel Smith, 2012.01.30 -- http://www.pagemac.com/projects/rfid/arduino_wiegand
 * 
 * Updated 2016-11-23 by Jon "ShakataGaNai" Davis.
 * See https://obviate.io/?p=7470 for more details & instructions
 * 
 * Further modified by Richard Kendall, 2017-06-11
 * Fixed a major error in the protocol decoding (1 and 0 were swapped)
 * Now it does not attempt to determine the type of card (as there is not much point)
 * It just prints the hex of the data read from the card to the serial line
 * I added some details from the Weigand documentation to explain why this works as well
 * 
 * for each card it prints the following string on a new line
 * [<number of bits>,<data in hexadecimal>]
 * 
*/
 
 
#define MAX_BITS 100                 // max number of bits 
#define WEIGAND_WAIT_TIME  3000      // time to wait for another weigand pulse.  
 
unsigned char databits[MAX_BITS];    // stores all of the data bits
unsigned char bitCount;              // number of bits currently captured
unsigned char flagDone;              // goes low when data is currently being captured
unsigned int weigand_counter;        // countdown until we assume there are no more bits
 
unsigned long cardCode = 0;          // decoded card code

int DATA_LINE = 2;                   // pin for the data line, green wire
int CLCK_LINE = 3;                   // pin for the clock line, white wire

// interrupt that happens when INTO goes low (0 bit)
// data line
// when low a 0 is being sent
void ISR_INT0() {
  databits[bitCount] = 1;
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME;  
}
 
// interrupt that happens when INT1 goes low (1 bit)
// clock line
// when low a 1 is being sent
void ISR_INT1() {
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME;  
}
 
void setup() {
  pinMode(DATA_LINE, INPUT);     // DATA0 (INT0) // green // data
  pinMode(CLCK_LINE, INPUT);     // DATA1 (INT1) // white // clock

  // open the serial line for communication with the consumer
  Serial.begin(9600);
 
  // binds the ISR functions to the falling edge of INTO and INT1
  attachInterrupt(digitalPinToInterrupt(DATA_LINE), ISR_INT0, FALLING);  
  attachInterrupt(digitalPinToInterrupt(CLCK_LINE), ISR_INT1, FALLING);

  weigand_counter = WEIGAND_WAIT_TIME;
}
 
void loop() {
  // This waits to make sure that there have been no more data pulses before processing data
  if (!flagDone) {
    if (--weigand_counter == 0) {
      flagDone = true;  
    }
  }
 
  // if we have bits and we the weigand counter went out
  if (bitCount > 0 && flagDone) {
    unsigned char i;
 
    Serial.print("[");
    Serial.print(bitCount);
    Serial.print(",");

    for(int i=0;i < bitCount;i++) {
      cardCode <<= 1;           // shift to the left to make room for next bit
      cardCode |= databits[i];  // bitwise OR the existing card code with the next bit
    }
    
    String hexCode = toHex(cardCode);
    Serial.print(hexCode);
    Serial.println("]");
    
    // clean up and get ready for the next card
    bitCount = 0;
    cardCode = 0;
    for (i = 0;i < MAX_BITS;i++) {
      databits[i] = 0;
    }
  }
}

String toHex(unsigned long d) {
  unsigned long r = d % 16;
  String result;
  if(d - r == 0) {
    if(r < 10) {
      result = r;
    } else {
      result = char(64 + (r - 9));
    }
  } else {
    if(r < 10) {
      result = toHex((d-r)/16) + r;
    } else {
      result = toHex((d-r)/16) + char(64 + (r - 9));
    }
  }
  return result;
}


