#include "storage.h"
#include <stdio.h>
#include "esp_spiffs.h"
#include "esp_log.h"

static const char *TAG = "STORAGE";

esp_err_t init_storage(void) {
    ESP_LOGI(TAG, "Inicializando SPIFFS...");
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };
    return esp_vfs_spiffs_register(&conf);
}

esp_err_t write_log(float temp, float hum, int lum) {
    FILE* f = fopen("/spiffs/log.txt", "a");
    if (f == NULL) {
        ESP_LOGE(TAG, "Erro ao abrir log.txt");
        return ESP_FAIL;
    }
    fprintf(f, "T:%.1f|H:%.1f|L:%d\n", temp, hum, lum);
    fclose(f);
    return ESP_OK;
}

void read_log_to_serial(void) {
    FILE* f = fopen("/spiffs/log.txt", "r");
    if (f == NULL) {
        printf("Nenhum dado registrado ainda.\n");
        return;
    }
    char line[64];
    printf("\n--- LOG DA ESTUFA ---\n");
    while (fgets(line, sizeof(line), f) != NULL) {
        printf("%s", line);
    }
    printf("---------------------\n");
    fclose(f);
}
