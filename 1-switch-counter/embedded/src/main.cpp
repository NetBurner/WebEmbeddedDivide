// Overwrite main.cpp in a NetBurner project

#include <init.h>
#include <json_lexer.h>
#include <webclient/http_funcs.h>
#include <config_obj.h>

const char *AppName = "SwitchCounter";

//Device specific configurable variables (customize via config system)
config_string gDeviceId{appdata, "My Device", "Device ID"};
config_string gTargetUrl{appdata, "http://change.me.example.com/device.php?resource=switches", "Target URL"};

void UserMain(void *pd)
{
	init(); // Initialize network stack

	WaitForActiveNetwork(TICKS_PER_SECOND * 5);// Wait for DHCP address

	iprintf("Application started\n");

	uint16_t last_dip_switch=0xFFFF; // Use a value that can't be correct as a default

	while(1) {
		uint8_t sw=getdipsw();
		if (sw!=last_dip_switch) {
			ParsedJsonDataSet jsonResponse;
			ParsedJsonDataSet jsonRequest;
			jsonRequest.StartBuilding();
			jsonRequest.Add("device", gDeviceId.c_str());
			jsonRequest.AddArrayStart("switches");
		    jsonRequest.AddArrayElement(((sw>>7) & 1));
		    jsonRequest.AddArrayElement(((sw>>6) & 1));
		    jsonRequest.AddArrayElement(((sw>>5) & 1));
		    jsonRequest.AddArrayElement(((sw>>4) & 1));
		    jsonRequest.AddArrayElement(((sw>>3) & 1));
		    jsonRequest.AddArrayElement(((sw>>2) & 1));
		    jsonRequest.AddArrayElement(((sw>>1) & 1));
		    jsonRequest.AddArrayElement(((sw>>0) & 1));
		    jsonRequest.EndArray();
		    jsonRequest.DoneBuilding();

		    char jsonOutBuffer[200];
		    jsonRequest.PrintObjectToBuffer(jsonOutBuffer, 200, false);

		    printf("Switches changed, POSTing %s to %s ...\r\n", jsonOutBuffer, gTargetUrl.c_str());

			bool result = DoJsonPost(gTargetUrl.c_str(),jsonOutBuffer,jsonResponse,0,TICKS_PER_SECOND*60);
			if (result) {
				printf("The POST was a success:\r\n");
				jsonResponse.PrintObject(true);
			} else {
				 printf("The POST failed\r\n");
			}

			last_dip_switch=sw;
		}
		OSTimeDly(TICKS_PER_SECOND);
	}

}
