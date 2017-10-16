# usb2lin06-HID-in-linux-for-LINAK-Desk-Control-Cable

This is a simple program for controlling LINAK Desk Control Cable in linux.
Its created as linux alternative to: https://www.linak.com/products/controls.aspx?product=LINAK+Desk+Control+SW

Tested on: Debian8 x64, raspbian jessie, Ubuntu 16.04.3, Arch (18August2017)
Tested on: Fedora26 (2Oct2017)
Tested on model: usb2lin06 with CONTROL BOX CBD6S.

### Dependencies
this is using: **libusb-1.0**
>to intall libusb-1.0 (debian)
```sh
$ sudo apt-get install libusb-1.0-0-dev
```
>to install libusb (fedora)
```sh
$ sudo dnf install libusb-devel
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

### To run as regular 
To run as regular user you will have to give access to device as the user you want to run it as. The easiest way to do this may be to implement a udev rule such that the devices when connected is reconized as user or group is given permission. 
You can do this by creating a new file at cat /etc/udev/rules.d/90-desk-permission.rules 
```sh
SUBSYSTEM=="usb", ATTR{idVendor}=="12d3", ATTR{idProduct}=="0002", GROUP="group", MODE="0660"
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
