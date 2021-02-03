#include "qp_port.h"
#include "bsp.h"
#include "game.h"
//#include "video.h"

#include <stdlib.h>
#include <pthread.h>

/* Port de MaRTE OS */
extern int screenWidth,screenHeight;
extern unsigned char *backBuffer;

volatile int thread_signal;
static pthread_t event_thread, kernel_thread;

//void print_exit_screen(void);

/* Local-scope objects -----------------------------------------------------*/
#ifdef LMC_19000
static QEvt const * l_missileQueueSto[2];
static QEvt const * l_shipQueueSto[3];
static QEvt const * l_tunnelQueueSto[GAME_MINES_MAX + 5];
#else
static QEvt const * l_actobjQueueSto[GAME_MINES_MAX + 5];
#endif /*LMC_19000*/

static union SmallEvents {
    void   *e0;                                  /* minimum event size */
    uint8_t e1[sizeof(QEvt)];
    /* ... other event types to go into this pool */
} l_smlPoolSto[10];                /* storage for the small event pool */

static union MediumEvents {
    void   *e0;                                  /* minimum event size */
    uint8_t e1[sizeof(ObjectPosEvt)];
    uint8_t e2[sizeof(ObjectImageEvt)];
    uint8_t e3[sizeof(MineEvt)];
    uint8_t e4[sizeof(ScoreEvt)];
    /* ... other event types to go into this pool */
} l_medPoolSto[2*GAME_MINES_MAX + 8];  /* storage for the medium event pool */

static QSubscrList    l_subscrSto[MAX_PUB_SIG];

/*..........................................................................*/
void *kernel_thread_handler(void *status) {
    double kernel_interval;
    struct timespec kernel_ts;
    int *thread_status = (int *) status;

    /** este intervalo es crítico para el funcionamiento del kernel **/
    /* el intervalo del kernel debe ser significativamente menor al intervalo
     * del hilo de eventos, pero existir (no puede ser omitido o ser 0). */
    kernel_interval = 0.1e-3;
    double_to_timespec(kernel_interval, &kernel_ts);

    for (;;) {
        if (*thread_status == 1)
            break;

        // ejecutar bucle de eventos del kernel QP
        QF_run_event_loop();

        // ESTA LÍNEA ES CRÍTICA PARA EL FUNCIONAMIENTO CORRECTO DEL KERNEL
        nanosleep(&kernel_ts, NULL);
    }
    return NULL;
}
/*..........................................................................*/
void *event_thread_handler(void *status) {
    double event_interval;
    struct timespec event_ts;
    int *thread_status = (int *) status;

    event_interval = 1.0 / BSP_TICKS_PER_SEC;
    double_to_timespec(event_interval, &event_ts);

    for (;;) {
        // procesar señales de control
        if (*thread_status == 1)
            break;

#ifndef LMC_6000
        // procesar entrada de teclado
        if (!keyboard_support())
            break;
#endif /*LMC_6000*/

//#ifndef LMC_7000
        // evento de reloj
        timer_event();
//#endif /*LMC_7000*/

        // dibujar pantalla
#ifdef LMC_10000
        Video_render();
#endif /*LMC_10000*/
        nanosleep(&event_ts, NULL);
    }
/*#ifdef LMC_19000
    print_exit_screen();
#endif*/ /*LMC_19000*/
    return NULL;
}
/*.....................................................................*/
/*.....................................................................*/
int main(int argc, char *argv[]) {
    /* Inicializar MaRTE OS */
    //make getchar() non-blocking: 'getchar' returns -1 inmediately 
    //when there is no characters available at the moment of the call. 
    //x86_arch/include/misc/console_management.h
    reset_blocking_mode();
    set_raw_mode();// Every character is made available to the 
                   // calling program as soon as it is typed, so
                   // no line editing is available in this mode.

    /* fin de inicialización de MaRTE OS */

    /* explicitly invoke the active objects' ctors... */
#ifndef LMC_1000
#ifdef LMC_19000
    Tunnel_ctor();
#else
    ActObj_ctor();
#endif /*LMC_19000*/
#endif /*LMC_1000*/

    QF_init();/* initialize the framework and the underlying RT kernel */

                                      /* initialize the event pools... */
    QF_poolInit(l_smlPoolSto, sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));
    QF_poolInit(l_medPoolSto, sizeof(l_medPoolSto), sizeof(l_medPoolSto[0]));

    QF_psInit(l_subscrSto, Q_DIM(l_subscrSto));/* init publish-subscribe */

                        /* send object dictionaries for event pools... */
    QS_OBJ_DICTIONARY(l_smlPoolSto);
    QS_OBJ_DICTIONARY(l_medPoolSto);

          /* send signal dictionaries for globally published events... */
    QS_SIG_DICTIONARY(TIME_TICK_SIG,      0);

                                        /* start the active objects... */
#ifdef LMC_18000
    QActive_start(AO_Tunnel,
                  3,                                        /* priority */
                  l_tunnelQueueSto,  Q_DIM(l_tunnelQueueSto),/*evt queue*/
                  (void *)0, 0,                  /* no per-thread stack */
                  (QEvt *)0);                /* no initialization event */
#else                       
    QActive_start(AO_ActObj,
                  3,                                        /* priority */
                  l_actobjQueueSto,  Q_DIM(l_actobjQueueSto),/*evt queue*/
                  (void *)0, 0,                  /* no per-thread stack */
                  (QEvt *)0);                /* no initialization event */
#endif /*LMC_18000*/

    BSP_init(argc, argv);       /* initialize the Board Support Package */

    /* hilos de programa */
#ifndef LMC_2000
    pthread_create(&kernel_thread, NULL, kernel_thread_handler, (void *)&thread_signal);
#ifndef LMC_4000
    pthread_create(&event_thread, NULL, event_thread_handler, (void *)&thread_signal);
#endif /*LMC_4000*/
    pthread_join(kernel_thread, NULL);
#ifndef LMC_5000
    pthread_join(event_thread, NULL);
#endif /*LMC_5000*/
#endif /*LMC_2000*/

    return 0;
}/*end main()*/


