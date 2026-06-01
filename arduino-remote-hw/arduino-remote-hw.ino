#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define BAUD_RATE 115200

// Arduino Uno
// Digital inputs
#define BTN_A 2
#define BTN_B 3
#define BTN_C 4
#define BTN_D 5
#define BTN_E 6
#define BTN_F 7
// Analog inputs
#define PIN_ANALOG_X 0
#define PIN_ANALOG_Y  1

//comment this out to not spam
#define ENABLE_ANALOG_REPEAT

#define ANALOG_MAX 1024ULL
#define ANALOG_DEADZONE_PERCENT 5ULL
#define ANALOG_DEADZONE_AMOUNT ANALOG_MAX * ANALOG_DEADZONE_PERCENT / 100ULL
#define ANALOG_DEADZONE_UPPER (ANALOG_MAX / 2) + ANALOG_DEADZONE_AMOUNT
#define ANALOG_DEADZONE_LOWER (ANALOG_MAX / 2) - ANALOG_DEADZONE_AMOUNT

// I2C to display
// SDA A4 (orange)
// SCL A5 (yellow)
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
  pinMode(BTN_C, INPUT_PULLUP);
  pinMode(BTN_D, INPUT_PULLUP);
  pinMode(BTN_E, INPUT_PULLUP);
  pinMode(BTN_F, INPUT_PULLUP);

  
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
  display.println(F("Awaiting Telemetry...\nBaud Rate: 115200"));
  display.display();
}

byte btn_state = 0;
byte prev_btn_state = 0;

byte all_analog_in_deadzone = 0;
byte prev_all_analog_in_deadzone = 0;

unsigned short x_val = 100;
unsigned short y_val = 200;

byte first_time = 1;

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

    // Save previous state for looking for state change
    prev_btn_state = btn_state;
    prev_all_analog_in_deadzone = all_analog_in_deadzone;
    
    btn_state = 0xc0; // all bits that aren't buttons are IDLE
    btn_state |= (digitalRead(BTN_A) << 0);  // active low buttons, 0 when pressed, 1 when idle 
    btn_state |= (digitalRead(BTN_B) << 1);
    btn_state |= (digitalRead(BTN_C) << 2);
    btn_state |= (digitalRead(BTN_D) << 3);
    btn_state |= (digitalRead(BTN_E) << 4);
    btn_state |= (digitalRead(BTN_F) << 5);
    
    x_val = analogRead(PIN_ANALOG_X);
    y_val = analogRead(PIN_ANALOG_Y);

    btn_state = ~btn_state; // invert to make status reporting active high
    all_analog_in_deadzone = !(x_val > ANALOG_DEADZONE_UPPER || x_val < ANALOG_DEADZONE_LOWER || y_val > ANALOG_DEADZONE_UPPER || y_val < ANALOG_DEADZONE_LOWER);
    
    #ifdef ENABLE_ANALOG_REPEAT
    if (!all_analog_in_deadzone || !prev_all_analog_in_deadzone || btn_state != prev_btn_state || first_time == 1){
    #endif
    #ifndef ENABLE_ANALOG_REPEAT
    if (btn_state != prev_btn_state || first_time == 1) {
    #endif
      // Open JSON
      Serial.print(F("{"));
      Serial.print(F("\"screen\" : {\"type\" : \"txt\", \"w\" : 21, \"h\" : 8},"));
      Serial.print(F("\"btn\" : {"));

      printButtonJSON("A", 0, true);
      printButtonJSON("B", 1, true);
      printButtonJSON("C", 2, true);
      printButtonJSON("D", 3, true);
      printButtonJSON("E", 4, true);
      printButtonJSON("F", 5, false);
      Serial.print(F("},"));
      
      Serial.print(F("\"analog\" : {"));
      printAnalogJSON("X", x_val, true);
      printAnalogJSON("Y", y_val, false);
      Serial.print(F("}"));
      
      // close JSON
      Serial.println(F("}"));
      first_time = 0;
    }
  }
}

void printButtonJSON(const char* input_name, byte pos, bool add_comma){
  Serial.print(F("\""));
  Serial.print(input_name);
  Serial.print(F("\" : "));
  Serial.print((btn_state & (1<<pos))>>pos, DEC);
  if (add_comma){
    Serial.print(F(","));
  }
}

void printAnalogJSON(const char* input_name, unsigned short val, bool add_comma){
  Serial.print(F("\""));
  Serial.print(input_name);
  Serial.print(F("\" : {\"val\" : "));
  Serial.print(val, DEC);
  Serial.print(F(", \"min\" : 0, \"max\" : 1023}"));
  if (add_comma){
    Serial.print(F(","));
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
