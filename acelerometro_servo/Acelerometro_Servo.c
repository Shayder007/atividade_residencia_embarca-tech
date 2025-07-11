#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h> // Para funções trigonométricas como atan2f e sqrtf
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

// --- Definições para o Sensor MPU6050 ---
#define MPU6050_ADDR         0x68
#define MPU6050_PWR_MGMT_1   0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B

#define ACCEL_SCALE_FACTOR   16384.0f

// --- Definições de Pinos e I2C para o MPU6050 (usando I2C0) ---
#define MPU6050_I2C_PORT    i2c0
#define MPU6050_SDA_PIN     0
#define MPU6050_SCL_PIN     1
#define MPU6050_I2C_BAUDRATE 100000

// --- Definições para o Servo Motor SG90 ---
#define PINO_SERVO 8
#define PWM_FREQ   50
#define PWM_WRAP   19999

#define SERVO_PULSE_MIN_US 500  
#define SERVO_PULSE_MAX_US 2500 

#define ANGULO_ALERTA_GRAUS 30.0f

// Protótipos
void mpu6050_init();
void mpu6050_read_raw_data(int16_t accel[3], int16_t gyro[3]);
void calcular_angulos(int16_t accel[3], float *inclinacao, float *balanco);
void init_servo_pwm();
uint32_t angulo_para_duty(float angulo);
void setar_angulo_servo(float angulo);

// LCD 320x240 (comentado)
/*
void display_alert_lcd(bool alert) {
    if (alert) {
        // Código para mostrar alerta no LCD
    } else {
        // Código para limpar alerta
    }
}
*/

void mpu6050_init() {
    i2c_init(MPU6050_I2C_PORT, MPU6050_I2C_BAUDRATE);
    gpio_set_function(MPU6050_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(MPU6050_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MPU6050_SDA_PIN);
    gpio_pull_up(MPU6050_SCL_PIN);

    printf("I2C0 para MPU6050 configurado.\n");
    sleep_ms(100);

    uint8_t buf[2] = {MPU6050_PWR_MGMT_1, 0x00}; 
    int ret = i2c_write_blocking(MPU6050_I2C_PORT, MPU6050_ADDR, buf, 2, false);
    if (ret == PICO_ERROR_GENERIC) {
        printf("Erro ao acordar MPU6050! Verifique conexoes e endereco I2C.\n");
    } else {
        printf("MPU6050 acordado e inicializado com sucesso.\n");
    }
    sleep_ms(100);
}

void mpu6050_read_raw_data(int16_t accel[3], int16_t gyro[3]) {
    uint8_t buffer[14];
    uint8_t reg_addr = MPU6050_ACCEL_XOUT_H;
    int ret = i2c_write_blocking(MPU6050_I2C_PORT, MPU6050_ADDR, &reg_addr, 1, true);
    if (ret == PICO_ERROR_GENERIC) {
        printf("Erro ao solicitar leitura de dados do MPU6050.\n");
        memset(accel, 0, sizeof(int16_t)*3);
        memset(gyro, 0, sizeof(int16_t)*3);
        return;
    }
    ret = i2c_read_blocking(MPU6050_I2C_PORT, MPU6050_ADDR, buffer, 14, false);
    if (ret == PICO_ERROR_GENERIC) {
        printf("Erro ao ler dados do MPU6050.\n");
        memset(accel, 0, sizeof(int16_t)*3);
        memset(gyro, 0, sizeof(int16_t)*3);
        return;
    }
    accel[0] = (int16_t)((buffer[0] << 8) | buffer[1]);
    accel[1] = (int16_t)((buffer[2] << 8) | buffer[3]);
    accel[2] = (int16_t)((buffer[4] << 8) | buffer[5]);
    gyro[0] = (int16_t)((buffer[8] << 8) | buffer[9]);
    gyro[1] = (int16_t)((buffer[10] << 8) | buffer[11]);
    gyro[2] = (int16_t)((buffer[12] << 8) | buffer[13]);
}

void calcular_angulos(int16_t accel[3], float *inclinacao, float *balanco) {
    float ax = (float)accel[0] / ACCEL_SCALE_FACTOR;
    float ay = (float)accel[1] / ACCEL_SCALE_FACTOR;
    float az = (float)accel[2] / ACCEL_SCALE_FACTOR;

    *inclinacao = atan2f(ax, sqrtf(ay * ay + az * az)) * 180.0f / M_PI;
    *balanco    = atan2f(ay, sqrtf(ax * ax + az * az)) * 180.0f / M_PI;
}

void init_servo_pwm() {
    gpio_set_function(PINO_SERVO, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PINO_SERVO);
    uint chan_num = pwm_gpio_to_channel(PINO_SERVO);
    pwm_set_wrap(slice_num, PWM_WRAP);
    pwm_set_clkdiv(slice_num, 125.0f);
    pwm_set_chan_level(slice_num, chan_num, 0);
    pwm_set_enabled(slice_num, true);
    printf("Servo PWM configurado no pino GP%d.\n", PINO_SERVO);
}

uint32_t angulo_para_duty(float angulo) {
    uint32_t duty_us = (uint32_t)(SERVO_PULSE_MIN_US + 
                                  (angulo / 180.0f) * (SERVO_PULSE_MAX_US - SERVO_PULSE_MIN_US));
    if (duty_us < SERVO_PULSE_MIN_US) duty_us = SERVO_PULSE_MIN_US;
    if (duty_us > SERVO_PULSE_MAX_US) duty_us = SERVO_PULSE_MAX_US;
    return duty_us;
}

void setar_angulo_servo(float angulo) {
    uint slice_num = pwm_gpio_to_slice_num(PINO_SERVO);
    uint chan_num = pwm_gpio_to_channel(PINO_SERVO);
    uint32_t duty_us = angulo_para_duty(angulo);
    pwm_set_chan_level(slice_num, chan_num, duty_us);
}

int main() {
    stdio_init_all();

    printf("Iniciando sistema de monitoramento de inclinacao e controle de servo...\n");

    init_servo_pwm();
    mpu6050_init();

    float inclinacao = 0.0f;
    float balanco = 0.0f;
    int16_t accel_data[3];
    int16_t gyro_data[3];
    char alerta_str[32];

    setar_angulo_servo(90.0f);
    sleep_ms(500);

    while (true) {
        mpu6050_read_raw_data(accel_data, gyro_data);
        calcular_angulos(accel_data, &inclinacao, &balanco);

        printf("Inclinação: %.2f°, Balanço: %.2f°\n", inclinacao, balanco);

        float angulo_servo = inclinacao + 90.0f;
        if (angulo_servo < 0.0f) angulo_servo = 0.0f;
        if (angulo_servo > 180.0f) angulo_servo = 180.0f;
        setar_angulo_servo(angulo_servo);

        if (fabsf(inclinacao) > ANGULO_ALERTA_GRAUS || fabsf(balanco) > ANGULO_ALERTA_GRAUS) {
            snprintf(alerta_str, sizeof(alerta_str), "ALERTA: Inclinado!");
            // display_alert_lcd(true);
        } else {
            alerta_str[0] = '\0';
            // display_alert_lcd(false);
        }

        sleep_ms(100);
    }

    return 0;
}
