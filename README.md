# usb2lin06-HID-in-linux-for-LINAK-Desk-Control-Cable

This is a simple program for controlling LINAK Desk Control Cable in linux.

Tested on model: usb2lin06.

this is using: libusb-1.0

### Capabilities
* setting height
* monitoring status - current height
* getting pressed buttons

>to compile examples:
```sh
$ make
```

### How to run examples
[ please run output file using "sudo"! ]
> get current height in centimeters
```sh
  $ sudo ./example-getHeight
```
>get 10 status reports every 2 seconds
```sh
  $ sudo ./example-getStatus 10 2.0
```
> move desk up (to height 6000 in my case its 62.2cm)
```sh
  $ sudo ./example-moveTo 6000
```
> move desk to very bottom
```sh
  $ sudo ./example-moveTo 0
```

### Wireshark 
traces of working program are included
> to monitor usb in wireshark:
```sh
$ modprobe usbmon
```

### LSUSB
```sh
$ lsusb -v -d 12d3:0002
```

### DONE:
1. update libusb to current version
2. add wireshark traces folder + descriptions
3. trigger moment up
   up, down
4. example program to set desk height

### TODO:
5. anazyze how save/restore position works
