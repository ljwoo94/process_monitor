#ifndef _MM_INFO_MODULE_H_
#define _MM_INFO_MODULE_H_

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/string.h>
#include <asm/pgtable.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/raid/pq.h>
#include <linux/list.h>

struct mm_info {
	
	pid_t pid;
	char *comm;
	unsigned long up_time;
	struct mm_struct *mm;
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

    struct list_head list;
};

extern struct list_head mm_list;
#endif