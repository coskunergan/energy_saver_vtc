/*
    Examples

    Created on: May 12, 2022

    Author: Coskun ERGAN
*/

#ifdef ESP32
#define LED_PIN  3
#elif defined(ESP8266)
#endif

#define SET 1
#define RESET 0
#define NUMBER_OF_ZONE 4

int LedState = 1;
long lastTime = 0;

unsigned int Total_Power_Limit = 3600;
unsigned char  Zone_Step[NUMBER_OF_ZONE]={7,7,7,7};
unsigned char  Zone_RunTime[NUMBER_OF_ZONE] = {0,0,0,0};

#define DELAY_PERIOD 100

/*****************************************************************/
void setup()
{
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(115200);
    Serial.println("Restart!");
}
/*****************************************************************/
void loop()
{
    if(LedState)
    {
        digitalWrite(LED_PIN, LOW);
    }
    else
    {
        digitalWrite(LED_PIN, HIGH);
    }

    if((millis() - lastTime) > DELAY_PERIOD)
    {
        //LedState = !LedState;
        Run_Procces();
        Vtc_Procces();
        lastTime = millis();
    }
}
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
