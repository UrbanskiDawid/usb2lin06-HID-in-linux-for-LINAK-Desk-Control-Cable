# usb2lin06-HID-in-linux-for-LINAK-Desk-Control-Cable

This is a simple program for controlling LINAK Desk Control Cable in linux.

Tested on model: usb2lin06.

*currently only monitoring and setting height is possible*

this is using: libusb-1.0

please run output file using "sudo"!

NOTE to monitor usb in wireshark:
$ modprobe usbmon

DONE:
1. update libusb to current version
2. add wireshark traces folder + descriptions
3. trigger moment up
   up, down
4. example program to set desk height

TODO:
5. anazyze how save/restore position works
   (remote buttons: "1":B2,"2":B3,"3":B4,"S"B5)
   *maybe status has this values?*
