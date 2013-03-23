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

#ifndef BUFSIZ
#define BUFSIZ		4096
#endif

extern struct kretprobe **mem_probes;
extern struct jprobe **func_probes;

struct proc_info {
	int pid;
	atomic64_t results[FUNCTION_NO];
	struct hlist_node hlh;
};

#endif
