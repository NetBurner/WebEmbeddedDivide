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

int DEFAULT_DURATION = 15; // 15 second washer/dryer times for demo purposes

// MQTT client which handles connection/etc via the config system, and pub/subs the below variables
MQTT::ConfiguredClient MqttClient;

// MQTT Config variables (customizable via config system or via MQTT)
// Set the MQTT broker URL appropriately via the config system to enable functionality
MQTT::mqtt_int washer1Credits(0,"washer1/credits");
MQTT::mqtt_int washer1Cycle(0,"washer1/cycle");
MQTT::mqtt_int washer1TimeLeft(0,"washer1/timeLeft");
MQTT::mqtt_int washer2Credits(0,"washer2/credits");
MQTT::mqtt_int washer2Cycle(0,"washer2/cycle");
MQTT::mqtt_int washer2TimeLeft(0,"washer2/timeLeft");

MQTT::mqtt_int dryer1Credits(0,"dryer1/credits");
MQTT::mqtt_int dryer1Cycle(0,"dryer1/cycle");
MQTT::mqtt_int dryer1TimeLeft(0,"dryer1/timeLeft");
MQTT::mqtt_int dryer2Credits(0,"dryer2/credits");
MQTT::mqtt_int dryer2Cycle(0,"dryer2/cycle");
MQTT::mqtt_int dryer2TimeLeft(0,"dryer2/timeLeft");

void setMachine(int num, bool val) {
	if (num == 1) {
		LEDs[0] = val;
	} else if (num == 2) {
		LEDs[1] = val;
	} else if (num == 3) {
		LEDs[2] = val;
	} else if (num == 4) {
		LEDs[3] = val;
	} else {
		printf("Invalid Machine Num %d\r\n", num);
		return;
	}
	printf("Set Machine %d to %s\r\n", num, (val ? "ON" : "OFF"));
}

void handleMachine(int num, MQTT::mqtt_int &credits, MQTT::mqtt_int &cycle, MQTT::mqtt_int &timeLeft) {
	if (credits > 0 && cycle > 0) { // user has paid and selected a cycle
		setMachine(num, TRUE); // ON
		credits--;
		timeLeft = DEFAULT_DURATION;
	} else if (cycle > 0 && timeLeft < 0) { // cycle has begun but time is more than expired
		setMachine(num, FALSE); // OFF
		cycle = 0; // reset the current cycle (to "off")
		timeLeft = 0; // we'll use 0 as a neutral "off" state, letting it dip negative when ending
	} else {
		printf("Machine %d is %d credits %d cycle %d time left\r\n", num, (int)credits, (int)cycle, (int)timeLeft);
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
	printf("Application started\n");

    //MqttClient.Connect();

    while (MqttClient.GetConnectionStatus() != MQTT::Client::eConnStat_Established)
    {
        printf("NotConnected\r\n");
        OSTimeDly(TICKS_PER_SECOND*5);
    }

    printf("Connected.\r\n");

	while(1) {
		handleMachine(1,washer1Credits,washer1Cycle,washer1TimeLeft); // washer 1: LED 1
		handleMachine(2,washer2Credits,washer2Cycle,washer2TimeLeft); // washer 2: LED 2
		handleMachine(3,dryer1Credits,dryer1Cycle,dryer1TimeLeft); // dryer 1: LED 3
		handleMachine(4,dryer2Credits,dryer2Cycle,dryer2TimeLeft); // dryer 2: LED 4
		printf("----\r\n");
		if (waitchar(TICKS_PER_SECOND*5))
        {
            switch(getchar()){
            	// emulate coins
            	case '1':
            		washer1Credits++;
            		break;
            	case '2':
            		washer2Credits++;
            		break;
            	case '3':
            		dryer1Credits++;
            		break;
            	case '4':
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
