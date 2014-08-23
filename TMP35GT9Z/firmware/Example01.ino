/*
  TMP35GT9Z Example Code
*/
 
// Pin 13 has an LED connected on most Arduino boards.
const int led = 13;
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
  {"Temperature",12.3f,2,1},
};


// the setup routine runs once when you press reset:
void setup()
{ 
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);     
  Serial.begin(9600);  //set xbee baud rate

  /*xbee reset start*/
  pinMode(xbeeReset, OUTPUT);
  digitalWrite(xbeeReset, LOW);
  delay(1);
  pinMode(xbeeReset, INPUT);
  /*xbee reset end*/

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

  //make a contents. 
  String s = makeContents(ch, sizeof(ch) / sizeof(ch[0]));
  //make a http header and output. 
  Serial.print( makeHeader(feedId, apiKey, s.length()) );
  //output a contents. 
  Serial.print( s );

  for(int i = 0; i < 30 * 2; i++ )
  {
    digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(250);               // wait for a second
    digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
    delay(250);               // wait for a second
  }
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

