#include "hardware/adc.h"
#include "pico/float.h"
#include "pico/cyw43_arch.h"

#ifndef PICO_POWER_SAMPLE_COUNT
#define PICO_POWER_SAMPLE_COUNT 3
#endif

// Pin used for ADC 0
#define PICO_FIRST_ADC_PIN 26


void power_voltage(float *voltage_result) {
    cyw43_thread_enter();
    // Make sure cyw43 is awake
    cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN);

    // setup adc
    adc_gpio_init(PICO_VSYS_PIN);
    adc_select_input(PICO_VSYS_PIN - PICO_FIRST_ADC_PIN);
 
    adc_fifo_setup(true, false, 0, false, false);
    adc_run(true);

    // We seem to read low values initially - this seems to fix it
    int ignore_count = PICO_POWER_SAMPLE_COUNT;
    while (!adc_fifo_is_empty() || ignore_count-- > 0) {
        (void)adc_fifo_get_blocking();
    }

    // read vsys
    uint32_t vsys = 0;
    for(int i = 0; i < PICO_POWER_SAMPLE_COUNT; i++) {
        uint16_t val = adc_fifo_get_blocking();
        vsys += val;
    }

    adc_run(false);
    adc_fifo_drain();

    vsys /= PICO_POWER_SAMPLE_COUNT;
    cyw43_thread_exit();
    // Generate voltage
    const float conversion_factor = 3.3f / (1 << 12);
    // ADC_VREF is connected to VSYS via a voltage divider, with ADC_VREF connected to VSYS via a 200k resistor and to ground via a 100k resistor
    // This effectivly performs a division by 3, so the voltage must be scaled by 3 to correct for this.
    *voltage_result = vsys * 3 * conversion_factor;
}

int power_percent(const float *voltage) {
    const float min_battery_volts = 3.4f;
    const float max_battery_volts = 4.1f;
    int percent_val = ((*voltage - min_battery_volts) / (max_battery_volts - min_battery_volts)) * 100;
    percent_val = percent_val < 0 ? 0 : percent_val; // clamp to 0
    percent_val = percent_val > 100 ? 100 : percent_val; // clamp to 100
    return percent_val;
}

void power_print() {
    char *power_str = (char*)"UNKNOWN";
    // Get voltage
    float voltage = 0;
    power_voltage(&voltage);
    // voltage = floorf(voltage * 100) / 100;

    char percent_buf[10] = {0};
    power_str = (char*)"POWERED";
    if (!cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN)) {
        power_str = (char*)"BATTERY";
        int percent_val = power_percent(&voltage);
        snprintf(percent_buf, sizeof(percent_buf), " (%d%%)", percent_val);
    }

    printf("%f, %s%s\n", voltage, power_str, percent_buf);
}