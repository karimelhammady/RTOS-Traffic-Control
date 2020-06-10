#include "p1_timers.h"

void timer_trafficLights_delay_ms(int ttimems)
{
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R0;
    TIMER0_CTL_R = 0x0;                   /* STEP 1: disable Timer before initialization */
    TIMER0_CFG_R = 0x0;                   /* STEP 2: 32-bit individual Timer only works with prescaler*/
    TIMER0_TAMR_R = 0x01;                 /* STEP 3: Periodic down-counter only works in simulation */
                                          /* STEP 4: Optional configuration  is not needed in this example */
    TIMER0_TAILR_R = 16000 * ttimems - 1; /* STEP 5: Timer A interval load value register */
                                          /* STEP 6: Interrupt is not needed in this example */
    TIMER0_CTL_R |= 0x01;                 /* STEP 7: enable Timer A after initialization*/
    while ((TIMER0_RIS_R & 0x1) == 0)
        ;
    TIMER0_ICR_R = 0x1;
}

void timer_pedestrians_delay_ms(int ttimems)
{
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;
    TIMER1_CTL_R = 0x0;                   /* STEP 1: disable Timer before initialization */
    TIMER1_CFG_R = 0x0;                   /* STEP 2: 32-bit individual Timer only works with prescaler*/
    TIMER1_TAMR_R = 0x01;                 /* STEP 3: Periodic down-counter only works in simulation */
                                          /* STEP 4: Optional configuration  is not needed in this example */
    TIMER1_TAILR_R = 16000 * ttimems - 1; /* STEP 5: Timer A interval load value register */
                                          /* STEP 6: Interrupt is not needed in this example */
    TIMER1_CTL_R |= 0x01;                 /* STEP 7: enable Timer A after initialization*/
    while ((TIMER1_RIS_R & 0x1) == 0)
        ;
    TIMER1_ICR_R = 0x1;
}

void timer_railway_delay_ms(int ttimems)
{
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R3;
    TIMER3_CTL_R = 0x0;                   /* STEP 1: disable Timer before initialization */
    TIMER3_CFG_R = 0x0;                   /* STEP 2: 32-bit individual Timer only works with prescaler*/
    TIMER3_TAMR_R = 0x01;                 /* STEP 3: Periodic down-counter only works in simulation */
                                          /* STEP 4: Optional configuration  is not needed in this example */
    TIMER3_TAILR_R = 16000 * ttimems - 1; /* STEP 5: Timer A interval load value register */
                                          /* STEP 6: Interrupt is not needed in this example */
    TIMER3_CTL_R |= 0x01;                 /* STEP 7: enable Timer A after initialization*/
    while ((TIMER3_RIS_R & 0x1) == 0)
        ;
    TIMER3_ICR_R = 0x1;
}
