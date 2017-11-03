#include <linux/bitops.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include<linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/irqdomain.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Xilinx Inc.");
MODULE_DESCRIPTION("cMemoryMap driver test");

#define DRIVER_NAME "memMap"
#define MEM_MAP_DEVICES 1

struct chMemMap_local{
	unsigned long mem_start;
	unsigned long mem_end;
	unsigned long mem_size;
	dev_t devt;
	struct cdev cdev;
	struct class *class;
	void __iomem *base_addr;
	int is_open;
};

static ssize_t chMemMap_read(struct file* F, char *buf, size_t count, loff_t *f_pos);
static ssize_t chMemMap_write(struct file* F, const char *buf, size_t count, loff_t *f_pos);
static int chMemMap_open(struct inode *inode, struct file *file);
static int chMemMap_close(struct inode *inode, struct file *file);
static int chMemMap_probe(struct platform_device *pdev);
static int chMemMap_remove(struct platform_device *pdev);

static struct of_device_id chMemMap_of_match[]={
	{.compatible = "xlnx,myMemMap-1.0",},
	{/*end of list*/},
};

static struct file_operations FileOps =
{
	.owner                = THIS_MODULE,
	.open                 = chMemMap_open,
	.read                 = chMemMap_read,
	.write                = chMemMap_write,
	.release              = chMemMap_close,
};

static struct platform_driver chMemMap_driver = {
	.probe = chMemMap_probe,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = chMemMap_of_match,
	},
	.remove = chMemMap_remove,
};


static ssize_t chMemMap_read(struct file* file, char *buf, size_t offset, loff_t *f_pos)
{
	struct chMemMap_local *drvdata=file->private_data;;
	unsigned char *kbuf;
	unsigned long long size_check;
	if(drvdata->base_addr+offset+1 > drvdata->base_addr+drvdata->mem_size){
		pr_info("chMemMap_read() : Out of memory range.\n");
		return -1;
	}else{
		pr_info("Memory Map character device module call read()\n");
		kbuf = drvdata->base_addr+offset;
		copy_to_user(buf, kbuf, 1);
		return 0x01;
	}
}
 
static ssize_t chMemMap_write(struct file* file, const char *buf, size_t offset, loff_t *f_pos)
{
	struct chMemMap_local *drvdata=file->private_data;;
	unsigned char *kbuf;
	unsigned char tmp;
	if(drvdata->base_addr+offset+1 > drvdata->base_addr+drvdata->mem_size){
		pr_info("chMemMap_write() : Out of memory range.\n");
		return -1;
	}else{
		pr_info("Memory Map character device module call write\n");
		kbuf = drvdata->base_addr+offset;
		copy_from_user(kbuf, buf, 1);
		return 0x01;
	}
}
  
static int chMemMap_open(struct inode *inode, struct file *file)
{
	pr_info("Memory Map character device module call open()\n");
	struct chMemMap_local *drvdata;
/*
	unsigned long mem_start;
	unsigned long mem_end;
	dev_t devt;
	struct cdev cdev;
	struct class *class;
	void __iomem *base_addr;
	container_of() 
*/
	drvdata = container_of(inode->i_cdev, struct chMemMap_local, cdev);
	if(drvdata->is_open == 1){
		pr_info("open error : Another user is using it.\n");
		return -1;
	}else{
		drvdata->is_open = 1;
		file->private_data = drvdata;
	}
	pr_info("base_addr : %X\n", drvdata->base_addr);
	return 0;
}
 
static int chMemMap_close(struct inode *inode, struct file *file)
{
	struct chMemMap_local *drvdata;
	drvdata = container_of(inode->i_cdev, struct chMemMap_local, cdev);
	pr_info("Memory Map character device module %s():%d\n",__func__,__LINE__);
	drvdata->is_open = 0;
	return 0;
}

static int chMemMap_probe(struct platform_device *pdev){
	struct resource *r_mem;
	struct device *dev = &pdev->dev;
	struct chMemMap_local *lp = NULL;
	dev_t devt;
	struct class *cl;
	int retval;

	pr_info("Device Tree Probing\n");
	lp = devm_kzalloc(&pdev->dev, sizeof(*lp), GFP_KERNEL);
	if (!lp)
		return -ENOMEM;

	r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!r_mem){
		dev_err(dev,"invalid address\n");
		return -ENODEV;
	}
	pr_info("probe start : %X\n", r_mem->start);
	pr_info("probe end : %X\n", r_mem->end);

	dev_set_drvdata(dev, lp);
	lp->mem_size = r_mem->end - r_mem->start +1;
	lp->base_addr = devm_ioremap_resource(&pdev->dev, r_mem);
	pr_info("probe base_addr : %X\n", lp->base_addr);
	pr_info("probe size : %X\n", lp->mem_size);

	if (IS_ERR(lp->base_addr))
		return PTR_ERR(lp->base_addr);

	/* pcw add */

	retval = alloc_chrdev_region(&devt, 0, MEM_MAP_DEVICES, DRIVER_NAME);
	if (retval < 0){
		platform_driver_unregister(&chMemMap_driver);
		printk( KERN_ALERT "character Device Registration failed\n" );
		return -1;
	}

	if ( (cl = class_create( THIS_MODULE, "chardev" ) ) == NULL )
	{
		platform_driver_unregister(&chMemMap_driver);
		printk( KERN_ALERT "chMemMap Class creation failed\n" );
		unregister_chrdev_region( devt, 1 );
		return -1;
	}

	if( device_create( cl, NULL, devt, NULL, DRIVER_NAME ) == NULL )
	{
		platform_driver_unregister(&chMemMap_driver);
		printk( KERN_ALERT "chMemMap Device creation failed\n" );
		class_destroy(cl);
		unregister_chrdev_region( devt, 1 );
		return -1;
	}

	lp->class = cl;
	lp->devt = devt;
	cdev_init(&lp->cdev, &FileOps);
	retval = cdev_add(&lp->cdev, devt, 1);
	if (retval) {
		platform_driver_unregister(&chMemMap_driver);
		printk( KERN_ALERT "chMemMap Device addition failed\n" );
		device_destroy( cl, devt);
		class_destroy( cl );
		unregister_chrdev_region( devt, 1 );
	}
	return 0;
}

static int chMemMap_remove(struct platform_device *pdev){
	struct device *dev = &pdev->dev;
	struct chMemMap_local *lp = dev_get_drvdata(dev);
	cdev_del(&lp->cdev);
	device_destroy( lp->class, lp->devt);
	class_destroy( lp->class );
	unregister_chrdev_region( lp->devt, 1 );
	release_mem_region(lp->mem_start, lp->mem_end - lp->mem_start+1);
	kfree(lp);
	dev_set_drvdata(dev, NULL);
	return 0;
}

MODULE_DEVICE_TABLE(of, chMemMap_of_match);

static int __init chMemMap_init(void)
{
	pr_info("hello Memory Map character device module\n");
	return platform_driver_register(&chMemMap_driver);
}

static void __exit chMemMap_exit(void)
{
	platform_driver_unregister(&chMemMap_driver);
	pr_info("goodbye Memory Map character device module\n");
}
subsys_initcall(chMemMap_init);

