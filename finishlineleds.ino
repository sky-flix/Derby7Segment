#include <AltSoftSerial.h>
#include <Wire.h>

//initialize an array of bytes containing the i2c addresses of each of our SparkFun displays
const byte LaneAddresses[8] =  {0x75, 0x74, 0x73, 0x72, 0x71, 0x70, 0x69, 0x68}; // Lanes 8 through 1 (As seen from the start gate)

//Create an array which we will store the lane times
String LaneTimesArray[8];

//Create an aray which we will use to store the positions of each finisher
char LanePositionsArray[8];

boolean stringComplete = false;
byte messageNumber;
char lanePos;
char displayWord[5];
byte laneInt;
char character;
String content = "";

//I chose to use a software serial library to make uploading new code to the Arduino easier
AltSoftSerial mySerial;

//These string indeces correspond directly to a variable on the Netduino indicating an error
const char* Decode[10]={
"uhoh    StArt   GAtE IS CloSEd  ",
"uhoh    StArt   GAtE IS OpEn    ",
"uhoh    LAnE1   rEAd incorrEctlY",
"uhoh    LAnE2   rEAd incorrEctlY",
"uhoh    LAnE3   rEAd incorrEctlY",
"uhoh    LAnE4   rEAd incorrEctlY",
"uhoh    LAnE5   rEAd incorrEctlY",
"uhoh    LAnE6   rEAd incorrEctlY",
"uhoh    LAnE7   rEAd incorrEctlY",
"uhoh    LAnE8   rEAd incorrEctlY"};
      

void setup() {
  Wire.begin(); //Join the bus as master
  mySerial.begin(57600); //TX-Pin9   RX-Pin8   (PWM not usable on pin10)
  //Serial.begin(9600);
  
  //Send the reset command to the display - this forces the cursor to return to the beginning of the display
  //we are also dimming the diplays to less than half of their normal output
  
  for (byte i = 0 ; i < 8 ; i++) {
    Wire.beginTransmission(LaneAddresses[i]);
    Wire.write('v');
    Wire.write(0x7A);
    Wire.write(40);
    Wire.endTransmission();
  }
  
  //Initialize the displays with their corresponding lane number. This is a good check to ensure that each
  //display's i2c address is set correctly and in the correct position on the track.
  i2cSendString(0x68," L1 ");
  i2cSendString(0x69," L2 ");
  i2cSendString(0x70," L3 ");
  i2cSendString(0x71," L4 ");
  i2cSendString(0x72," L5 ");
  i2cSendString(0x73," L6 ");
  i2cSendString(0x74," L7 ");
  i2cSendString(0x75," L8 ");
}


void loop() {
  while(mySerial.available() > 0) {
    character = mySerial.read();
    if (character == '\n')
    {
      stringComplete = true;
    }
    else
    {
      content.concat(character);
    }
  }
  //we now have a String variable called content, but is it "complete" with a new-line indicating end of input
 
  
  if (stringComplete) {
    
    //convert the string array to a character array to make data acquisition easier
    char InString[content.length()];
    
    content.toCharArray(InString, content.length()+1);
    
    //what command did we receive?
    switch (InString[0]) {
      
      case 'D': // we received a command to decode a variable from the Netduino Timer - ex: D3 indicates that Lane 2
                // was read incorrectly
        
        messageNumber = (InString[1] - '0'); // Get the integer value of a number character by subtracting character 0 from it.
        
        if (messageNumber >= 0 & messageNumber < 10) // Is this a valid index in our Decode array?
        {
          for (byte l=0; l<8; l++) // cycle through each lane
          {
            for (byte c=0; c<4; c++) displayWord[c]=Decode[messageNumber][(l*4) + c]; // Populate a character array string called displayWord for each lane
            displayWord[4]='\0'; // null terminate the character array
            i2cSendChar(LaneAddresses[l], displayWord); // send the character array to the 7-segment display
          }
        }
        break;
        
      
      case 'F':
      //We must have received a string of finish times - ex: "F 1 5.7653 2 5.8888 3 6.1111"
        char LaneTime[5];
        for (byte l=0; l<8; l++)
        {
          LaneTime[0] = InString[( (l+1) * 9) - 5]; // set the number of whole seconds. ex: Lane 1 starts at position 4
          for (byte p=1; p < 4; p++)
          {
            LaneTime[p] = InString[( (l+1) * 9 ) - 4 + p ]; // set the remaining digits, skipping the decimal point
          }
          LaneTime[4] = '\0';
          String str(LaneTime);
          LaneTimesArray[l] = str;
        }
        
        break;
      
      case 'L': // we received lane positions indicator - ex: L41 means Lane 4 finished 1st
        laneInt = InString[1] - '0';
        lanePos = InString[2];
        LanePositionsArray[laneInt - 1] = lanePos;
        Wire.beginTransmission(LaneAddresses[8-laneInt]);
        Wire.write(0x76);
        Wire.write(0x79);
        Wire.write(0x03);
        Wire.write(lanePos);
        Wire.endTransmission();
        
        break;
      
      
      case 'P': // Print positions
        for (byte l = 0 ; l < 8 ; l++) {
          Wire.beginTransmission(LaneAddresses[7-l]);
          Wire.write(0x76);
          Wire.write(0x79);
          Wire.write(0x03);
          Wire.write(LanePositionsArray[l]);
          Wire.endTransmission();
        }
        break;

      case 'R': // update all lane displays with raw text - ex: Rln 8ln 7ln 6ln 5ln 4ln 3ln 2ln 1
        
        for (byte l=0; l<8; l++)
        {
          for (byte c=0; c<4; c++) displayWord[c]=InString[(l*4) + 1 + c];
          displayWord[4]='\0';
          i2cSendChar(LaneAddresses[l], displayWord);
        }
        break;

      case 'S': // we received a single lane text string - ex: S6tESt means display "tESt" on Lane 6
        laneInt = InString[1] - '0';
        
        for (byte c=0; c<4; c++) displayWord[c]=InString[2 + c];
          displayWord[4]='\0';
          i2cSendChar(LaneAddresses[8-laneInt], displayWord);
        
        break;
      
      
              
      case 'T': // Print finish times per lane
        for (byte l=0; l<8; l++)
          i2cSendTime(LaneAddresses[7-l],LaneTimesArray[l]);
        break;
      
      
      case 'X': // clear the display entirely
        for (byte i = 0 ; i < 8 ; i++)
        {
          Wire.beginTransmission(LaneAddresses[i]);
          Wire.write('v');
          Wire.endTransmission();
        }  

      default:
        break;  
        
    } // end switch
    stringComplete = false;
    content="";
    while(Serial.read() >= 0){};  
    
  } // end if string complete section
} // end main loop


void i2cSendString(byte LEDAddress, String toSendStr)
{
  char toSend[5];
  toSendStr.toCharArray(toSend, 5);
  Wire.beginTransmission(LEDAddress);
  Wire.write(0x76);
  for (byte x = 0 ; x < 4 ; x++) Wire.write(toSend[x]);
  Wire.endTransmission();
}

void i2cSendChar(byte LEDAddress, char *toSendChar)
{
  Wire.beginTransmission(LEDAddress);
  Wire.write(0x76);
  for (byte x = 0 ; x < 4 ; x++) Wire.write(toSendChar[x]);
  Wire.endTransmission();
}

void i2cSendTime(byte LEDAddress, String toSendStr)
{
  toSendStr.toCharArray(displayWord, 5);
  
  Wire.beginTransmission(LEDAddress);
  Wire.write('v');
  for (byte x = 0 ; x < 4 ; x++) Wire.write(displayWord[x]);
  Wire.write(0x77);
  Wire.write(0x01);
  Wire.endTransmission();
}

