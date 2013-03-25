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
static void print_proc_info(struct seq_file *s, int pid)
{
	struct proc_info *p_info;
	struct hlist_node *i;

	hash_for_each_possible(procs, p_info, i, hlh, pid) {
		if (p_info->pid != pid)
			continue;

		seq_printf(s, "%-6d%-9lld%-7lld%-13lld%-11lld"
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
		return;
	}
}

/*
 * The start function is called at the begginig of the iteration
 */
static void *tr_seq_start(struct seq_file *s, loff_t *pos)
{
	/* The ending condition for iterator */
	if (*pos > 0)
		return NULL;

	*pos = -1;

	/* The return value can be ignored */
	return pos;
}

/*
 * The next function moves the iterator to the next element;
 * It returns NULL if there is nothing left in the sequence;
 * For this device it returns the next smalles pid.
 */
static void *tr_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	int last_pid, aux_pid, pid_diff, found, k;
	struct proc_info *p_info;
	struct hlist_node *i, *tmp;


	/* Get the last pid from position index */
	last_pid = *pos;
	aux_pid = -1;
	pid_diff = PID_MAX;
	k = found = 0;


	/* Get the smallest pid in aux_pid */
	hash_for_each_safe(procs, k, i, tmp, p_info, hlh)
		if (p_info->pid > last_pid &&
				p_info->pid - last_pid < pid_diff) {
			aux_pid = p_info->pid;
			pid_diff = p_info->pid - last_pid;
			found = 1;
		}

	/* Finish iteration if nothing found */
	if (!found)
		return NULL;
	*pos = aux_pid;
	return pos;
}

/*
 * When the kernel is done with the iterator it calls "stop"
 * to clean up eventual data.
 */
static void tr_seq_stop(struct seq_file *s, void *v){};

/*
 * In between the calls to iterator the kernel calls
 * "show" to output information to userspace
 */
static int tr_seq_show(struct seq_file *s, void *v)
{
	int *pid = (int *)v;

	/* Print the header line */
	if (*pid < 0)
		seq_printf(s, "PID   kmalloc  kfree  kmalloc_mem  kfree_mem  "
				"sched  up   down  lock  unlock\n");
	else
		/* Print a process info */
		print_proc_info(s, *pid);

	return 0;
}

static struct seq_operations seq_ops = {
	.start = tr_seq_start,
	.next = tr_seq_next,
	.stop = tr_seq_stop,
	.show = tr_seq_show
};

/*
 * The open function that connects the proc file and the seq operations
 */
static int tr_proc_open(struct inode *inode, struct file *filp)
{
	return seq_open(filp, &seq_ops);
}

/* File operations for our file "/proc/tracer" */
struct file_operations tr_proc_ops = {
	.owner = THIS_MODULE,
	.open = tr_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};
