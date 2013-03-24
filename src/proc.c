/*
 * Claudiu Ghioc 341 C1
 * KTracer - Kprobe based tracer
 */

#include "ktracer.h"

struct proc_dir_entry *proc_kt;
DEFINE_SPINLOCK(proc_lock);

/*
 * Read function for the proc_entry used by the tracer
 */
int tracer_read(char *buff, char **buff_start, off_t off, int buff_len,
	int *eof, void *data)
{
	int len, k = 0;
	struct hlist_node *i, *tmp;
	struct proc_info *p_info;


	/* The header line */
	len = sprintf(buff, "PID  kmalloc  kfree  kmalloc_mem  kfree_mem  "
		"sched  up   down  lock  unlock\n");


	/* Print information for every process monitored */
	hash_for_each_safe(procs, k, i, tmp, p_info, hlh) {
		len += sprintf(buff + len, "%-5d%-9lld%-7lld%-13lld%-11lld"
			"%-7lld%-5lld%-6lld%-6lld%-6lld\n",
			p_info->pid,
			atomic64_read(&p_info->results[KMALLOC_INDEX]),
			atomic64_read(&p_info->results[KFREE_INDEX]),
			atomic64_read(&p_info->results[KMALLOC_MEM_INDEX]),
			atomic64_read(&p_info->results[KFREE_MEM_INDEX]),
			atomic64_read(&p_info->results[SCHEDULE_INDEX]),
			atomic64_read(&p_info->results[UP_INDEX]),
			atomic64_read(&p_info->results[DOWN_INT_INDEX]),
			atomic64_read(&p_info->results[MUTEX_LCK_INDEX]),
			atomic64_read(&p_info->results[MUTEX_ULK_INDEX]));
	}
	*eof = 1;
	return len;
}
