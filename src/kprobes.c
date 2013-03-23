/*
 * Claudiu Ghioc 341 C1
 * KTracer - Kprobe based tracer
 */

#include "ktracer.h"

/* kmalloc handler */
static int kmalloc_h(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	return 0;
}

/* kmalloc entry handler */
static int kmalloc_eh(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	return 0;

}

/* kfree handler */
static int kfree_h(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	return 0;
}

/* kfree entry handler */
static int kfree_eh(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	return 0;
}

/* schedule entry handler */
static void schedule_en(void)
{
	printk(LOG_LEVEL "Schedule \n");
	jprobe_return();
}

/* up entry handler */
static void up_en(struct semaphore *sem)
{
	jprobe_return();
}

/* down interruptible entry handler */
static int down_en(struct semaphore *sem)
{
	printk(LOG_LEVEL "Sem down interruptible\n");
	jprobe_return();
	return 0;
}

/* mutex_lock entry handler */
static void mutex_lock_en(struct mutex *lock)
{
	printk(LOG_LEVEL "Mutex lock\n");
	jprobe_return();
}

/* mutex_unlock entry handler */
static void mutex_unlock_en(struct mutex *lock)
{
	printk(LOG_LEVEL "Mutex unlock\n");
	jprobe_return();
}

struct kretprobe **mem_probes = (struct kretprobe *[]){

	/* Kretprobe for kmalloc */
	& (struct kretprobe) {
		.kp = {
			.symbol_name = "__kmalloc"
		},
		.entry_handler	= kmalloc_eh,
		.handler	= kmalloc_h,
		.maxactive	= NR_CPUS,
		.data_size	= BUFFER_SIZE
	},

	/* Kretprobe for kfree */
	& (struct kretprobe) {
		.kp = {
			.symbol_name = "kfree"
		},
		.entry_handler	= kfree_eh,
		.handler	= kfree_h,
		.maxactive	= NR_CPUS,
		.data_size	= BUFFER_SIZE
	}
};

struct jprobe **func_probes = (struct jprobe *[]) {

	/* Jprobe for schedule */
	& (struct jprobe) {
		.kp = {
			.symbol_name = "schedule"
		},
		.entry = schedule_en
	},

	/* Jprobe for up */
	& (struct jprobe) {
		.kp = {
			.symbol_name = "up"
		},
		.entry = up_en
	},

	/* Jprobe for down_interruptible*/
	& (struct jprobe) {
		.kp = {
			.symbol_name = "down_interruptible"
		},
		.entry = down_en
	},

	/* Jprobe for mutex_lock */
	& (struct jprobe) {
		.kp = {
			.symbol_name = "mutex_lock_nested"
		},
		.entry = mutex_lock_en
	},

	/* Jprobe for mutex_unlock */
	& (struct jprobe) {
		.kp = {
			.symbol_name = "mutex_unlock"
		},
		.entry = mutex_unlock_en
	}
};
