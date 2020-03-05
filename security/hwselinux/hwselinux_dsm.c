#include <linux/audit.h>
#include <linux/lsm_audit.h>
#include "chipset_common/log_usertype/log-usertype.h"

extern bool runmode_is_factory(void);

void selinux_dsm_process(struct audit_buffer *ab, void *a, u32 denied)
{
	if (!ab || !a) {
		return;
	}

	if (!runmode_is_factory()) {
		unsigned int log_type = get_logusertype_flag();
		if ((log_type == BETA_USER) || (log_type == OVERSEA_USER)) {
			audit_log_format(ab, " dsm=1");
		}
	}
}
