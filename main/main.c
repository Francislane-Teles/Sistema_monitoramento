#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "storage.h"

static const char *TAG = "MAIN_ESTUFA";

static int g_setpoint;
static bool g_logging_enabled = true;
static bool g_system_on = true;

void serial_command_task(void *pvParameters) {
    char c;
    while(1) {
        c = getchar();
        if (c != EOF && c != '\n' && c != '\r') {
            if (c == 'r' || c == 'R') {
                read_log_to_serial();
            } else if (c == 's' || c == 'S') {
                g_system_on = !g_system_on;
                printf("\n>>> Sistema de Controle alternado para: %s <<<\n", g_system_on ? "LIGADO" : "DESLIGADO");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void greenhouse_control_task(void *pvParameters) {
    float current_temp = 24.5; 
    float current_hum = 55.0;
    int current_lum = 0;

    while(1) {
        // Simulação de oscilação dos sensores para teste inicial
        current_temp += 0.2;
        if(current_temp > 30.0) current_temp = 22.0;

        // Leitura simulada do LDR via ADC usando o canal configurado no menuconfig
        current_lum = adc1_get_raw(CONFIG_ESTUFA_LDR_CHANNEL);

        printf("\n[MONITORAMENTO] Temp: %.1f C | Umid: %.1f%% | Lum: %d | Setpoint: %d C\n", 
               current_temp, current_hum, current_lum, g_setpoint);

        if (g_system_on) {
            if (current_temp > g_setpoint) {
                ESP_LOGW(TAG, "ALERTA: Temperatura (%.1f) ACIMA do Setpoint (%d)! Ativando cooler.", current_temp, g_setpoint);
            } else {
                ESP_LOGI(TAG, "Status: Temperatura sob controle.");
            }
        }

        if (g_logging_enabled) {
            write_log(current_temp, current_hum, current_lum);
        }

        vTaskDelay(pdMS_TO_TICKS(3000)); 
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Iniciando Sistema da Estufa...");

    // Pega o valor padrão definido no seu menuconfig
    g_setpoint = CONFIG_ESTUFA_DEFAULT_SETPOINT;

    if (init_storage() != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao iniciar SPIFFS!");
    }

    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(CONFIG_ESTUFA_LDR_CHANNEL, ADC_ATTEN_DB_11);

    xTaskCreate(greenhouse_control_task, "greenhouse_task", 4096, NULL, 5, NULL);
    xTaskCreate(serial_command_task, "serial_task", 2048, NULL, 4, NULL);
}
