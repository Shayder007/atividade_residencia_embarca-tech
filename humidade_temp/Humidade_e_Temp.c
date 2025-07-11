#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#define AHT10_ADDR 0x38
#define AHT10_CMD_INITIALIZE 0xE1
#define AHT10_CMD_MEASURE 0xAC
#define AHT10_CMD_SOFT_RESET 0xBA
#define AHT10_STATUS_BUSY_MASK 0x80
#define AHT10_STATUS_CAL_MASK 0x08

#define AHT10_I2C_PORT i2c0
#define AHT10_SDA_PIN 0
#define AHT10_SCL_PIN 1
#define AHT10_I2C_BAUDRATE 100000

#include "inc/ssd1306.h"

#define OLED_I2C_PORT i2c1
#define OLED_SDA_PIN 14
#define OLED_SCL_PIN 15
#define OLED_I2C_BAUDRATE 400000

struct render_area frame_area;
uint8_t ssd_buffer[ssd1306_buffer_length];

void aht10_init();
void aht10_reset();
bool aht10_read_data(float *humidity, float *temperature);

void init_oled();
void clear_oled_display();
void display_message_oled(const char *message, int line);

// Simulação de funções de LCD IDC 320x240 - para uso futuro
/*
void lcd_init() {
    // Inicialização do LCD 320x240 (ex: ILI9341)
}

void lcd_clear() {
    // Limpar tela do LCD
}

void lcd_print(const char *text, int x, int y) {
    // Mostrar texto na tela
}

void lcd_show_alert(const char *alert) {
    // Exibir alerta visual no LCD
}
*/

void aht10_init() {
    i2c_init(AHT10_I2C_PORT, AHT10_I2C_BAUDRATE);
    gpio_set_function(AHT10_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(AHT10_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(AHT10_SDA_PIN);
    gpio_pull_up(AHT10_SCL_PIN);

    aht10_reset();

    uint8_t init_cmd[3] = {AHT10_CMD_INITIALIZE, 0x08, 0x00};
    i2c_write_blocking(AHT10_I2C_PORT, AHT10_ADDR, init_cmd, 3, false);
    sleep_ms(300);

    uint8_t status;
    i2c_read_blocking(AHT10_I2C_PORT, AHT10_ADDR, &status, 1, false);
    if (!(status & AHT10_STATUS_CAL_MASK)) {
        printf("AHT10 nao calibrado!\n");
        display_message_oled("AHT10 Nao Calibrado", 0);
    } else {
        printf("AHT10 OK!\n");
        display_message_oled("AHT10 OK!", 0);
    }
}

void aht10_reset() {
    uint8_t reset_cmd = AHT10_CMD_SOFT_RESET;
    i2c_write_blocking(AHT10_I2C_PORT, AHT10_ADDR, &reset_cmd, 1, false);
    sleep_ms(20);
}

bool aht10_read_data(float *humidity, float *temperature) {
    uint8_t measure_cmd[3] = {AHT10_CMD_MEASURE, 0x33, 0x00};
    i2c_write_blocking(AHT10_I2C_PORT, AHT10_ADDR, measure_cmd, 3, false);
    sleep_ms(80);

    uint8_t status_byte;
    i2c_read_blocking(AHT10_I2C_PORT, AHT10_ADDR, &status_byte, 1, false);

    if (status_byte & AHT10_STATUS_BUSY_MASK) return false;

    uint8_t data[6];
    i2c_read_blocking(AHT10_I2C_PORT, AHT10_ADDR, data, 6, false);

    uint32_t raw_humidity = ((uint32_t)data[1] << 16) |
                            ((uint32_t)data[2] << 8) |
                            data[3];
    raw_humidity >>= 4;

    uint32_t raw_temperature = ((uint32_t)(data[3] & 0x0F) << 16) |
                               ((uint32_t)data[4] << 8) |
                               data[5];

    *humidity = (float)raw_humidity * 100.0f / 1048576.0f;
    *temperature = (float)raw_temperature * 200.0f / 1048576.0f - 50.0f;

    return true;
}

void init_oled() {
    i2c_init(OLED_I2C_PORT, OLED_I2C_BAUDRATE);
    gpio_set_function(OLED_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(OLED_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(OLED_SDA_PIN);
    gpio_pull_up(OLED_SCL_PIN);

    ssd1306_init();
    frame_area.start_column = 0;
    frame_area.end_column = ssd1306_width - 1;
    frame_area.start_page = 0;
    frame_area.end_page = ssd1306_n_pages - 1;
    calculate_render_area_buffer_length(&frame_area);
    memset(ssd_buffer, 0, ssd1306_buffer_length);

    sleep_ms(100);
}

void clear_oled_display() {
    memset(ssd_buffer, 0, ssd1306_buffer_length);
    render_on_display(ssd_buffer, &frame_area);
}

void display_message_oled(const char *message, int line) {
    ssd1306_draw_string(ssd_buffer, 5, line * 8, message);
}

int main() {
    stdio_init_all();

    init_oled();
    clear_oled_display();
    display_message_oled("Iniciando...", 0);
    render_on_display(ssd_buffer, &frame_area);
    sleep_ms(2000);

    clear_oled_display();
    aht10_init();

    float humidity, temperature;
    char temp_str[32];
    char hum_str[32];
    int leitura_num = 0;

    while (true) {
        if (aht10_read_data(&humidity, &temperature)) {
            leitura_num++;
            snprintf(temp_str, sizeof(temp_str), "Temp: %.1f C", temperature);
            snprintf(hum_str, sizeof(hum_str), "Umid: %.1f %%RH", humidity);

            const char* temp_status;
            const char* hum_status;

            // Classificação da temperatura
            if (temperature < 20.0f) {
                temp_status = "Temp: Baixa";
            } else if (temperature >= 27.0f) {
                temp_status = "Temp: Alta";
            } else {
                temp_status = "Temp: Amena";
            }

            // Classificação da umidade
            if (humidity < 30.0f) {
                hum_status = "Umid: Muito Seca";
            } else if (humidity <= 60.0f) {
                hum_status = "Umid: Ideal";
            } else {
                hum_status = "Umid: Muito Umida";
            }

            // === OLED ===
            clear_oled_display();
            display_message_oled(temp_str, 0);
            display_message_oled(hum_str, 1);
            display_message_oled(temp_status, 2);
            display_message_oled(hum_status, 3);

            if (humidity > 70.0f) {
                display_message_oled("ALERTA: Umid >70%", 5);
            }

            if (temperature < 20.0f) {
                display_message_oled("ALERTA: Temp <20C", 6);
            }

            render_on_display(ssd_buffer, &frame_area);

            // === SERIAL MONITOR ===
            printf("========== Leitura #%d ==========\n", leitura_num);
            printf("Temperatura: %.1f C\n", temperature);
            printf(" -> %s\n", temp_status);
            printf("Umidade: %.1f %%RH\n", humidity);
            printf(" -> %s\n", hum_status);

            if (humidity > 70.0f) {
                printf("!!! ALERTA: Umidade acima de 70%% !!!\n");
            }
            if (temperature < 20.0f) {
                printf("!!! ALERTA: Temperatura abaixo de 20°C !!!\n");
            }
            printf("\n");

            // === LCD simulado ===
            /*
            lcd_clear();
            lcd_print(temp_str, 10, 30);
            lcd_print(hum_str, 10, 60);
            lcd_print(temp_status, 10, 90);
            lcd_print(hum_status, 10, 120);
            if (humidity > 70.0f) {
                lcd_show_alert("ALERTA: Umidade > 70%");
            }
            if (temperature < 20.0f) {
                lcd_show_alert("ALERTA: Temp. < 20C");
            }
            */

        } else {
            printf("Erro ao ler AHT10. Resetando...\n");
            clear_oled_display();
            display_message_oled("Erro AHT10!", 0);
            render_on_display(ssd_buffer, &frame_area);
            aht10_reset();
        }

        sleep_ms(3000);
    }

    return 0;
}
