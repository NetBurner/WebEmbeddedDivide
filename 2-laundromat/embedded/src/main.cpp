// Overwrite main.cpp in a NetBurner project

#include <init.h>
#include <json_lexer.h>
#include <mqtt/mqtt.h>
#include <config_obj.h>
#include <bsp_devboard.h> // for dev board pins

#if (!PLAT_HAS_FILESYSTEM)
#include "fs_main.h"
#endif

const char *AppName = "Laundromat";

int DEFAULT_DURATION_SEC = 30; // 30 second washer/dryer times for demo purposes

// MQTT client which handles connection/etc via the config system, and pub/subs the below variables
MQTT::ConfiguredClient MqttClient;

uint16_t readWriteFlags = MQTT::eObj_Flag_PubOnWrite|MQTT::eObj_Flag_Subscribe|MQTT::eObj_Flag_Sub_NoLocal|MQTT::eObj_Flag_PubRetain;
uint16_t writeOnlyFlags = MQTT::eObj_Flag_PubOnWrite|MQTT::eObj_Flag_PubRetain;

// MQTT Config variables (customizable via config system or via MQTT)
// Set the MQTT broker URL appropriately via the config system to enable functionality
MQTT::mqtt_int washer1Credits(0,"laundryExample/washer1/credits", NULL, readWriteFlags);
MQTT::mqtt_int washer1Cycle(0,"laundryExample/washer1/cycle", NULL, readWriteFlags);
MQTT::mqtt_int washer1TimeLeft(0,"laundryExample/washer1/timeLeft", NULL, writeOnlyFlags); // remote side can't set
MQTT::mqtt_int washer2Credits(0,"laundryExample/washer2/credits", NULL, readWriteFlags);
MQTT::mqtt_int washer2Cycle(0,"laundryExample/washer2/cycle", NULL, readWriteFlags);
MQTT::mqtt_int washer2TimeLeft(0,"laundryExample/washer2/timeLeft", NULL, writeOnlyFlags); // remote side can't set

MQTT::mqtt_int dryer1Credits(0,"laundryExample/dryer1/credits", NULL, readWriteFlags);
MQTT::mqtt_int dryer1Cycle(0,"laundryExample/dryer1/cycle", NULL, readWriteFlags);
MQTT::mqtt_int dryer1TimeLeft(0,"laundryExample/dryer1/timeLeft", NULL, writeOnlyFlags); // remote side can't set
MQTT::mqtt_int dryer2Credits(0,"laundryExample/dryer2/credits", NULL, readWriteFlags);
MQTT::mqtt_int dryer2Cycle(0,"laundryExample/dryer2/cycle", NULL, readWriteFlags);
MQTT::mqtt_int dryer2TimeLeft(0,"laundryExample/dryer2/timeLeft", NULL, writeOnlyFlags); // remote side can't set

long timeCounters[4] = {0};
bool machineStates[4] = {FALSE};

void handleMachine(int num, MQTT::mqtt_int &credits, MQTT::mqtt_int &cycle, MQTT::mqtt_int &timeLeft) {

	if (num < 0 || num > 3) {
		printf("\r\nInvalid machine num %d", num);
		return;
	}

	long timeCalc = timeCounters[num] - Secs;
	if (timeCalc < 0) timeCalc = 0;
	if (timeLeft != timeCalc) timeLeft = timeCalc; // only set mqtt_ints when needed

	// user has paid and selected a cycle but it's not started yet
	if (credits > 0 && cycle > 0 && machineStates[num] == FALSE) {
		printf("\r\nSetting machine %d to ON", num);
		machineStates[num] = TRUE;
		LEDs[num] = TRUE;
		credits--;
		timeCounters[num] = Secs + DEFAULT_DURATION_SEC;
	}
	// cycle has begun and running but time is expired
	else if (cycle > 0 && timeLeft <= 0 && machineStates[num] == TRUE)
	{
		printf("\r\nSetting machine %d to OFF", num);
		machineStates[num] = FALSE;
		LEDs[num] = FALSE;
		cycle = 0; // reset the current cycle (to "off")
		timeCounters[num] = 0;
	}
	else
	{
		printf("\r\nMachine %d is %s [%d credits, %d cycle, %d time left]", num, machineStates[num] ? "ON" : "OFF", (int)credits, (int)cycle, (int)timeLeft);
	}
}

void UserMain(void *pd)
{
#if (!PLAT_HAS_FILESYSTEM)
    fs_main();     // We use the filesystem to store MQTT cert/key; initialize it if necessary
#endif
	init(); // Initialize network stack
	WaitForActiveNetwork(TICKS_PER_SECOND * 5); // Wait for DHCP address
	EnableSystemDiagnostics(); // Remove in production
	printf("\nApplication started");

    // MqttClient.Connect();

    while (MqttClient.GetConnectionStatus() != MQTT::Client::eConnStat_Established)
    {
        printf("\r\nNotConnected");
        OSTimeDly(TICKS_PER_SECOND*5);
    }

    printf("\r\nConnected.");

	while(1) {
		// LEDs and timeCounters are 0-indexed so we write 0-3 for machines 1-4
		handleMachine(0,washer1Credits,washer1Cycle,washer1TimeLeft); // washer 1: LED 1
		handleMachine(1,washer2Credits,washer2Cycle,washer2TimeLeft); // washer 2: LED 2
		handleMachine(2,dryer1Credits,dryer1Cycle,dryer1TimeLeft); // dryer 1: LED 3
		handleMachine(3,dryer2Credits,dryer2Cycle,dryer2TimeLeft); // dryer 2: LED 4
		printf("\r\n----");
		if (waitchar(TICKS_PER_SECOND*5))
        {
            switch(getchar()){
            	// emulate coins
            	case '0':
            		washer1Credits++;
            		break;
            	case '1':
            		washer2Credits++;
            		break;
            	case '2':
            		dryer1Credits++;
            		break;
            	case '3':
            		dryer2Credits++;
            		break;
            	case 's': // start
            		washer1Cycle = 4;
            		washer2Cycle = 3;
            		dryer1Cycle = 2;
            		dryer2Cycle = 1;
            		break;
            	case 'x': // stop
            		washer1TimeLeft = 0;
            		washer2TimeLeft = 0;
            		dryer1TimeLeft = 0;
            		dryer2TimeLeft = 0;
            		break;
            }
        }
	}

}
