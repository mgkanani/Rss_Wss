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
MODULE_DESCRIPTION("Assignment#2 calculating WSS for given process-id.");

unsigned int pid=0;
module_param(pid,int,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

unsigned long s_time=2;//2 seconds
static struct timer_list t_timer;
struct mm_struct *mm=NULL;
static unsigned long wss=0;


void reset(void){
	//declaration.
	pgd_t *pgd;//page global directory pointer.
	pud_t *pud;//page upper directory.
	pmd_t *pmd;//page middle directory.
	pte_t *ptep, pte;//page table entry.
        unsigned long virt_addr;
	if(mm!=NULL){
		const struct vm_area_struct *vma = mm->mmap;
        	while (vma != NULL) {
			for (virt_addr = vma->vm_start; virt_addr < vma->vm_end; virt_addr += PAGE_SIZE) {
				pgd = pgd_offset(mm, virt_addr);
				if (pgd_none(*pgd) || pgd_bad(*pgd))
					continue;
				pud = pud_offset(pgd, virt_addr);
				if (pud_none(*pud) || pud_bad(*pud))
					continue;
				pmd = pmd_offset(pud, virt_addr);
				if (pmd_none(*pmd) || pmd_bad(*pmd))//check for page middle directory entry validation.
					continue;
				ptep = pte_offset_map(pmd, virt_addr);
				if (!ptep)
					continue;
				pte=*ptep;
				if(pte_present(pte))
				{
					set_bit(_PAGE_BIT_PRESENT,(unsigned long *) &pte);
					clear_bit(_PAGE_BIT_PROTNONE,(unsigned long *) &pte);
					*ptep=pte;
				        pte_update(mm, virt_addr, ptep);
				}
				pte_unmap(ptep); 
        	    	}
	            	vma = vma->vm_next;
        	}
	}
}


static void wss_setbits(void){
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

				if(pte_present(pte))//if page is in memory, then clear present bit and set protnone bit.
				{
					rss++;
					//clear accessed bit for acounting wss.
					//clear_bit(_PAGE_BIT_ACCESSED,(unsigned long *) &pte);
					clear_bit(_PAGE_BIT_PRESENT,(unsigned long *) &pte);
					set_bit(_PAGE_BIT_PROTNONE,(unsigned long *) &pte);
					*ptep=pte;
				        pte_update(mm, virt_addr, ptep);
				}
				//this must be at last.
				pte_unmap(ptep); 
        	    	}
	            	vma = vma->vm_next;
        	}
		printk("virtual_mem=%lukB,\trss=%lukB,\twss=%lukB\n",(total*PAGE_SIZE)/1024,(rss*PAGE_SIZE)/(1024),(wss*PAGE_SIZE)/(1024));
		wss=0;
	}else{
		printk("mm_struct is null\n");
	}
	ret = mod_timer( &t_timer, round_jiffies(jiffies + s_time * HZ));
        if(ret)
       		printk("Error in mod_timer\n");
}




extern void (*pnt_wss)(unsigned int,struct mm_struct *, unsigned long);
static void (*orig_ref)(unsigned int,struct mm_struct *, unsigned long );
//hook function.
void calculate_wss(unsigned int temp_pid, struct mm_struct *mm, unsigned long virt_addr){

        pgd_t *pgd;//page global directory pointer.
        pud_t *pud;//page upper directory.
        pmd_t *pmd;//page middle directory.
        pte_t *ptep, pte;//page table entry.
//        unsigned long temp_addr;
	if(pid==temp_pid && mm!=NULL){
//check whether address is within vma.
		const struct vm_area_struct *vma = mm->mmap;
		if(vma==NULL)
			goto out;
	        pgd = pgd_offset(mm, virt_addr);
                if (pgd_none(*pgd) || pgd_bad(*pgd))//checks for validation for pgd entry,if it is not valid then terminate from these looping.
                	goto out;
                pud = pud_offset(pgd, virt_addr);
                if (pud_none(*pud) || pud_bad(*pud))//check for page upper directory entry validation.
                        goto out;
                //if architecture is 32 bit then pgd_val(pgd) and pud_val(pud) will have same value.
                pmd = pmd_offset(pud, virt_addr);
                if (pmd_none(*pmd) || pmd_bad(*pmd))//check for page middle directory entry validation.
			goto out;
		//pte_offset is replaced by pte_offset_map 2.6 onwards.
		ptep = pte_offset_map(pmd, virt_addr);
		if (!ptep)
			goto out;
		pte=*ptep;

		if((pte_flags(pte) & (_PAGE_PROTNONE)) && !(pte_flags(pte) & (_PAGE_PRESENT)))//page is in memory.
		{
			wss++;
			set_bit(_PAGE_BIT_PRESENT,(unsigned long *) &pte);
			clear_bit(_PAGE_BIT_PROTNONE,(unsigned long *) &pte);
			*ptep=pte;
                        pte_update(mm, virt_addr, ptep);
		}
		pte_unmap(ptep);
	}
	out:
		return;
}

struct task_struct* get_task(int pid){
        struct task_struct *task,*to_ret=NULL;
	if(pid<=0){
		printk(KERN_INFO "Please give valid pid\n");
		return to_ret;
	}
        for_each_process(task)
        {
                if(pid==task->pid){
                                to_ret=task;
                                break;
                        }
        }
	return to_ret;
}


static void check_rss(unsigned long temp){
        struct task_struct *task=NULL;
        //task=get_task(pid);
	struct pid *pid_struct = find_get_pid(pid);
        task=pid_task(pid_struct,PIDTYPE_PID);
	if(task!=NULL){
		mm=task->mm;
		//printk("Time out %p\n",mm);
		wss_setbits();
	}else{
		printk("task_struct is null\n");
	}
	mm=NULL;
}

int init_module(void)
{
	orig_ref    = pnt_wss;
	pnt_wss = &calculate_wss;
	printk("Module inserted\n");
	init_timer(&t_timer);
	t_timer.function=check_rss;
	t_timer.data = 1;
	t_timer.expires = round_jiffies(jiffies + s_time * HZ);
	add_timer( &t_timer);
	return 0;
}

void cleanup_module(void)
{
	int ret;
	pnt_wss=orig_ref;
	reset();
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
