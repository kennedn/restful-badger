# restful-badger
Interface for interacting with RESTful APIs on the Badger 2040 W


# Building

The following definitions are required to build the project:

| Definition          | Description                                    |
|---------------------|------------------------------------------------|
| WIFI_SSID           | Name of wifi network to join                   |
| WIFI_PASSWORD       | Password of wifi network to join               |
| WIFI_BSSID          | BSSID of wifi network to join                  |
| WIFI_CHANNEL        | Channel of wifi network to join                |
| API_SERVER          | API server to use for HTTP calls               |

Additional definitions have default values that can be overridden:

| Definition          | Default value  | Description                            |
|---------------------|----------------|----------------------------------------|
| NTP_SERVER          | pool.ntp.org   | NTP server to retrieve time from       |
| IP_ADDRESS          | 192.168.1.203  | Static IP address to use on network    |
| IP_GATEWAY          | 192.168.1.1    | Default gateway to use on network      |
| IP_DNS              | 192.168.1.1    | DNS address to use for name resolution |

> NOTE: The Pico SDK also requires PICO_BOARD be set to pico_w to build wifi projects

Clone Repo and cd:

```bash
git clone https://github.com/kennedn/restful-badger
cd restful-badger
```

Make build directory and cd:

```bash
mkdir build
cd build
```

Run cmake with definitions:

```bash
cmake .. -DPICO_BOARD=pico_w -DWIFI_SSID="cool-wifi-ssid" -DWIFI_PASSWORD="cool-wifi-password" -DWIFI_BSSID=66:55:44:33:22:11 -DWIFI_CHANNEL=9 -DAPI_SERVER="api.cool.com"
```

Cd to src folder and make:

```bash
cd src
make
```