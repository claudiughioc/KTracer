/*
 * Claudiu Ghioc 341 C1
 * KTracer - Kprobe based tracer
 */

#include "ktracer.h"

struct proc_dir_entry *proc_kt;
DEFINE_SPINLOCK(proc_lock);

/* Print information for a single process */
static int print_proc_info(int pid, char *buff)
{
	struct proc_info *p_info;
	struct hlist_node *i;
	int tlen;

	hash_for_each_possible(procs, p_info, i, hlh, pid) {
		if (p_info->pid != pid)
			continue;

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
	return 0;
}

/*
 * Read function for the proc_entry used by the tracer
 */
int tracer_read(char *buff, char **buff_start, off_t off, int buff_len,
	int *eof, void *data)
{
	int len, k = 0, curr_pid, aux_pid, pid_diff, finished;
	struct hlist_node *i, *tmp;
	struct proc_info *p_info;
	curr_pid = aux_pid =-1;
	pid_diff = PID_MAX;
	finished = 0;


	/* The header line */
	len = sprintf(buff, "PID   kmalloc  kfree  kmalloc_mem  kfree_mem  "
		"sched  up   down  lock  unlock\n");

	/* Print process information starting with the smallest pid */
	while (1) {
		finished = 1;
		pid_diff = PID_MAX;

		/* Get the smallest pid in aux_pid */
		hash_for_each_safe(procs, k, i, tmp, p_info, hlh) {
			if (p_info->pid > curr_pid &&
				p_info->pid - curr_pid < pid_diff) {
				aux_pid = p_info->pid;
				pid_diff = p_info->pid - curr_pid;
				finished = 0;
			}
		}
		if (finished)
			break;

		/* Print information for the current pid */
		len += print_proc_info(aux_pid, buff + len);
		curr_pid = aux_pid;
	}

	*eof = 1;
	return len;
}
