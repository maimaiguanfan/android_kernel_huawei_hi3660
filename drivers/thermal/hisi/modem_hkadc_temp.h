#ifndef __MODEM_HKADC_TEMP_H
#define __MODEM_HKADC_TEMP_H
#include <linux/thermal.h>
#include <linux/hisi/hisi_adc.h>

struct hisi_mdm_adc_t {
    struct thermal_zone_device  *tz_dev;
    const char	    *name;
    const char	    *ntc_name;
    unsigned int	channel;
    unsigned int	mode;
};

struct hisi_mdm_adc_chip {
    int tsens_num_sensor;
    struct hisi_mdm_adc_t hisi_mdm_sensor[0];
};

extern  int hisi_mdm_ntc_2_temp(struct hisi_mdm_adc_t *mdm_sensor, int *temp, int ntc);

#endif
