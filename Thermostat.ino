#include "arduino_secrets.h"


#define DHTPIN 12        // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22 
#define RelayPin 0       // DEFINE RELAY PIN ! 

#include "thingProperties.h"
#include <DHT.h>;
DHT dht = DHT (DHTPIN, DHTTYPE); //// Initialize DHT sensor

float currentTemp;    // Stores current temperature value
float settedTemp = 0;     // Stores the temperature that needs to rich
boolean heatingStatus = false;
boolean seasonStatus = false;

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(115200);
  pinMode(RelayPin,OUTPUT);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  vTaskDelay(1500); 
  dht.begin();
  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
     //Create aditional Tasks for Core 0
    xTaskCreatePinnedToCore(
        TaskWeb,     /* Task function.                            */
        "TaskWeb",   /* name of task.                             */
        1024 * 7,    /* Stack size of task                        */
        NULL,        /* parameter of the task                     */
        1,           /* priority of the task                      */
        NULL,        /* Task handle to keep track of created task */
        0            /* Core                                      */
    );

    xTaskCreatePinnedToCore(
        TaskReadTemp,  /* Task function.                            */
        "TaskReadTemp",/* name of task.                             */
        1024 * 7,      /* Stack size of task                        */
        NULL,          /* parameter of the task                     */
        1,             /* priority of the task                      */
        NULL,          /* Task handle to keep track of created task */
        0              /* Core                                      */
    );
    
    xTaskCreatePinnedToCore(
        TaskHeatControl,   /* Task function.                            */
        "TaskHeatControl", /* name of task.                             */
        1024 * 7,          /* Stack size of task                        */
        NULL,              /* parameter of the task                     */
        1,                 /* priority of the task                      */
        NULL,              /* Task handle to keep track of created task */
        0                  /* Core                                      */
    );
//  setDebugMessageLevel(2);
//  ArduinoCloud.printDebugInfo();
}

void loop() {
  
  // Your code here 
}

void TaskWeb(void *pvParameters)  // First Task
{
  (void) pvParameters;
  for (;;) // A Task shall never return or exit.
  {
    vTaskDelay(10);
    isActive = heatingStatus;
    currentTemperature = currentTemp;
    ArduinoCloud.update();
    
  }
}

void TaskReadTemp(void *pvParameters)  // Second Task
{
  (void) pvParameters;
  for (;;) // A Task shall never return or exit.
  {
    vTaskDelay(10);
    Serial.println("READING TEMPERATURE SENSOR VALUE");
    getTemp();
  }
}

//// Read data and store it to variable currentTemp ////
void getTemp()
{
   float readedTemp = dht.readTemperature();
    // Check if any reads failed and exit early (to try again):
    if ( isnan(readedTemp)) {
      return;
     // currentTemp = 10;
    } 
    currentTemp = readedTemp;
    //Print temperature value to serial monitor
    Serial.print("Temp: ");
    Serial.print(currentTemp);
    Serial.println(" Celsius");
    
}


void TaskHeatControl(void *pvParameters)  // Third Task
{
  (void) pvParameters;
  for (;;) // A Task shall never return or exit.
  {
    vTaskDelay(1000);
    Serial.println("Check if HEATING is needed");
    if((currentTemperature < settedTemp) && seasonStatus)  // If current temperature is lower then the temperature setted
       startHeating();             // turn on heating
  }
}

void startHeating(){
  while((currentTemperature < settedTemp) && seasonStatus)
  {
    //digitalWrite(RelayPin,HIGH);
    Serial.println("HEATING ON");
    heatingStatus = true;
    vTaskDelay(1000);
  }
  //digitalWrite(RelayPin,LOW);
  heatingStatus = false;
  Serial.println("HEATING OFF");
}


/*
  Since SetTemperature is READ_WRITE variable, onSetTemperatureChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onSetTemperatureChange()  {
  settedTemp = setTemperature;
  Serial.print(" Setted temperature: ");Serial.println(settedTemp);
}

void onSeasonBtnChange()  {
  // Add your code here to act upon SeasonBtn change
  seasonStatus = seasonBtn;
}
