#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/pci.h>
#include <linux/fs.h> 
#include <asm/io.h>
#include <asm/current.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/sysfs.h>
#include <linux/miscdevice.h>

MODULE_LICENSE("GPL"); 

static int bctadc_major = 0;

//we need this 3 IOPORTS to read from the board:
static unsigned long conversionControl,inputSelect,inputSample;


static struct pci_device_id bctadc_ids [] = {
  { PCI_DEVICE(0x13c7,0x0adc) },
  { 0 ,}
};

MODULE_DEVICE_TABLE(pci,bctadc_ids);


ssize_t bctadc_read (struct file *filp, char __user *buf, size_t count, loff_t *pos) {
   if(*pos<31) {
    unsigned int channel=0,sample=0;
    int tries=0;
	char hex_sample[5];
    printk(KERN_DEBUG "process %i (%s) going to read at offset %lu\n",
	   current->pid, current->comm,*pos);
	channel=*pos>>2; //every channel read takes 4 hex chars
	
    outb(channel << 4 ,inputSelect); //channel 0, gain 0, non-differential; 
    mdelay(5); //settle time
    outb(4,conversionControl); // sw trigger
    while(inb(conversionControl)&4) //is bit 2 clear?
      tries++;
    sample=inw(inputSample);
    printk(KERN_DEBUG "Read %04x . tries %x\n" , sample, tries);

    sprintf(hex_sample,"%04x", sample);
	copy_to_user(buf, hex_sample, 4); 

	
    // sprintf(buf,"Read: %x\n", sample);
    *pos+=4;
    return 4;
  } else
    return 0; /* EOF */
}


struct file_operations bctadc_fops = {
  .owner = THIS_MODULE,
  .read =  bctadc_read,
};


static struct miscdevice bctadc_dev = {
  /*
   * We don't care what minor number we end up with, so tell the
   * kernel to just pick one.
   */
  MISC_DYNAMIC_MINOR,
  /*
   * Name ourselves /dev/bctadc.
   */
  "bctadc",
  /*
   * What functions to call when a program performs file
   * operations on the device.
   */
  &bctadc_fops
};

int probe (struct pci_dev *dev, const struct pci_device_id *id) {
  /* called by the kernel when the pci device is found */
	
  int ret;
  unsigned long bar;
  printk( KERN_ALERT "BCTADC: Probed!!\n");
  if(!pci_enable_device (dev)) { 
    printk(KERN_ALERT "BCTADC: enabled\n");
    //Now get tha IOPorts needed
    bar=pci_resource_start(dev, 2);
    conversionControl = bar +0xC;
    inputSelect = bar + 0xD;
    inputSample = pci_resource_start(dev,3); // +0x0
		

    /*
     * Create the "hello" device in the /sys/class/misc directory.
     * Udev will automatically create the /dev/hello device using
     * the default rules.
     */
    ret = misc_register(&bctadc_dev);
    if (ret)
      printk(KERN_ERR
	     "Unable to register bctadc misc device\n");

    return ret;
  }
  else 
    return -1;
}


void remove (struct pci_dev *dev) {
  //what should we do here?? Is it ok to disable the device?
  unregister_chrdev(bctadc_major, "bctadc");
  pci_disable_device (dev);
};

static struct pci_driver pci_driver = {
  .name = "bctadc",
  .id_table = bctadc_ids,
  .probe = probe,
  .remove = remove
};

static int bctadc_init(void) { 
  printk(KERN_ALERT "BCTADC Loaded\n");
  return pci_register_driver(&pci_driver); 
} 

static void bctadc_exit(void) { 
  printk(KERN_ALERT "BCTADC removed\n"); 
  misc_deregister(&bctadc_dev);
  pci_unregister_driver(&pci_driver);
} 

module_init(bctadc_init); 
module_exit(bctadc_exit); 

