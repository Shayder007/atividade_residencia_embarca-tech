#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"

// Pinos RGB
#define VERDE 11
#define AZUL 12
#define VERMELHO 13

// BH1750 - usando i2c0
#define I2C_BH1750_PORT i2c0
#define I2C_BH1750_SDA 0  // GP0
#define I2C_BH1750_SCL 1  // GP1

// Servo motor
#define PINO_SERVO 8
#define PERIODO_SERVO 20

// PWM
const uint16_t periodo = 200;
const float divisor_pwm = 16.0;

// BH1750
#define BH1750_ADDR 0x23
#define BH1750_CONTINUOUS_HIGH_RES_MODE 0x10

void enviar_pulso(uint duty_us) {
    gpio_put(PINO_SERVO, 1);
    sleep_us(duty_us);
    gpio_put(PINO_SERVO, 0);
    sleep_ms(PERIODO_SERVO - (duty_us / 1000));
}

void config_pwm(int led) {
    gpio_set_function(led, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(led);
    pwm_set_clkdiv(slice, divisor_pwm);
    pwm_set_wrap(slice, periodo);
    pwm_set_gpio_level(led, 0);
    pwm_set_enabled(slice, true);
}

void bh1750_init() {
    uint8_t buf[1] = {BH1750_CONTINUOUS_HIGH_RES_MODE};
    i2c_write_blocking(I2C_BH1750_PORT, BH1750_ADDR, buf, 1, false);
}

float bh1750_read_lux() {
    uint8_t data[2];
    int result = i2c_read_blocking(I2C_BH1750_PORT, BH1750_ADDR, data, 2, false);
    if (result != 2) {
        printf("Erro ao ler o sensor BH1750\n");
        return -1;
    }
    uint16_t raw = (data[0] << 8) | data[1];
    return raw / 1.2;
}

void atualizar_por_lux(float lux) {
    printf("Luminosidade: %.1f lx - ", lux);

    if (lux < 10) {
        printf("Ambiente escuro)\n");
        pwm_set_gpio_level(VERDE, 0);
        pwm_set_gpio_level(AZUL, periodo * 0.1);
        pwm_set_gpio_level(VERMELHO, 0);
    } 
    else if (lux < 50) {
        printf("Ambiente com luz baixa\n");
        pwm_set_gpio_level(VERDE, 0);
        pwm_set_gpio_level(AZUL, periodo * 0.4);
        pwm_set_gpio_level(VERMELHO, periodo * 0.2);
        enviar_pulso(600);
    } 
    else if (lux < 200) {
        printf("Ambiente iluminado\n");
        pwm_set_gpio_level(VERDE, periodo);
        pwm_set_gpio_level(AZUL, 0);
        pwm_set_gpio_level(VERMELHO, periodo * 0.5);
        enviar_pulso(1000);
    } 
    else if (lux < 1000) {
        printf("Ambiente claro\n");
        pwm_set_gpio_level(VERDE, periodo * 0.5);
        pwm_set_gpio_level(AZUL, periodo);
        pwm_set_gpio_level(VERMELHO, 0);
        enviar_pulso(1500);
    } 
    else {
        printf("Ambiente muito claro\n");
        pwm_set_gpio_level(VERDE, 0);
        pwm_set_gpio_level(AZUL, 0);
        pwm_set_gpio_level(VERMELHO, periodo);
        enviar_pulso(2000);
    }
}

int main() {
    stdio_init_all();

    gpio_init(PINO_SERVO);
    gpio_set_dir(PINO_SERVO, GPIO_OUT);

    // Inicializa I2C0 (BH1750)
    i2c_init(I2C_BH1750_PORT, 100 * 1000);
    gpio_set_function(I2C_BH1750_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_BH1750_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_BH1750_SDA);
    gpio_pull_up(I2C_BH1750_SCL);

    // Inicializa sensor BH1750
    bh1750_init();

    // Inicializa LEDs PWM
    config_pwm(VERDE);
    config_pwm(AZUL);
    config_pwm(VERMELHO);

    printf("Sistema iniciado...\n");

    while (true) {
        float lux = bh1750_read_lux();
        if (lux >= 0) {
            atualizar_por_lux(lux);
        }
        sleep_ms(1000);
    }

    return 0;
}
