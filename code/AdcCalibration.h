//这段校准代码来自arduino论坛

#include "esp_adc_cal.h"
#define ADC_CALI_SCHEME ESP_ADC_CAL_VAL_EFUSE_TP
#define ADC_ATTEN ADC_ATTEN_DB_2_5

static esp_adc_cal_characteristics_t adc1_chars;
static esp_adc_cal_characteristics_t adc2_chars;
bool cali_enable;

static bool adc_calibration_init(void)
{
    esp_err_t ret;
    bool cali_enable = false;

    ret = esp_adc_cal_check_efuse(ADC_CALI_SCHEME);
    if (ret == ESP_ERR_NOT_SUPPORTED)
    {
        Serial.println("Calibration scheme not supported, skip software calibration");
    }
    else if (ret == ESP_ERR_INVALID_VERSION)
    {
        Serial.println("eFuse not burnt, skip software calibration");
    }
    else if (ret == ESP_OK)
    {
        cali_enable = true;
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH_BIT_12, 0, &adc1_chars);
        esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN, ADC_WIDTH_BIT_12, 0, &adc2_chars);
    }
    else
    {
        Serial.println("Invalid arg");
    }
    return cali_enable;
}
