
#include "inc/data/hwaa_data.h"
#include <linux/rwsem.h>
#include "inc/data/hwaa_running_apps.h"
#include "inc/data/hwaa_trusted_pids.h"
#include "inc/data/hwaa_packages.h"

static struct rw_semaphore g_data_lock;

void hwaa_data_init(void)
{
	init_rwsem(&g_data_lock);
	down_write(&g_data_lock);
	hwaa_trusted_pids_init();
	up_write(&g_data_lock);
}

void hwaa_data_deinit(void)
{
	down_write(&g_data_lock);

	hwaa_trusted_pids_deinit();
	hwaa_running_apps_delete_all();
	hwaa_packages_delete_all();

	up_write(&g_data_lock);
}

void hwaa_data_read_lock(void)
{
	down_read(&g_data_lock);
}

void hwaa_data_read_unlock(void)
{
	up_read(&g_data_lock);
}

void hwaa_data_write_lock(void)
{
	down_write(&g_data_lock);
}

void hwaa_data_write_unlock(void)
{
	up_write(&g_data_lock);
}
