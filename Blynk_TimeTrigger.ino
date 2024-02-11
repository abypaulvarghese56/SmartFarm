/*************************************************************

  This is a simple demo of sending and receiving some data.
  Be sure to check out other examples!
 *************************************************************/

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "<insert Blynk Template>"
#define BLYNK_TEMPLATE_NAME         "Quickstart Template"
#define BLYNK_AUTH_TOKEN            "<insert BLYNK AUTH Token from BLYNK>"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <esp_task_wdt.h>


#define BLYNK_FIRMWARE_VERSION        "0.1.1"

//Watchdog Timer Delay- 5 minutes.
#define WDT_TIMEOUT 300

#ifdef __cplusplus
  extern "C" {
 #endif

  uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

uint64_t uS_TO_S_FACTOR =1000000ull;
uint64_t TIME_TO_SLEEP = 3600*3;

uint64_t SLEEP_INTERVAL = TIME_TO_SLEEP * uS_TO_S_FACTOR; 
uint64_t SLEEP_TIMER_INTERVAL = TIME_TO_SLEEP * 1000;
uint8_t temprature_sens_read();

IPAddress local_IP(192, 168, 1, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress dns(192, 168, 1, 1);

// Your WiFi credentials.
// Set password to "" for open networks.
//char ssid[] = "Airel_Aby";
char ssid[] = "<Your_wifi_ssid>";
char pass[] = "<Your wifi password>";
int LED_BUILTIN = 2;
int RELAY_PIN = 23 ;
int FAN_PIN = 22;
int BUZZER_PIN = 34;

int internetConnectedState  =0;
int PreviousMinimumFreeMemory = 0;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;

//V2- CPU Temp
//V4- Last System updated Time
//V5 - Last Relay activatted Time
//V7-  Last Relay De activated Time
//V6 - System Operation Status
//V8- Free Memory since boot
//V9 - Get Wifi Signal strength

BlynkTimer timer;
BlynkTimer timerFAN;


// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable
  int value = param.asInt();

  // Update state
  Blynk.virtualWrite(V1, value);
  //digitalWrite(LED_BUILTIN, value);
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}


void printLocalTime()
{
  time_t rawtime;
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
   return;
  }
  char timeStringBuff[50]; //50 chars should be enough
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  //print like "const char*"
  Serial.println(timeStringBuff);

  //Optional: Construct String object 
  String LastUpdatedtime(timeStringBuff);
  if (connectToBlynk() )
  Blynk.virtualWrite(V4 , LastUpdatedtime); 
  Blynk.run();
}
void myTimerEventForFAN()
{
      
      Serial.println("Fan event");

}
void UpdateWifiSignal()
{
  
  if (connectToBlynk() )
  {
    Blynk.virtualWrite(V9 , WiFi.RSSI()); 
    Blynk.run();
  }
  
}
// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  //Blynk.virtualWrite(V2, millis() / 1000);
 // Blynk.virtualWrite(V4, 1);
  if( (WiFi.status() != WL_CONNECTED) )
  {
    connectToWifi();
  }
  printLocalTime();
  struct tm timeinfo,timeinfonew;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  int hour = timeinfo.tm_hour;
  int minute= timeinfo.tm_min;
  int sec= timeinfo.tm_sec;
  Serial.print("Hour:");
  Serial.println(hour);

  Serial.print("Minute:");
  Serial.println(minute);

  
  Serial.print("Temperature: ");
  
  // Convert raw temperature in F to Celsius degrees
  float cpu_temp = (temprature_sens_read() - 32) / 1.8 ;
  Serial.print(cpu_temp);
  
  Serial.println(" C");
  if(Blynk.connected())
  Blynk.virtualWrite(V2, cpu_temp);
  
  if ((hour > 5 ) && (hour < 22))
  {
  Serial.println("Enabling the Relay Trigger");
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(BUZZER_PIN,HIGH);
  if(connectToBlynk())
  Blynk.virtualWrite(V6, "Watered");
  Blynk.run();

  char timeStringBuff[50]; //50 chars should be enough
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  //print like "const char*"
  Serial.println(timeStringBuff);

   
  delay(180000);
  if(!getLocalTime(&timeinfonew)){
    Serial.println("Failed to obtain time");
    //return;
  }
   char timeStringBuffStopTime[50]; //50 chars should be enough
   strftime(timeStringBuffStopTime, sizeof(timeStringBuffStopTime), "%A, %B %d %Y %H:%M:%S", &timeinfonew);
  //print like "const char*"
  Serial.println(timeStringBuffStopTime);

    Serial.print("Watered Duration:");
    int WateringDuration = timeinfonew.tm_min-timeinfo.tm_min;
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(BUZZER_PIN,LOW);

     if(WiFi.status() != WL_CONNECTED)
     connectToWifi();
   
    if(WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Connected to Internet");

      if (connectToBlynk() > 0)
      {
        Serial.println("Connected to Blynk");
      //while (Blynk.connect() == false) {}
      Blynk.run();
      Blynk.virtualWrite(V5, timeStringBuff);
      Blynk.virtualWrite(V7, timeStringBuffStopTime);
      Serial.println("Values updated");
      Blynk.run();
      
      }
      else
      {
        Serial.println("Not connected to Blynk");
  
      }
    
    //Blynk.virtualWrite(V7, WateringDuration);

    }
    else
    {
      Serial.println("Not connected to Internet");

    }
     
    Serial.println(WateringDuration);
 
    }
  else
  {
    if(Blynk.connected())
    Blynk.virtualWrite(V6, "Exception Hours");
  
  }

//  int Delta = 0;
//  if(PreviousMinimumFreeMemory >0)
//  Delta=PreviousMinimumFreeMemory- ESP.getMinFreeHeap();
//  else
//  Delta = ESP.getMinFreeHeap();
  
  PreviousMinimumFreeMemory = ESP.getMinFreeHeap();
  
  Blynk.virtualWrite(V8, (PreviousMinimumFreeMemory));

  Serial.println("Done.");
  //while (Blynk.connect() == false) {}
  Blynk.run();

  delay(20000);
  //esp_deep_sleep_start();
}
int connectToWifi()
{
  WiFi.begin(ssid, pass);
  int number_retry = 0;
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print('.');
    delay(1000);
    number_retry+=1;
    if(number_retry >=10)
    
   {
       Serial.println("Did not find internet hence resuming without internet.");
       internetConnectedState =0;
      return internetConnectedState;
   }
  }
  internetConnectedState = 1;
  Serial.println("Connected to Internet..");
  return internetConnectedState;
  
  }
void setup()
{
  // Debug console
  Serial.begin(115200);
  
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  pinMode (LED_BUILTIN, OUTPUT);
  pinMode (RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  pinMode (FAN_PIN, OUTPUT);
  
  digitalWrite(RELAY_PIN, LOW);
  
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  if (!WiFi.config(local_IP, gateway, subnet,dns,dns )) {
    Serial.println("STA Failed to configure");
  }
  connectToWifi();
  if(WiFi.status() == WL_CONNECTED)
  Blynk.config(BLYNK_AUTH_TOKEN);
  // You can also specify server:
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);
   
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  //while (Blynk.connect() == false) {}
  
  printLocalTime();
  if(connectToBlynk())
  Blynk.virtualWrite(V6, "Device Started");
  Blynk.run();
  delay(5000);
  // Setup a function to be called every second
  //timer.setInterval(150000L, myTimerEvent);
  timer.setInterval(SLEEP_TIMER_INTERVAL, myTimerEvent);
  timerFAN.setInterval(1000*300, myTimerEventForFAN);
  myTimerEvent();
  
 
  //esp_sleep_enable_timer_wakeup(SLEEP_INTERVAL);
  

}

int connectToBlynk()
{
  int numberofRetries =0;
  if(WiFi.status() != WL_CONNECTED)
  {
    if (connectToWifi() < 1 )
      {
       return 0;
      }
   
  }

  while (Blynk.connect() == false) 
   {
          numberofRetries++;
            if (numberofRetries >=10)
            {
               Serial.println("Connection to Blynk FAILED..");

              return 0;
            }
    }
      
  if(Blynk.connected())
       {
          Serial.println("Connected to Blynk..");

          return 1; 
       } 
       
  return 0;
}

void loop()
{
  Blynk.run();
  timer.run();
  timerFAN.run();

  esp_task_wdt_reset();

  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
    

}
