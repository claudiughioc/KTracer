/*
 * Claudiu Ghioc 341 C1
 * KTracer - Kprobe based tracer
 */

#include "ktracer.h"

MODULE_DESCRIPTION("Kprobe based tracer");
MODULE_AUTHOR("Claudiu Ghioc");
MODULE_LICENSE("GPL");

DEFINE_HASHTABLE(procs, 8);

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

/* Add a pid to the hashtable of monitored processes */
static int add_process(int pid) {
	struct proc_info *p_info;

	/* Allocate space for a new proc_info */
	p_info = kmalloc(sizeof(*p_info), GFP_KERNEL);
	if (p_info == NULL)
		return -ENOMEM;

	/* Initialize and add structure to the hashtable */
	p_info->pid = pid;
	hash_add(procs, &p_info->hlh, pid);

	return 0;
}

/* Remove a pid from the hashtable of monitored processes */
static int remove_process(int pid) {
	struct hlist_node *i, *tmp;
	struct proc_info *p_info;

	hash_for_each_possible_safe(procs, p_info, i, tmp, hlh, pid) {
		if (p_info->pid != pid)
			continue;
		hash_del(i);
		kfree(p_info);
		return 0;
	}

	return -EINVAL;
}

/* ioctl handler for the tracer device */
static long tr_ioctl (struct file *file, unsigned int cmd,
	unsigned long arg)
{
	int pid, ret = 0;

	/* Get the pid of the process */
	if(copy_from_user(&pid, (int __user *)arg, sizeof(int)))
		return -EFAULT;

	printk(LOG_LEVEL "Process %d ", pid);
	switch (cmd) {
	case TRACER_ADD_PROCESS:

		printk(LOG_LEVEL "will be monitored\n");
		ret = add_process(pid);
		break;
	case TRACER_REMOVE_PROCESS:

		printk(LOG_LEVEL "will be removed\n");
		ret = remove_process(pid);
		break;
	default:
		ret = -ENOTTY;
	}

	return ret;
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
	int ret = 0, i;
	struct proc_info *p_info;

	/* Register kretprobes */
	ret = register_kretprobes(mem_probes, 2);
	if (ret) {
		printk(LOG_LEVEL "Unable to register krets\n");
		return ret;
	}


	/* Register jprobes */
	ret = register_jprobes(func_probes, JPROBE_NO);
	if (ret) {
		printk(LOG_LEVEL "Unable to register jprobes %d\n", ret);
		return ret;
	}


	/* Register tracer device */
	if (misc_register(&tracer_dev))
		return -EINVAL;
	printk("Register tracer device\n");

	for (i = 0; i < 10; i++) {
		p_info = kmalloc(sizeof(*p_info), GFP_KERNEL);
		if (p_info == NULL)
			return -ENOMEM;
		p_info->pid = i;
		hash_add(procs, &p_info->hlh, i);
	}

	return ret;
}

static void ktracer_exit(void)
{
	int k = 0;
	struct hlist_node *i, *tmp;
	struct proc_info *p_info;

	/* Unregister tracer device */
	if (misc_deregister(&tracer_dev)) {
		printk(LOG_LEVEL "Unable to unregister tracer device\n");
		return;
	}
	printk(LOG_LEVEL "Unregistered device\n");

	/* Delete the hashtable */
	hash_for_each_safe(procs, k, i, tmp, p_info, hlh) {
		printk("Removing proc info for %d\n", p_info->pid);
		hash_del(i);
		kfree(p_info);
	}


	/* Unregister jprobes */
	unregister_jprobes(func_probes, JPROBE_NO);
	printk(LOG_LEVEL "Everything is clean\n");
}

module_init(ktracer_init);
module_exit(ktracer_exit);
