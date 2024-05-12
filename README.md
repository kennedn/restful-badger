# restful-badger
Interface for interacting with RESTful APIs on the Badger 2040 W

![](./media/demo.gif)

# Preparing the build environment

Install build requirements:

```shell
sudo apt update
sudo apt install cmake gcc-arm-none-eabi build-essential
```

Install the Pico SDK:

```shell 
git clone -b master https://github.com/raspberrypi/pico-sdk.git --recursive
export PICO_SDK_PATH="$(pwd)/pico-sdk"
```

The `PICO_SDK_PATH` set above will only last the duration of your session.

You should should ensure your `PICO_SDK_PATH` environment variable is set in your profile, e.g `~/.bash_profile`

```shell
export PICO_SDK_PATH="/path/to/pico-sdk"
```

## Grab my fork of the Pimoroni libraries

```shell
git clone https://github.com/kennedn/pimoroni-pico /path/to/pimoroni-pico
export PIMORONI_PICO_PATH="/path/to/pimoroni-pico"
```

# Configuration

> NOTE: As much WIFI information as possible is specified at compile time in an effort to reduce wifi connect times since these occur each time the device wakes up

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
| JSON_FILEPATH       | PROJECT_ROOT/config/tiles.json | JSON file containing tile definitions |

> NOTE: The Pico SDK also requires PICO_BOARD be set to pico_w to build wifi projects

## JSON file

A JSON file is required to render an array of tiles in restfulBadger. It's default location is `PROJECT_ROOT/config/tiles.json`. Each tile consists of:

### Tile

| Key                        | Description                                   |
|----------------------------|-----------------------------------------------|
| name                       | Tile name, shown under the tiles icon         |
| image_idx                  | Index of icon to display from [image_tiles[]](/src/images.h#54) |
| action_request             | **Optional** Initial HTTP request without response checking   |
| status_request             | **Optional** Secondary HTTP request with response checking   |

#### Example

```json
{
    "name": "office",
    "image_idx": 0,
    "action_request": {},
    "status_request": {}
}
```


### HTTP Request

| Key                        | Description                                   |
|----------------------------|-----------------------------------------------|
| method                     | HTTP method                                   |
| endpoint                   | HTTP URL Endpoint, appended to API_SERVER     |
| json_body                  | JSON string to send to the endpoint           |
| key                        | Key of the value to extract from the HTTP response         |
| on_value                   | Compared with key value, produces a tick mark |
| off_value                  | Compared with key value, produces a cross mark |


#### Example

```json
{
    "method": "POST",
    "endpoint": "/v2/meross/office",
    "json_body": "{\"code\": \"status\"}",
    "key": "onoff",
    "on_value": "1",
    "off_value": "0"
}
```

# Building 

Clone Repo and cd:

```bash
git clone https://github.com/kennedn/restful-badger
cd restful-badger
```
Configure the JSON file at `PROJECT_ROOT/config/tiles.json`

```bash
cd config
vi tiles.json
cd ..
```

> NOTE: If **tiles.json** is updated in the future, `cmake ..` must be run from the build directory again to apply any changes

Make build directory and cd:

```bash
mkdir build
cd build
```


Run cmake with definitions:

```bash
cmake .. \
  -DPICO_BOARD=pico_w \
  -DWIFI_SSID="cool-wifi-ssid" \
  -DWIFI_PASSWORD="cool-wifi-password" \
  -DWIFI_BSSID=66:55:44:33:22:11 \
  -DWIFI_CHANNEL=9 \
  -DAPI_SERVER="api.cool.com"
```

Cd to src folder and make:

```bash
cd src
make
```