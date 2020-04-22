
#ifndef _HWAA_DATA_H
#define _HWAA_DATA_H

void hwaa_data_init(void);

void hwaa_data_deinit(void);

void hwaa_data_read_lock(void);

void hwaa_data_read_unlock(void);

void hwaa_data_write_lock(void);

void hwaa_data_write_unlock(void);

#endif
