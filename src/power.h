#include "hardware/adc.h"
#include "pico/float.h"
#include "pico/cyw43_arch.h"

#ifndef PICO_POWER_SAMPLE_COUNT
#define PICO_POWER_SAMPLE_COUNT 3
#endif

// Pin used for ADC 0
#define PICO_FIRST_ADC_PIN 26


void power_voltage(float *voltage_result);
int power_percent(const float *voltage);
bool power_is_charging();
void power_print();