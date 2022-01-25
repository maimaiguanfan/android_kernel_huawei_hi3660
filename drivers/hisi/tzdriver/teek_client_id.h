/**
 * @file tee_client_id.h
 *
 * Copyright(C), 2008-2013, Huawei Tech. Co., Ltd. ALL RIGHTS RESERVED. \n
 *
 * ��������ȫ�����ṩ�İ�ȫ�����������ͷ�ļ�\n
 */
#ifndef _TEE_CLIENT_ID_H_
#define _TEE_CLIENT_ID_H_

/**
 *  * @ingroup  TEE_ID
 *   *
 *    * secboot��
 *     */
#define TEE_SERVICE_SECBOOT \
{ \
	0x08080808, \
	0x0808, \
	0x0808, \
	{ \
		0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08 \
	} \
}

/*e7ed1f64-4687-41da-96dc-cbe4f27c838f*/
#define TEE_SERVICE_ANTIROOT \
{ \
	0xE7ED1F64, \
	0x4687, \
	0x41DA, \
	{ \
		0x96, 0xDC, 0xCB, 0xE4, 0xF2, 0x7C, 0x83, 0x8F \
	} \
}

/**
 * @ingroup  TEE_COMMON_DATA
 *
 * ��ȫ����secboot֧�ֵ�����ID
 */
enum SVC_SECBOOT_CMD_ID {
	SECBOOT_CMD_ID_INVALID = 0x0,	/**< Secboot Task ��ЧID*/
	SECBOOT_CMD_ID_COPY_VRL,	/**< Secboot Task ����VRL*/
	SECBOOT_CMD_ID_COPY_DATA,	/**< Secboot Task ��������*/
	SECBOOT_CMD_ID_VERIFY_DATA,	/**< Secboot Task ��֤*/
	SECBOOT_CMD_ID_RESET_IMAGE,	/**< Secboot Task ��λSoC����*/
	SECBOOT_CMD_ID_COPY_VRL_TYPE,  /**< Secboot Task ����VRL��������SoC Type*/
	SECBOOT_CMD_ID_COPY_DATA_TYPE,	/**< Secboot Task ��������,������SoC Type*/
	SECBOOT_CMD_ID_VERIFY_DATA_TYPE, /**< Secboot Task У�飬������SoC Type��У��ɹ��⸴λSoC*/
	SECBOOT_CMD_ID_VERIFY_DATA_TYPE_LOCAL, /**< Secboot Taskԭ��У�飬������SoC Type,У��ɹ��⸴λSoC*/
	SECBOOT_CMD_ID_COPY_IMG_TYPE,          /*<Secboot Task Copy img from secure buffer to run addr>*/
	SECBOOT_CMD_ID_BSP_MODEM_CALL,           /**< Secboot Task ִ�ж�Ӧ����*/
	SECBOOT_CMD_ID_BSP_MODULE_VERIFY,        /**< Secboot Task modem moduleУ�麯��*/
	SECBOOT_CMD_ID_BSP_ICC_OPEN_THREAD,      /**< Secboot Task icc open����*/
	SECBOOT_CMD_ID_BSP_RFILE_RW_THREAD,      /**< Secboot Task rfile thread����*/
};

/**
 * @ingroup TEE_COMMON_DATA
 *
 * ��ȫ����secboot֧�ֵľ�������
 */
 /* ���ö�ٵ��޸���Ҫͬ���޸�һ�¼����ط�
mbb tzdriver
vendor/hisi/system/kernel/drivers/hisi/tzdriver/teek_client_id.h
mbb secos
vendor/hisi/system/secure_os/trustedcore/include/TEE/tee_common.h
vendor/hisi/system/secure_os/trustedcore/platform/balong/include/bsp_param_cfg.h
mbb��phone:ccore modem
vendor/hisi/system/kernel/drivers/hisi/modem/drv/common/include/param_cfg_to_sec.h
phone tzdriver
vendor/hisi/ap/kernel/drivers/tzdriver/teek_client_id.h
phone secos
vendor/thirdparty/secure_os/trustedcore/include/TEE/tee_common.h
vendor/thirdparty/secure_os/trustedcore/platform/kirin/secureboot/secboot.h
*/

#ifdef CONFIG_HISI_SECBOOT_IMG
enum SVC_SECBOOT_IMG_TYPE {
    MODEM,
    DSP,
    XDSP,
    TAS,
    WAS,
    CAS,
    MODEM_DTB,
    NVM,
    NVM_S,
    MBN_R,
    MBN_A,
    MODEM_COLD_PATCH,
    DSP_COLD_PATCH,
    MODEM_CERT,
    MAX_SOC_MODEM,
    HIFI,
    ISP,
    IVP,
    SOC_MAX
};
#else
enum SVC_SECBOOT_IMG_TYPE {
    MODEM,
    HIFI,
    DSP,
    XDSP,
    TAS,
    WAS,
    CAS,
    MODEM_DTB,
    ISP,
/*
miami c30����Ҫ֧���䲹�����ԣ��÷�֧��modem������miami_c30 Modem���빲��֧��
�����䲹��ö����Ϊ���ڸ÷�֧����ͨ��miami�汾����ȫOS��û�����Ӷ�Ӧ��ö�٣��÷�֧�ϱ����miami�汾��֧���䲹�����ԣ�
��֧��Ӧ��ϵ���ӣ�http://3ms.huawei.com/hi/group/8729/wiki_5190309.html
*/
#ifdef CONFIG_COLD_PATCH
    MODEM_COLD_PATCH,
    DSP_COLD_PATCH,
#endif
    SOC_MAX
};
#endif

#endif

