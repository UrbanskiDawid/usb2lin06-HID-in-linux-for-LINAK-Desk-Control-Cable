#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mutex.h>

#include <linux/time.h>

struct sData {
  struct usb_device *udev;
  int curentHeight;
  int targetHeight;

  struct mutex read_lock;
  struct mutex move_lock;

  struct timespec updateTime;//tv_nsec tv_sec 10−9

} DATA;

unsigned int distance(int a,int b)
{
  int d;
  d = a-b;
  if(d<0) d = -d;
  return d;
}

struct timespec getDataAge(void)
{
  struct timespec timeNow;
  getnstimeofday(&timeNow);
  return timespec_sub(timeNow,DATA.updateTime);
}

bool isDateOlderThan(int sec,long nanoSec)
{
  struct timespec timeNow;
  struct timespec timeDelta;
  getnstimeofday(&timeNow);
  timeDelta = timespec_sub(timeNow,DATA.updateTime);

  if(timeDelta.tv_sec == sec
     &&
    timeDelta.tv_nsec < nanoSec)
  {
    return true;
  }
  if(timeDelta.tv_sec < sec)
  {
    return true;
  }

  return false;
}

static void updateStatus(int minAgeSec,long minAgeNanoSec)
{
  unsigned char buf[64];
  int ret;
  struct StatusReport report;

  if(DATA.udev==NULL)
  {
    printk(KERN_ERR "updateStatus() no dev");
    return;
  }

  //check if data was pulled in last 'minAgeSec' seconds
  if(isDateOlderThan(minAgeSec,minAgeNanoSec))
  {
    printk(KERN_INFO "data is younger than %d.%ld seconds",minAgeSec,minAgeNanoSec);
    return;
  }

  ret = usb_control_msg(
    DATA.udev,
    usb_rcvctrlpipe(DATA.udev,0),
    URB_getStatus.bRequest,
    URB_getStatus.bmRequestType,
    URB_getStatus.wValue,
    URB_getStatus.wIndex,
    buf,
    URB_getStatus.wLength,
    3000
  );
  if(ret==64)
  {
    memcpy(&report, buf, sizeof(report));
    buf[63]=0;
    printk(KERN_INFO "updateStatus() OK current height: %d",report.ref1.pos);

    DATA.curentHeight = (int)report.ref1.pos;
    getnstimeofday(&DATA.updateTime);
  }else{
    if(ret==-EPIPE)  printk(KERN_WARNING "updateStatus() broken pipe!");
    else             printk(KERN_WARNING "updateStatus() ctrl message fail: '%d'",ret);
  }
}

static void updateStatus_force(void){ updateStatus(0,0);}
static void updateStatus_default(void){ updateStatus(0,500000000);}

bool move(int16_t targetHeight)
{
  int ret;
  unsigned char data[64];
  memset (data,0,sizeof(data));

  data[0]=0x05;
  memcpy( data+1, &targetHeight, sizeof(targetHeight) );
  memcpy( data+3, &targetHeight, sizeof(targetHeight) );
  memcpy( data+5, &targetHeight, sizeof(targetHeight) );
  memcpy( data+7, &targetHeight, sizeof(targetHeight) );

  ret = usb_control_msg(
    DATA.udev,
    usb_sndctrlpipe(DATA.udev,0),
    URB_move.bRequest,
    URB_move.bmRequestType,
    URB_move.wValue,
    URB_move.wIndex,
    data,
    URB_move.wLength,
    3000
  );

  return (64==ret);
}



struct task_struct *task;
int thread_function(void *data)
{
    struct sData * dataRef = data;
    int i;
    unsigned int dist;//distance left to target
    unsigned int d1;
    int stuck;
    int target;
    int lastH;

mutex_lock(&dataRef->move_lock);

    stuck = 4;
    target= dataRef->targetHeight;
    lastH = dataRef->curentHeight;

    for(i=0; i<1000; i++)
    {
        mutex_lock(&dataRef->read_lock);
        if(!move(target))
        { printk(KERN_ERR "can't move!"); }
        msleep(10);
        updateStatus_force();

        dist=distance(dataRef->curentHeight,target);

	printk(KERN_INFO "moving %d/1000 cur: %d target: %d delta: %d",i,dataRef->curentHeight,target,dist);

        //distance check
        if(dist<=13)
        {
           printk(KERN_INFO "cur: %d target: %d DONE", dataRef->curentHeight,target);
           break;
        }

        //last, current check
        d1=distance(dataRef->curentHeight,lastH);
        if( d1 <=13)
        {
          stuck--;
	  printk(KERN_INFO "bump! %d/4 %d",stuck,d1);
          if(stuck==0) break;
        }else{
          stuck=4;
        }
        lastH = dataRef->curentHeight;

        mutex_unlock(&dataRef->read_lock);

      msleep(200);
    }

    printk(KERN_INFO "move ended at '%d' target: %d",dataRef->curentHeight,target);

mutex_unlock(&dataRef->move_lock);
if(mutex_is_locked(&dataRef->read_lock))
mutex_unlock(&dataRef->read_lock);

    return 0;
}

static void dataInit(void)
{
 DATA.udev = NULL;
 DATA.curentHeight = 0;
 DATA.targetHeight = 0;
 mutex_init(&DATA.move_lock);
 mutex_init(&DATA.read_lock);
 DATA.updateTime.tv_sec = 0;
 DATA.updateTime.tv_nsec = 0;
}

static ssize_t devRead(struct file *f, char __user *buf, size_t len, loff_t *off)
{
  int l;
  static char napis[264];

  if(!mutex_is_locked(&DATA.move_lock))
  {
    updateStatus_default();
  }

  mutex_lock(&DATA.read_lock);
    sprintf(napis,"current height: '%d'\n", DATA.curentHeight);
  mutex_unlock(&DATA.read_lock);

  l=strlen(napis);
  if(l>=len) return -EFAULT;

  if(*off!=0) return 0;

  if (copy_to_user(buf, &napis, l) != 0) return -EFAULT;
  *off=l;

  return l;
}

static ssize_t devWrite(struct file *f, const char *page,  size_t len, loff_t *offset)
{
  ssize_t bytes;
  static char napis[20];
  int newHeight;

  if(len >= 20) return -EFAULT;
  bytes=len;

  memset(napis,0,sizeof(napis));

  if(copy_from_user(napis, page, bytes)!=0)  return bytes;

  if(kstrtoint(napis, 10, &newHeight)==0)
  {
    if(mutex_is_locked(&DATA.move_lock))
    {
      printk(KERN_WARNING "cant set target while moving!");
      return -EFAULT;
    }

    DATA.targetHeight = newHeight;
    printk(KERN_INFO "new target: %d", DATA.targetHeight);

    task = kthread_run(&thread_function,(void *)&DATA,"pradeep");
    printk(KERN_INFO"Kernel Thread : %s\n",task->comm);
  }
  else
  {
    printk(KERN_WARNING "cant convert input: '%s' to int", napis);
  }

  return len;
}

static struct file_operations devFops =
{
  .owner = THIS_MODULE,
  .read  = devRead,
  .write = devWrite
};
