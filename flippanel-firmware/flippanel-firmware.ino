
#define PIN_CCLK 7
#define PIN_CDAT 8
#define PIN_CRES 9

#define PIN_RST 2
#define PIN_SRCLK 3
#define PIN_RCLK 4
#define PIN_OE 5
#define PIN_SER 6

#define DISPLAY_COLS 90
#define DISPLAY_ROWS 24
#define DISPLAY_ROWS_BLKS 3

/// Delay for TTL logic sync up in microseconds
#define DELAY_LOGIC 500

#define DELAY_PIXEL_FLIP 20

byte currentBuffer[DISPLAY_COLS][DISPLAY_ROWS_BLKS];
byte nextBuffer[DISPLAY_COLS][DISPLAY_ROWS_BLKS];

int read_serial_byte() {
  while (!Serial.available()) { }
  
  int c = Serial.read();
  Serial.print("GOT=");
  Serial.println(c, HEX);
  return c;
}

int read_serial_hex() {
  byte hexdigits[3] = { 255, 255, 0 };
  const byte *hexptr = hexdigits;
  Serial.readBytes(hexdigits, 2);

  int c = strtol(hexdigits, &hexptr, 16);

//  Serial.print("hexdigits[0]=");
//  Serial.println(hexdigits[0]);
//  Serial.print("hexdigits[1]=");
//  Serial.println(hexdigits[1]);
//  Serial.print("hexdigits[2]=");
//  Serial.println(hexdigits[2]);
//  Serial.println(*hexptr);

  if (*hexptr == '\0')
    return c;
    
  return -1;
}

void read_serial_commands() {
  led_on();
  Serial.println("CMD?");

  char ch = read_serial_byte();
  led_off();

  if (ch == 'd') {
    // read 570 hex characters (0-9A-F)
    Serial.println("READING BYTES");
    bool all_ok = true;
    for (int ii = 0; ii < DISPLAY_COLS; ii++) {
      for (int jj = 0; jj < DISPLAY_ROWS_BLKS; jj++) {
        int c = read_serial_hex();
        if (c < 0)
          all_ok = false;
          
        if (!all_ok)
          break;
  
        led_on();
        delay(50);
        led_off();
        delay(50);
      }
      
      if (!all_ok)
        break;
    };

    Serial.println("ACK");
    //draw_buffer_differential();
    Serial.println("DONE");
  }

  if (ch == 'c') {
    // clear
    //draw_buffer();
  }

  if (ch == 'f') {
    //fill
    Serial.println("EXEC: FILL");
  }

  if (ch == '1') {
    Serial.println("EXEC: test_basic_1");
    test_basic_1();
  }
}

inline void led_on() {
  digitalWrite(LED_BUILTIN, HIGH);
}

inline void led_off() {
  digitalWrite(LED_BUILTIN, LOW);
}

inline void cols_reset() {
  digitalWrite(PIN_CDAT, HIGH);
  digitalWrite(PIN_CRES, HIGH);
  delayMicroseconds(DELAY_LOGIC);
  digitalWrite(PIN_CRES, LOW);
  delayMicroseconds(DELAY_LOGIC);
}

inline void cols_set_data_true() {
  digitalWrite(PIN_CDAT, LOW);
}

inline void cols_set_data_false() {
  digitalWrite(PIN_CDAT, HIGH);
}

inline void cols_tick() {
  digitalWrite(PIN_CCLK, LOW);
  delayMicroseconds(DELAY_LOGIC);
  digitalWrite(PIN_CCLK, HIGH);
  delayMicroseconds(DELAY_LOGIC);
}

inline void rows_reset() {
  digitalWrite(PIN_RST, LOW);
  delayMicroseconds(DELAY_LOGIC);
  digitalWrite(PIN_RST, HIGH);
  delayMicroseconds(DELAY_LOGIC);
}

inline void rows_set_data_true() {
  digitalWrite(PIN_SER, HIGH);
  delayMicroseconds(DELAY_LOGIC);
}

inline void rows_set_data_false() {
  digitalWrite(PIN_SER, LOW);
  delayMicroseconds(DELAY_LOGIC);
}

inline void rows_tick() {
  digitalWrite(PIN_SRCLK, HIGH);
  delayMicroseconds(DELAY_LOGIC);
  digitalWrite(PIN_SRCLK, LOW);
  delayMicroseconds(DELAY_LOGIC);
}

inline void rows_activate() {
  // activate the rows latch to transmit the 
  digitalWrite(PIN_RCLK, HIGH);
  delayMicroseconds(DELAY_LOGIC);
  digitalWrite(PIN_RCLK, LOW);

  // now the current is flowing, wait for pixel flip time
  delay(DELAY_PIXEL_FLIP);

  // now reset the shift registers
  digitalWrite(PIN_RST, LOW);
  delayMicroseconds(DELAY_LOGIC);
  digitalWrite(PIN_RST, HIGH);

  // activate again the rows latch, setting all to zero
  digitalWrite(PIN_RCLK, HIGH);
  delayMicroseconds(DELAY_LOGIC);
  digitalWrite(PIN_RCLK, LOW);
}

// this should set pixel 1 and 3 to on for the first column
void test_basic_1() {
  Serial.println("TEST begin low level directives 1");

  // activate the first column (high)
  cols_reset();
  cols_set_data_true();
  cols_tick();
  cols_set_data_false();
  //cols_tick();

  // activate the first row (high) rows 1 and 3
  rows_reset();
  rows_set_data_true();
  rows_tick();
  rows_set_data_false();
  rows_tick();
  rows_set_data_true();
  rows_tick();
//  rows_activate();

  for (int i = 0; i < 24; i++) {
    rows_tick();
  }

  rows_activate();

  Serial.println("TEST end");
}

void test_basic_2() {
  Serial.println("TEST begin low level directives 2");

  // activate the first column (high)
  cols_reset();

  rows_set_data_true();
  
  Serial.println("TEST end");
}

/* ------------------------------------------------------------------------ */


/**
 * Enables selected rows in a given color
 *
 * We have 24 rows, each row can be set to Vflip or to GND, or can be left
 * inactive (high impedance). To decide this, we have a 48 bits shift register
 * to tune, the first 24 bits will set rows 1-24 to Vflip, the following 24
 * bits will set the same rows 1-24 to GND, so it is very important that the
 * same row is not set to both Vflip and GND at the same time.
 *
 * This is decided by the "color" parameter, so we can either set a row in one
 * of the two states.
 */
void pulse_rows(uint32_t rows, byte color) {
  // reset the shift register
  rows_reset();

  // pipe in the lowest 24 bits to the shift registers
  for (byte i = 0; i < DISPLAY_ROWS; ++i) {
    if (rows & 1)
      rows_set_data_true();
    else
      rows_set_data_false();

    rows_tick();

    rows = rows >> 1;
  }

  // if we want to clear the pixel, we need to tick 24 more times to move the
  // piped in value to the second part of the shift bits
  // thus we pulse 24 additional times
  if (!color) {
    rows_set_data_false();
    for (byte i = 0; i < DISPLAY_ROWS; i++)
      rows_tick();
  }

  // activate the latch output
  rows_activate();
}

void test_clear_all() {
  for (int i = 0; i < DISPLAY_COLS; ++i) {
    /* skip to the "clear" column */
    rows_tick();

    /* pulse the rows to low */
    pulse_rows(0, 0);

    /* go to the next column "set" */
    rows_tick();
  }
}

void test_set_all() {
  for (int i = 0; i < DISPLAY_COLS; ++i) {
    /* pulse the rows to high */
    pulse_rows(0xffffff, 1);

    /* go to the next column (skipping the "clear" phase) */
    rows_tick();
    rows_tick();
  }
}

/* ------------------------------------------------------------------------ */

/**
 * ...
 */
void draw_buffer() {
  // restart from the first column
  cols_reset();

  // for each column we have two phases, first flip on, then flip off */
  for (int i = 0; i < DISPLAY_COLS; ++i) {
    uint32_t rows = currentBuffer[i][0] |
                    currentBuffer[i][1] << 8;
                    currentBuffer[i][2] << 16;

    // first activate only the rows that we need to be flipped on
    pulse_rows(rows, 1);

    // now tick the column shift register to move to the "off" transistor
    rows_tick();

    // lastly, activate the rows that need to be turned off
    pulse_rows(~rows, 0);

    // tick again the columns to prepare for the next one
    rows_tick();
  }
}

/**
 * ...
 */
void draw_buffer_differential() {
  // restart from the first column
  cols_reset();

  for (int i = 0; i < DISPLAY_COLS; i++) {
    uint32_t prows = currentBuffer[i][0] |
                     currentBuffer[i][1] << 8;
                     currentBuffer[i][2] << 16;

    uint32_t nrows = nextBuffer[i][0] |
                     nextBuffer[i][1] << 8;
                     nextBuffer[i][2] << 16;

    // we are in this situation for this column:
    //   prows    0010011 ^  current rows
    //   nrows    1010001    next rows
    //            -------
    //   xrows    1000010    changed rows
    //
    //   srows    1000000    rows to be set
    //   crows    0000010    rows to be cleared

    uint32_t xrows = prows ^ nrows;
    uint32_t srows = xrows & nrows;
    uint32_t crows = xrows & prows;

    if (srows)
      pulse_rows(srows, 1);
    cols_tick();

    if (crows)
      pulse_rows(crows, 0);
    cols_tick();
  }
}

/* ------------------------------------------------------------------------ */

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_CCLK, OUTPUT);
  pinMode(PIN_CDAT, OUTPUT);
  pinMode(PIN_CRES, OUTPUT);

  pinMode(PIN_RST, OUTPUT);
  pinMode(PIN_SRCLK, OUTPUT);
  pinMode(PIN_RCLK, OUTPUT);
  pinMode(PIN_OE, OUTPUT);
  pinMode(PIN_SER, OUTPUT);

  // reset init
  digitalWrite(PIN_RST, LOW);
  digitalWrite(PIN_OE, LOW);
  digitalWrite(PIN_SRCLK, LOW);
  digitalWrite(PIN_RCLK, LOW);
  digitalWrite(PIN_SER, LOW);
  delayMicroseconds(DELAY_LOGIC);

  // clear the reset line of the shift registers
  digitalWrite(PIN_RST, HIGH);
  
  Serial.begin(115200);

  Serial.println("INIT");
}

void loop() {
  Serial.println("LOOP");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  
  while (true) {
    read_serial_commands();
  }
}

