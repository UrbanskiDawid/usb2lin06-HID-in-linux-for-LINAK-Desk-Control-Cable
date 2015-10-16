#define DEVICE_NAME "usb2lin06"
#define DEVICE_PERMISSION 0666

struct sDevice {
  dev_t dev; // device number
  struct cdev c_dev; // character device structure
  struct class *cl; // device class
} DEVICE;

static int my_dev_uevent(struct device *dev, struct kobj_uevent_env *env)
{
  add_uevent_var(env, "DEVMODE=%#o", DEVICE_PERMISSION);
  return 0;
}

static int deviceCreate(void)
{
  printk(KERN_INFO "creating device");

  if (alloc_chrdev_region(&DEVICE.dev, 0, 1, "dawid") < 0)
  {
    printk(KERN_WARNING "cannot alloc_chrdev_region");
    return -1;
  }

  if ((DEVICE.cl = class_create(THIS_MODULE, "chardrv")) == NULL)
  {
    printk(KERN_WARNING "cannot class_create");
    unregister_chrdev_region(DEVICE.dev, 1);
    return -1;
  }

  DEVICE.cl->dev_uevent = my_dev_uevent;//this sets permissions

  if (device_create(DEVICE.cl, NULL, DEVICE.dev, NULL, DEVICE_NAME) == NULL)
  {
    printk(KERN_WARNING "cannot device_create");
    class_destroy(DEVICE.cl);
    unregister_chrdev_region(DEVICE.dev, 1);
    return -1;
  }

  cdev_init(&DEVICE.c_dev, &devFops);

  if (cdev_add(&DEVICE.c_dev, DEVICE.dev, 1) == -1)
  {
    printk(KERN_WARNING "cannot cdev_add");
    device_destroy(DEVICE.cl, DEVICE.dev);
    class_destroy(DEVICE.cl);
    unregister_chrdev_region(DEVICE.dev, 1);
    return -1;
  }

  printk(KERN_INFO "device /dev/%s registered", DEVICE_NAME);
  return 0;
}

static void deviceDestroy(void)
{
  cdev_del(&DEVICE.c_dev);
  device_destroy(DEVICE.cl, DEVICE.dev);
  class_destroy(DEVICE.cl);
  unregister_chrdev_region(DEVICE.dev, 1);
  printk(KERN_INFO "device unregistered");
}
