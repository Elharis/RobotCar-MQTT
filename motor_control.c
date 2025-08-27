#include "motor_control.h"

static inline uint32_t _arr(TIM_HandleTypeDef *htim) {
    return __HAL_TIM_GET_AUTORELOAD(htim);
}
static inline uint16_t _clamp_u16(int v, int lo, int hi) {
    if (v < lo) return (uint16_t)lo;
    if (v > hi) return (uint16_t)hi;
    return (uint16_t)v;
}
static inline uint32_t _duty_from(MotorCtrl *mc, TIM_HandleTypeDef *htim, uint16_t v){
    uint16_t maxc = (mc->max_cmd == 0) ? 1000 : mc->max_cmd;
    return ((uint32_t)_clamp_u16(v,0,maxc) * _arr(htim)) / maxc;
}

static void _set_dir_pair(GPIO_TypeDef *port, uint16_t in1, uint16_t in2, int forward){
    if (forward > 0) {          // ileri
        HAL_GPIO_WritePin(port, in1, GPIO_PIN_SET);
        HAL_GPIO_WritePin(port, in2, GPIO_PIN_RESET);
    } else if (forward < 0) {   // geri
        HAL_GPIO_WritePin(port, in1, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(port, in2, GPIO_PIN_SET);
    } else {                    // fren (her iki pin 1) -> L298N'de kısa devre freni
        HAL_GPIO_WritePin(port, in1, GPIO_PIN_SET);
        HAL_GPIO_WritePin(port, in2, GPIO_PIN_SET);
    }
}

static void _apply_one_side(TIM_HandleTypeDef *htim, uint32_t ch, uint16_t sp){
    __HAL_TIM_SET_COMPARE(htim, ch, _clamp_u16(sp,0, (int)_arr(htim)));
}

/* --- Public API --- */

void Motor_Init(MotorCtrl *mc)
{

    HAL_TIM_PWM_Start(mc->htim_left,  mc->ch_left);
    HAL_TIM_PWM_Start(mc->htim_right, mc->ch_right);


    _set_dir_pair(mc->dir_port, mc->l_in1, mc->l_in2, 0);
    _set_dir_pair(mc->dir_port, mc->r_in1, mc->r_in2, 0);
    __HAL_TIM_SET_COMPARE(mc->htim_left,  mc->ch_left,  0);
    __HAL_TIM_SET_COMPARE(mc->htim_right, mc->ch_right, 0);
}

void Motor_Stop(MotorCtrl *mc)
{

    _set_dir_pair(mc->dir_port, mc->l_in1, mc->l_in2, 0);
    _set_dir_pair(mc->dir_port, mc->r_in1, mc->r_in2, 0);
    __HAL_TIM_SET_COMPARE(mc->htim_left,  mc->ch_left,  0);
    __HAL_TIM_SET_COMPARE(mc->htim_right, mc->ch_right, 0);
}

void Motor_SetRaw(MotorCtrl *mc, int16_t left, int16_t right)
{
    if (mc->invert_left)  left  = -left;
    if (mc->invert_right) right = -right;

    _set_dir_pair(mc->dir_port, mc->l_in1, mc->l_in2, (left  > 0) ? +1 : (left  < 0 ? -1 : 0));
    _set_dir_pair(mc->dir_port, mc->r_in1, mc->r_in2, (right > 0) ? +1 : (right < 0 ? -1 : 0));

    uint16_t lsp = (uint16_t)((left  >= 0) ? left  : -left);
    uint16_t rsp = (uint16_t)((right >= 0) ? right : -right);

    _apply_one_side(mc->htim_left,  mc->ch_left,  _duty_from(mc, mc->htim_left,  lsp));
    _apply_one_side(mc->htim_right, mc->ch_right, _duty_from(mc, mc->htim_right, rsp));
}

void Motor_Forward(MotorCtrl *mc, uint16_t sp)
{
    Motor_SetRaw(mc, sp, sp);
}

void Motor_Backward(MotorCtrl *mc, uint16_t sp)
{
    Motor_SetRaw(mc, -(int16_t)sp, -(int16_t)sp);
}

void Motor_Left(MotorCtrl *mc, uint16_t sp)
{
    Motor_SetRaw(mc, (int16_t)sp, -(int16_t)sp);
}

void Motor_Right(MotorCtrl *mc, uint16_t sp)
{
    Motor_SetRaw(mc, -(int16_t)sp, (int16_t)sp);
}
