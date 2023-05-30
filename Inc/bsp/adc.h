#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"


extern ADC_HandleTypeDef hadc2;


void ADC2_Init(void);

uint32_t get_adc_rp5(void);
uint32_t get_adc_rp6(void);


#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */