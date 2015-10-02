# usb2lin06-HID-in-linux-for-LINAK-Desk-Control-Cable

This is a simple program for controlling LINAK Desk Control Cable in linux.

Tested on model: usb2lin06.

*currently only monitoring is possible*

this is using ancient: libusb-0.1.12

please run output file using "sudo"!

NOTE to monitor usb in wireshark:
$ modprobe usbmon

TODO:
1. update libusb to current version
2. add wireshark traces folder + descriptions
3. trigger moment up
   up, down
4. example program to set desk height
5. anazyze how save/restore position works
   (remote buttons: "1":B2,"2":B3,"3":B4,"S"B5)
   *maybe status has this values?*
