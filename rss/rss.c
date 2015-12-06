#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/mm.h>
//#include<asm/pgtable.h>
#include<asm/pgtable_types.h>//for accessing _PAGE_BIT_ACCESSED macro
#include<asm/pgtable_32.h>
#include <linux/highmem.h>
#include <linux/timer.h>// for timer api
//#include<linux/delay.h>//for msleep	

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahendra");
MODULE_DESCRIPTION("Assignment#2 calculating RSS for given process-id.");

unsigned int pid=0;
module_param(pid,int,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

unsigned long s_time=3;//3 seconds
//module_param(s_time,long,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

static struct timer_list t_timer;

struct mm_struct *mm=NULL;
bool dbg=false;

//void check_rss(unsigned long data){
static void calculate_rss(void){
	//declaration.
	int ret=0;
	pgd_t *pgd;//page global directory pointer.
	pud_t *pud;//page upper directory.
	pmd_t *pmd;//page middle directory.
	pte_t *ptep, pte;//page table entry.
        unsigned long virt_addr;
	unsigned long rss=0,total=0;

	if(mm!=NULL){//check iff mm is not null.
		const struct vm_area_struct *vma = mm->mmap;
		// now iterate over the virtual addresses and fetch corresponding page table entry. Since we want ot check whether the page is present or not we can skip other 4095 virtual address(in case of PAGE_SIZE=4KB).
		if(dbg)
			printk("vma=%p\n",vma);
        	while (vma != NULL) {
			for (virt_addr = vma->vm_start; virt_addr < vma->vm_end; virt_addr += PAGE_SIZE) {
				total++;
				//pgd_offset macro accesses mm->pgd and performs addition operation with virt_address.
				pgd = pgd_offset(mm, virt_addr);
				if (pgd_none(*pgd) || pgd_bad(*pgd))//checks for validation for pgd entry, if it is not valid then terminate from these looping.
					continue;
				pud = pud_offset(pgd, virt_addr);
				if (pud_none(*pud) || pud_bad(*pud))//check for page upper directory entry validation.
					continue;
				//if architecture is 32 bit then pgd_val(pgd) and pud_val(pud) will have same value.
				pmd = pmd_offset(pud, virt_addr);
				if (pmd_none(*pmd) || pmd_bad(*pmd))//check for page middle directory entry validation.
					continue;
				//pte_offset is replaced by pte_offset_map 2.6 onwards.
				ptep = pte_offset_map(pmd, virt_addr);
				if (!ptep)
					continue;
				pte=*ptep;

				if(pte_present(pte))//page is in memory.
				{
					rss++;
				}
				//this must be at last.
				pte_unmap(ptep); 
        	    	}
	            	vma = vma->vm_next;
			//printk("vma=%p \tstart=%lu, end=%lu\n",vma,vma->vm_start,vma->vm_end);
        	}
		//printk("pgd_t=%llx \t pud_t=%llx \t pmd_t=%llx \tpte_t=%llx \tis_present=%d \ttotal=%u,rss=%u\n",pgd_val(*pgd),pud_val(*pud),pmd_val(*pmd),pte_val(*pte),pte_present(*pte),total,rss);
		printk("virtual_mem=%lukB,\trss=%lukB\n",(total*PAGE_SIZE)/1024,(rss*PAGE_SIZE)/(1024));

		//msleep(s_time);
	}
	else{
		printk("mm_struct is NULL\n");
	}
		//ret = mod_timer_msecs( &t_timer, s_time);
		ret = mod_timer( &t_timer, round_jiffies(jiffies + s_time * HZ));
                if(ret)
                        printk("Error in mod_timer\n");
		
		return;
}


/*
struct task_struct* get_task(int pid){
        struct task_struct *task,*to_ret=NULL;
	if(pid<=0){
		printk(KERN_INFO "Please give valid pid\n");
		return to_ret;
	}
        for_each_process(task)
        {
		//printk("name=%s pid=%d ppid=%d group_id=%d accu_rss=%llu\n",task->comm , task->pid, task->real_parent->pid,task->tgid,task->acct_rss_mem1);
                if(pid==task->pid){
                                to_ret=task;
                                break;
                        }
        }
	return to_ret;
}
*/

static struct task_struct *task=NULL;
static void check_rss(unsigned long temp){
	task=pid_task(find_vpid(pid), PIDTYPE_PID);
	if(dbg)
		printk("timer timeout %p\n",task);
	//task=find_task_by_vpid(pid);
	if(task!=NULL){
		mm=task->mm;
		if(dbg)
			printk("mm_struct=%p\n",mm);
		calculate_rss();
	}
	else{
		printk("task_struct is NULL\n");
	}
	//mm=NULL;
}

int init_module(void)
{
	printk("Module inserted\n");
        //task=get_task(pid);
	init_timer(&t_timer);
	t_timer.function=check_rss;
	t_timer.data = 1;
	//t_timer.expires = jiffies + HZ;
	t_timer.expires = round_jiffies(jiffies + s_time * HZ);
	//t_timer.expires = jiffies + msecs_to_jiffies(s_time);//in milisecs
	add_timer( &t_timer);
	return 0;
}

void cleanup_module(void)
{
	int ret;
	ret = del_timer_sync( &t_timer );
	if(ret)
		printk("The timer is still in use...\n");
	printk(KERN_INFO "Module Unloaded.\n");
	return;
}

/*

references:-
https://www.kernel.org/doc/gorman/html/understand/understand006.html
http://stackoverflow.com/questions/15994603/sleep-in-linux-kernel-space
http://www.ibm.com/developerworks/library/l-timers-list/
http://osdir.com/ml/linux-kernel/2009-05/msg07172.html
*/
