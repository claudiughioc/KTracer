/*
 * Claudiu Ghioc 341 C1
 * KTracer - Kprobe based tracer
 */

#include "ktracer.h"

MODULE_DESCRIPTION("Kprobe based tracer");
MODULE_AUTHOR("Claudiu Ghioc");
MODULE_LICENSE("GPL");

/* open device handler for the tracer */
static int tr_open(struct inode *in, struct file *filp)
{
	printk(LOG_LEVEL "TR device opened\n");
	return 0;
}

/* release device handler for the tracer */
static int tr_release(struct inode *in, struct file *filp)
{
	printk(LOG_LEVEL "TR device closed \n");
	return 0;
}

/* ioctl handler for the tracer device */
static long tr_ioctl (struct file *file, unsigned int cmd,
	unsigned long arg)
{
	printk(LOG_LEVEL "TR device ioctl\n");
	return 0;
}

const struct file_operations tr_fops = {
	.owner = THIS_MODULE,
	.open = tr_open,
	.unlocked_ioctl = tr_ioctl,
	.release = tr_release
};

static struct miscdevice tracer_dev = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = MODULE_NAME,
	.fops   = &tr_fops,
};

static int ktracer_init(void)
{
	int ret = 0;

	/* Register tracer device */
	if (misc_register(&tracer_dev))
		return -EINVAL;

	printk("Register tracer device\n");

	return ret;
}

static void ktracer_exit(void)
{

	/* Unregister tracer device */
	if (misc_deregister(&tracer_dev)) {
		printk(LOG_LEVEL "Unable to unregister tracer device\n");
		return;
	}

	printk(LOG_LEVEL "Unregistered device\n");
}

module_init(ktracer_init);
module_exit(ktracer_exit);
