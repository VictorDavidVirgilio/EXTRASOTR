//#define LMC_15000
#define LMC_16000
/** bomb4.c - FSM Time Bomb. */
//#include "qep_port.h"           /* the port of the QEP event processor */
#include "qp_port.h"           
#include "bsp.h"                              /* board support package */
#include "game.h"
//#include "alarm.h"
#include <stdlib.h>    /* exit() */

Q_DEFINE_THIS_FILE
/* Local objects ------------------------------------------------------*/
typedef struct Bomb4Tag {
  QFsm super;                                      /* derive from QFsm */
//  QHsm super;                                      /* derive from QHsm */
//  QActive super;                                /* derive from QActive */
  uint8_t timeout;                 /* number of seconds till explosion */
  uint8_t fine_time;      /* 1/10's of second */
  uint8_t fine_hour;
  bool clock_time;
  uint8_t code;           /* currently entered code to disarm the bomb */
  uint8_t defuse;             /* secret defuse code to disarm the bomb */
  uint8_t hour;
  uint8_t p;
  bool onoff;
} Bomb4;
/*//funciones de alarma
QState Alarm_initial(Alarm *me, QEvent const *e);
QState Alarm_off (Alarm *me, QEvent const *e);
QState Alarm_on (Alarm *me, QEvent const *e);
*/
static QState Bomb4_initial(Bomb4 *me, QEvent const *e);
static QState Bomb4_setting(Bomb4 *me, QEvent const *e);
static QState Bomb4_timing (Bomb4 *me, QEvent const *e);
static QState Bomb4_unused (Bomb4 *me, QEvent const *e);
//static Bomb4 l_bomb4;      /* the sole instance of Bomb4 active object */
static Bomb4 l_bomb4[GAME_MINES_MAX];           /* a pool of time bomb */
                        /* helper macro to provide the ID of this bomb */
#define BOMB_ID(me_)    ((me_) - l_bomb4)

/*....................................................................*/
/* global objects ----------------------------------------------------*/
/* opaque pointer to Bomb4 AO */
//QActive * const AO_Bomb4 = (QActive *)&l_bomb4; 
/*....................................................................*/
//void Bomb4_ctor(Bomb4 *me, uint8_t defuse) {
QFsm *Bomb4_ctor(uint8_t id,uint8_t defused) {
  Bomb4 *me;
  Q_REQUIRE(id < GAME_MINES_MAX);
  me = &l_bomb4[id];
  //QHsm_ctor(&me->super, (QStateHandler)&Bomb4_initial);
  QFsm_ctor(&me->super, (QStateHandler)&Bomb4_initial);
  me->defuse = defused;/*the defuse code is hardcoded at instantiation*/
  me->code = 0;
  return (QFsm *)me;
}/*end Bomb4_ctor()*/
/*....................................................................*/
//QState Bomb4_initial(Bomb4 *me, QEvent const *e) {
QState Bomb4_initial(Bomb4 *me, QEvt const *e) {
  static  uint8_t dict_sent;
  (void)e;  /* avoid the "unreferenced parameter" warning */
  //QHsm_init((QHsm *)&me->super,(QEvt *)0);
  //QActive_subscribe((QActive *)me,TIME_TICK_SIG);
  //QActive_subscribe((QActive *)me,UP_SIG);
  //QActive_subscribe((QActive *)me,DOWN_SIG);
  //QActive_subscribe((QActive *)me,ARM_SIG);
  if (!dict_sent) {
    QS_OBJ_DICTIONARY(&l_bomb4[0]);/* object dictionary for Bomb4 pool */
    //QS_OBJ_DICTIONARY(&l_bomb4[1]);
    //QS_OBJ_DICTIONARY(&l_bomb4[2]);
    //QS_OBJ_DICTIONARY(&l_bomb4[3]);
    //QS_OBJ_DICTIONARY(&l_bomb4[4]);
    QS_FUN_DICTIONARY(&Bomb4_initial);/*function dictionary for Bomb4 FSM*/
    QS_FUN_DICTIONARY(&Bomb4_setting);
    QS_FUN_DICTIONARY(&Bomb4_timing);
    QS_FUN_DICTIONARY(&Bomb4_unused);
    dict_sent = 1;
  }
                                                     /* local signals */
//  QS_SIG_DICTIONARY(MINE_DISABLED_SIG, me);
  QS_SIG_DICTIONARY(MINE_PLANT_SIG, me);
  QS_SIG_DICTIONARY(SHIP_IMG_SIG,      me);
  //QS_SIG_DICTIONARY(BOMB_SIG,          me);
  QS_SIG_DICTIONARY(UP_SIG,            me);
  QS_SIG_DICTIONARY(DOWN_SIG,          me);
  QS_SIG_DICTIONARY(ARM_SIG,           me);
  QS_SIG_DICTIONARY(ALARM_ON_SIG,      me); /* turn the alarm on */
  QS_SIG_DICTIONARY(ALARM_OFF_SIG,     me); /* turn the alarm off */
  QS_SIG_DICTIONARY(CLOCK_12H_SIG,     me); /* set the clock in 12H mode */
  QS_SIG_DICTIONARY(CLOCK_24H_SIG,     me); /* set the clock in 24H mode */

  me->timeout = INIT_TIMEOUT;
  me->clock_time=true;//true=12H
  me->p=0;
  me->onoff=true;
//  me->clock_time=true;
#ifdef LMC_16000
  clrscr();       /*Clear the screen*/
  disable_echo(); /*Input characters are not echoed*/
  set_cursorxy(0,0); 
//  printf("[Orthogonal Component patern]                \n");
  printf("Press 'o' to turn the Alarm ON               \n");
  printf("Press 'f' to turn the Alarm OFF              \n");
  printf("Press 'u' or 'd' to set the Alarm time       \n");
  printf("Press 'a' to set the clock in 12 hour mode   \n");
  printf("Press 'b' to set the clock in 24 hour mode   \n");  
  printf("Press t to Initiate clock...                 \n");
if (me->clock_time==false){
  printf("24-Hour mode");
  printf("\nalarma = %2d:%2d\n",me->p+me->hour,me->timeout);
}else{
  printf("12-hour mode");
  printf("\nalarma = %2d:%2d\n",me->p+me->hour,me->timeout);
}
  printf("alarm ON (default)\n");
  //printf("Press A to arm the bomb and transition to Bomb4_timing state\n");
#endif /*LMC_16000*/
  //me->timeout = INIT_TIMEOUT;
  me->fine_time = 0;
  return Q_TRAN(&Bomb4_setting);
  /**Tambi\'en se puede hacer transici\'on directo al estado timing*/
  //return Q_TRAN(&Bomb4_timing);  
  /** return Q_TRAN(&Bomb4_unused);
   *  La transici\'on de estado usada en las primeras pruebas
   */
  return Q_TRAN(&Bomb4_unused);
}/*end Bomb4_initial()*/
/*....................................................................*/
QState Bomb4_unused(Bomb4 *me, QEvt const *e) {
    
 switch (e->sig) {
        case MINE_PLANT_SIG: {
            return Q_TRAN(&Bomb4_timing);
        }
    }
  //return Q_SUPER(&QFsm_top);
  return Q_IGNORED();
}
/*....................................................................*/
QState Bomb4_setting(Bomb4 *me, QEvent const *e) {

  switch (e->sig) {
    case UP_SIG: {
      if (me->hour < 12) {
        ++me->timeout;
        if (me->timeout==60){
         ++me->hour;
         me->timeout=0;
         }
#ifdef LMC_16000
        set_cursorxy(0,7);
        if (me->timeout < 10) {        
        printf("alarma = %2d:0%d\n\n",me->p+me->hour,me->timeout);
        }else{
        printf("alarma = %2d:%2d\n\n",me->p+me->hour,me->timeout);
        }
#else
        BSP_display4(me->timeout);
#endif /*LMC_16000*/
      }
      return Q_HANDLED();
    }
    case DOWN_SIG: {
      if ((me->hour!=0)||(me->timeout!=0)) {
        if (me->timeout==0){
         --me->hour;
         me->timeout=60;
         }
	--me->timeout;
#ifdef LMC_16000
        set_cursorxy(0,7);
        if (me->timeout < 10) {
        printf("alarma = %2d:0%d\n\n",me->p+me->hour,me->timeout);
        }else{
        printf("alarma = %2d:%2d\n\n",me->p+me->hour,me->timeout);
        }
//#else
//        BSP_display4(me->timeout);
#endif /*LMC_16000*/
      }
      return Q_HANDLED();
    }
    case CLOCK_12H_SIG: {
    me->clock_time=true;
    me->p=0;
#ifdef LMC_16000
set_cursorxy(0,6);
	if (me->clock_time==false) {
         printf("24-Hour mode\n\n\n");
         }else{
         printf("12-Hour mode\n\n\n");
         }
//#else
//	BSP_display3(me->clock_time);
#endif
    return Q_HANDLED();
     }
     case CLOCK_24H_SIG: {
     me->clock_time=false;
	me->p=12;
#ifdef LMC_16000
set_cursorxy(0,6);
         if (me->clock_time==false) {
         printf("24-Hour mode\n\n\n");
         }else{
         printf("12-Hour mode\n\n\n");
         }
#else
	BSP_display3(me->clock_time);
#endif
    return Q_HANDLED();
     }
    case ARM_SIG: {
#ifdef LMC_16000
//       clrscr();
#else
       BSP_unboom();
#endif /*LMC_16000*/
       return Q_TRAN(&Bomb4_timing);        /* transition to "timing" */
    }
  }/*end switch()*/
//while(1) { printf("CCCC\n"); }
  return Q_IGNORED();
}/*end Bomb4_setting()*/
/*....................................................................*/
QState Bomb4_timing(Bomb4 *me, QEvent const *e) {
  static uint8_t num_ticks = 0;
  switch (e->sig) {
    case Q_ENTRY_SIG: {
      me->code = 0;                          /* clear the defuse code */
      return Q_HANDLED();
    }
    case UP_SIG: {
      me->code <<= 1;
      me->code |=1;  /*If only UP button is pressed: 1,3,7,15, etc. */
      //BSP_display5(me->code);
      return Q_HANDLED();
    }
    case DOWN_SIG: {
      me->code <<= 1;
      //BSP_display5(me->code);
      return Q_HANDLED();
    }
    case ALARM_OFF_SIG:{
      me->onoff=false;
#ifdef LMC_16000
printf("ALARM OFF\n");
#endif
goto keep;
       return Q_TRAN(&Bomb4_setting); 
    }
    case ALARM_ON_SIG:{
      me->onoff=true;
#ifdef LMC_16000
printf("ALARM ON\n");
#endif
goto keep;
       return Q_TRAN(&Bomb4_setting);
    }
    case CLOCK_12H_SIG: {
    me->clock_time=true;
     me->p=0;
#ifdef LMC_16000
        if (me->clock_time==false) {
         printf("24-Hour mode\n");
         }else{
         printf("12-Hour mode\n");
          }
//#else
//      BSP_display3(me->clock_time);
#endif
goto keep;
 return Q_TRAN(&Bomb4_setting);
}

     case CLOCK_24H_SIG: {
     me->clock_time=false;
      me->p=12;
#ifdef LMC_16000
         if (me->clock_time==false) {
         printf("24-Hour mode\n");
         }else{
         printf("12-Hour mode\n");
           }
#endif
goto keep;
return Q_TRAN(&Bomb4_setting);
}   
case ARM_SIG: {
keep:
      if (me->code == me->defuse) {
        return Q_TRAN(&Bomb4_setting);
      }
      return Q_HANDLED();
    }
    /*case TICK_SIG: {*/
    case TIME_TICK_SIG: {
      num_ticks++;
      /* ?`ya ha transcurrido 1/10 segundo? */
      if ((num_ticks % (BSP_TICKS_PER_SEC/*/10*/)) == 0) {
        ++me->fine_time; //"minutos transcurridos"
        if (me->fine_time==60){
        ++me->fine_hour;
        me->fine_time=0;
	}
        if ((me->fine_time>=me->timeout)&&(me->fine_hour>=me->hour)) {/* d\'ecimas > 0 */
		if (me->onoff==false){
		printf("ALARM OFF (missed)  ");
		}else{
        	goto boom;//}//++me->timeout;
		}
 // me->fine_time = 0;
        }
#ifdef LMC_16000
//        set_cursorxy(0,7);
         if (me->fine_time< 10) {
         printf("%2d:0%d\n",me->p+me->fine_hour,me->fine_time);
         }else{
         printf("%2d:%2d\n",me->p+me->fine_hour,me->fine_time);
         }
#else
        BSP_display2(me->timeout);
        BSP_display3(me->fine_time);
#endif /*LMC_16000*/
      }
      return Q_HANDLED();
boom:
#ifdef LMC_16000
//      set_cursorxy(0,7);      
if (me->fine_time< 10){      

          printf("%2d:0%d\n",me->p+me->fine_hour,me->fine_time);
          }else{
          printf("%2d:%2d\n",me->p+me->fine_hour,me->fine_time);
}
 //     set_cursorxy(0,9);
      

      printf("WAKE UP!!!!!!!!!\n");
      exit(0);
#else
      BSP_boom();
#endif /*LMC_16000*/
      num_ticks = 0;
      return Q_TRAN(&Bomb4_setting);
    }
  }/*end switch()*/
  return Q_IGNORED();
}/*end Bomb4_timing()*/





