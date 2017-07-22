
// This sketch can be used with an Arduino board to interface a Playstation2 DDR dance pad.
// The board is configured to work as a HID, which makes it possible to use the pad as a PC-compatible game controller. 
// Information about the communication protocol, etc: http://www.raphnet.net/electronique/psx_adaptor/Playstation.txt


//#define DEBUG_COM
//#define DEBUG_BTN

#define PIN_CLK     7
#define PIN_ATT     6
#define PIN_CMD     5
#define PIN_DAT     4

#define CLK_HOLD_US 50

#define BTN_UP      16
#define BTN_DOWN    64
#define BTN_LEFT    128
#define BTN_RIGHT   32

#define BTN_START   8
#define BTN_SELECT  1
#define BTN_X       16384
#define BTN_O       8192

typedef struct {
  uint16_t value;
  unsigned key;
  char str[8];
} Key;

Key button_map[8] = {
  {BTN_UP, KEY_UP_ARROW, "UP"},
  {BTN_DOWN, KEY_DOWN_ARROW, "DOWN"},
  {BTN_LEFT, KEY_LEFT_ARROW, "LEFT"},
  {BTN_RIGHT, KEY_RIGHT_ARROW, "RIGHT"},
  {BTN_START, KEY_RETURN, "START"},
  {BTN_SELECT, KEY_ESC, "SELECT"},
  {BTN_X, KEY_RETURN, "X"},
  {BTN_O, KEY_RETURN, "O"}
};


uint8_t exhange_data(uint8_t in) {
  uint8_t out = 0;

  for (int b = 0; b < 8 ; b++) {

    if (in & (1 << b)) {
      digitalWrite(PIN_CMD, HIGH);
    } else {
      digitalWrite(PIN_CMD, LOW);
    }

    digitalWrite(PIN_CLK, LOW);
    delayMicroseconds(CLK_HOLD_US);

    if (digitalRead(PIN_DAT)) {
      out |= (1 << b);
    } else {
      out &= ~(1 << b);
    }

    digitalWrite(PIN_CLK, HIGH);
    delayMicroseconds(CLK_HOLD_US);
  }
  return out;
}

uint16_t read_pad() {

  uint8_t data_in[5] = {0x01, 0x42, 0, 0, 0};
  uint8_t data_out[5];

  digitalWrite(PIN_ATT, LOW);
  for (int i = 0; i < 5; i++) {
    data_out[i] = exhange_data(data_in[i]);

    #ifdef DEBUG_COM
    Serial.print(data_in[i], HEX);
    Serial.print("-");
    Serial.println(data_out[i], HEX);
    #endif

  }
  digitalWrite(PIN_ATT, HIGH);

  return (data_out[4] << 8) | data_out[3];
}


void setup() {
  Serial.begin(9600);
  Keyboard.begin();

  pinMode(PIN_CLK, OUTPUT);
  pinMode(PIN_CMD, OUTPUT);
  pinMode(PIN_ATT, OUTPUT);
  pinMode(PIN_DAT, INPUT_PULLUP);

  digitalWrite(PIN_CLK, HIGH);
  digitalWrite(PIN_CMD, HIGH);

  Serial.println("Starting");
  delay(1000);
}

uint16_t pressed_new = 0;
uint16_t pressed_prev = 0;
uint16_t modified = 0;
Key key_pressed;

void loop() {
  
  delay(16);

  pressed_prev = pressed_new;
  pressed_new = ~read_pad();
  modified = pressed_new ^ pressed_prev;

  if (modified) {
    for (int i = 0; i < 8; i++) {
      key_pressed = button_map[i];
      
      if (key_pressed.value & modified) {
        if (key_pressed.value & pressed_new) {
          #ifndef DEBUG_BTN
          Keyboard.press(key_pressed.key);
          #else
          Serial.print(key_pressed.str);
          Serial.println(" DOWN");
          #endif
        }
        else {
          #ifndef DEBUG_BTN
          Keyboard.release(key_pressed.key);
          #else
          Serial.print(key_pressed.str);
          Serial.println(" UP");
          #endif
        }
      }
    }
  }
}



