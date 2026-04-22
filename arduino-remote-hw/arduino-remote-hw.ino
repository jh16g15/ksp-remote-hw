#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define BAUD_RATE 115200

// Arduino Uno
// Digital inputs
#define BTN_A 7
#define BTN_B 6

// I2C to display
// SDA A4
// SCL A5
// SSD1306 I2C address 0x3C
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define CHARS_PER_LINE 21u // in font size 1
#define LINES_PER_SCREEN 8u 
#define CHARS_PER_SCREEN CHARS_PER_LINE * LINES_PER_SCREEN 
char INPUT_BUF[CHARS_PER_SCREEN]; // buffer for processing serial input, defines max message size

void setup() {
  // put your setup code here, to run once:
  Serial.begin(BAUD_RATE);
  delay(500); // wait for display

  clearInputBuf();

  // setup buttons as inputs
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  
  // NOTE may want to skip display if not plugged in!

  // WARNING this allocates 1024 bytes of runtime local vars to store display contents that we need to keep space for
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen (delete to save flash space if needed?).
  display.display();
  delay(500); // splash screen display
  
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner (pixels)
  display.println(F("Hello, world from Joe!")); // text wraps automatically, 21 chars per line

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  
  
  delay(250); // splash screen display
  
  
  //* test all chars working
  // NOTE byte=unsigned char 0 to 255, char=-128 to 127
  for (byte i=0;i<sizeof(INPUT_BUF);i++){
    INPUT_BUF[i]='x';
  }
  printInputBuf();
  clearInputBuf();
  delay(1000); 
  //*/ 

  display.clearDisplay();
  display.setCursor(0,0);             // Start at top-left corner (pixels)
  display.println(F("Awaiting Telemetry..."));
  display.display();
}

byte btn_state = 0;
byte prev_btn_state = 0;

unsigned short x_val = 100;
unsigned short y_val = 200;

void loop() {
  // put your main code here, to run repeatedly:
  char c;
  byte index = 0;

  while(1){

    if (Serial.available() > 0){
      // process received char
      c = Serial.read();
      if (index < sizeof(INPUT_BUF)){ // then it will fit
        if (c != '\n'){
          INPUT_BUF[index] = c; // save to message buffer unless newline, which triggers display
          index++;
        }
      }
      if (c == '\n'){
          // print to screen
        printInputBuf();
        index = 0;
        clearInputBuf();
      } 
    }
    
    prev_btn_state = btn_state;
    btn_state = 0xfc; // all bits that aren't buttons are IDLE
    btn_state |= (digitalRead(BTN_A) << 0);  // active low buttons, 0 when pressed, 1 when idle 
    btn_state |= (digitalRead(BTN_B) << 1);
    
    btn_state = ~btn_state; // invert to make status reporting active high
    if (btn_state != prev_btn_state) {
      // Open JSON
      Serial.print(F("{"));
      Serial.print(F("\"screen\" : {\"type\" : \"txt\", \"w\" : 21, \"h\" : 8},"));
      Serial.print(F("\"btn\" : {"));
      Serial.print(F("\"A\" : "));
      Serial.print((btn_state & (1<<0))>>0, DEC);
      Serial.print(F(","));
      Serial.print(F("\"B\" : "));
      Serial.print((btn_state & (1<<1))>>1, DEC);
      Serial.print(F("},"));
      
      Serial.print(F("\"analog\" : {"));
      Serial.print(F("\"X\" : {\"val\" : "));
      Serial.print(x_val, DEC);
      Serial.print(F(", \"min\" : 0, \"max\" : 1023},"));
      Serial.print(F("\"Y\" : {\"val\" : "));
      Serial.print(y_val, DEC);
      Serial.print(F(", \"min\" : 0, \"max\" : 1023}"));
      Serial.print(F("}"));
      
      // close JSON
      Serial.println(F("}"));
    }
  }
}

void printInputBuf(){
  display.clearDisplay();
  display.setCursor(0,0);             // Start at top-left corner (pixels)
  display.print(INPUT_BUF); // text wraps automatically
  display.display();
}

int clearInputBuf(){
  for (byte i=0;i<sizeof(INPUT_BUF);i++){
    INPUT_BUF[i]=' ';
  }
}
