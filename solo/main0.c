 /*---------------------------------------------------------------------
 * File 'main0.c'                               By LMC.
 *
 * 
 * A concurrent program.
 *
 *----------------------------------------------------------------------*/

#include <stdio.h>
#include <debug_marte.h> // For Debugging
#include <stdbool.h> /*for bool type name*/
#include <stdint.h>  /*for uint8_t type name*/
#include <misc/console_management.h>  /*reset_blocking_mode,set_raw_mode*/
#include <pthread.h>
#include <misc/timespec_operations.h> /*double_to_timespec()*/
#include <stdlib.h> /*exit()*/
static pthread_t thread1,thread2;
volatile int thread_signal;

#define SETTING	1
#define TIMING	2
uint8_t estado;

typedef struct time_bomb {
  uint8_t timeout;
  uint8_t fine_time;
  uint8_t code;
  uint8_t defuse;
  uint8_t fine_hour;
  bool clock_time;
  uint8_t hour;
  uint8_t p;
  bool onoff;

} TimeBomb;
TimeBomb TB;
TimeBomb *tb = &TB;

char keyboard_input(void) {
  //static bool result = 0;
  // check keyboard and flush buffer (flushing so works in raw mode)
  char key = getchar();
  while (getchar() != -1 );

  return key;
}/*keyboard_input()*/

void *thread1_handler (void *arg) {
  double thread1_interval = 1;   /* 0.1 second */
  struct timespec thread1_ts;
  //int *thread1_status = (int *)status;
  (void)arg; /*to avoid the unused parameter compiler warning*/
  double_to_timespec(thread1_interval,&thread1_ts);
  for (;;) {
    if (estado == TIMING) {
        ++tb->fine_time; //"minutos transcurridos"
        if (tb->fine_time==60){
        ++tb->fine_hour;
        tb->fine_time=0;
	}
        if ((tb->fine_time>=tb->timeout)&&(tb->fine_hour>=tb->hour)) {/* d\'ecimas > 0 */
		if (tb->onoff==false){
		printf("ALARM OFF (missed)  ");
		}else{
        	goto boom;//}//++me->timeout;
		}
	}
//
	if (tb->fine_time< 10) {
         printf("%2d:0%d\n",tb->p+tb->fine_hour,tb->fine_time);
         }else{
         printf("%2d:%2d\n",tb->p+tb->fine_hour,tb->fine_time);
         }
	}
    nanosleep(&thread1_ts,NULL);
  } /*end for()*/
boom:
 // set_cursorxy(0,5);
 if (tb->fine_time< 10){      

          printf("%2d:0%d\n",tb->p+tb->fine_hour,tb->fine_time);
          }else{
          printf("%2d:%2d\n",tb->p+tb->fine_hour,tb->fine_time);
}   
printf("WAKE UP!!!!!!!!!\n");
  exit(0);
  return NULL;
}/*end thread1_handler()*/

void *thread2_handler (void *arg) {
  double thread2_interval = 0.01; /* 0.01 second */
  struct timespec thread2_ts;
  char c;
  (void)arg; /*to avoid the unused parameter compiler warning*/
  double_to_timespec(thread2_interval,&thread2_ts);
  for (;;) {
    c = keyboard_input();
    switch (c) {
      case 't':{
        if (estado == SETTING) {
          estado = TIMING; 
          //clrscr();
        }else if (estado == TIMING) {
          if (tb->code == tb->defuse) {
            estado = SETTING;
            tb->code = 0;
            tb->fine_time = 0;
	    tb->clock_time=true;//true=12H
            tb->p=0;
            tb->onoff=true;

            set_cursorxy(0,0); 

		printf("Press 'o' to turn the Alarm ON               \n");
  		printf("Press 'f' to turn the Alarm OFF              \n");
  		printf("Press 'u' or 'd' to set the Alarm time       \n");
  		printf("Press 'a' to set the clock in 12 hour mode   \n");
  		printf("Press 'b' to set the clock in 24 hour mode   \n");  
  		printf("Press t to Initiate clock...                 \n");
 		if (tb->clock_time==false){
  		printf("24-Hour mode");
  		printf("\nalarma = %2d:%2d\n",tb->p+tb->hour,tb->timeout);
		}else{
  		printf("12-hour mode");
  		printf("\nalarma = %2d:%2d\n",tb->p+tb->hour,tb->timeout);
		}
  		printf("alarm ON (default)\n");

          }
        }
        break;
      }
      case 'd':{
        if (estado == SETTING) {
        if ((tb->hour!=0)||(tb->timeout!=0)) {
		if (tb->timeout==0){
         	--tb->hour;
         	tb->timeout=60;
         	}
	--tb->timeout;
        set_cursorxy(0,7);
        	if (tb->timeout < 10) {
        	printf("alarma = %2d:0%d\n\n",tb->p+tb->hour,tb->timeout);
        	}else{
        	printf("alarma = %2d:%2d\n\n",tb->p+tb->hour,tb->timeout);
        	}
        }
	}
        break;
      }      
    case 'u':{
        if (estado == SETTING) {
        if (tb->hour < 12) {
        ++tb->timeout;
        	if (tb->timeout==60){
         	++tb->hour;
         	tb->timeout=0;
         	}
        set_cursorxy(0,7);
        	if (tb->timeout < 10) {        
        	printf("alarma = %2d:0%d\n\n",tb->p+tb->hour,tb->timeout);
        	}else{
        	printf("alarma = %2d:%2d\n\n",tb->p+tb->hour,tb->timeout);
		}
          	
	}
	}
        break;
      }
      case 'o':{
	if (estado == TIMING) {
      tb->onoff=true;
	printf("ALARM ON\n");
	}
//goto keep;
       break; 
    }
      case 'f':{
        if (estado == TIMING) {
      tb->onoff=false;
        printf("ALARM OFF\n");
        }
//goto keep;
       break;
    }
      case 'a': {
	tb->clock_time=true;
    	tb->p=0;
	if (estado == SETTING) {
	set_cursorxy(0,6);
	if (tb->clock_time==false) {
         printf("24-Hour mode\n\n\n");
         }else{
         printf("12-Hour mode\n\n\n");
         }
	}
	if (estado == TIMING) {
    	if (tb->clock_time==false) {
         printf("24-Hour mode\n");
         }else{
         printf("12-Hour mode\n");
          }
	}
	break;
       }
       case 'b': {
       tb->clock_time=false;
       tb->p=12;
       if (estado == SETTING) {
       set_cursorxy(0,6);
       if (tb->clock_time==false) {
        printf("24-Hour mode\n\n\n");
         }else{
         printf("12-Hour mode\n\n\n");
         }
        }
        if (estado == TIMING) {
        if (tb->clock_time==false) {
         printf("24-Hour mode\n");
         }else{
         printf("12-Hour mode\n");
          }
        }
        break;
        }

    }/*---------------switch()*/
    nanosleep(&thread2_ts,NULL);
  }/*--------for()*/
  return NULL;
}/*end thread2_handler()*/

int main()
{
  // For Debugging
  //init_serial_communication_with_gdb (SERIAL_PORT_1);
  //set_break_point_here;

  //make getchar() non-blocking: 'getchar' returns -1 inmediately 
  //when there is no characters available at the moment of the call. 
  //x86_arch/include/misc/console_management.h
  reset_blocking_mode();
  set_raw_mode();// Every character is made available to the 
                 // calling program as soon as it is typed, so
                 // no line editing is available in this mode.
  disable_echo();// Input characters are not echoed
  clrscr();

  tb->timeout = 10;
  tb->code = 0;
  tb->defuse = 7;
  tb->onoff= true;
  tb->clock_time=true;

  //estado = TIMING;
  estado = SETTING;
  set_cursorxy(0,0); 
  
  printf("Press 'o' to turn the Alarm ON               \n");
  printf("Press 'f' to turn the Alarm OFF              \n");
  printf("Press 'u' or 'd' to set the Alarm time       \n");
  printf("Press 'a' to set the clock in 12 hour mode   \n");
  printf("Press 'b' to set the clock in 24 hour mode   \n");
  printf("Press t to Initiate clock...                 \n");
  if (tb->clock_time==false){
  printf("24-Hour mode");
  printf("\nalarma = %2d:%2d\n",tb->p+tb->hour,tb->timeout);
  }else{
  printf("12-hour mode");
  printf("\nalarma = %2d:%2d\n",tb->p+tb->hour,tb->timeout);
  }
  printf("alarm ON (default)\n");
  pthread_create(&thread1,NULL,thread1_handler,(void*)thread_signal);
  pthread_create(&thread2,NULL,thread2_handler,(void*)thread_signal);
  pthread_join(thread1,NULL);
  pthread_join(thread2,NULL);
  return 0;
}/*end main()*/


