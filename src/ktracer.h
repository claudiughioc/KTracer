#ifndef __KTRACER_H__
#define __KTRACER_H__		1

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/hashtable.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/cdev.h>
#include <linux/kprobes.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include "tracer.h"

#define BUFFER_SIZE		256

#define LOG_LEVEL		KERN_ALERT

#define TRACER_MAJOR		10
#define NUM_MINORS		1
#define FUNCTION_NO		9
#define JPROBE_NO		6

#define KMALLOC_INDEX		0
#define KFREE_INDEX		1
#define KMALLOC_MEM_INDEX	2
#define KFREE_MEM_INDEX		3
#define SCHEDULE_INDEX		4
#define UP_INDEX		5
#define DOWN_INT_INDEX		6
#define MUTEX_LCK_INDEX		7
#define MUTEX_ULK_INDEX		8

#define MY_HASH_BITS		8
#define MY_HASH_SIZE		(1 << MY_HASH_BITS)

#define PROC_FILE		"tracer"
#define PROC_MODE		0444
#define PID_MAX			65000

#ifndef BUFSIZ
#define BUFSIZ			4096
#endif

extern void *sys_call_table[];
extern struct kretprobe *mem_probe;
extern struct jprobe **func_probes;
extern struct hlist_head procs[MY_HASH_SIZE];
extern struct proc_dir_entry *proc_kt;

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

int tracer_read(char *page, char **start, off_t off, int count,
	int *eof, void *data);
#endif
