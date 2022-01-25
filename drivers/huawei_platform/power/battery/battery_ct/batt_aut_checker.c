#include <huawei_platform/power/battery_type_identify.h>
#include "batt_aut_checker.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG batt_aut_checker
HWLOG_REGIST();

/* this list contain all possible IC for all battery checkers */
LIST_HEAD(batt_aut_ic_head);
void add_to_aut_ic_list(ct_ops_reg_list* entry)
{
    list_add_tail(&entry->node, &batt_aut_ic_head);
}

static batt_ic_type get_ic_type_wrapper(batt_checker_data *drv_data)
{
	batt_ic_type type = LOCAL_IC_TYPE;

	if (drv_data->ic_ops.get_ic_type) {
		apply_batt_type_mode(BAT_ID_SN);
		type = drv_data->ic_ops.get_ic_type();
		release_batt_type_mode();
	}
	return type;
}

static int get_batt_type_wrapper(batt_checker_data *drv_data,
	const unsigned char **type, unsigned int *type_len)
{
	int ret = -1;

	if (drv_data->ic_ops.get_batt_type) {
		apply_batt_type_mode(BAT_ID_SN);
		ret = drv_data->ic_ops.get_batt_type(drv_data->ic, type, type_len);
		release_batt_type_mode();
	}
	return ret;
}

static int get_batt_sn_wrapper(batt_checker_data *drv_data, resource *res,
	const unsigned char **sn, unsigned int *sn_len_bits)
{
	int ret = -1;

	if (drv_data->ic_ops.get_batt_sn) {
		apply_batt_type_mode(BAT_ID_SN);
		ret = drv_data->ic_ops.get_batt_sn(drv_data->ic, res, sn, sn_len_bits);
		release_batt_type_mode();
	}
	return ret;
}

static int prepare_wrapper(batt_checker_data *drv_data, enum RES_TYPE type,
	resource *res)
{
	int ret = -1;

	if (drv_data->ic_ops.prepare) {
		apply_batt_type_mode(BAT_ID_SN);
		ret = drv_data->ic_ops.prepare(drv_data->ic, type, res);
		release_batt_type_mode();
	}
	return ret;
}

static int certification_wrapper(batt_checker_data *drv_data,
	resource *res, enum KEY_CR *result)
{
	int ret = -1;

	if (drv_data->ic_ops.certification) {
		apply_batt_type_mode(BAT_ID_SN);
		ret = drv_data->ic_ops.certification(drv_data->ic, res, result);
		release_batt_type_mode();
	}
	return ret;
}

static int set_batt_safe_info_wrapper(batt_checker_data *drv_data,
	batt_safe_info_type type, void *value)
{
	int ret = -1;

	if (drv_data->ic_ops.set_batt_safe_info) {
		apply_batt_type_mode(BAT_ID_SN);
		ret = drv_data->ic_ops.set_batt_safe_info(drv_data->ic, type, value);
		release_batt_type_mode();
	}
	return ret;
}

static int get_batt_safe_info_wrapper(batt_checker_data *drv_data,
	batt_safe_info_type type, void *value)
{
	int ret = -1;

	if (drv_data->ic_ops.get_batt_safe_info) {
		apply_batt_type_mode(BAT_ID_SN);
		ret = drv_data->ic_ops.get_batt_safe_info(drv_data->ic, type, value);
		release_batt_type_mode();
	}
	return ret;
}

/*-----------------------------------------------------------------------------------------------*/
/*------------------------------------delayed work start-----------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
static int legal_sn_check_0(batt_checker_data* drv_data)
{
    drv_data->result.sn_status = SN_PASS;
    return 0;
}

static const legal_sn_check_t legal_sn_checkers[] = {
    legal_sn_check_0,
};
static void data_to_dev_info(batt_checker_data* data, batt_info_subcmd_t cmd, nl_dev_info* info) {
    info->id_in_grp = data->id_in_grp;
    info->chks_in_grp = data->chks_in_grp;
    info->id_of_grp = data->id_of_grp;
    info->ic_type = data->ic_type;
    info->subcmd = cmd;
}
static void check_key_nop(struct work_struct *work)
{
    return;
}
static void check_key_func(struct work_struct *work)
{
    batt_checker_data* drv_data = container_of(work, batt_checker_data, check_key_work);
    struct batt_ct_wrapper_ops *bco = &drv_data->bco;
    resource res;
    nl_dev_info info;

    hwlog_info("Battery checker(NO:%d Group:%d) prepared to certification.\n",
               drv_data->id_in_grp, drv_data->id_of_grp);
    if (bco->prepare(drv_data, RES_CT, &res)) {
        hwlog_err("Battery checker(NO:%d Group:%d) prepear for key failed.\n",
                  drv_data->id_in_grp, drv_data->id_of_grp);
        drv_data->result.key_status = KEY_FAIL_IC;
        return;
    }
    if(res.len > 0) {
        data_to_dev_info(drv_data, CT_PREPARE, &info);
        if(send_batt_info_mesg(&info, (void *)res.data, res.len)) {
            hwlog_err("Send attributes failed in %s.\n", __func__);
            drv_data->result.key_status = KEY_FAIL_TIMEOUT;
            return;
        }
        wait_for_completion(&drv_data->key_prepare_ready);
    }
    hwlog_info("Battery checker(NO:%d Group:%d) certification started.\n",
               drv_data->id_in_grp, drv_data->id_of_grp);
    if (bco->certification(drv_data, &drv_data->key_res,
        &drv_data->result.key_status)) {
        hwlog_err("Battery checker(NO:%d Group:%d) certification failed.\n",
                  drv_data->id_in_grp, drv_data->id_of_grp);
        drv_data->result.key_status = KEY_FAIL_IC;
        return;
    }
}
static void check_sn_nop(struct work_struct *work)
{
    return;
}
static void check_sn_func(struct work_struct *work)
{
    batt_checker_data* drv_data = container_of(work, batt_checker_data, check_sn_work);
    resource res;
    struct batt_ct_wrapper_ops *bco = &drv_data->bco;
    nl_dev_info info;

    if (bco->prepare(drv_data, RES_SN, &res)) {
        hwlog_err("battery(NO:%d Group:%d) prepear for key failed in %s.\n",
                  drv_data->id_in_grp, drv_data->id_of_grp, __func__);
        drv_data->result.sn_status = SN_FAIL_IC;
        return;
    }
    if(res.len > 0) {
        data_to_dev_info(drv_data, SN_PREPARE, &info);
        if(send_batt_info_mesg(&info, (void *)res.data, res.len)) {
            hwlog_err("battery(NO:%d Group:%d) send attributes failed in %s.\n",
                      drv_data->id_in_grp, drv_data->id_of_grp, __func__);
            drv_data->result.sn_status = SN_FAIL_TIMEOUT;
            return;
        }
        wait_for_completion(&drv_data->sn_prepare_ready);
    }
    if (bco->get_batt_sn(drv_data, &drv_data->sn_res,
        &drv_data->sn, &drv_data->sn_len)) {
        hwlog_err("battery(NO:%d Group:%d) get batt sn failed in %s.\n",
                  drv_data->id_in_grp, drv_data->id_of_grp, __func__);
        drv_data->result.sn_status = SN_FAIL_IC;
        return;
    };
    if(drv_data->sn_checker(drv_data)) {
        hwlog_err("battery(NO:%d Group:%d) sn_checker failed in %s.\n",
                  drv_data->id_in_grp, drv_data->id_of_grp, __func__);
        return;
    }
}
static const work_func_t check_key_funcs[] = {
    check_key_nop,
    check_key_func,
};
static const work_func_t check_sn_funcs[] = {
    check_sn_nop,
    check_sn_func,
};

/*-----------------------------------------------------------------------------------------------*/
/*----------------------------------------work process end---------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------*/
/*----------------------------------------sys node start-----------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
static const char *ic_type2name[] = {
    [LOCAL_IC_TYPE] = "INVALID_DEFAULT_IC",
    [MAXIM_DS28EL15_TYPE] = "DS28EL15",
    [MAXIM_DS28EL16_TYPE] = "DS28EL16",
    [NXP_A1007_TYPE] = "A1007",
};

static ssize_t ic_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int type;
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, snprintf(buf, PAGE_SIZE, "Error driver data"));
    type = drv_data->bco.get_ic_type(drv_data);
    if(type < ARRAY_SIZE(ic_type2name) && type >= 0) {
        return snprintf(buf, PAGE_SIZE, "%s", ic_type2name[type]);
    } else {
        return snprintf(buf, PAGE_SIZE, "ERROR_TYPE");
    }
}

static ssize_t batt_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int i;
    int total_size = 0;
    const unsigned char *type;
    unsigned int type_len;
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, snprintf(buf, PAGE_SIZE, "Error driver data"));
    if (drv_data->bco.get_batt_type(drv_data, &type, &type_len)) {
        hwlog_err("Get ic id failed in %s.\n", __func__);
        return snprintf(buf, PAGE_SIZE, "ERROR:Can't get type from IC");
    }

    for(i = 0; i < type_len; i++) {
        total_size += snprintf(&(buf[total_size]), BATT_ID_PRINT_SIZE_PER_CHAR, "%02x", type[i]);
    }
    return total_size;
}

static ssize_t batt_id_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    const unsigned char *sn;
    unsigned int sn_len;
    int ret;
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, snprintf(buf, PAGE_SIZE, "Error driver data"));
    ret = drv_data->bco.get_batt_sn(drv_data, &drv_data->sn_res, &sn, &sn_len);
    if(ret) {
        hwlog_err("Get battery SN failed in %s.\n", __func__);
        return snprintf(buf, PAGE_SIZE, "ERROR:Can't get sn from IC");
    }
    memcpy(buf, sn, sn_len);
    buf[sn_len] = 0;
    return sn_len;
}

static ssize_t ic_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, snprintf(buf, PAGE_SIZE, "Error driver data"));
    return snprintf(buf, PAGE_SIZE, "%d", drv_data->result.ic_status);
}

static ssize_t key_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, snprintf(buf, PAGE_SIZE, "Error driver data"));
    return snprintf(buf, PAGE_SIZE, "%d", drv_data->result.key_status);
}

static ssize_t sn_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, snprintf(buf, PAGE_SIZE, "Error driver data"));
    return snprintf(buf, PAGE_SIZE, "%d", drv_data->result.sn_status);
}
static ssize_t check_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, snprintf(buf, PAGE_SIZE, "Error driver data"));
    return snprintf(buf, PAGE_SIZE, "%d", drv_data->result.check_mode);
}
#ifdef BATTERY_LIMIT_DEBUG
static ssize_t rematchable_onboot_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, snprintf(buf, PAGE_SIZE, "Error driver data"));
    return snprintf(buf, PAGE_SIZE, "%d", drv_data->batt_rematch_onboot);
}

static ssize_t rematchable_current_show(struct device *dev, struct device_attribute *attr,
                                        char *buf)
{
    int ret;
    enum BATT_MATCH_TYPE temp;
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, snprintf(buf, PAGE_SIZE, "Error driver data"));
    ret = drv_data->bco.get_batt_safe_info(drv_data, BATT_MATCH_ABILITY, &temp);
    if(ret) {
        return snprintf(buf, PAGE_SIZE, "%s", "Error");
    }
    return snprintf(buf, PAGE_SIZE, "%s", temp == BATTERY_REMATCHABLE ? "YES":"NO");
}

static ssize_t lock_battery_store(struct device *dev, struct device_attribute *attr,
                                  const char *buf, size_t count)
{
    int ret;
    enum BATT_MATCH_TYPE temp = BATTERY_UNREMATCHABLE;
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, -1);
    if (count < 1 || buf[0] != '1')
    {
        return -1;
    }
    ret = drv_data->bco.set_batt_safe_info(drv_data, BATT_MATCH_ABILITY, &temp);
    if(ret) {
        return -1;
    }
    return count;
}

static ssize_t free_cycles_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, snprintf(buf, PAGE_SIZE, "Error driver data"));
    return snprintf(buf, PAGE_SIZE, "%d", drv_data->free_cycles);
}

static ssize_t free_cycles_store(struct device *dev, struct device_attribute *attr,
                                 const char *buf, size_t count)
{
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, -1);
    if(buf[count - 1] == '\n' && !kstrtoint(buf, 10, &drv_data->free_cycles)) {
        return -1;
    }
    return count;
}

static ssize_t check_result_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, snprintf(buf, PAGE_SIZE, "Error driver data"));
    return snprintf(buf, PAGE_SIZE, "%d %d %d", drv_data->result.ic_status,
                    drv_data->result.key_status, drv_data->result.sn_status);
}
static ssize_t check_result_store(struct device *dev, struct device_attribute *attr,
                                  const char *buf, size_t count)
{
    char *sub, *cur;
    int temp0, temp1, temp2;
    char str[64] = {0};
    size_t len;
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, -1);
    len = min_t(size_t, sizeof(str) - 1, count);
    memcpy(str, buf, len);
    cur = &str[0];
    sub = strsep(&cur, " ");
    if( !sub || kstrtoint(sub, 0, &temp0)) {
        return -1;
    }
    sub = strsep(&cur, " ");
    if( !sub || kstrtoint(sub, 0, &temp1)) {
        return -1;
    }
    if( !cur || kstrtoint(cur, 0, &temp2)) {
        return -1;
    }
    drv_data->result.ic_status = temp0;
    drv_data->result.key_status = temp1;
    drv_data->result.sn_status = temp2;
    return count;
}
static ssize_t sn_checker_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, snprintf(buf, PAGE_SIZE, "Error driver data"));
    return snprintf(buf, PAGE_SIZE, "%pf", drv_data->sn_checker);
}
static ssize_t sn_checker_store(struct device *dev, struct device_attribute *attr,
                                const char *buf, size_t count)
{
    batt_checker_data* drv_data;
    int temp;

    DEV_GET_DRVDATA(drv_data, dev, -1);
    if(count >= 5 && !memcmp(buf, "type", 4)) {
        temp = buf[4] - '0';
        if(temp >= 0 && temp <= ARRAY_SIZE(legal_sn_checkers)) {
            drv_data->sn_checker = legal_sn_checkers[temp];
        }
    }
    return count;
}
static ssize_t check_key_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, snprintf(buf, PAGE_SIZE, "Error driver data"));
    return snprintf(buf, PAGE_SIZE, "%pf", drv_data->check_key_work.func);
}
static ssize_t check_key_type_store(struct device *dev, struct device_attribute *attr,
                                 const char *buf, size_t count)
{
    batt_checker_data* drv_data;
    int temp;

    DEV_GET_DRVDATA(drv_data, dev, -1);
    if(count >= 5 && !memcmp(buf, "type", 4)) {
        temp = buf[4] - '0';
        if(temp >= 0 && temp <= ARRAY_SIZE(check_key_funcs)) {
            drv_data->check_key_work.func = check_key_funcs[temp];
        }
    }
    return count;
}
static ssize_t check_sn_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, snprintf(buf, PAGE_SIZE, "Error driver data"));
    return snprintf(buf, PAGE_SIZE, "%pf", drv_data->check_sn_work.func);
}
static ssize_t check_sn_type_store(struct device *dev, struct device_attribute *attr,
                                   const char *buf, size_t count)
{
    batt_checker_data* drv_data;
    int temp;

    DEV_GET_DRVDATA(drv_data, dev, -1);
    if(count >= 5 && !memcmp(buf, "type", 4)) {
        temp = buf[4] - '0';
        if(temp >= 0 && temp <= ARRAY_SIZE(check_sn_funcs)) {
            drv_data->check_sn_work.func = check_sn_funcs[temp];
        }
    }
    return count;
}
static ssize_t check_key_store(struct device *dev, struct device_attribute *attr,
                                   const char *buf, size_t count)
{
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, -1);
    if(count >= 5 && !memcmp(buf, "check", 5)) {
        schedule_work(&drv_data->check_key_work);
    }
    return count;
}
static ssize_t check_sn_store(struct device *dev, struct device_attribute *attr,
                                   const char *buf, size_t count)
{
    batt_checker_data* drv_data;

    DEV_GET_DRVDATA(drv_data, dev, -1);
    if(count >= 5 && !memcmp(buf, "check", 5)) {
        schedule_work(&drv_data->check_sn_work);
    }
    return count;
}
#endif

const static DEVICE_ATTR_RO(ic_type);
const static DEVICE_ATTR_RO(batt_type);
const static DEVICE_ATTR_RO(ic_status);
const static DEVICE_ATTR_RO(key_status);
const static DEVICE_ATTR_RO(sn_status);
const static DEVICE_ATTR_RO(batt_id);
const static DEVICE_ATTR_RO(check_mode);
#ifdef BATTERY_LIMIT_DEBUG
const static DEVICE_ATTR_RO(rematchable_onboot);
const static DEVICE_ATTR_RO(rematchable_current);
const static DEVICE_ATTR_WO(lock_battery);
const static DEVICE_ATTR_RW(free_cycles);
const static DEVICE_ATTR_RW(check_result);
const static DEVICE_ATTR_RW(sn_checker);
const static DEVICE_ATTR_RW(check_key_type);
const static DEVICE_ATTR_RW(check_sn_type);
const static DEVICE_ATTR_WO(check_key);
const static DEVICE_ATTR_WO(check_sn);
#endif


static const struct attribute *batt_checker_attrs[] = {
    &dev_attr_ic_type.attr,
    &dev_attr_batt_type.attr,
    &dev_attr_ic_status.attr,
    &dev_attr_key_status.attr,
    &dev_attr_sn_status.attr,
    &dev_attr_batt_id.attr,
    &dev_attr_check_mode.attr,
#ifdef BATTERY_LIMIT_DEBUG
    &dev_attr_rematchable_onboot.attr,
    &dev_attr_rematchable_current.attr,
    &dev_attr_lock_battery.attr,
    &dev_attr_free_cycles.attr,
    &dev_attr_check_result.attr,
    &dev_attr_sn_checker.attr,
    &dev_attr_check_key_type.attr,
    &dev_attr_check_sn_type.attr,
    &dev_attr_check_key.attr,
    &dev_attr_check_sn.attr,
#endif
    NULL, /* sysfs_create_files need last one be NULL */
};

/* create all node needed by driver */
static int batt_checker_node_create(struct platform_device *pdev)
{
    if(sysfs_create_files(&pdev->dev.kobj, batt_checker_attrs)) {
        hwlog_err("Can't create all expected nodes under %s in %s.\n",
                  pdev->dev.kobj.name, __func__);
        return BATTERY_DRIVER_FAIL;
    }
    return BATTERY_DRIVER_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------sys node end------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------*/
/*----------------------------------------driver init start--------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
static int battery_charge_cycles_cb(struct notifier_block *nb, unsigned long action, void *data)
{
    int charge_cycles;
    int ret;
    batt_checker_data* drv_data = container_of(nb, batt_checker_data, batt_cycles_listener);
    enum BATT_MATCH_TYPE temp = BATTERY_UNREMATCHABLE;

    if(action == HISI_EEPROM_CYC) {
        if(!data) {
            hwlog_err("Null data point found in %s.\n", __func__);
            return NOTIFY_BAD;
        }
        /* coul-data:charger_cycles is 100*real charge cycles */
        charge_cycles = (*(unsigned *)data)/100;
        hwlog_info("%s recv charge cycles:%u.\n", __func__, *(unsigned *)data);
        if(drv_data->batt_rematch_current == BATTERY_REMATCHABLE &&
           drv_data->batt_rematch_onboot == BATTERY_REMATCHABLE &&
           drv_data->free_cycles <= charge_cycles) {
            ret = drv_data->bco.set_batt_safe_info(drv_data,
                BATT_MATCH_ABILITY, &temp);
            if(ret) {
                hwlog_err("Set battery to unrematchable failed in %s.\n", __func__);
            } else {
                drv_data->batt_rematch_current = BATTERY_UNREMATCHABLE;
            }
        }
        return NOTIFY_OK;
    }
    return NOTIFY_DONE;
}

static int check_batt_ct_ops(batt_checker_data *drv_data)
{
    int ret = BATTERY_DRIVER_SUCCESS;
    if (!drv_data->ic_ops.get_batt_type) {
        ret = BATTERY_DRIVER_FAIL;
        hwlog_err("battery information interface: get_batt_type is not valid!");
    }
    if (!drv_data->ic_ops.certification) {
        ret = BATTERY_DRIVER_FAIL;
        hwlog_err("battery information interface: certification is not valid!");
    }
    if (!drv_data->ic_ops.get_batt_sn) {
        ret = BATTERY_DRIVER_FAIL;
        hwlog_err("battery information interface: get_batt_sn is not valid!");
    }
    if (!drv_data->ic_ops.prepare) {
        ret = BATTERY_DRIVER_FAIL;
        hwlog_err("battery information interface: ct_prepare is not valid!");
    }
    if (!drv_data->ic_ops.set_batt_safe_info) {
        ret = BATTERY_DRIVER_FAIL;
        hwlog_err("battery information interface: set_batt_safe_info is not valid!");
    }
    if (!drv_data->ic_ops.get_batt_safe_info) {
        ret = BATTERY_DRIVER_FAIL;
        hwlog_err("battery information interface: get_batt_safe_info is not valid!");
    }
    return ret;
}

static void init_wrapper_ops(batt_checker_data *drv_data)
{
	drv_data->bco.get_ic_type = get_ic_type_wrapper;
	drv_data->bco.get_batt_type = get_batt_type_wrapper;
	drv_data->bco.get_batt_sn = get_batt_sn_wrapper;
	drv_data->bco.prepare = prepare_wrapper;
	drv_data->bco.certification = certification_wrapper;
	drv_data->bco.set_batt_safe_info = set_batt_safe_info_wrapper;
	drv_data->bco.get_batt_safe_info = get_batt_safe_info_wrapper;
}

static int get_ic_ops(struct device_node* node, batt_checker_data* drv_data)
{
    int i,j,ret;
    int ic_unready = -1;
    ct_ops_reg_list *pos;
    ct_ops_reg_list *pos_temp;
    phandle* dev_ic_phandles;
    phandle ic_phandle;
    int num;

    num = of_property_count_elems_of_size(node, "matchable", sizeof(phandle));
    if(num <= 0) {
        hwlog_err("No ic matchable for battery checker(%s).\n", node->name);
        return ic_unready;
    }
    dev_ic_phandles = kzalloc(sizeof(phandle) * num, GFP_KERNEL);
    if(!dev_ic_phandles) {
        hwlog_err("%s alloc for dev_ic_phandles failed.\n", node->name);
        return ic_unready;
    }
    ret = of_property_read_u32_array(node, "matchable", dev_ic_phandles, num);
    if(ret) {
        hwlog_err("%s read dev_ic_phandles failed(%d).\n", node->name, ret);
        goto free_dev_ic_phandles;
    }

    /* find ic first */
    for( i = 0; i < 5; i++) {
        list_for_each_entry(pos, &batt_aut_ic_head, node) {
            if(pos->ic_dev) {
                ic_phandle = pos->ic_dev->dev.of_node->phandle;
            } else {
                continue;
            }
            for( j = 0; j < num; j++ ) {
                if(ic_phandle == dev_ic_phandles[j]) {
                    break;
                }
            }
            /* if this node is not for this batt_info */
            if(j == num) {
                continue;
            }
            if(pos->ct_ops_register != NULL) {
                apply_batt_type_mode(BAT_ID_SN);
                ic_unready = pos->ct_ops_register(&drv_data->ic_ops);
                release_batt_type_mode();
            }
            /* if node for this batt_info found */
            if(!ic_unready) {
                hwlog_info("%s find valid battery certification ic.\n", node->name);
                drv_data->ic = pos->ic_dev;
                drv_data->ic_type = drv_data->bco.get_ic_type(drv_data);
                break;
            }
        }
        if(!ic_unready) {
            list_del_init(&pos->node);
            break;
        }
    }
    /* release useless memory allocated for ic */
    list_for_each_entry_safe(pos, pos_temp, &batt_aut_ic_head, node) {
        if(pos->ic_dev) {
            ic_phandle = pos->ic_dev->dev.of_node->phandle;
        } else {
            continue;
        }
        for( j = 0; j < num; j++ ) {
            if(ic_phandle == dev_ic_phandles[j] && pos->ic_memory_release != NULL) {
                list_del_init(&pos->node);
                pos->ic_memory_release();
                break;
            }
        }
    }

    if(ic_unready) {
        drv_data->result.ic_status = IC_FAIL_UNKOWN;
        hwlog_info("%s battery can't find ic.\n", node->name);
        goto free_dev_ic_phandles;
    }

    /* check battery battery information interface */
    if(check_batt_ct_ops(drv_data)) {
        drv_data->result.ic_status = IC_FAIL_UNKOWN;
        hwlog_err("%s did not have enough operations!/n", node->name);
        goto free_dev_ic_phandles;
    }

    drv_data->result.ic_status = IC_PASS;
free_dev_ic_phandles:
    kfree(dev_ic_phandles);
    return ic_unready;
}

static int battery_cycles_limit_init(struct platform_device *pdev)
{
    batt_checker_data* drv_data;
    int ret;

    drv_data = platform_get_drvdata(pdev);
    if(!drv_data) {
        hwlog_err("%s can't find valid platform device drvdata.\n", pdev->dev.of_node->name);
        return BATTERY_DRIVER_FAIL;
    }
    /* battery charge cycles limitation */
    ret = of_property_read_u32(pdev->dev.of_node, "free-cycles", &drv_data->free_cycles);
    if(ret) {
        hwlog_info("%s read free cycles failed.\n", pdev->dev.of_node->name);
        return BATTERY_DRIVER_FAIL;
    };

    ret = drv_data->bco.get_batt_safe_info(drv_data, BATT_MATCH_ABILITY,
                                           &drv_data->batt_rematch_onboot);
    if(ret) {
        hwlog_err("get battery matchable status failed.\n");
        return BATTERY_DRIVER_FAIL;
    }
    /* if on factory mode, battery always rematchable */
#ifdef BATTBD_FORCE_MATCH
    drv_data->batt_rematch_onboot = BATTERY_REMATCHABLE;
#endif
    drv_data->batt_rematch_current = drv_data->batt_rematch_onboot;
    /* register notifier list for update battery to unrematchable */
    drv_data->batt_cycles_listener.notifier_call = battery_charge_cycles_cb;
    if(drv_data->batt_rematch_onboot == BATTERY_REMATCHABLE) {
        if(hisi_coul_register_blocking_notifier(&drv_data->batt_cycles_listener)) {
            hwlog_err("batt_cycles_listener added to coul list failed.\n");
            return BATTERY_DRIVER_FAIL;
        }
    }
    return BATTERY_DRIVER_SUCCESS;
}

void init_battery_check_result(battery_check_result* result)
{
    if(!result) {
        return;
    }
    /* initial battery check result */
    result->ic_status = IC_FAIL_UNKOWN;
    result->key_status = KEY_UNREADY;
    result->sn_status = SN_UNREADY;
#ifdef BATTBD_FORCE_MATCH
    result->check_mode = FACTORY_CHECK_MODE;
#else
    result->check_mode = COMMERCIAL_CHECK_MODE;
#endif
}

static batt_checker_data* battery_checker_drv_data_init(struct platform_device *pdev)
{
    batt_checker_data* drv_data;
    unsigned int sn_check_type;
    int ret;

    /*set up device's driver data now */
    drv_data = devm_kzalloc(&pdev->dev, sizeof(*drv_data), GFP_KERNEL);
    if(!drv_data) {
        hwlog_err("%s alloc for drv_data failed.\n", pdev->dev.of_node->name);
        return NULL;
    }
    platform_set_drvdata(pdev, drv_data);

    ret = of_property_read_u8(pdev->dev.of_node, "id-in-grp", &drv_data->id_in_grp);
    if(ret) {
        hwlog_err("%s read grp_no failed.\n", pdev->dev.of_node->name);
        return NULL;
    }
    ret = of_property_read_u8(pdev->dev.of_node, "chks-in-grp", &drv_data->chks_in_grp);
    if(ret) {
        hwlog_err("%s read grp_total failed.\n", pdev->dev.of_node->name);
        return NULL;
    }
    ret = of_property_read_u8(pdev->dev.of_node, "id-of-grp", &drv_data->id_of_grp);
    if(ret) {
        hwlog_err("%s read grp_id failed.\n", pdev->dev.of_node->name);
        return NULL;
    }
    ret = of_property_read_u32(pdev->dev.of_node, "sn-check-type", &sn_check_type);
    if(ret || sn_check_type >= ARRAY_SIZE(legal_sn_checkers)) {
        hwlog_err("%s read sn_check_type failed.\n", pdev->dev.of_node->name);
        return NULL;
    }
    drv_data->sn_checker = legal_sn_checkers[sn_check_type];

    INIT_WORK(&drv_data->check_key_work, check_key_func);
    INIT_WORK(&drv_data->check_sn_work, check_sn_func);
    init_completion(&drv_data->key_prepare_ready);
    init_completion(&drv_data->sn_prepare_ready);
    INIT_LIST_HEAD(&drv_data->entry.node);
    drv_data->entry.pdev = pdev;
    init_battery_check_result(&drv_data->result);

    return drv_data;
}

static int battery_aut_checker_probe(struct platform_device *pdev)
{
    int local_ic;
    batt_checker_data* drv_data;

    hwlog_info("%s is going to probing.\n", pdev->dev.of_node->name);
    if(!(drv_data = battery_checker_drv_data_init(pdev))) {
        hwlog_err("battery checker(%s) probe failed.\n", pdev->dev.of_node->name);
        return BATTERY_DRIVER_FAIL;
    }
    init_wrapper_ops(drv_data);

    /* get ic ops for battery type, new or old */
    local_ic = get_ic_ops(pdev->dev.of_node, drv_data);
    if(local_ic) {
        hwlog_err("%s get ic ops failed.\n", pdev->dev.of_node->name);
        return BATTERY_DRIVER_FAIL;
    }
    /* get battery cycles which distingush the old from the new */
    if(battery_cycles_limit_init(pdev)) {
        hwlog_err("%s determin battery new or bad failed.\n", pdev->dev.of_node->name);
        return BATTERY_DRIVER_FAIL;
    }

    /* battery node initialization */
    if(batt_checker_node_create(pdev)) {
        hwlog_err("%s create nodes failed.\n", pdev->dev.of_node->name);
        return BATTERY_DRIVER_FAIL;
    }
    add_to_batt_checkers_lists(&drv_data->entry);

    hwlog_info("%s probed successfully.\n", pdev->dev.of_node->name);
    return BATTERY_DRIVER_SUCCESS;
}

static int  battery_aut_checker_remove(struct platform_device *pdev)
{
    return BATTERY_DRIVER_SUCCESS;
}

static const struct of_device_id battery_checkers_match_table[] = {
    {
        .compatible = "huawei,battery-authentication",
    },
    { /*end*/},
};

const struct of_device_id* get_battery_checkers_match_table(void)
{
    return battery_checkers_match_table;
}

static struct platform_driver battery_checkers_driver = {
    .probe      = battery_aut_checker_probe,
    .remove     = battery_aut_checker_remove,
    .driver     = {
        .name = "Battery_Checker",
        .owner = THIS_MODULE,
        .of_match_table = battery_checkers_match_table,
    },
};

int __init battery_checkers_init(void)
{
    hwlog_info("battery checks driver init...\n");
    return platform_driver_register(&battery_checkers_driver);
}

void __exit battery_checkers_exit(void)
{
    hwlog_info("battery checks driver exit...\n");
    platform_driver_unregister(&battery_checkers_driver);
}

subsys_initcall_sync(battery_checkers_init);
module_exit(battery_checkers_exit);
/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------driver init end---------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

MODULE_LICENSE("GPL");
