
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

extern customers;
extern barbers;
extern mutex;

extern c_start;	//下一个可以直接赋值的位置
extern c_end;		//上一个可以直接取值的位置
extern b_start;
extern b_end;
extern m_start;
extern m_end; 

int customerId;
int waiting;
int CHAIRS;

/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
        u8              privilege;
        u8              rpl;
        int             eflags;
	for (i = 0; i < NR_TASKS+NR_PROCS; i++) {
                if (i < NR_TASKS) {     /* 任务 */
                        p_task    = task_table + i;
                        privilege = PRIVILEGE_TASK;
                        rpl       = RPL_TASK;
                        eflags    = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1 */
                }
                else {                  /* 用户进程 */
                        p_task    = user_proc_table + (i - NR_TASKS);
                        privilege = PRIVILEGE_USER;
                        rpl       = RPL_USER;
                        eflags    = 0x202; /* IF=1, bit 2 is always 1 */
                }

		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
		p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = eflags;

		p_proc->nr_tty = 0;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	proc_table[0].ticks = proc_table[0].priority =  1;
	proc_table[1].ticks = proc_table[1].priority =  50;
	proc_table[2].ticks = proc_table[2].priority =  50;
	proc_table[3].ticks = proc_table[3].priority =  50;
	
	//Added by lionel
	proc_table[4].ticks = proc_table[4].priority =  50;
	proc_table[5].ticks = proc_table[5].priority =  50;	

	//Added by lionel
        //proc_table[1].nr_tty = 0;
        //proc_table[2].nr_tty = 1;
        //proc_table[3].nr_tty = 1;

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	init_clock();
        init_keyboard();

	//Added by lionel
	customers = 0;
	barbers = 0;
	mutex = 1;

	c_start = 0;	//下一个可以直接赋值的位置
	c_end = 0;		//上一个可以直接取值的位置
	b_start = 0;
	b_end = 0;
	m_start = 0;
	m_end = 0;
	
	customerId = 0;
	waiting = 0;
	CHAIRS = 3;
	
	disp_pos=0;
	for(i=0;i<80*25;i++){
		disp_str(" ");
	}
	disp_pos=0;
	
	restart();
	
	while(1){}
}

/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	int i = 0;
	while (1) {
		user_disp_color_str("Normal",1);
		//disp_int(disp_pos);
		//printf("<Ticks:%x>", get_ticks());
		//user_process_sleep(1000);
		//user_sem_p("customer");
		//sleep();
		milli_delay(10000);
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	int i = 0x1000;
	while(1){
 		user_sem_p("customers");
		user_sem_p("mutex");
		waiting--;
		user_sem_v("barbers");
		user_sem_v("mutex");
		//user_disp_color_str("\n",2);
		user_disp_color_str("cutHair\n",2);
		//disp_int(disp_pos);
		//睡两个时间片
		user_process_sleep(20); 
		
		//milli_delay(200);
	}
}

/*======================================================================*
                               TestC
 *======================================================================*/
void TestC()
{
	int i = 0x2000;
	while(1){
 		user_sem_p("mutex");
		
		customerId ++;
		int customer = customerId;
		
		if(waiting < CHAIRS){
			disp_int(customer);
			user_disp_str("wait");
			user_disp_str("\n");
			//disp_int(disp_pos);
			waiting ++;
			user_sem_v("customers");
			user_sem_v("mutex");
			user_sem_p("barbers");
			//user_disp_color_str("\n",3);
			disp_int(customer);
			user_disp_str("get cut");
			user_disp_str("\n");
			//disp_int(disp_pos);
		}
		else{
			disp_int(customer);
			user_disp_str("full!");
			user_disp_str("\n");
			user_sem_v("mutex");
		}
		disp_int(customer);
		user_disp_str("leave");
		user_disp_str("\n");
		//disp_int(disp_pos);
		user_process_sleep(10);
	}
}

void TestD()
{
	int i = 0x3000;
	
	while(1){
 		user_sem_p("mutex");
		
		customerId ++;
		int customer = customerId;
		
		if(waiting < CHAIRS){
			disp_int(customer);
			user_disp_str("wait");
			user_disp_str("\n");
			//disp_int(disp_pos);
			waiting ++;
			user_sem_v("customers");
			user_sem_v("mutex");
			user_sem_p("barbers");
			// user_disp_color_str("\n",3);
			disp_int(customer);
			user_disp_str("getcut");
			user_disp_str("\n");
			//disp_int(disp_pos);
		}
		else{
			disp_int(customer);
			user_disp_str("full!");
			user_disp_str("\n");
			user_sem_v("mutex");
		}
		disp_int(customer);
		user_disp_str("leave");
		user_disp_str("\n");
		//disp_int(disp_pos);
		user_process_sleep(10);
	}
}

void TestE()
{
	int i = 0x4000;
	while(1){
 		user_sem_p("mutex");
		
		customerId ++;
		int customer = customerId;
		
		if(waiting < CHAIRS){
			disp_int(customer);
			user_disp_str("wait");
			user_disp_str("\n");
			//disp_int(disp_pos);
			waiting ++;
			user_sem_v("customers");
			user_sem_v("mutex");
			user_sem_p("barbers");
			// user_disp_color_str("\n",3);
			disp_int(customer);
			user_disp_str("getcut");
			user_disp_str("\n");
			//disp_int(disp_pos);
		}
		else{
			disp_int(customer);
			user_disp_str("full!");
			user_disp_str("\n");
			user_sem_v("mutex");
		}
		disp_int(customer);
		user_disp_str("leave");
		user_disp_str("\n");
		//disp_int(disp_pos);
		user_process_sleep(10);
	}
}
