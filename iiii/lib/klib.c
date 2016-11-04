
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            klib.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

int customers = 0;
int barbers = 0;
int mutex = 1;

//简易队列模型
PROCESS* cList[5];
PROCESS* bList[5];
PROCESS* mList[5];

//start == end 代表队列为空，取值或赋值之前进行判断，每次取值或赋值完后+1，并判断是否越界

int c_start = 0;	//下一个可以直接赋值的位置
int c_end = 0;		//上一个可以直接取值的位置
int b_start = 0;
int b_end = 0;
int m_start = 0;
int m_end = 0;



/*======================================================================*
                               itoa
 *======================================================================*/
PUBLIC char * itoa(char * str, int num)/* 数字前面的 0 不被显示出来, 比如 0000B800 被显示成 B800 */
{
	char *	p = str;
	char	ch;
	int	i;
	int	flag = 0;

	*p++ = '0';
	*p++ = 'x';

	if(num == 0){
		*p++ = '0';
	}
	else{	
		for(i=28;i>=0;i-=4){
			ch = (num >> i) & 0xF;
			if(flag || (ch > 0)){
				flag = 1;
				ch += '0';
				if(ch > '9'){
					ch += 7;
				}
				*p++ = ch;
			}
		}
	}

	*p = 0;

	return str;
}


/*======================================================================*
                               disp_int
 *======================================================================*/
PUBLIC void disp_int(int input)
{
	char output[16];
	itoa(output, input);
	disp_str(output);
}

/*======================================================================*
                               delay
 *======================================================================*/
PUBLIC void delay(int time)
{
	int i, j, k;
	for(k=0;k<time;k++){
		/*for(i=0;i<10000;i++){	for Virtual PC	*/
		for(i=0;i<10;i++){/*	for Bochs	*/
			for(j=0;j<10000;j++){}
		}
	}
}

//Added by lionel
PUBLIC void sys_disp_str(char *str){
	disp_str(str);
}

PUBLIC void sys_disp_color_str(char * info, int color){
	disable_irq(CLOCK_IRQ);
	disp_color_str(info,color);
	enable_irq(CLOCK_IRQ);
}

PUBLIC void sys_sem_p(char *name){
	//disable_int();
	//customer
	if(name[0] == 'c'){
		//disp_str("P");
		//disp_int(customers);
		customers--;
		
		if(customers < 0){
			cList[c_start] = p_proc_ready;
			
			if(c_start == 4){
				c_start = 0;
			}else{
				c_start++;
			}
			//disp_str("---CCC");
			enable_int();
			sleep();
		}
	}
	//barber
	else if(name[0] == 'b'){
		barbers--;
		if(barbers<0){
			bList[b_start] = p_proc_ready;
			
			//b_start++;
			if(b_start == 4){
				b_start = 0;
			}else{
				b_start++;
			}
			
			enable_int();
			sleep();
		}
	}
	//mutex
	else{
		mutex--;
		if(mutex<0){
			mList[m_start] = p_proc_ready;
			
			//m_start++;
			if(m_start == 4){
				m_start = 0;
			}else{
				m_start++;
			}
			
			enable_int();
			sleep();
		}
	}
}

PUBLIC void sys_sem_v(char *name){
	
	//disable_int();
	//customer
	if(name[0] == 'c'){
		//disp_str("V");
		//disp_int(customers);
		customers++;
		if(customers<=0){
			PROCESS* p = cList[c_end];
			
			//c_end++;
			if(c_end == 4){
				c_end = 0;
			}
			else{
				c_end++;
			}
			
			enable_int();
			wakeup(p);
		}
	}
	//barber
	else if(name[0] == 'b'){
		barbers++;
		if(barbers<=0){
			PROCESS* p = bList[b_end];
			
			//c_end++;
			if(b_end == 4){
				b_end = 0;
			}
			else{
				b_end++;
			}
			
			enable_int();
			wakeup(p);
		}
	}
	//mutex
	else{
		mutex++;
		if(mutex<=0){
			PROCESS* p = mList[m_end];
			
			//c_end++;
			if(m_end == 4){
				m_end = 0;
			}
			else{
				m_end++;
			}
			
			enable_int();
			wakeup(p);
		}
	}
}



