/*
 * Claudiu Ghioc 341 C1
 * KTracer - Kprobe based tracer
 */

#include "ktracer.h"

MODULE_DESCRIPTION("Kprobe based tracer");
MODULE_AUTHOR("Claudiu Ghioc");
MODULE_LICENSE("GPL");

DEFINE_HASHTABLE(procs, MY_HASH_BITS);
spinlock_t hlocks[MY_HASH_SIZE];
DEFINE_SPINLOCK(hash_lock);

/* The original exit_group syscall handler */
asmlinkage void (*exitg_syscall) (int);

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
/* Destroy the list of associations address - size */
static void destroy_list(struct list_head *list)
{
	struct list_head *i, *n;
	struct mem_data *mm_data;

	list_for_each_safe(i, n, list) {
		mm_data = list_entry(i, struct mem_data, lh);
		list_del(i);
		kfree(mm_data);
	}
}

/* Destroy the hashtable with process information */
static void destroy_hasht(void)
{
	int k = 0;
	struct hlist_node *i, *tmp;
	struct proc_info *p_info;

	hash_for_each_safe(procs, k, i, tmp, p_info, hlh) {
		destroy_list(&p_info->mm);
		hash_del(i);
		kfree(p_info);
	}
}

/* Add a pid to the hashtable of monitored processes */
static int add_process(int pid) {
	struct proc_info *p_info;
	int i, hash;

	/* Allocate space for a new proc_info */
	p_info = kmalloc(sizeof(*p_info), GFP_KERNEL);
	if (p_info == NULL)
		return -ENOMEM;

	/* Initialize and add structure to the hashtable */
	p_info->pid = pid;
	INIT_LIST_HEAD(&p_info->mm);
	for (i = 0; i < FUNCTION_NO; i++)
		atomic64_set(&p_info->results[i], 0);

	hash = hash_min(pid, HASH_BITS(procs));
	hash_add(procs, &p_info->hlh, pid);

	return 0;
}

/* Remove a pid from the hashtable of monitored processes */
static int remove_process(int pid) {
	struct hlist_node *i, *tmp;
	struct proc_info *p_info;
	int hash;

	hash = hash_min(pid, HASH_BITS(procs));
	printk("Iau spinlock la remove\n");
	spin_lock(&hash_lock);
	printk("Am luat spinlock la remov\n");
	//spin_lock(&hlocks[hash]);
	hash_for_each_possible_safe(procs, p_info, i, tmp, hlh, pid) {
		if (p_info->pid != pid)
			continue;
		destroy_list(&p_info->mm);
		hash_del(i);
		kfree(p_info);
		//spin_unlock(&hlocks[hash]);
		printk("Dau drumul la spinlock la remove\n");
		spin_unlock(&hash_lock);
		printk("Am dat drumul la spinlock la remove\n");
		return 0;
	}
	printk("Dau drumul la spinlock la remove\n");
	spin_unlock(&hash_lock);
	printk("Am dat drumul la spinlock la remove\n");
	//spin_unlock(&hlocks[hash]);

	return -EINVAL;
}

/* ioctl handler for the tracer device */
static long tr_ioctl (struct file *file, unsigned int cmd,
	unsigned long pid)
{
	int ret = 0;

	printk(LOG_LEVEL "Process %ld ", pid);
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
	.minor  = TRACER_DEV_MINOR,
	.name   = TRACER_DEV_NAME,
	.fops   = &tr_fops,
};

/* exit_group syscall handler */
asmlinkage void my_exit_group(int status)
{
	remove_process(current->pid);
	exitg_syscall(status);
}

static int ktracer_init(void)
{
	int ret = 0, i;

	/* Replace the exit_group syscall */
	exitg_syscall = sys_call_table[__NR_exit_group];
	sys_call_table[__NR_exit_group] = my_exit_group;

	for (i = 0; i < MY_HASH_SIZE; i++)
		spin_lock_init(&hlocks[i]);

	/* Register kprobes */
	ret = register_kretprobe(mem_probe);
	if (ret) {
		printk(LOG_LEVEL "Unable to register kretprobe\n");
		goto restore_eg;
	}
	ret = register_jprobes(func_probes, JPROBE_NO);
	if (ret) {
		printk(LOG_LEVEL "Unable to register jprobes %d\n", ret);
		goto restore_eg;
	}


	/* Register tracer device */
	if (misc_register(&tracer_dev)) {
		ret = -EINVAL;
		goto ureg_kp;
	}


	/* Create entry in /proc */
	if (!(proc_kt = create_proc_entry(PROC_FILE, PROC_MODE, NULL))) {
		printk(LOG_LEVEL "Unable to create /proc entry\n");
		goto ureg_dev;
	}
	proc_kt->proc_fops = &tr_proc_ops;
	printk(LOG_LEVEL "Device 'tracer' initiated\n");

	// FIXME: remove this test
	for (i = 0; i < 40; i++)
		add_process(i);

	return 0;

ureg_dev:
	misc_deregister(&tracer_dev);

ureg_kp:
	unregister_kretprobe(mem_probe);
	unregister_jprobes(func_probes, JPROBE_NO);

restore_eg:
	sys_call_table[__NR_exit_group] = exitg_syscall;
	return ret;
}

static void ktracer_exit(void)
{
	/* Unregister tracer device */
	if (misc_deregister(&tracer_dev))
		printk(LOG_LEVEL "Unable to unregister tracer device\n");

	/* Delete the hashtable */
	destroy_hasht();

	/* Unregister kprobes */
	unregister_kretprobe(mem_probe);
	unregister_jprobes(func_probes, JPROBE_NO);


	/* Restore the original exit_group */
	sys_call_table[__NR_exit_group] = exitg_syscall;
	printk(LOG_LEVEL "Everything is clean\n");


	/* Remove the /proc entry */
	if (proc_kt)
		remove_proc_entry(PROC_FILE, NULL);
}

module_init(ktracer_init);
module_exit(ktracer_exit);
