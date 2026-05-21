#ifndef STORAGE_H
#define STORAGE_H

#include "esp_err.h"

esp_err_t init_storage(void);
esp_err_t write_log(float temp, float hum, int lum);
void read_log_to_serial(void);

#endif
