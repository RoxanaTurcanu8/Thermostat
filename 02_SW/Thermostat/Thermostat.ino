#include "arduino_secrets.h"


#define DHTPIN 12        // what pin we're connected to
#define DHTTYPE DHT22    // DHT 22 
#define RelayPin 5       // RELAY PIN 

#include "thingProperties.h"
#include <DHT.h>;
DHT dht = DHT (DHTPIN, DHTTYPE); //// Initialize DHT sensor

float settedTemp = 0;          // Stores the temperature that needs to be reached
boolean seasonStatus = false;
xSemaphoreHandle xSemaphore;  // Declare the semaphore
void TaskWeb( void *pvParameters );
void TaskReadTemp( void *pvParameters );
void TaskHeatControl( void *pvParameters );

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(115200);
  pinMode(RelayPin,OUTPUT);
  dht.begin();
  // Defined in thingProperties.h
  initProperties();
  
   xSemaphore = xSemaphoreCreateBinary();


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
    xSemaphoreGive(xSemaphore);
}

void loop() {
  
  // Your code here 
}

void TaskWeb(void *pvParameters)  // First Task
{
  (void) pvParameters;
  for (;;) // A Task shall never return or exit.
  {
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    Serial.println("Start WEB service");
    ArduinoCloud.update();
    xSemaphoreGive(xSemaphore);
    vTaskDelay(100 / portTICK_PERIOD_MS ); //delay of 100ms
  }
}

void TaskReadTemp(void *pvParameters)  // Second Task
{
  (void) pvParameters;
  for (;;) // A Task shall never return or exit.
  {
    getTemp();
    vTaskDelay(100 / portTICK_PERIOD_MS ); //delay of 100ms
  }
}

//// Read data and store it to variable currentTemp ////
void getTemp()
{
   float readedTemp = dht.readTemperature();
    // Check if any reads failed and exit early (to try again):
    if (isnan(readedTemp)) 
    {
      return;
    }  
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    currentTemperature = readedTemp;
    //Print temperature value to serial monitor
    Serial.print("Temp: ");
    Serial.print(currentTemp);
    Serial.println(" Celsius");
    xSemaphoreGive(xSemaphore);
}


void TaskHeatControl(void *pvParameters)  // Third Task
{
  (void) pvParameters;
  for (;;) // A Task shall never return or exit.
  {
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    if(seasonStatus)  // If the thermostat is set on winter mode start the heating
    {
       xSemaphoreGive(xSemaphore);
       startHeating();             // turn on the heating
    }
    else 
    {
      xSemaphoreGive(xSemaphore);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS ); //delay of 100ms
  }
}

void startHeating(){
  xSemaphoreTake(xSemaphore, portMAX_DELAY);
  if(currentTemperature < settedTemp))
  {
    digitalWrite(RelayPin,HIGH);
    Serial.println("HEATING ON");
    isActive = true;
    xSemaphoreGive(xSemaphore);
  }
  else
  {
    digitalWrite(RelayPin,LOW);
    isActive = false;
    Serial.println("HEATING OFF");
    xSemaphoreGive(xSemaphore);
  }
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
