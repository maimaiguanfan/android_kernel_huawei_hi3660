

#ifndef _RIL_SIM_NETLINK_H
#define _RIL_SIM_NETLINK_H

enum {
	NETLINK_KNL_SIM_NULL_MSG                = 0,
	NETLINK_KNL_SIM_UPDATE_IND_MSG          = 1,
	NETLINK_KNL_SIM_GET_PIN_INFO_REQ        = 2,
	NETLINK_KNL_SIM_GET_PIN_INFO_CNF        = 3,
	NETLINK_KNL_SIM_QUERY_VARIFIED_REQ      = 4,
	NETLINK_KNL_SIM_QUERY_VARIFIED_CNF      = 5,
	NETLINK_KNL_SIM_CLEAR_ALL_VARIFIED_FLG  = 6,
	NETLINK_KNL_SIM_BUTT
};

#define MAX_SIM_CNT                     3
#define KNL_CIPHER_TEXT_LEN             32
#define KNL_IV_LEN                      32
#define KNL_HMAC_LEN                    64

typedef struct tag_knl_sim_pin_info {
	int  sim_slot_id;
	int  is_verified;
	int  is_valid_flg;
	char cipher_text[KNL_CIPHER_TEXT_LEN + 1];  /* save cipher pin */
	char init_vector[KNL_IV_LEN + 1];
	char hmac[KNL_HMAC_LEN + 1];
	char reserved;
} knl_sim_pin_info;

typedef struct tag_knl_ril_sim_card_info {
	int  sim_slot_id;
} knl_ril_sim_card_info;

#endif /*_RIL_SIM_NETLINK_H*/
