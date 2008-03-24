#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/pci.h>
#include <linux/fs.h> 
#include <asm/io.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL"); 

static int bctadc_major = 0;

//we need this 3 IOPORTS to read from the board:
static unsigned long conversionControl,inputSelect,inputSample;

static struct pci_device_id bctadc_ids [] = {
	{ PCI_DEVICE(0x13c7,0x0adc) },
	{ 0 ,}
};

MODULE_DEVICE_TABLE(pci,bctadc_ids);


ssize_t bctadc_read (struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
	static int fim=0;
	fim = !fim;
	if(fim) {
	unsigned int sample=0;
	int tries=0;
//	printk(KERN_DEBUG "process %i (%s) going to read\n",
//			current->pid, current->comm);
	outb(0,inputSelect); //channel 0, gain 0, non-differential; 
	mdelay(5); //settle time
	outb(4,conversionControl); // sw trigger
	while(inb(conversionControl)&4) //is bit 2 clear?
		tries++;
	sample=inw(inputSample);
	printk(KERN_DEBUG "Read %x . tries %x\n" , sample, tries);
	sprintf(buf,"Read: %x\n", sample);
	return 10; /* EOF */
	} else
	return 0;
}


struct file_operations bctadc_fops = {
	.owner = THIS_MODULE,
	.read =  bctadc_read,
};



int probe (struct pci_dev *dev, const struct pci_device_id *id) {
	/* called by the kernel when the pci device is found */
		
	int result;
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
		 * Register your major, and accept a dynamic number
		 */
		result = register_chrdev(bctadc_major, "bctadc", &bctadc_fops);
		if (result < 0)
			return result;
		if (bctadc_major == 0)
			bctadc_major = result; /* dynamic */
		return 0;
	} else
		return -1;
};


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
 
static int hello_init(void) 
{ 
    printk(KERN_ALERT "BCTADC Loaded\n"); 
    return pci_register_driver(&pci_driver); 
} 

static void hello_exit(void) 
{ 
    printk(KERN_ALERT "BCTADC removed\n"); 
    pci_unregister_driver(&pci_driver);
} 

module_init(hello_init); 
module_exit(hello_exit); 

