#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "main.h"

typedef struct {
    GPIO_TypeDef *dir_port;
    uint16_t      l_in1;
    uint16_t      l_in2;
    uint16_t      r_in1;
    uint16_t      r_in2;

    TIM_HandleTypeDef *htim_left;
    uint32_t           ch_left;
    TIM_HandleTypeDef *htim_right;
    uint32_t           ch_right;

    uint16_t max_cmd;

    uint8_t invert_left;
    uint8_t invert_right;
} MotorCtrl;

void Motor_Init(MotorCtrl *mc);
void Motor_Stop(MotorCtrl *mc);

// Yüksek seviye hareketler
void Motor_Forward(MotorCtrl *mc, uint16_t sp);
void Motor_Backward(MotorCtrl *mc, uint16_t sp);
void Motor_Left(MotorCtrl *mc, uint16_t sp);
void Motor_Right(MotorCtrl *mc, uint16_t sp);

void Motor_SetRaw(MotorCtrl *mc, int16_t left, int16_t right);

#endif /* MOTOR_CONTROL_H */
