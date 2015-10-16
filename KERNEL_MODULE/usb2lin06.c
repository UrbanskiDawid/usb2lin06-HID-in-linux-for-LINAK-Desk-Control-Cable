#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dawid Urbanski");
MODULE_DESCRIPTION("Simple character device driver");

#include <linux/usb.h>

#include "../usb2lin06.h"
#include "deviceActions.c"  //fops
#include "deviceCreation.c"

static int find_linak(struct usb_device *udev, void *unused)
{
  if( udev->descriptor.idVendor  == 0x12d3
      &&
      udev->descriptor.idProduct == 0x0002 )
  {
    printk(KERN_INFO "found : '%s' '%s' %d %d",udev->manufacturer,udev->product,udev->descriptor.bDeviceClass,udev->descriptor.bDeviceSubClass);
    DATA.udev = udev;
    usb_get_dev(DATA.udev);//increments the reference count of the usb device structure
    return 1;
  }
  return 0;
}

static int __init moduleInit(void)
{
  if(deviceCreate()!=0) return -1;

  dataInit();

  usb_for_each_dev(NULL, find_linak);
  if(DATA.udev == NULL)
  {
    printk(KERN_ERR "no device found");
    return -1;
  }

  updateStatus_force();

  return 0;
}
module_init(moduleInit);

static void __exit moduleExit(void)
{
  usb_put_dev(DATA.udev);
  deviceDestroy();
}
module_exit(moduleExit);
