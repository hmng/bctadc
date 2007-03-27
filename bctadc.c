#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/pci.h>

MODULE_LICENSE("GPL"); 

static struct pci_device_id bctadc_ids [] = {
	{ PCI_DEVICE(0x1102,0x7002) },
	{ PCI_DEVICE(0x0005,0x0584) } ,
	{ 0 ,}
};

MODULE_DEVICE_TABLE(pci,bctadc_ids);

int probe (struct pci_dev *dev, const struct pci_device_id *id) {
	printk( KERN_ALERT "Probed!!\n");
return -1;
};


void remove (struct pci_dev *dev) {

};

static struct pci_driver pci_driver = {
	.name = "bctadc",
	.id_table = bctadc_ids,
	.probe = probe,
	.remove = remove
};
 
static int hello_init(void) 
{ 
    printk(KERN_ALERT "Hello, world\n"); 
    return pci_register_driver(&pci_driver); 
} 

static void hello_exit(void) 
{ 
    printk(KERN_ALERT "Goodbye, cruel world\n"); 
    pci_unregister_driver(&pci_driver);
} 

module_init(hello_init); 
module_exit(hello_exit); 

