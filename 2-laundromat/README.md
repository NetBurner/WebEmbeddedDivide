## Requirements

Decide on an MQTT server and set it up. The default "test.mosquitto.org" server
with no username/password may work, however it should not be considered reliable
or used outside of development tests. Mosquitto.org is sometimes overloaded, so
if it doesn't work consider setting up your own test MQTT server: my personal
server is set up through HomeAssistant, and I tend to use AWS IoT Core or Azure
Event Grid MQTT for work projects.

## Setup

Edit `wwwroot/env.dist` and save as `wwwroot/.env`

Note that Docker will need to be rebuilt after any file changes.

## Running Web Portion

### Directly

- npm install
- npm start

### Via Docker

- docker build . -t laundromat
- docker run -p 8000:8000 -it laundromat

Rebuild and re-run if you make any source code or configuration changes.

## Running Embedded portion

Copy-paste into a NBEclipse project or install the NNDK and run `make load -e DEVIP=192.168.1.100`
(changing the IP to match your NetBurner device)

> To find your device's IP, you can visit https://discover.netburner.com .

> If you encounter error messages about missing header files, ensure that your other environment variables like NNDK_ROOT and PLATFORM are set properly. You can always provide them on the command line like `make load -e PLATFORM=SOMRT1061 -e DEVIP=192.168.1.100`

## Final steps

1. Configure your NetBurner device by going to i.e. http://192.168.1.100:20034 and
setting the following options under MqttClient:

	- ClientId (i.e. `laundromat-test`)
	- Hostname (i.e. `test.mosquitto.org` or the IP/Domain of your MQTT server)
	- Password (password of your laundromat-test MQTT user, if any)
	- Port (default 1883 for unencrypted, or 8883 for encrypted)
	- TLS_Enabled (check if encrypted)
	- Username (i.e. `laundromat-test`, if any)

	Note some MQTT servers ("brokers") will disconnect clients with duplicated
	usernames and/or client IDs, so if you have issues with immediate disconnections
	try making a separate username (and client ID) for each client. The web and embedded
	portions of this project each count as one client.

2. Click Update Record to save. Reboot your NetBurner device so the settings take effect.
3. Connect a serial console to your NetBurner device and notice the output:

        Primary network interface configured for DHCP
        MAC Address = 00:03:f4:12:34:56
        Type "A" to abort boot

        Application started
        NotConnected
        NotConnected
        Connected.
        Machine 0 is OFF [0 credits, 0 cycle, 0 time left]
        Machine 1 is OFF [0 credits, 0 cycle, 0 time left]
        Machine 2 is OFF [0 credits, 0 cycle, 0 time left]
        Machine 3 is OFF [0 credits, 0 cycle, 0 time left]

  You can press 0, 1, 2, or 3 to add "credits" to a machine, and S to start the machine(s). The status will be sent up via MQTT to the web app.

4. Visit the webpage of your web app: http://localhost:8000
  - If this doesn't work, double check your Docker networking settings and firewalls.
  - You can also run the project directly with NodeJS via npm.
  - The port number should appear in your system's "listening ports" output.
    - You can try `netstat -an | grep 8000` on Linux, Mac, and Windows Powershell.

  You can select a cycle and press Start to start a machine, and the status/request will be sent via MQTT to the NetBurner.