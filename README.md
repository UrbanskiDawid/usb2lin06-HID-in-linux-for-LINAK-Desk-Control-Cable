# usb2lin06-HID-in-linux-for-LINAK-Desk-Control-Cable

This is a simple program for controlling LINAK Desk Control Cable in linux.
Its created as linux alternative to: https://www.linak.com/products/controls.aspx?product=LINAK+Desk+Control+SW

Testes on: Debian8 x64, raspbian jessie
Tested on model: usb2lin06 with CONTROL BOX CBD6S.

### Dependencies
this is using: **libusb-1.0**
>to intall libusb-1.0 (debian)
```sh
$ sudo apt-get install libusb-1.0-0-dev
```

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
> to see if you have your device connected:
```sh
$ lsusb -v -d 12d3:0002
```

### TODO:
anazyze how save/restore position works
