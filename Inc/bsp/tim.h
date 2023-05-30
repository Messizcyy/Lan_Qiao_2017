#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern TIM_HandleTypeDef htim2;

extern TIM_HandleTypeDef htim3;

extern TIM_HandleTypeDef htim15;

extern TIM_HandleTypeDef htim17;

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

void PLUS1_Init(void);
void PLUS2_Init(void);

void PA6_Iint(void);
void PA7_Iint(void);

void GPIO_out_low(void);

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */

