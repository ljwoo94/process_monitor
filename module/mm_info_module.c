#include <linux/module.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/interrupt.h>

//header file for data structure.
#include "mm_info_module.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("2014190150_Jungwoo Lee");
MODULE_DESCRIPTION("MM informations of all user processes");

static char *pid_name;

//default directory
#define MYDIR "hw2"

//default period value when period is not entered with module
static int period = 5;
module_param(period, int, 0000);

static struct proc_dir_entry *dir = NULL;
static struct proc_dir_entry *mm_file = NULL;

//Use of seq_file data structure.
//seq_file works with step of
//start -> show -> next -> show ->... -> stop
static void *mm_start(struct seq_file *s, loff_t *pos)
{
    s->private = "";   
    return seq_list_start(&mm_list, *pos);
}

//iterate to the next entry of data list.
static void *mm_next(struct seq_file *s, void *v, loff_t *pos)
{
    s->private = "\n";   
    return seq_list_next(v, &mm_list, pos);
}

//finish iteration
static void mm_stop(struct seq_file *s, void *v)
{
    printk(KERN_INFO "s", __func__);
}

//for Print bar operation
static void print_bar(struct seq_file *m)
{
	seq_printf(m, "******************************************************************\n");
}

//when cat operation is invoked
static int mm_show(struct seq_file *s, void *v) 
{
    struct mm_info *info = list_entry(v, struct mm_info, list);

	//to save pid of task in entry.
    char s_pid[7];
    //printk("FILENAME: sprintf Part");
    sprintf(s_pid, "%d", info->pid);

	//compare pid
    if(strcmp(pid_name, s_pid) == 0)
    {
		//print format as given.
		printk("[FILENAME] , called process: %s\n", pid_name);
        print_bar(s);
			seq_printf(s,"Virtual Memory Address Information\n");
			seq_printf(s, "Process (%15s:%lu)\n", info->comm, info->pid);
			seq_printf(s,"Last update time %ld ms\n", info->up_time);
		
			print_bar(s);

			//Code Area
			seq_printf(s,"0x%08lx - 0x%08lx : Code Area, %lu page(s)\n",
					info->mm->start_code, info->mm->end_code, 
					(info->mm->end_code - info->mm->start_code)/4092);

			//Data area
			seq_printf(s,"0x%08lx - 0x%08lx : Data Area, %lu page(s)\n",
					info->mm->start_data, info->mm->end_data, 
					(info->mm->end_data - info->mm->start_data)/4092);
			
			//BSS area
			seq_printf(s,"0x%08lx - 0x%08lx : BSS Area, %lu page(s)\n",
					info->mm->end_data, info->mm->start_brk, 
					(info->mm->start_brk - info->mm->end_data)/4092);

			//Heap Area
			seq_printf(s,"0x%08lx - 0x%08lx : Heap Area, %lu page(s)\n",
						info->mm->start_brk, info->mm->brk, 
						(info->mm->brk - info->mm->start_brk)/4092);
			
			//Shared Libraries Area include all
			seq_printf(s,"0x%08lx - 0x%08lx : Shared Libraries Area, %lu page(s)\n",		
					info->mm->mmap->vm_start, info->mm->mmap->vm_end, 
					(info->mm->mmap->vm_end - info->mm->mmap->vm_start)/4092);
			
			//Stack Area
			seq_printf(s,"0x%08lx - 0x%08lx : Stack Area, %lu page(s)\n",		
					info->mm->start_stack, info->mm->arg_start, 
					(info->mm->arg_start - info->mm->start_stack)/4092);

			print_bar(s);

			//1st level = pgd
			seq_printf(s,"1 Level Paging: Page Global Directory Entry Information\n");			
			print_bar(s);
			
			//er4 register value
			seq_printf(s,"PGD	Base Address		: 0x%08lx\n", info->mm->pgd);

			//temporary variable to use easily with data structre.
			pgd_t *pgd;
			pgd = info->pgd;
			seq_printf(s,"code	PGD  Address		: 0x%08lx\n", pgd);
			
			pgdval_t pgd_value;
			pgd_value = pgd_val(*pgd);
			//no need to cut out accroding to the FAQs
			//value of selected entry
			seq_printf(s,"	PGD  Value		: 0x%08lx\n", pgd_value);
			seq_printf(s,"	+PFN Addrerss		: 0x%08lx\n", pgd_pfn(*pgd));

			//use Flag and the given definitions in kernel,
			//Use AND operation for mask.
			pgdval_t pgd_flag;
			pgd_flag = pgd_flags(*pgd);

			pgdval_t pgd_pse;
			pgd_pse = pgd_flag & _PAGE_PSE;
			if(!pgd_pse)
				seq_printf(s,"	+Page Size		: %s\n","4KB");
			else
				seq_printf(s,"	+Page Size		: %s\n","4MB" );
			
			//check accessed bit flag
			pgdval_t pgd_acc;
			pgd_acc = pgd_flag & _PAGE_ACCESSED;
			if(!pgd_acc)
				seq_printf(s,"	+Accessed Bit		: %s\n", "0");
			else
				seq_printf(s,"	+Accessed Bit		: %s\n", "1");
			
			//check CDB flag
			pgdval_t pgd_pcd;
			pgd_pcd = pgd_flag & _PAGE_PCD;
			if(!pgd_pcd)
				seq_printf(s,"	+Cache Disable Bit	: %s\n", "true");
			else
				seq_printf(s,"	+Cache Disable Bit	: %s\n", "false");

			//check PWT flag
			pgdval_t pgd_pwt;
			pgd_pwt = pgd_flag & _PAGE_PWT;
			if(!pgd_pwt)
				seq_printf(s,"	+Page Write-Through	: %s\n", "write-back");
			else	
				seq_printf(s,"	+Page Write-Through	: %s\n", "write-through");

			//check the type of task
			pgdval_t pgd_user;
			pgd_user = pgd_flag & _PAGE_USER;
			if(!pgd_user)
				seq_printf(s,"	+User/Supervisor Bit	: %s\n", "supervisor");
			else
				seq_printf(s,"	+User/Supervisor Bit	: %s\n", "user");
			
			//check file type flag
			pgdval_t pgd_rw;
			pgd_rw = pgd_flag & _PAGE_RW;
			if(!pgd_rw)
				seq_printf(s,"	+Read/Write Bit		: %s\n", "read-only");
			else
				seq_printf(s,"	+Read/Write Bit		: %s\n", "read-write");

			//check wheter the page actually present in physical memory
			int pgd_pre;
			pgd_pre = pgd_present(*pgd);
			if(!pgd_pre)
				seq_printf(s,"	+Page Present Bit	: %s\n", "0");
			else
				seq_printf(s,"	+Page Present Bit	: %s\n", "1");

			//2 Level Paging
			print_bar(s);
			seq_printf(s,"2 Level Paging: Page Upper Directory Entry Information\n");
			print_bar(s);

			//no use of p4d. pgtable_l5_enabled == 0. Thus just return pgd
			//Thus p4d == pgd
			pud_t *pud;
			pud = info->pud;
			seq_printf(s,"code	PUD  Address		: 0x%08lx\n", pud);
			seq_printf(s,"	PUD  Value		: 0x%08lx\n", pud_val(*pud));
			seq_printf(s,"	+PFN Address		: 0x%08lx\n", pud_pfn(*pud));

			//3 Level Paging
			print_bar(s);	
			seq_printf(s,"3 Level Paging: Page Middle Directory Entry Information\n");
			print_bar(s);

			pmd_t *pmd;
			pmd = info->pmd;
			seq_printf(s,"code	PMD  Address		: 0x%08lx\n", pmd);
			seq_printf(s,"	PMD  Value		: 0x%08lx\n", pmd_val(*pmd));
			seq_printf(s,"	+PFN Address		: 0x%08lx\n", pmd_pfn(*pmd));

			//4 level paging - The last	
			print_bar(s);	
			seq_printf(s,"4 Level Paging: Page Table Entry Information\n");
			print_bar(s);

			pte_t *pte;
			pte = info->pte;
			seq_printf(s,"code	PTE  Address		: 0x%08lx\n", pte);
			seq_printf(s,"	PTE  Value		: 0x%08lx\n", pte_val(*pte));
			seq_printf(s,"	+Page Base Address	: 0x%08lx\n", pte_pfn(*pte));
			
			//same way as we checked flag of entry in PGD entry
			pteval_t pte_flag;
			pte_flag = pte_flags(*pte);

			int pte_dir;
			pte_dir = pte_dirty(*pte);
			if(!pte_dir)
				seq_printf(s,"	+Dirty Bit		: %s\n","0" );
			else
				seq_printf(s,"	+Dirty Bit		: %s\n","1" );
			
			int pte_acc;
			pte_acc = pte_young(*pte);
			if(!pte_acc)
				seq_printf(s,"	+Accessed Bit		: %s\n","0" );
			else
				seq_printf(s,"	+Accessed Bit		: %s\n","1" );
			
			pteval_t pte_pcd;
			pte_pcd = pte_flag & _PAGE_PCD;
			if(!pte_pcd)
				seq_printf(s,"	+Cache Disable Bit	: %s\n", "false");
			else
				seq_printf(s,"	+Cache Disable Bit	: %s\n", "true");
			
			pteval_t pte_pwt;
			pte_pwt = pte_flag & _PAGE_PWT;
			if(!pte_pwt)
				seq_printf(s,"	+Page Write-Through	: %s\n", "write-back");
			else	
				seq_printf(s,"	+Page Write-Through	: %s\n", "write-through");
		
			pteval_t pte_user;
			pte_user = pte_flag & _PAGE_USER;
			if(!pte_user)
				seq_printf(s,"	+User/Supervisor Bit	: %s\n", "kernel" );
			else
				seq_printf(s,"	+User/Supervisor Bit	: %s\n", "user");
						
			int pte_rw;
			pte_rw = pte_write(*pte);
			if(!pte_rw)
				seq_printf(s,"	+Read/Write Bit		: %s\n", "read-only");
			else
				seq_printf(s,"	+Read/Write Bit		: %s\n", "read-write");
			
			int pte_pres;
			pte_pres = pte_present(*pte);
			if(!pte_pres)
				seq_printf(s,"	+Page Present Bit	: %s\n", "0");
			else
				seq_printf(s,"	+Page Present Bit	: %s\n", "1");
			
		
			print_bar(s);
			//LSB 12 == flags thus shift, and get physical address
			pteval_t str_addr = pte_val(*pte) << 12;
			seq_printf(s,"Start of Physical Address	: 0x%08lx\n", str_addr);
			print_bar(s);
    }

    return 0;
}

//seq_operations that will be used in seq file.
static const struct seq_operations mm_seq_ops = 
{
    .start = mm_start,
    .next = mm_next,
    .stop = mm_stop,
    .show = mm_show,
};

//the function that will be invoked when the file is called to read.
static int mm_proc_open(struct inode *inode, struct file *file) 
{
    //get file name we want to load.
    pid_name = file->f_path.dentry->d_name.name;
    printk("%s, is a file pid name [FILENAME]", pid_name);
    return seq_open(file, &mm_seq_ops);
}

//file operations of fs.
static const struct file_operations mm_proc_ops = {

    .owner = THIS_MODULE,
    .open = mm_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release,
};

//head of list that save all the data.
LIST_HEAD(mm_list);

//function to add data in the list according to given task.
static int add_data(struct task_struct *tsk)
{
    struct mm_info *info;

    //printk(KERN_INFO "kzalloc invoked");
	//to maintain data.
    info = kzalloc(sizeof(*info), GFP_KERNEL);
    if(!info)
        return -1;

    //printk("add info into list");
	//use single linked list and add new entry as head of list.
    INIT_LIST_HEAD(&info->list);

	//add all data into entry of list of data structure
    struct mm_struct *mm;
    mm = tsk->mm;
    info->pid = tsk->pid;
    info->comm = tsk->comm;
    info->mm = mm;

	info->pgd = pgd_offset(mm, mm->mmap->vm_start);
	pgd_t *tmp_pgd = info->pgd;
    info->pud = pud_offset((p4d_t *)tmp_pgd, mm->mmap->vm_start);
    info->pmd = pmd_offset(info->pud, mm->mmap->vm_start);
    info->pte = pte_offset_kernel(info->pmd, mm->mmap->vm_start);   
	info->up_time = jiffies_to_msecs(jiffies);

    //printk("mm_list update");
	//add entry to list
    list_add(&info->list, &mm_list);

    return 0;
}

//to prevent leaking memory.
static int remove_data(void)
{
    struct mm_info *tmp;
    struct list_head *node, *q;
    list_for_each_safe(node, q, &mm_list)
    {
        tmp = list_entry(node, struct mm_info, list);
        list_del(node);
        kfree(tmp);
    }
    return 0;
}

//tasklet handler function.
//remove previous data,files,dirctories -> update new data -> make directories and files
static void tasklet_handler_function(unsigned long data)
{
	printk("Tasklet handling");
	//remove preivous if exists
    remove_proc_subtree(MYDIR, NULL);
	if(mm_file != NULL)
    	proc_remove(mm_file);
    if(dir != NULL)
		proc_remove(dir);

	//remove data from the info list
	remove_data();
	//update data of current time
	struct task_struct *tsk;
    struct mm_struct *mm;
    char s_pid[7];
    int tmp_pid;
    struct task_struct *to_update;
    struct mm_struct *tmp_mm_to_update;
    //printk("Data updating...[MMINFO]");
    for_each_process(to_update)
    {
		//to check whether the process is a user process or kernel process
		//kernel process have NULL *mm value.
        tmp_mm_to_update = to_update->mm;
        if(tmp_mm_to_update != NULL) 
        {
            add_data(to_update);
        }
    }

	//directory make
	//printk("make directory [MMINFO]");
    dir = proc_mkdir(MYDIR, NULL);
    if(dir == NULL)
    {
        printk("proc_mkdir failed [MM_INFO]");
        return -1;
    }

	//make each proc files for each tasks
	//printk("Make each files[MMINFO]");
    for_each_process(tsk)
    {
		//check the user/kernel process
        mm = tsk->mm;
        if(mm!=NULL)
        {
			//to use as a file name.
            tmp_pid = (int) tsk->pid;
            sprintf(s_pid, "%d", tmp_pid);
            
			//create proc files for each process
            mm_file = proc_create(s_pid, 0, dir, &mm_proc_ops);
            if(mm_file == NULL)
            {
                printk("Cannot proc_create [MM_INFO]");
                remove_proc_entry(s_pid, NULL);
                return -1;
            }
        }
    }
}

//Declare my tasklet for updata information
DECLARE_TASKLET(update_tasklet, tasklet_handler_function, 0);

//dynamic timer to update information for every given period
static struct timer_list update_timer;

//function that will be called after timeout of timer.
static void update_timer_callback(struct timer_list *timer)
{
	int ret;
	//schedule tasklet and updata information.
	tasklet_schedule(&update_tasklet);

	//use mod_timer to update expire time and restart timer.
	ret = mod_timer(&update_timer, jiffies + (period*HZ));
	if(ret)
		pr_err("%s: timer failed to update");
}

//initiallize module.
static int proc_init(void)
{
	//use tasklet to get information
	tasklet_schedule(&update_tasklet);

	//set up initial timer. set the expire time after period secs.
	printk("timer initialized at %ld", jiffies_to_msecs(jiffies)/1000);
	timer_setup(&update_timer, update_timer_callback, 0);
	int ret;
	ret = mod_timer(&update_timer, jiffies + (period*HZ));
    if(ret)
	{
		pr_err("Timer failed at initial");
	}

    return 0;

}

//Unload module. 
//Must remove all the allocated data
//Must kill tasklet and timer.
//Must delete directory and all the proc files.
static void proc_exit(void)
{
    remove_data();
    remove_proc_subtree(MYDIR, NULL);
    proc_remove(mm_file);
    proc_remove(dir);

	int ret;
	ret = del_timer(&update_timer);
	if(ret)
		pr_err("%s: The timer is still is use ...\n", __func__);
	
	printk("%s: Timer module unloaded\n", __func__);
	tasklet_kill(&update_tasklet);

    printk(KERN_INFO "Remove Module success\n");
}

module_init(proc_init);
module_exit(proc_exit);
