#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/mfd/hisi_pmic.h>


struct hisi_usb_vbus {
	int vbus_connect_irq;
	int vbus_disconnect_irq;
	int vbus_status;
	bool initialized;
	void *tcpc_data;
};
struct hisi_usb_vbus _vbus = {
	.vbus_connect_irq = 0,
	.vbus_disconnect_irq = 0,
	.vbus_status = 0,
	.initialized = false,
};
struct hisi_usb_vbus *hisi_vbus = &_vbus;

extern void hisi_tcpc_vbus_irq_handler(void *data, int vbus_status);

static irqreturn_t hisi_usb_vbus_intr(int irq, void *data)
{
	struct hisi_usb_vbus *vbus = (struct hisi_usb_vbus *)data;

	if (irq == vbus->vbus_connect_irq) {
		pr_err("%s: vbus_connect_irq\n", __func__);
		vbus->vbus_status = 1;
	} else {
		pr_err("%s: vbus_disconnect_irq\n", __func__);
		vbus->vbus_status = 0;
	}

	hisi_tcpc_vbus_irq_handler(vbus->tcpc_data, vbus->vbus_status);

	return IRQ_HANDLED;
}

int hisi_usb_vbus_status(void)
{
	return hisi_vbus->vbus_status;
}

void hisi_usb_vbus_init(void *tcpc_data)
{
	struct hisi_usb_vbus *vbus = hisi_vbus;
	int ret;

	pr_info("%s: +\n", __func__);
	if (vbus->initialized) {
		pr_info("%s: hisi usb vbus already initialized!\n", __func__);
		WARN_ON(true);
		return;
	}

	if (!tcpc_data) {
		pr_err("%s: tcpc_data NULL\n", __func__);
		WARN_ON(true);
		return;
	}

	vbus->tcpc_data = tcpc_data;

	vbus->vbus_status = hisi_pmic_get_vbus_status();
	vbus->vbus_connect_irq = hisi_get_pmic_irq_byname(VBUS_CONNECT);
	vbus->vbus_disconnect_irq = hisi_get_pmic_irq_byname(VBUS_DISCONNECT);
	pr_info("%s: vbus_status %d\n", __func__, vbus->vbus_status);

	ret = request_irq(vbus->vbus_connect_irq, hisi_usb_vbus_intr,
				IRQF_SHARED | IRQF_NO_SUSPEND, "hisi_vbus_connect", vbus);
	if (ret) {
		pr_err("%s: request vbus_connect irq error ret %d\n", __func__, ret);
		return;
	}

	ret = request_irq(vbus->vbus_disconnect_irq, hisi_usb_vbus_intr,
				IRQF_SHARED | IRQF_NO_SUSPEND, "hisi_vbus_disconnect", vbus);
	if (ret) {
		pr_err("%s: request vbus_connect irq error ret %d\n", __func__, ret);
		free_irq(vbus->vbus_connect_irq, vbus);
	}
	pr_info("%s: -\n", __func__);
}

void hisi_usb_vbus_exit(void)
{
	struct hisi_usb_vbus *vbus = hisi_vbus;
	free_irq(vbus->vbus_connect_irq, vbus);
	free_irq(vbus->vbus_disconnect_irq, vbus);
	vbus->initialized = false;
}
