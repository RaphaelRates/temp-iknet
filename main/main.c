#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "onewire_bus.h"
#include "ds18b20.h"

static const char *TAG = "IKNET";
static const char *classificar(float temperatura)
{
    if (temperatura < 21) return "FRIO";
    if (temperatura <= 40) return "NORMAL";
    if (temperatura <= 50) return "ATENCAO";
    return "CRITICO";
}

// Procura um DS18B20 e guarda seu endereco no handle do driver.
static esp_err_t procurar_sensor(onewire_bus_handle_t bus,
                                ds18b20_device_handle_t *sensor)
{
    onewire_device_iter_handle_t iter = NULL;
    esp_err_t erro = onewire_new_device_iter(bus, &iter);
    if (erro != ESP_OK) return erro;

    onewire_device_t dispositivo;
    const ds18b20_config_t config = {0};
    while ((erro = onewire_device_iter_get_next(iter, &dispositivo)) == ESP_OK) {
        erro = ds18b20_new_device_from_enumeration(&dispositivo, &config, sensor);
        if (erro == ESP_OK) {
            ESP_LOGI(TAG, "DS18B20 encontrado: %016llX",
                     (unsigned long long)dispositivo.address);
            break;
        }
        if (erro != ESP_ERR_NOT_SUPPORTED) break;
    }
    onewire_del_device_iter(iter);
    return erro;
}

void app_main(void)
{
    onewire_bus_handle_t bus = NULL;
    const onewire_bus_config_t config = {
        .bus_gpio_num = CONFIG_IKNET_SENSOR_GPIO,
    };
    const onewire_bus_rmt_config_t rmt = { .max_rx_bytes = 10 };
    ESP_ERROR_CHECK(onewire_new_bus_rmt(&config, &rmt, &bus));
    ds18b20_device_handle_t sensor = NULL;
    ESP_LOGI(TAG, "DS18B20 no GPIO%d; temperatura no terminal.", CONFIG_IKNET_SENSOR_GPIO);

    while (true) {
        // Sensor ausente na partida ou desconectado: tenta encontra-lo novamente.
        esp_err_t erro = sensor ? ESP_OK : procurar_sensor(bus, &sensor);
        float temperatura = NAN;
        if (erro == ESP_OK) erro = ds18b20_trigger_temperature_conversion(sensor);
        if (erro == ESP_OK) erro = ds18b20_get_temperature(sensor, &temperatura);

        bool valida = erro == ESP_OK && isfinite(temperatura) &&
                      temperatura >= -55 && temperatura <= 125;
        const char *nivel = valida ? classificar(temperatura) : "FALHA SENSOR";
        if (!valida) {
            ESP_LOGW(TAG, "Sem leitura valida: %s. nova tentativa em 1 s.",
                     erro == ESP_OK ? "fora da faixa do sensor" : esp_err_to_name(erro));
            if (sensor) {
                ds18b20_del_device(sensor);
                sensor = NULL;
            }
        } else {
            ESP_LOGI(TAG, "Temperatura: %.2f C | Nivel: %s",
                     (double)temperatura, nivel);
        }
        // A conversao tambem consome tempo; nao representa amostragem exata de 1 Hz.
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
