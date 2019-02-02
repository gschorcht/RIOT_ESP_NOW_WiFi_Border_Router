# ESP-NOW to WiFi border router for RIOT-OS

## Overview

This application realizes a border router for [RIOT-OS](https://riot-os.org) using the [GNRC stack](https://riot-os.org/api/group__net__gnrc.html) which connects an ESP-NOW based mesh network to an infrastructure mode WiFi network using an ESP32 or an ESP8266. It can be used to extend a network of ESP32 and ESP8266 nodes where not all nodes are able to connect to an AP of the infrastructure mode WiFi network.

The border router uses the modules `esp_now` and `esp_wifi`

- to create two GNRC network interfaces (the inner interface using ESP-NOW and the outer interface using WiFi) and
- to route packets between the ESP-NOW based mesh networks and the infrastructure mode WiFi network.

**Please note**
- Since the WiFi interface of ESP nodes can only operate on one channel at a time, the ESP-NOW based mesh network has to use the same channel as it is used by the AP of the infrastructure-mode WiFi network. For that purpose the border router and all ESP-NOW nodes have to be compiled using configuration parameter `ESP_NOW_CHANNEL`.

- Although an ESP8266 could also be used as a border router, it is strongly recommended to use an ESP32 because of the very small RAM of the ESP8266. Because ESP-NOW is compatible on ESP832 and ESP8266 nodes, the ESP-NOW based mesh network can be any mix of ESP32 and ESP8266 nodes.

## How to compile

Clone the border router repository.

```shell
git clone https://github.com/gschorcht/RIOT_ESP_NOW_WiFi_Border_Router.git
```

Change to the directory.

```shell
cd RIOT_ESP_NOW_WiFi_Border_Router
```

Set variable `RIOTBASE` to your RIOT-OS directory.

```shell
export RIOTBASE=<RIOT-OS directory>
```

Compile an flash the border router using the following command

```shell
CFLAGS='-DESP_NOW_CHANNEL=<channel> -DESP_WIFI_SSID=\"<SSID>\" -DESP_WIFI_PASS=\"<passphrase>\"' \
make BOARD=... PORT=/dev/... flash
```

`<channel>` specifies the channel, `<SSID>` the SSID and `<passphrase>` the PSK as used by the AP of your infrastructure mode WiFi network. Declare variables `BOARD` and `PORT` according to your configuration. If `BOARD` is omitted, `esp32-wroom-32` is used as default.

If you would like to use RPL for routing in the ESP-NOW based mesh network, just add module `gnrc_rpl` as following.

```shell
USEMODULE=gnrc_rpl \
CFLAGS='-DESP_NOW_CHANNEL=<channel> -DESP_WIFI_SSID=\"<SSID>\" -DESP_WIFI_PASS=\"<passphrase>\"' \
make BOARD=... PORT=/dev/... flash
```

Compile and flash all ESP-NOW nodes with same `<channel>` as the border router.

```shell
cd $RIOTBASE
USEMODULE='esp_now' CFLAGS='-DESP_NOW_CHANNEL=<channel>' \
make -C examples/gnrc_networking BOARD=... PORT=/dev/... flash
```

## How to use

Once the border router and all ESP-NOW nodes are flashed, a terminal program can be used to connect to the border router and the nodes, for example with:

```shell
python -m serial.tools.miniterm /dev/... 115200
```

Within the terminal, different shell commands like `ifconfig`, `ping6` or `rpl` can be used to get information about the status of the nodes. By typing `help` you will get the list of available shell commands.

### Border router

On the border router, the `ps` command should show the following two processes

```
	pid | name                 | state    Q | pri | stack  ( used) | base addr  | current
          ...
	  7 | esp-now              | bl rx    _ |  10 |   2048 (  960) | 0x3ffb64d0 | 0x3ffb6ab0 
	  8 | esp-wifi             | bl rx    _ |   9 |   2048 (  892) | 0x3ffb6e1c | 0x3ffb73f0 
```

and the `ifconfig` command should give two interfaces, for example,

```shell
Iface  7  HWaddr: 30:AE:A4:18:7A:3D 
          MTU:1280  HL:64  RTR  
          RTR_ADV  6LO  Source address length: 6
          Link type: wireless
          inet6 addr: fe80::32ae:a4ff:fe18:7a3d  scope: local  VAL
          inet6 addr: 2001:db8::32ae:a4ff:fe18:7a3d  scope: global  VAL
          inet6 group: ff02::2
          inet6 group: ff02::1
          inet6 group: ff02::1:ff18:7a3d
          inet6 group: ff02::1a
          
Iface  8  HWaddr: 30:AE:A4:18:7A:3C  Link: up 
          MTU:1440  HL:255  RTR  
          Source address length: 6
          Link type: wireless
          inet6 addr: fe80::32ae:a4ff:fe18:7a3c  scope: local  VAL
          inet6 addr: 2003:c1:e715:fc23:32ae:a4ff:fe18:7a3c  scope: global  VAL
          inet6 group: ff02::2
          inet6 group: ff02::1
          inet6 group: ff02::1:ff18:7a3c
```

where interface `7` is the ESP-NOW interface and interface `8` the WiFi interface. The ESP-NOW interface should already have a global unicast address with prefix `2001:db8::/64`. The WiFi interface only gets a global unicast address if the router in your LAN provides a global routing prefix.

If you have enabled RPL by module `gnrc_rpl`, it should have been initialized and command `rpl` should result into something like the following.

```
instance table:	[X]	
parent table:	[ ]	[ ]	[ ]	

instance [0 | Iface: 7 | mop: 2 | ocp: 0 | mhri: 256 | mri 0]
	dodag [2001:db8::32ae:a4ff:fe18:7a3d | R: 256 | OP: Router | PIO: on | TR(I=[8,20], k=10, c=1, TC=2s)]
```

### ESP-NOW nodes

On ESP-NOW nodes, you sould only see one process like 

```
	pid | name                 | state    Q | pri | stack  ( used) | base addr  | current     
	  ...
	  8 | esp-now              | bl rx    _ |  10 |   2560 (  736) | 0x3fff4340 | 0x3fff4be0 
```

and the `ifconfig` command should give one interface, for example:

```
Iface  8  HWaddr: 62:01:94:82:7A:1D 
          MTU:1280  HL:64  RTR  
          RTR_ADV  6LO  Source address length: 6
          Link type: wireless
          inet6 addr: fe80::6001:94ff:fe82:7a1d  scope: local  VAL
          inet6 addr: 2001:db8::6001:94ff:fe82:7a1d  scope: global  VAL
          inet6 group: ff02::2
          inet6 group: ff02::1
          inet6 group: ff02::1:ff82:7a1d
          inet6 group: ff02::1a
```

The ESP-NOW interface should already have a global unicast address with prefix `2001:db8::/64`.

If RPL was enabled on border router, command `rpl` should result into something like the following after some seconds.

```
instance [0 | Iface: 8 | mop: 2 | ocp: 0 | mhri: 256 | mri 0]
	dodag [2001:db8::32ae:a4ff:fe18:7a3d | R: 512 | OP: Router | PIO: on | TR(I=[8,20], k=10, c=0, TC=8s)]
		parent [addr: fe80::32ae:a4ff:fe18:7a3d | rank: 256]
```

You should be able to ping the border router and other ESP-NOW nodes using their IPv6 global unicast address with prefix `2001:db8::/64`, for example:

```shell
ping6 2001:db8::5ccf:7fff:fe94:76a9
```

Furthermore, if you configure a route to `2001:db8::/64` on your machine in the LAN using the link local unicast address of the WiFi interface of the border router, for example,

```shell
sudo ip -6 route add 2001:db8::/64 via fe80::32ae:a4ff:fe18:7a3c dev eth0
```

you should be able to ping the ESP-NOW nodes from your machine and vise versa.

```shell
ping6 2001:db8::6001:94ff:fe82:7a1d
```

That is, ESP-NOW nodes are then able now to communicate with arbitrary networks via the border router provided that packets with prefix `2001:db8::/64` are routed to the WiFi interface of your border router.


