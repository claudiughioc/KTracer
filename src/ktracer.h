#ifndef __KTRACER_H__
#define __KTRACER_H__		1

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/hashtable.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/cdev.h>
#include <linux/kprobes.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include "tr_ioctl.h"

#define BUFFER_SIZE		256

#define LOG_LEVEL	KERN_ALERT

#define TRACER_MAJOR		10
#define TRACER_MINOR		42
#define NUM_MINORS		1
#define MODULE_NAME		"tracer"
#define FUNCTION_NO		9
#define KRETPROBE_NO		2
#define JPROBE_NO		5

#define KMALLOC_INDEX			0
#define KFREE_INDEX			1
#define KMALLOC_MEM_INDEX		2
#define KFREE_MEM_INDEX			3
#define SCHEDULE_INDEX			4
#define UP_INDEX			5
#define DOWN_INT_INDEX			6
#define MUTEX_LCK_INDEX			7
#define MUTEX_ULK_INDEX			8

#define MY_HASH_BITS		8
#define MY_HASH_SIZE	(1 << MY_HASH_BITS)

#ifndef BUFSIZ
#define BUFSIZ		4096
#endif

extern void *sys_call_table[];
extern struct kretprobe **mem_probes;
extern struct jprobe **func_probes;
extern struct hlist_head procs[MY_HASH_SIZE];

struct proc_info {
	int pid;
	atomic64_t results[FUNCTION_NO];
	struct list_head mm;
	struct hlist_node hlh;
};

struct mem_data {
	long address;
	long size;
	struct list_head lh;
};

#endif
