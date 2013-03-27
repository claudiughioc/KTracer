/*
 * Claudiu Ghioc 341 C1
 * KTracer - Kprobe based tracer
 *
 * Implementation of read function for /proc/tracer
 * based on seq_files using an iterator
 */

#include "ktracer.h"

struct proc_dir_entry *proc_kt;
DEFINE_SPINLOCK(proc_lock);

/* Print information for a single process */
static int print_proc_info(struct proc_info *p_info, char *buff)
{
	int tlen;

	tlen = sprintf(buff, "%-6d%-9lld%-7lld%-13lld%-11lld"
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
	return tlen;
}

/*
 *  * Read function for the proc_entry used by the tracer
 *   */
int tracer_read(char *buff, char **buff_start, off_t off, int buff_len,
		    int *eof, void *data)
{
	int len, k = 0, curr_pid, aux_pid, pid_diff, finished;
	struct hlist_node *i;
	struct proc_info *p_info;
	curr_pid = aux_pid =-1;
	pid_diff = PID_MAX;
	finished = 0;


	/* The header line */
	len = sprintf(buff, "PID   kmalloc  kfree  kmalloc_mem  kfree_mem  "
			"sched  up   down  lock  unlock\n");

	for (k = 0; k < HASH_SIZE(procs); k++) {
		spin_lock(&hlocks[k]);

		hlist_for_each_entry(p_info, i, &procs[k], hlh) {
			printk("Print info for pid %d\n", p_info->pid);
			len += print_proc_info(p_info, buff + len);
		}

		spin_unlock(&hlocks[k]);
	}

	*eof = 1;
	return len;
}
