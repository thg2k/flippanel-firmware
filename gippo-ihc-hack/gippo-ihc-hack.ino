//all pins inverted
#define CCLK 7
#define CDAT 8
#define CRES 9


#define RST 2 //reset
#define SRCLK 3
#define RCLK 4
#define OE 5
#define SER 6

#define DelLogic 0
#define DelPx 10

#define DISPLAY_COLS 90
#define DISPLAY_ROWS 24
#define DISPLAY_ROWS_BLKS 3

int f = 0;
byte Regs[6];

byte buff[90][3];

#define Cnum 8


int read_serial_byte() {
  while (!Serial.available()) { }
  
  int c = Serial.read();
  //Serial.print("GOT=");
  //Serial.println(c, HEX);
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


inline void led_on() {
  digitalWrite(LED_BUILTIN, HIGH);
}

inline void led_off() {
  digitalWrite(LED_BUILTIN, LOW);
}

void read_serial_commands() {
  led_on();
  Serial.println("CMD?");

  char ch = read_serial_byte();
  led_off();

  if (ch == 'd') {
        for (int ii = 0; ii < DISPLAY_COLS; ii++) {
          for (int jj = 0; jj < DISPLAY_ROWS_BLKS; jj++) {
            buff[ii][jj] = 0;
      }
    }
        
    // read 570 hex characters (0-9A-F)
    Serial.println("READING BYTES");
    bool all_ok = true;
    for (int ii = 0; ii < DISPLAY_COLS; ii++) {
      for (int jj = 0; jj < DISPLAY_ROWS_BLKS; jj++) {
        int c = read_serial_hex();
        if (c < 0)
          all_ok = false;
        else
          buff[ii][jj] = (byte) c;
        
        if (!all_ok)
          break;
  
        //led_on();
        //delay(50);
        //led_off();
        //delay(50);
      }
      
      if (!all_ok)
        break;
    };

    Serial.println("ACK");

    Cpush();
    //draw_buffer_differential();
    Serial.println("DONE");
  }

  if (ch == 'c') {
    // clear
    //draw_buffer();
                for (int ii = 0; ii < DISPLAY_COLS; ii++) {
      for (int jj = 0; jj < DISPLAY_ROWS_BLKS; jj++) {
buff[ii][jj] = 0x00;
      }
        }
Cpush();

  }

  if (ch == 'f') {
    //fill
            for (int ii = 0; ii < DISPLAY_COLS; ii++) {
      for (int jj = 0; jj < DISPLAY_ROWS_BLKS; jj++) {
buff[ii][jj] = 0xff;
      }
        }
Cpush();
    Serial.println("EXEC: FILL");
  }

  if (ch == '1') {
    Serial.println("EXEC: test_basic_1");
    //test_basic_1();
  }
}


void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CCLK, OUTPUT);
  pinMode(CDAT, OUTPUT);
  pinMode(CRES, OUTPUT);

  InitRows();
  Serial.begin(9600);

  buff[0][0] = 0xF8;
  buff[0][1] = 0x81;
  buff[0][2] = 0x1F;
  buff[1][0] = 0x04;
  buff[1][1] = 0x42;
  buff[1][2] = 0x20;
  buff[2][0] = 0x02;
  buff[2][1] = 0x24;
  buff[2][2] = 0x40;
  buff[3][0] = 0x01;
  buff[3][1] = 0x18;
  buff[3][2] = 0x80;
  buff[4][0] = 0x01;
  buff[4][1] = 0x18;
  buff[4][2] = 0x80;
  buff[5][0] = 0x01;
  buff[5][1] = 0x18;
  buff[5][2] = 0x80;
  buff[6][0] = 0x01;
  buff[6][1] = 0x18;
  buff[6][2] = 0x80;
  buff[7][0] = 0x01;
  buff[7][1] = 0x18;
  buff[7][2] = 0x80;

  buff[8][0] = 0x02;
  buff[8][1] = 0x24;
  buff[8][2] = 0x40;
  buff[9][0] = 0x04;
  buff[9][1] = 0x42;
  buff[9][2] = 0x20;
  buff[10][0] = 0xF8;
  buff[10][1] = 0x81;
  buff[10][2] = 0x1F;

  buff[11][0] = 0x40;
  buff[11][2] = 0x02;
  for(int k=12; k < 81;k++)
  {  
    buff[k][0] = 0x40;  
    buff[k][2] = 0x02;
  }
  buff[81][0] = 0x80;
  buff[81][2] = 0x01;
  buff[82][1] = 0x81;
  buff[83][1] = 0x42;
  buff[84][1] = 0x3C;
  
  
 // for(int i = 0; i < 90; i++)
 //   for(int j = 0; j < 3; j++)
 //     buff[i][j] = 0xFF;
      
  
}

void loop() {

  
  //reset-init
  digitalWrite(CCLK, HIGH);
  digitalWrite(CDAT, HIGH);
  digitalWrite(CRES, HIGH);
  
  delay(10);
  digitalWrite(CRES, LOW);

  //for(byte j = 0; j < 8; j++)
  //  Regs[j]=0x05;
    
//  push();
  //digitalWrite(OE, HIGH);
   //while(Serial.read() != '\n');
    digitalWrite(CRES, HIGH);
    delay(100);
    digitalWrite(CRES, LOW);

  
  digitalWrite(LED_BUILTIN, HIGH);
  int i = 0;
Serial.println("pre cpush");

  while (true) {
    read_serial_commands();
  }

//  Cpush();
  
  while(1){
    

    //Regs[7] = i<<4;
    //Serial.println(i);
    //Cpush();
    //i+;
  };
}

void Cpush()
{
  byte t;

  digitalWrite(CDAT, LOW);

  for(byte j = 0; j < 180; j++)
  {
     // while (Serial.read() != '\n') ;
      Serial.println("next");
      digitalWrite(CCLK, LOW);
      delay(DelLogic);
      digitalWrite(CCLK, HIGH);
      delay(DelLogic);


      if ((j % 2) == 0)
         PC(buff[j/2][0], buff[j/2][1], buff[j/2][2], 1);
      else
         PC(buff[j/2][0], buff[j/2][1], buff[j/2][2], 0);
      digitalWrite(CDAT, HIGH);

      digitalWrite(CCLK, LOW);
      delay(DelLogic);
      digitalWrite(CCLK, HIGH);
      delay(DelLogic);

  }
}



void InitRows() {

  
  pinMode(RST, OUTPUT);
  pinMode(SRCLK, OUTPUT);
  pinMode(RCLK, OUTPUT);
  pinMode(OE, OUTPUT);
  pinMode(SER, OUTPUT);
  
  //reset-init
  digitalWrite(RST, LOW);
  digitalWrite(OE, LOW);
  digitalWrite(SRCLK, LOW);
  digitalWrite(RCLK, LOW);
  digitalWrite(SER, LOW);
  
  delay(10);
  digitalWrite(RST, HIGH);
}

void PC(byte R0, byte R1, byte R2, byte color)
{
  if (!color) {
    Regs[0] = ~R0;
    Regs[1] = ~R1;
    Regs[2] = ~R2;
    Regs[3] = 0;
    Regs[4] = 0;
    Regs[5] = 0;
  }
  else {
    Regs[0] = 0;
    Regs[1] = 0;
    Regs[2] = 0;
    Regs[3] = R0;
    Regs[4] = R1;
    Regs[5] = R2;
  }
  Rpush();
}

void Rpush()
{
  byte t;
  for(byte i = 0; i < 6; i++)
  {
    t = Regs[i];
    for(byte j = 0; j < 8; j++)
    {
      //digitalWrite(SER, HIGH);
      if(t & 1) digitalWrite(SER, HIGH);
      else  digitalWrite(SER, LOW);
      t = t >> 1;
      digitalWrite(SRCLK, HIGH);
      delay(DelLogic);
      digitalWrite(SRCLK, LOW);
      delay(DelLogic);
    }
  }
      digitalWrite(RCLK, HIGH);
      delay(DelLogic);
      digitalWrite(RCLK, LOW);
      
      delay(DelPx); //pixel set time

      digitalWrite(RST, LOW);
      delay(DelLogic);
      digitalWrite(RST, HIGH);
      delay(DelLogic);

      digitalWrite(RCLK, HIGH);
      delay(DelLogic);
      digitalWrite(RCLK, LOW);
      delay(DelLogic);      
}


