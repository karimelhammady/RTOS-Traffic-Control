#include "p1_timers.h"

// Train Related definitions
#define GATE_LED (1 << 1) // PF1
#define SENSOR_1 (1 << 4) // PF4
#define SENSOR_2 (1 << 0) // PF0

// Pedestrian switches definitions
#define PED_LT (1 << 0) // PB0
#define PED_LB (1 << 1) // PB1
#define PED_RT (1 << 2) // PB2
#define PED_RB (1 << 3) // PB3

// Traffic Lights definitions
#define LED_RED_V (1 << 4)	 // PB4
#define LED_GREEN_V (1 << 5) // PB5
#define LED_RED_H (1 << 6)	 // PB6
#define LED_GREEN_H (1 << 7) // PB7

// Delay definitions in milliseconds
#define TGNS 10000
#define TGEW 5000
#define TCROSS 10000
#define TSAFETY 3000
#define FLASH_FREQ 500

// Task Handlers
xTaskHandle trafficLightsTaskHandle;
xTaskHandle pedestriansTaskHandle;
xTaskHandle railwayTaskHandle;

char trafficFlowing = 1;
char currentRedLight;

void PORTF_Init(void)
{
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
	GPIO_PORTF_LOCK_R = 0x4c4f434b;
	GPIO_PORTF_CR_R |= GATE_LED | SENSOR_1 | SENSOR_2;
	GPIO_PORTF_DEN_R |= GATE_LED | SENSOR_1 | SENSOR_2;
	GPIO_PORTF_PUR_R |= SENSOR_1 | SENSOR_2;
	GPIO_PORTF_DIR_R |= GATE_LED;
	GPIO_PORTF_DIR_R &= ~(SENSOR_1 | SENSOR_2);
}

void PORTB_Init(void)
{
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
	GPIO_PORTB_LOCK_R = 0x4c4f434b;
	GPIO_PORTB_CR_R = 0XFF;
	GPIO_PORTB_DEN_R = 0XFF;
	GPIO_PORTB_PUR_R = 0x0F;
	GPIO_PORTB_DIR_R = 0xF0;
}

/* this task is responsible to operate the traffic lights. The vertical road will operate for TGNS msec  and
	the horizontal road will operate for TGEW msec. This task will keep operating both traffic lights until being
	interrupted by other tasks (padestrian or train)*/
void trafficLightsTask(void *pvParameters)
{
	while (1)
	{
		GPIO_PORTB_DATA_R &= ~(LED_RED_V | LED_GREEN_H);
		GPIO_PORTB_DATA_R |= (LED_GREEN_V | LED_RED_H);
		trafficFlowing = 1;
		currentRedLight = 'H';
		timer_trafficLights_delay_ms(TGNS);

		GPIO_PORTB_DATA_R &= ~(LED_GREEN_H | LED_GREEN_V);
		GPIO_PORTB_DATA_R |= (LED_RED_V | LED_RED_H);
		trafficFlowing = 0;
		taskYIELD();

		GPIO_PORTB_DATA_R &= ~(LED_RED_H | LED_GREEN_V);
		GPIO_PORTB_DATA_R |= (LED_GREEN_H | LED_RED_V);
		trafficFlowing = 1;
		currentRedLight = 'V';
		timer_trafficLights_delay_ms(TGEW);

		GPIO_PORTB_DATA_R &= ~(LED_GREEN_H | LED_GREEN_V);
		GPIO_PORTB_DATA_R |= (LED_RED_V | LED_RED_H);
		trafficFlowing = 0;
		taskYIELD();
	}
}

/* this task will receive requests from padestrians and wait for all traffic lights to turn red.
	it will then interrupt the traffic lights task, by changing the priority so it will be greater than traffic lights task, and allow the padestrians to cross the roads for
	TCROSS msec */
void pedestriansTask(void *pvParameters)
{
	while (1)
	{
		if (!(GPIO_PORTB_DATA_R & PED_LT) || !(GPIO_PORTB_DATA_R & PED_LB) || !(GPIO_PORTB_DATA_R & PED_RT) || !(GPIO_PORTB_DATA_R & PED_RB))
		{
			while (!(GPIO_PORTB_DATA_R & PED_LT) || !(GPIO_PORTB_DATA_R & PED_LB) || !(GPIO_PORTB_DATA_R & PED_RT) || !(GPIO_PORTB_DATA_R & PED_RB))
				;
			while (trafficFlowing)
				;
			vTaskPrioritySet(NULL, 2);
			vTaskPrioritySet(railwayTaskHandle, 2);
			GPIO_PORTB_DATA_R &= ~(LED_RED_V | LED_RED_H | LED_GREEN_H | LED_GREEN_V);
			GPIO_PORTB_DATA_R |= LED_RED_V;
			GPIO_PORTB_DATA_R |= LED_RED_H;
			currentRedLight = 'P';
			timer_pedestrians_delay_ms(TCROSS);
			vTaskPrioritySet(railwayTaskHandle, 1);
			vTaskPrioritySet(NULL, 1);
		}
	}
}

/* this task will check if the train has passed the first sensor to interrupt all tasks
	, keep flashing the red traffic lights , and close the gates. When the train passes the 
	second sensor, all tasks can continue to operate normally and the gate will be opened.*/
void railwayTask(void *pvParameters)
{
	int i;
	while (1)
	{
		if (!(GPIO_PORTF_DATA_R & SENSOR_1))
		{

			vTaskPrioritySet(NULL, 3);
			GPIO_PORTB_DATA_R &= ~(LED_RED_V | LED_RED_H | LED_GREEN_H | LED_GREEN_V);
			GPIO_PORTF_DATA_R |= GATE_LED;
			while (GPIO_PORTF_DATA_R & SENSOR_2)
			{
				GPIO_PORTB_DATA_R ^= (LED_RED_H | LED_RED_V);
				timer_railway_delay_ms(FLASH_FREQ);
			}
			for (i = 0; i < 5 * TSAFETY / FLASH_FREQ; i++)
			{
				GPIO_PORTB_DATA_R ^= (LED_RED_H | LED_RED_V);
				timer_railway_delay_ms(FLASH_FREQ / 5);
			}
			GPIO_PORTF_DATA_R &= ~GATE_LED;
			switch (currentRedLight)
			{
			case 'P':
				GPIO_PORTB_DATA_R &= ~(LED_GREEN_V | LED_GREEN_H);
				GPIO_PORTB_DATA_R |= LED_RED_V | LED_RED_H;
				break;
			case 'H':
				GPIO_PORTB_DATA_R &= ~(LED_RED_V | LED_GREEN_H);
				GPIO_PORTB_DATA_R |= (LED_GREEN_V | LED_RED_H);
				break;
			case 'V':
				GPIO_PORTB_DATA_R &= ~(LED_RED_H | LED_GREEN_V);
				GPIO_PORTB_DATA_R |= (LED_GREEN_H | LED_RED_V);
				break;
			default:
				break;
			}
			vTaskPrioritySet(NULL, 1);
		}
		else if (!(GPIO_PORTF_DATA_R & SENSOR_2))
		{
			vTaskPrioritySet(NULL, 3);
			GPIO_PORTB_DATA_R &= ~(LED_RED_V | LED_RED_H | LED_GREEN_H | LED_GREEN_V);
			GPIO_PORTF_DATA_R |= GATE_LED;
			while (GPIO_PORTF_DATA_R & SENSOR_1)
			{
				GPIO_PORTB_DATA_R ^= (LED_RED_H | LED_RED_V);
				timer_railway_delay_ms(FLASH_FREQ);
			}
			for (i = 0; i < 5 * TSAFETY / FLASH_FREQ; i++)
			{
				GPIO_PORTB_DATA_R ^= (LED_RED_H | LED_RED_V);
				timer_railway_delay_ms(FLASH_FREQ / 5);
			}
			GPIO_PORTF_DATA_R &= ~GATE_LED;
			switch (currentRedLight)
			{
			case 'P':
				GPIO_PORTB_DATA_R &= ~(LED_GREEN_V | LED_GREEN_H);
				GPIO_PORTB_DATA_R |= LED_RED_V | LED_RED_H;
				break;
			case 'H':
				GPIO_PORTB_DATA_R &= ~(LED_RED_V | LED_GREEN_H);
				GPIO_PORTB_DATA_R |= (LED_GREEN_V | LED_RED_H);
				break;
			case 'V':
				GPIO_PORTB_DATA_R &= ~(LED_RED_H | LED_GREEN_V);
				GPIO_PORTB_DATA_R |= (LED_GREEN_H | LED_RED_V);
				break;
			default:
				break;
			}
			vTaskPrioritySet(NULL, 1);
		}
	}
}

int main(void)
{
	PORTF_Init();
	PORTB_Init();
	xTaskCreate(railwayTask, (const portCHAR *)"Train Task", configMINIMAL_STACK_SIZE, NULL, 1, &railwayTaskHandle);
	xTaskCreate(pedestriansTask, (const portCHAR *)"Pedestrian Task", configMINIMAL_STACK_SIZE, NULL, 1, &pedestriansTaskHandle);
	xTaskCreate(trafficLightsTask, (const portCHAR *)"Normal Task", configMINIMAL_STACK_SIZE, NULL, 1, &trafficLightsTaskHandle);
	vTaskStartScheduler();
	while (1)
		;
}
