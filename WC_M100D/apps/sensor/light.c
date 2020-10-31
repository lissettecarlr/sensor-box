#include "light.h"
#include "stm32l1xx.h"
//#include "timer.h"

#define ADC_LIGHT_MEASUREMENT_PIN           GPIO_Pin_13          
#define ADC_LIGHT_MEASUREMENT_GPIO_PORT     GPIOB               
#define ADC_MEASUREMENT_ADC_CHANNEL         ADC_Channel_19       

uint16_t LightData=0;

Sensor light_elc =
{
    "light",
	  "0.1v",
	  SENSOR_LIGHT,
	  VOLTAGE,
	  light_r_init,
	  light_r_configuration,
	  light_r_read,
	  light_r_update,
};


void light_r_init(void)
{
  ADC_InitTypeDef  ADC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOB Clock */  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  /* Configure IDD Measurement pin (ADC Channelxx) as analog input -----------*/
  GPIO_InitStructure.GPIO_Pin = ADC_LIGHT_MEASUREMENT_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_Init(ADC_LIGHT_MEASUREMENT_GPIO_PORT, &GPIO_InitStructure);
  
/* ADC1 configuration --------------------------------------------------------*/
  /* Enable HSI clock for ADC clock */
  RCC_HSICmd(ENABLE);

  /*!< Wait till HSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET)
  {}
    
/* Enable ADC clock ----------------------------------------------------------*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

/* de-initialize ADC ---------------------------------------------------------*/
  ADC_DeInit(ADC1);

/*  ADC configured as follows:
  - NbrOfChannel = 1 - ADC_Channel_1b
  - Mode = Single ConversionMode(ContinuousConvMode Enabled)
  - Resolution = 12Bits
  - Prescaler = /1
  - Sampling time 192 */
  /* ADC Configuration */
  ADC_BankSelection(ADC1, ADC_Bank_A);//ADC_Bank_A/ADC_Bank_B
  
  /* ADC Configuration */
  ADC_StructInit(&ADC_InitStructure);
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel4 configuration */
  ADC_RegularChannelConfig(ADC1, ADC_MEASUREMENT_ADC_CHANNEL, 1, ADC_SampleTime_192Cycles);

  ADC_DelaySelectionConfig(ADC1, ADC_DelayLength_Freeze);

  ADC_PowerDownCmd(ADC1, ADC_PowerDown_Idle_Delay, ENABLE);
  
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
  
  /* Wait until ADC1 ON status */
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET)
  {
  }
}

uint16_t light_r_read(void)
{

   return  LightData;
}

void light_r_configuration(int time)
{

}

SensorState light_r_update(void)
{
	int i = 0x00;
  LightData = 0;
  /* ADC1 configuration ------------------------------------------------------*/
  light_r_init();

  /* Start ADC1 Conversion using Software trigger */
  ADC_SoftwareStartConv(ADC1);
     
  for (i = 4; i > 0; i--)
  {  
    /* Wait until ADC Channel 15 end of conversion */
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
    {}
    /* Read ADC conversion result */
    LightData += ADC_GetConversionValue(ADC1);
  }
    LightData =(LightData >> 2); 
	
	return SENSOR_STATE_OK;
}
