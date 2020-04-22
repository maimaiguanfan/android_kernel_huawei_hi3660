

#ifndef _MPTCP_HEARTBEAT_H
#define _MPTCP_HEARTBEAT_H

/* include headers */
#include <linux/in.h>
#include <linux/net.h>

/* macro */
#define HEADER_BUFFER_SIZE                       (52)
#define LINK_NUM                                 (2)        // the total links availiable, currently 2, wifi and cellular
#define IF_NAME_LIST                             {"wlan0", "rmnet0"}     // if LINK_NUM is changed, this field must be changed accordingly

/* struct definition */
typedef struct __heartbeat_header
{
	// mutp_header heartbeart;
	char        cbuffer[HEADER_BUFFER_SIZE];
} heartbeat_header;

typedef enum __detection_result
{
	LINK_OK,
	LINK_BROKEN,
} detection_result;

typedef enum __detection_type    // argment of function "heartbeat_trigger" may be a combination of the below, eg. 3 means WIFI & LTE
{
	WIFI_DETECTION = (1<<0),
	LTE_DETECTION = (1<<1),
} detection_type;

/* callback definition */
typedef void (*HEARTBEAT_DETECTION_CALLBACK)(int type, int result);

/* function declaration */
void heartbeat_init(struct sockaddr_in* link_addrs, int retry_count);
void heartbeat_deinit(void);
void heartbeat_trigger(int type, HEARTBEAT_DETECTION_CALLBACK cb);

#endif /* _MPTCP_HEARTBEAT_H */
