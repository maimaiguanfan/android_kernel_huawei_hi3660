/*noc err bus*/
enum noc_error_bus {
	 NOC_ERRBUS_SYS_CONFIG,
	 NOC_ERRBUS_VCODEC,
	 NOC_ERRBUS_VIVO,
	 NOC_ERRBUS_NPU,
	 NOC_ERRBUS_FCM,
	 NOC_ERRBUS_BOTTOM,
};

/*noc err type*/
enum noc_error_reason {
	NOC_ERROR_REASON_SLAVE_RETURN_ERR_RESP,
	NOC_ERROR_REASON_MST_ACCESS_REV_MEM,
	NOC_ERROR_REASON_MST_SEND_ILLIGEL_BURST,
	NOC_ERROR_REASON_MST_ACCESS_POWDOWN,
	NOC_ERROR_REASON_MST_PERMISSION_ERR,
	NOC_ERROR_REASON_MST_REV_HIDE_ERR_RESP,
	NOC_ERROR_REASON_MST_ACCESS_SLAVE_TIMEOUT,
	NOC_ERROR_REASON_NONE,
	NOC_ERROR_REASON_BOTTOM,
};
struct noc_err_para_s {
	u32 masterid;
	u32 targetflow;
	enum noc_error_bus bus;
	enum noc_error_reason reason;
};

struct noc_mid_modid_trans_s {
	struct list_head s_list;
	struct noc_err_para_s err_info_para;
	u32 modid;
	void* reserve_p;
};
void noc_modid_register(struct noc_err_para_s noc_err_info,u32 modid);

