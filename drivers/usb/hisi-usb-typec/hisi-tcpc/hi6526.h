#ifndef _HI6526_H_
#define _HI6526_H_

#include "include/std_tcpci_v10.h"

#define HISI_TCPC_VID 0x12d1
#define HISI_TCPC_PID 0x6526

/*
 * TCPC
 */
#define TCPCI_REG_VENDOR_ID			0x00
#define TCPCI_REG_PRODUCT_ID			0x02
#define TCPCI_REG_DEVICE_ID			0x04
#define TCPCI_REG_USBTYPEC_REV			0x06
#define TCPCI_REG_USBPD_REV_VER			0x08
#define TCPCI_REG_PD_INTERFACE_REV		0x0A
#define TCPCI_REG_ALERT				0x10
#define TCPCI_REG_ALERT_MASK			0x12

#define TCPCI_REG_POWER_STATUS_MASK		0x14
#define TCPCI_REG_FAULT_STATUS_MASK		0x15
#define TCPCI_REG_CONFIG_STANDARD_OUTPUT	0x18
#define TCPCI_REG_TCPC_CONTROL			0x19
#define TCPCI_REG_ROLE_CONTROL			0x1A
#define TCPCI_REG_FAULT_CONTROL			0x1B
#define TCPCI_REG_POWER_CONTROL			0x1C
#define TCPCI_REG_CC_STATUS			0x1D
#define TCPCI_REG_POWER_STATUS			0x1E
#define TCPCI_REG_FAULT_STATUS			0x1F
#define TCPCI_REG_COMMAND			0x23

#define TCPCI_REG_DEVICE_CAPABILITIES_1		0x24
#define TCPCI_REG_DEVICE_CAPABILITIES_2		0x26

#define TCPCI_REG_STANDARD_INPUT_CAPABILITIES	0x28
#define TCPCI_REG_STANDARD_OUTPUT_CAPABILITIES	0x29

#define  TCPCI_REG_MESSAGE_HEADER_INFO 		0x2E
#define  TCPCI_REG_RECEIVE_DETECT		0x2F
#define  TCPCI_REG_RECEIVE_BYTE_COUNT		0x30

#define  TCPCI_REG_RX_BUF_FRAME_TYPE		0x31
#define  TCPCI_REG_RX_BUF_HEADER_BYTE_0		0x32
#define  TCPCI_REG_RX_BUF_HEADER_BYTE_1		0x33

/*
 * vendor defined
 */
#define REG_PD_VDM_CFG_0			0x7A
#define REG_PD_VDM_ENABLE			0x7B
#define REG_PD_VDM_CFG_1			0x7C
# define PD_SNK_DISC_BY_CC	(1 << 4)
# define PD_RX_PHY_SOFT_RESET	(1 << 3)
# define PD_TX_PHY_SOFT_RESET	(1 << 2)
# define PD_FSM_RESET		(1 << 1)
# define TC_FSM_RESET		1
# define PD_TC_ALL_RESET	(PD_RX_PHY_SOFT_RESET | PD_TX_PHY_SOFT_RESET \
				| PD_FSM_RESET | TC_FSM_RESET)
#define REG_PD_DBG_RDATA_CFG			0x7D
# define PD_DBG_RDATA_EN			(1 << 7)
# define PD_DBG_MODULE_SEL(x)			(((x) & 0x7) << 4)
# define PD_DBG_RDATA_SEL(x)			((x) & 0xf)
#define REG_PD_DBG_RDATA			0x7E
#define REG_VDM_PAGE_SELECT 			0x7F

/*
 * PAGE0
 */
#define REG_FCP_ISR1				(0x80 + 0x19)	/* FCP �ж�1�Ĵ��� */
#define REG_FCP_ISR2				(0x80 + 0x1A)	/* FCP �ж�2�Ĵ��� */
#define REG_FCP_IMR1				(0x80 + 0x1B)	/* FCP �ж�����1�Ĵ��� */
#define REG_FCP_IMR2				(0x80 + 0x1C)	/* FCP �ж�����2�Ĵ��� */
#define REG_FCP_IRQ5				(0x80 + 0x1D)	/* FCP�ж�5�Ĵ��� */
#define REG_FCP_IRQ5_MASK			(0x80 + 0x1E)	/* FCP�ж�����5�Ĵ��� */

#define REG_PD_CDR_CFG_0			(0x80 + 0x58)	/* PDģ��BMCʱ�ӻָ���·���üĴ��� */
#define REG_PD_CDR_CFG_1			(0x80 + 0x59)	/* PDģ��BMCʱ�ӻָ���·���üĴ��� */
#define REG_PD_DBG_CFG_0			(0x80 + 0x5A)	/* PDģ��Debug�����üĴ��� */
#define REG_PD_DBG_CFG_1			(0x80 + 0x5B)	/* PDģ��Debug�����üĴ��� */
#define REG_PD_DBG_RO_0				(0x80 + 0x5C)	/* PDģ��Debug�ûض��Ĵ��� */
#define REG_PD_DBG_RO_1				(0x80 + 0x5D)	/* PDģ��Debug�ûض��Ĵ��� */
#define REG_PD_DBG_RO_2				(0x80 + 0x5E)	/* PDģ��Debug�ûض��Ĵ��� */
#define REG_PD_DBG_RO_3				(0x80 + 0x5F)	/* PDģ��Debug�ûض��Ĵ��� */

#define REG_IRQ_FAKE_SEL			(0x80 + 0x60)	/* ʵ���жϺ�α�ж�ѡ���ź� */
#define REG_IRQ_FAKE				(0x80 + 0x61)	/* α�ж�Դ */

#define REG_IRQ_FLAG				(0x80 + 0x62)	/* Read Only */
#define REG_IRQ_FLAG_0				(0x80 + 0x63)
#define REG_IRQ_FLAG_1				(0x80 + 0x64)
#define REG_IRQ_FLAG_2				(0x80 + 0x65)
#define REG_IRQ_FLAG_3				(0x80 + 0x66)
#define REG_IRQ_FLAG_4				(0x80 + 0x67)
#define REG_IRQ_FLAG_5				(0x80 + 0x68)
#define REG_IRQ_FLAG_6				(0x80 + 0x69)
#define REG_IRQ_FLAG_7				(0x80 + 0x6A)
#define REG_WDT_SOFT_RST			(0x80 + 0x6B)	/* ���Ź���λ���ƼĴ��� */
#define REG_WDT_CTRL				(0x80 + 0x6C)	/* ι��ʱ����ƼĴ��� */

#define REG_OTG_CFG				(0x80 + 0x70)
# define BIT_SC_OTG_EN				(1 << 7)
# define BIT_SC_OTG_DMD_RAMP			(7 << 4)
# define BIT_SC_OTG_DMD_OFS			(0xf << 0)
#define REG_PULSE_CHG_CFG0			(0x80 + 0x71)
# define BIT_SC_CHG_EN				(1 << 4)
#define REG_GLB_SOFT_RST_CTRL			(0x80 + 0x78)


/*
 * PAGE1
 */
#define REG_FCP_IRQ3				(0x180 + 0x00)	/* FCP�ж�3�Ĵ��� */
#define REG_FCP_IRQ4				(0x180 + 0x01)	/* FCP�ж�4�Ĵ��� */
#define REG_FCP_IRQ3_MASK			(0x180 + 0x02)	/* FCP�ж�����3�Ĵ��� */
#define REG_FCP_IRQ4_MASK			(0x180 + 0x03)	/* FCP�ж�����4�Ĵ��� */



#define REG_IRQ_MASK		(0x180 + 0x48)
# define BIT_IRQ_MASK_GLB	(1 << 7) /* bit[7]��ȫ�����μĴ�����ʹ��Ĭ��ֵ�����Բ�Ʒ���ţ�1: ���������ж��ϱ� 0�������������ж��ϱ� */
# define BIT_IRQ_MASK_SRC	(1 << 6) /* bit[6]���ж�����Դͷѡ��ʹ��Ĭ��ֵ�����Բ�Ʒ���ţ�1���ж�����λ�����ж�Դͷ 0���ж�����λ�������ж�Դͷ����������� */
# define BIT_IRQ_MASK_BUCK	(1 << 4) /* bit[4]��BUCK���ж����Σ�1�������ϱ��жϣ�0�������ϱ��жϣ�*/
# define BIT_IRQ_MASK_LVC_SC	(1 << 3) /* bit[3]��LVC_SC���ж����Σ� 1�������ϱ��жϣ�0�������ϱ��жϣ�*/
# define BIT_IRQ_MASK_PD	(1 << 2) /* bit[2]��PD���ж����� 1: �����ж��ϱ� 0���������ж��ϱ�*/
# define BIT_IRQ_MASK_OTHERS	(1 << 1) /* bit[1]��Others���ж������ź� 1: �����ϱ��ж� 0���������ϱ��ж�*/
# define BIT_IRQ_MASK_FCP	(1 << 0) /* bit[0]��FCP���ж������ź� 1: �����ϱ��ж� 0���������ϱ��ж�*/
#define REG_IRQ_MASK_0				(0x180 + 0x49)
#define REG_IRQ_MASK_1				(0x180 + 0x4A)
#define REG_IRQ_MASK_2				(0x180 + 0x4B)
#define REG_IRQ_MASK_3				(0x180 + 0x4C)
#define REG_IRQ_MASK_4				(0x180 + 0x4D)
#define REG_IRQ_MASK_5				(0x180 + 0x4E)
#define REG_IRQ_MASK_6				(0x180 + 0x4F)
#define REG_IRQ_MASK_7				(0x180 + 0x50)
#define REG_IRQ_STATUS_0			(0x180 + 0x51)
#define REG_IRQ_STATUS_1			(0x180 + 0x52)
#define REG_IRQ_STATUS_2			(0x180 + 0x53)
#define REG_IRQ_STATUS_3			(0x180 + 0x54)
#define REG_IRQ_STATUS_4			(0x180 + 0x55)
#define REG_IRQ_STATUS_5			(0x180 + 0x56)
#define REG_IRQ_STATUS_6			(0x180 + 0x57)
#define REG_IRQ_STATUS_7			(0x180 + 0x58)
#define REG_IRQ_STATUS_8			(0x180 + 0x59)


/*
 * PAGE2
 */
#define REG_TCPC_CFG_REG_1			(0x280 + 0x0E)	/* TCPC_���üĴ���_1 */
#define REG_TCPC_CFG_REG_2			(0x280 + 0x0F)	/* TCPC_���üĴ���_2 */
#define REG_TCPC_CFG_REG_3			(0x280 + 0x10)	/* TCPC_���üĴ���_3 */
#define REG_TCPC_RO_REG_5			(0x280 + 0x11)	/* TCPC_ֻ���Ĵ���_5 */
#define SCHG_LOGIC_CFG_REG_2			(0x280 + 0x14)

#define REG_BUCK_CFG_REG_0		(0x280 + 0x50)
#define REG_BUCK_CFG_REG_1		(0x280 + 0x51)
#define REG_BUCK_CFG_REG_2		(0x280 + 0x52)
#define REG_BUCK_CFG_REG_3		(0x280 + 0x53)		/* BUCK_���üĴ���_3 */
#define REG_BUCK_CFG_REG_4		(0x280 + 0x54)            /* BUCK_���üĴ���_4 */
#define REG_BUCK_CFG_REG_5		(0x280 + 0x55)            /* BUCK_���üĴ���_5 */
#define REG_BUCK_CFG_REG_6		(0x280 + 0x56)            /* BUCK_���üĴ���_6 */
#define REG_BUCK_CFG_REG_7		(0x280 + 0x57)            /* BUCK_���üĴ���_7 */
#define REG_BUCK_CFG_REG_8		(0x280 + 0x58)            /* BUCK_���üĴ���_8 */
#define REG_BUCK_CFG_REG_9		(0x280 + 0x59)            /* BUCK_���üĴ���_9 */
#define REG_BUCK_CFG_REG_10		(0x280 + 0x5A)            /* BUCK_���üĴ���_10 */
#define REG_BUCK_CFG_REG_11		(0x280 + 0x5B)            /* BUCK_���üĴ���_11 */
#define REG_BUCK_CFG_REG_12		(0x280 + 0x5C)            /* BUCK_���üĴ���_12 */
#define REG_BUCK_CFG_REG_13		(0x280 + 0x5D)            /* BUCK_���üĴ���_13 */
#define REG_BUCK_CFG_REG_14		(0x280 + 0x5E)            /* BUCK_���üĴ���_14 */
#define REG_BUCK_CFG_REG_15		(0x280 + 0x5F)            /* BUCK_���üĴ���_15 */
#define REG_BUCK_CFG_REG_16		(0x280 + 0x60)            /* BUCK_���üĴ���_16 */
#define REG_BUCK_CFG_REG_17		(0x280 + 0x61)            /* BUCK_���üĴ���_17 */
#define REG_BUCK_CFG_REG_18		(0x280 + 0x62)            /* BUCK_���üĴ���_18 */
#define REG_BUCK_CFG_REG_19		(0x280 + 0x63)            /* BUCK_���üĴ���_19 */
#define REG_BUCK_CFG_REG_20		(0x280 + 0x64)            /* BUCK_���üĴ���_20 */
#define REG_BUCK_CFG_REG_21		(0x280 + 0x65)            /* BUCK_���üĴ���_21 */
#define REG_BUCK_CFG_REG_22		(0x280 + 0x66)            /* BUCK_���üĴ���_22 */
#define REG_BUCK_CFG_REG_23		(0x280 + 0x67)            /* BUCK_���üĴ���_23 */
#define REG_BUCK_CFG_REG_24		(0x280 + 0x68)            /* BUCK_���üĴ���_24 */
#define REG_BUCK_CFG_REG_25		(0x280 + 0x69)            /* BUCK_���üĴ���_25 */
#define REG_BUCK_RO_REG_26		(0x280 + 0x6A)            /* BUCK_ֻ���Ĵ���_26 */
#define REG_BUCK_RO_REG_27		(0x280 + 0x6B)            /* BUCK_ֻ���Ĵ���_27 */
#define REG_BUCK_RO_REG_28		(0x280 + 0x6C)            /* BUCK_ֻ���Ĵ���_28 */

#define REG_OTG_CFG_REG_0			(0x280 + 0x6D)	/* OTG_���üĴ���_0 */
#define REG_OTG_CFG_REG_1			(0x280 + 0x6E)    /* OTG_���üĴ���_1 */
#define REG_OTG_CFG_REG_2			(0x280 + 0x6F)    /* OTG_���üĴ���_2 */
#define REG_OTG_CFG_REG_3			(0x280 + 0x70)    /* OTG_���üĴ���_3 */
#define REG_OTG_CFG_REG_4			(0x280 + 0x71)    /* OTG_���üĴ���_4 */
#define REG_OTG_CFG_REG_5			(0x280 + 0x72)    /* OTG_���üĴ���_5 */
#define REG_OTG_CFG_REG_6			(0x280 + 0x73)    /* OTG_���üĴ���_6 */
# define BIT_DA_OTG_PFM_V_EN			(1 << 4)
# define BIT_DA_OTG_SWITCH			(1 << 3)
# define BIT_DA_OTG_UVP_EN			(1 << 2)
# define BIT_DA_WL_OTG_MODE			(1 << 1)
# define BIT_OTG_CLP_L_SET			(1 << 0)
#define REG_OTG_RO_REG_7			(0x280 + 0x74)    /* OTG_ֻ���Ĵ���_7 */
#define REG_OTG_RO_REG_8			(0x280 + 0x75)    /* OTG_ֻ���Ĵ���_8 */
#define REG_OTG_RO_REG_9			(0x280 + 0x76)    /* OTG_ֻ���Ĵ���_9 */

int hisi_tcpc_block_read(u32 reg, int len, void *dst);
int hisi_tcpc_block_write(u32 reg, int len, void *src);
s32 hisi_tcpc_i2c_read8(struct i2c_client *client, u32 reg);
int hisi_tcpc_i2c_write8(struct i2c_client *client, u32 reg, u8 value);
s32 hisi_tcpc_i2c_read16(struct i2c_client *client, u32 reg);
int hisi_tcpc_i2c_write16(struct i2c_client *client, u32 reg, u16 value);

extern struct i2c_client *hi6526_i2c_client;
extern int hi6526_irq_gpio;

#endif
