/*
  TMP35GT9Z Example Code
*/

#include <avr/sleep.h>
 
const int xbeeDtr = 6;
const int xbeeReset = 8;
const int onSleep = 2;
const char feedId[] = "";  //paste your feed id.
const char apiKey[] = "";
  //paste your api key.

struct DATA_RECORD
{
  const char *name;
  float val;
  int integer;  //
  int small;  //
} ch[] =
{
  {"Temperature",0.0f,2,1},
  {"AVCC",0.0f,1,3},
  {"MONITOR",0.0f,3,0},
};


// the setup routine runs once when you press reset:
void setup()
{ 
  Serial.begin(9600);  //set xbee baud rate

  /*xbee reset start*/
  pinMode(xbeeReset, OUTPUT);
  digitalWrite(xbeeReset, LOW);
  delay(1);
  pinMode(xbeeReset, INPUT);
  /*xbee reset end*/

  ADCSRA = 0b10000100;
  analogReference(DEFAULT);

  pinMode(xbeeDtr, OUTPUT);
  pinMode(onSleep, INPUT);
  digitalWrite(xbeeDtr, LOW);
  while(digitalRead(onSleep) == LOW) ;
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

//  digitalWrite(xbeeDtr, HIGH);  //the xbee will be sleep.
//  enterSleep();
  delay(10 * 1000UL);
}

// the loop routine runs over and over again forever:
void loop()
{
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0) & 0x03ff;  //dummy read
  sensorValue = analogRead(A0) & 0x03ff;
  int tmp35_I = (int)(((long)sensorValue * 3300L) / 1024) - 250;
  ch[0].val = ((float)tmp35_I / 10.0f) + 25.0f;

  // read the bandgap value:
  ADMUX = 0x40 | 14;  //set bandgap channel
  //dummy read
  ADCSRA |= _BV(ADSC);  //start adc
  loop_until_bit_is_set(ADCSRA,ADIF);  //wait for conversion
  int vgap = ADC & 0x03ff;  //read from adc register
  delay(2);
  //read bandgap data from adc
  ADCSRA |= _BV(ADSC);  //start adc
  loop_until_bit_is_set(ADCSRA,ADIF);  //wait for conversion
  vgap = ADC & 0x03ff;  //read from adc register
  ch[1].val = (1.1f * 1024.0f) / (float)vgap;

  //incremental monitor
  ch[2].val += 1.0f;
  if(ch[2].val > 1000.0f) ch[2].val = 0.0f;

  //make a contents. 
  String s = makeContents(ch, sizeof(ch) / sizeof(ch[0]));
  //make a http header and output. 
  Serial.print( makeHeader(feedId, apiKey, s.length()) );
  //output a contents. 
  Serial.print( s );

  findMsg("HTTP",5000UL);  //wait xively's responce
  while(Serial.available() > 0)
  {
    int dummy = Serial.read();
  }

  digitalWrite(xbeeDtr, HIGH);  //the xbee will be sleep.

  enterSleep();
  //delay( 26 * 1000UL );
}

String makeHeader(const char id[], const char key[], int size)
{
  String header = "PUT /v2/feeds/";
  header += id;
  header += 
    ".json HTTP/1.1\r\n"
    "Host: api.xively.com\r\n"
    "User-Agent: ARDUINO-PRO-MINI/1.0\r\n"
    "Connection: close\r\n"
    "X-ApiKey: ";
  header += key; header += "\r\n";
  header += "Content-Length: ";
  header += size; header += "\r\n\r\n";

  return header;
}

String makeContents(struct DATA_RECORD ch[], int sz)
{
  String contents =
    "{" "\"version\":\"1.0.0\"," "\"datastreams\":[\r\n";

  for( int i = 0; i < sz; i++ )
  {
    contents += "{\"id\":\"";
    contents += ch[i].name;
    contents += "\",\"current_value\":\"";

    //avr-libc's sprintf dose not treate float model.
    char buf[16];
    dtostrf( ch[i].val, ch[i].integer + ch[i].small + 1, ch[i].small, buf );
    contents += buf;
    contents += ((i + 1) == sz) ? "\"}\r\n" : "\"},\r\n";
  }

  contents += "]" "}\r\n";

  return contents;
}

void enterSleep()
{
  attachInterrupt(0,wakeByINT0,RISING);  //enable interrupt 0
  sleep_mode();
}

void wakeByINT0()
{
//  detachInterrupt(0);  //disable interrupt 0
}

boolean findMsg( const char msg[], unsigned long tim )
{
  boolean result = true;
  int len = strlen(msg);
  unsigned long lastTim = millis();
  unsigned long accumulationTim = 0;

  for(int i = 0; len > 0;)
  {
    while(true)
    {
      accumulationTim += millis() - lastTim;
      lastTim = millis();
      if(accumulationTim >= tim)
        return false;

      if(Serial.available() > 0)
        break;
    }
    char c = Serial.read();
    if(msg[i] == c)
    {
      i++;
      len--;
    }
    else
    {
      i = 0;
      len = strlen(msg);
    }
  }
  return result;
}

