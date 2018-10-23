#include<SoftwareSerial.h>
     
    SoftwareSerial esp8266(2,3); // make RX Arduino line is pin 2, make TX Arduino line is pin 3.
                                 // This means that you need to connect the TX line from the esp to the Arduino's pin 2
                                 // and the RX line from the esp to the Arduino's pin 3
#define SSID "Csg"
#define PASSWORD "csgocsgo"
#define IP "http://api.thingspeak.com"
String GET = "GET https://api.thingspeak.com/update?api_key=SCWCUEW1RWEYP7NE&field1=0";
#define SensorPin 0          //pH meter Analog output to Arduino Analog Input 0
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;

int photocellPin = A1;    //connect cell to analog pin 0 with 10k pull down resistor to ground
int ledPin = 13;  //Connect LED to run pin 13 in series with appropriate resistor
String apiKey="SCWCUEW1RWEYP7NE";     // Enter your Write API key from ThingSpeak


void setup() {
 Serial.begin(9600);
      esp8266.begin(9600); // your esp's baud rate might be different
  
  Serial.begin(9600);  //Begin serial connection between Arduino and PC
  pinMode (ledPin, OUTPUT);  //Set pin to outut
 pinMode (photocellPin, INPUT);

  pinMode(13,OUTPUT);  
  Serial.begin(9600);  
  Serial.println("Ready");    //Test the serial monitor
sendDebug("AT");
  delay(5000);
  if(Serial.find("OK"))
  {
    esp8266.println("RECEIVED: OK\nData ready to sent!");
    connectWiFi();
  }
//initEsp();
}

void loop() {
   if(esp8266.available()) // check if the esp is sending a message 
      {
        while(esp8266.available())
        {
          // The esp has data so display its output to the serial window 
          char c = esp8266.read(); // read the next character.
          Serial.write(c);
        }  
        }
      
     
      
      if(Serial.available())
      {
        // the following delay is required because otherwise the arduino will read the first letter of the command but not the rest
        // In other words without the delay if you use AT+RST, for example, the Arduino will read the letter A send it, then read the rest and send it
        // but we want to send everything at the same time.
        delay(1000); 
        
        String command="";
        
        while(Serial.available()) // read the command character by character
        {
            // read one character
          command+=(char)Serial.read();
        }
        esp8266.println(command); // send the read character to the esp8266
      }
 //Serial.print("Cell = "); 
 //Serial.println(analogRead(photocellPin));  //print analog read to serial monitor on PC

float val = analogRead(photocellPin);  //create variable to take in analog reading from cell
//Serial.print("val ");
//Serial.println(val);
digitalWrite(13, LOW);
float ardval = val*0.00488758553;  //arduino value units 
//Serial.print("ardval ");
//Serial.println(ardval);
float r1 = (50000/ardval)-10000; //R1 value when using Ohm's law
//Serial.print("r1 ");
//Serial.println(r1);
float I = ardval/r1; //value of I which we are solving for
float NTUval = I*70000;  //200 = units in NTU
//Serial.print("I ");
//Serial.println(I);

Serial.print("NTUval for Turbidity: ");
Serial.print(NTUval);
Serial.println(" ");
delay(1000);

 for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(SensorPin);
    delay(10);
  }
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue=3.5*phValue;                      //convert the millivolt into pH value
  Serial.print("pH value: ");  
  Serial.print(phValue,2);
  Serial.println(" ");
  digitalWrite(13, HIGH);       
  delay(800);
  digitalWrite(13, LOW); 
    String NTU =String(NTUval);// turn integer to string
  String PH= String(phValue);// turn integer to string

updateTS(NTU,PH);
}

void updateTS( String N, String P )
{
  // ESP8266 Client
  String cmd = "AT+CIPSTART=\"TCP\",\"";// Setup TCP connection
  cmd += IP;
  cmd += "\",80";
  sendDebug(cmd);
  delay(2000);
  if( Serial.find( "Error" ) )
  {
    esp8266.print( "RECEIVED: Error\nExit1" );
    return;
  }

  cmd = GET + "&field1=" + N +"&field2="+ P +"\r\n";
  Serial.print( "AT+CIPSEND=" );
  Serial.println( cmd.length() );
  if(Serial.find( ">" ) )
  {
    esp8266.print(">");
    esp8266.print(cmd);
    Serial.print(cmd);
  }
  else
  {
    sendDebug( "AT+CIPCLOSE" );//close TCP connection
  }
  if( Serial.find("OK") )
  {
    esp8266.println( "RECEIVED: OK" );
  }
  else
  {
    esp8266.println( "RECEIVED: Error\nExit2" );
  }
}

void sendDebug(String cmd)
{
  esp8266.print("SEND: ");
  esp8266.println(cmd);
  Serial.println(cmd);
}

boolean connectWiFi()
{
  Serial.println("AT+CWMODE=1");//WiFi STA mode - if '3' it is both client and AP
  delay(2000);
  //Connect to Router with AT+CWJAP="SSID","Password";
  // Check if connected with AT+CWJAP?
  String cmd="AT+CWJAP=\""; // Join accespoint
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASSWORD;
  cmd+="\"";
  sendDebug(cmd);
  delay(5000);
  if(Serial.find("OK"))
  {
    esp8266.println("RECEIVED: OK");
    return true;
  }
  else
  {
    esp8266.println("RECEIVED: Error");
    return false;
  }

  cmd = "AT+CIPMUX=0";// Set Single connection
  sendDebug( cmd );
  if( Serial.find( "Error") )
  {
    esp8266.print( "RECEIVED: Error" );
    return false;
  }
}

