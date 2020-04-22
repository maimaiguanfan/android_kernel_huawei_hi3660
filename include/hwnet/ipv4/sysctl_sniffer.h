#ifndef __HW_WIFI_SNIFFER_H
#define __HW_WIFI_SNIFFER_H

/**************************************************************/
#define PCAP_FILE_NUM				5
#define PCAP_FILE_NUM_MAX			5
#define PCAP_FILE_NUM_MIN			1
#define PCAP_FILE_LEN				(4*1024*1024)
#define PCAP_FILE_LEN_MIN			(500*1024)
#define PCAP_FILE_LEN_MAX			(30*1024*1024)

#define PCAP_PKT_HDR_MAGIC			0xa1b2c3d4
#define PCAP_PKT_HDR_VER_MAJ		0x2
#define PCAP_PKT_HDR_VER_MIN		0x4
#define PCAP_PKT_HDR_THISZONE		0x0
#define PCAP_PKT_HDR_SIGFIGS		0x0
#define PCAP_PKT_HDR_SNAPLEN		0xffff
#define PCAP_PKT_HDR_LINKTYPE		0x69

#define IEEE80211_MAC_HDR_LEN		26
#define IEEE80211_DATA_PAYLOAD_LEN	48
#define IEEE80211_FRAME_TYPE_DATA	0x02
#define IEEE80211_FRAME_SUBTYPE_QOS	0x08

#define CMD_BUFF_MAGIC_NUM			0xaabbccdd
#define CMD_BUF_MAX_PARAM		    12

#define WIFI_LOG_PATH 				"/data/vendor/log/wifi/"
#define SNIFFER_START                   (static_key_enabled(&wifi_sniffer_enable))
#define SNIFFER_MAX_FILE_LEN            (g_configs.max_file_len)
#define SNIFFER_MAX_FILE_NUM            (g_configs.max_file_num)
#define SNIFFER_MAX_PACKET_LEN          (g_configs.data_packet_max_len)
#define SNIFFER_ENOUGH_TO_SAVE(packet_len)  ((g_configs.curpos[g_configs.cur_file] + packet_len) <= g_configs.max_file_len)
#define SNIFFER_FILE_BUFFER(i)   (g_configs.buffer[i])
#define SNIFFER_FILE_POS(i)      (g_configs.curpos[i])
#define SNIFFER_CUR_FILE_BUFFER   SNIFFER_FILE_BUFFER(g_configs.cur_file)
#define SNIFFER_CUR_FILE_POS      SNIFFER_FILE_POS(g_configs.cur_file)
#define SNIFFER_IS_CUR_FILE_EMPTY (SNIFFER_CUR_FILE_POS == 0)
#define SNIFFER_IS_CUR_FULL       (g_configs.file_full[g_configs.cur_file])
#define SNIFFER_CUR_FILE_BUFF_POS (g_configs.buffer[g_configs.cur_file] + g_configs.curpos[g_configs.cur_file])

#define FILE_READ  			0
#define FILE_WRITE 			1

#define DIRECTION_RX  		0
#define DIRECTION_TX  		1

#define TRIM_NEEDED	  		1
#define TRIM_NOT_NEEDED		0

/******************************************************************/
/* frame control structure */
typedef struct
{
    unsigned short  bit_protocol_version    : 2,    	/* protocol version */
                	bit_type                : 2,        /* frame type */
                	bit_sub_type            : 4,        /* subtype */
                	bit_to_ds               : 1,        /* to DS */
                	bit_from_ds             : 1,        /* from DS */
                	bit_more_frag           : 1,        /* more fragment */
                	bit_retry               : 1,        /* retransmit frame */
                	bit_power_mgmt          : 1,        /* power management */
                	bit_more_data           : 1,        /* more data flag */
                	bit_protected_frame     : 1,        /* encrypt flag */
                	bit_order               : 1;        /* order bit */
} mac_header_control_stru;

typedef struct {
    int cur_file;
    int data_packet_max_len;
    int max_file_num;
    int max_file_len;

    int file_full[PCAP_FILE_NUM];
    char *buffer[PCAP_FILE_NUM];
    int  curpos[PCAP_FILE_NUM];
} proc_file_config_struct;

typedef enum
{
	CMD_BUFF_CMDID_TOP					=  0xaa000000,
	CMD_BUFF_CMDID_SNIFFER_ENABLE		=  0xaa010001,	/* 1  parameter: 0 stop, 1 start */
	CMD_BUFF_CMDID_SNIFFER_MODE			=  0xaa020001,	/* 1  parameter: 0 Hybrid mode (my own), 1 Monitor mode (all) */
	CMD_BUFF_CMDID_CHANNDEL				=  0xaa030001,	/* 1  parameter: channel number */
	CMD_BUFF_CMDID_BANDWIDTH			=  0xaa040001,	/* 1  parameter: 20/40/80/160 */
	CMD_BUFF_CMDID_PROTOTYPE			=  0xaa050001,	/* 1  parameter: a/b/g/n/ac/ax(1~6 respectively) */
	CMD_BUFF_CMDID_CURFILELEN			=  0xaa060001,	/* 1  parameter: file number */
	CMD_BUFF_CMDID_FILESPEC				=  0xaa070001,	/* 2 parameters: file numbers(1~5); file length */
	CMD_BUFF_CMDID_DATAPAYLOAD			=  0xaa080001,	/* 1  parameter: 802.11 payload length after trimming */
	CMD_BUFF_CMDID_BUTTOM				=  0xaaff0000,
} cmd_buff_cmdid_enum;
typedef int	cmd_buff_cmdid_enum_int;

typedef enum
{
	CMD_BUFF_RDWR_READ					=  0x00000001,
	CMD_BUFF_RDWR_WRITE					=  0x00000002,
} cmd_buff_read_write_enum;

typedef struct {
	int iMagicNum;	/* fixed : CMD_BUFF_MAGIC_NUM */
	int iCmdID;		/* cmd_buff_cmdid_enum (0xaa000000 ~ 0xaaff0000) */
	int iRdWr;		/* 1 Read, 2 Write */
	int iParaNum;	/* parameter numbers, with max value CMD_BUF_MAX_PARAM */
} CtlCmdHdrStru;

typedef struct {
	CtlCmdHdrStru	CtlCmdHdr;
	int				iParas[CMD_BUF_MAX_PARAM];
} CtlCmdStru;


#pragma pack(1)
typedef struct
{
	unsigned int		magic;
	unsigned short		version_major;
	unsigned short		version_minor;
	unsigned int		thiszone;
	unsigned int		sigfigs;
	unsigned int		snaplen;
	unsigned int		linktype;
} oam_pcap_filehdr_stru;
#pragma pack()

/* 16 Bytes PCAP frame header's timestamp */
typedef struct
{
	unsigned int		GMTtime;			/* ms, between now and 1970-01-01 00:00:00 */
	unsigned int		microTime;			/* us, between now and 1970-01-01 00:00:00 */
} oam_pcap_time_stru;

/* PCAP frame header (16 Bytes) */
#pragma pack(1)
typedef struct
{
	oam_pcap_time_stru	time;				/* PCAP Frame Header's timestamp */
	unsigned int		caplen;				/* length of (Frame Header + Frame Body) */
	unsigned int		len;				/* packet's actual length, len == caplen in common */
} oam_pcap_pkthdr_stru;
#pragma pack()


typedef struct {
	unsigned char	dsap;
	unsigned char	ssap;
	unsigned char   cmd;
	unsigned char   vendID[3];
	unsigned short	protocol;  /* 0x0800 for IPv4, 0x0806 for ARP, 0x888e for EAP (802.1X Authentication) */
} __attribute__((packed)) ieee802_2_llc_hdr_stru;

typedef struct {
	unsigned char	version_ihl;
	unsigned char	temp_tos_1;
	unsigned short	temp_tot_len_1;
	unsigned short	temp_id_1;
	unsigned short	temp_frag_off_1;
	unsigned char	temp_ttl_1;
	unsigned char	protocol;  /* 0x06 for TCP, 0x11 for UDP */
	unsigned short	temp_check_1;
	unsigned int	temp_saddr_1;
	unsigned int	temp_daddr_1;
} __attribute__((packed)) eth_ipv4_hdr_stru;

typedef struct {
	unsigned char	version;
	unsigned short	traffic_class;
	unsigned char	flow_label[3];
	unsigned short	payload_len;
	unsigned char	next_hdr;
	unsigned char	hop_limit;
	unsigned char	saddr[16];
	unsigned char	daddr[16];
} __attribute__((packed)) eth_ipv6_hdr_stru;

typedef struct {
	unsigned short	sport;    /* 0x0035 or others for DNS, 0x0043 or 0x0044 for DHCP */
	unsigned short	dport;    /* others or 0x0035 for DNS, 0x0044 or 0x0043 for DHCP */
	unsigned short	len;
	unsigned short	temp_check_1;
} __attribute__((packed)) eth_ipv4_udp_hdr_stru;


typedef struct {
	unsigned short s_port;
	unsigned short d_port;
}ST_PORTPAIR;
typedef struct {
	unsigned short llc_type;
	unsigned char  protocol_type;
	unsigned char  port_count;
	ST_PORTPAIR    port_list[10];
}ST_PACKET_FILTER;

#define PORT_ANY     (0xffff)
#define PROTOCOL_ANY (0xff)
#define GET_LLC_TYPE(p_llc_hdr)       		(((ieee802_2_llc_hdr_stru *)p_llc_hdr)->protocol)
#define GET_IPV4_PROTOCOL_TYPE(p_llc_hdr)  	(((eth_ipv4_hdr_stru *)(p_llc_hdr + ETH_PKT_LLC_HDR_LEN))->protocol)

/* thought we use UDP header structure here.  For TCP header, the first two short data are sport and dport too . */
#define GET_IPV4_SRC_PORT(p_llc_hdr)       (((eth_ipv4_udp_hdr_stru *)(p_llc_hdr + ETH_PKT_LLC_HDR_LEN + ETH_PKT_IPV4_HDR_LEN))->sport)
#define GET_IPV4_DST_PORT(p_llc_hdr)       (((eth_ipv4_udp_hdr_stru *)(p_llc_hdr + ETH_PKT_LLC_HDR_LEN + ETH_PKT_IPV4_HDR_LEN))->dport)

#define GET_FILTER_SPORT(loop,port_loop)  (packet_filter[loop].port_list[port_loop].s_port)
#define GET_FILTER_DPORT(loop,port_loop)  (packet_filter[loop].port_list[port_loop].d_port)

#define ETH_PKT_TYPE_IPV4				(ntohs(0x0800))
#define ETH_PKT_TYPE_IPV4_ICMP			0x01
#define ETH_PKT_TYPE_IPV4_TCP			0x06
#define ETH_PKT_TYPE_IPV4_UDP			0x11
#define ETH_PKT_TYPE_IPV4_UDP_DNS		(ntohs(0x0035))
#define ETH_PKT_TYPE_IPV4_UDP_DHCP_A	(ntohs(0x0043))
#define ETH_PKT_TYPE_IPV4_UDP_DHCP_B	(ntohs(0x0044))

#define ETH_PKT_TYPE_IPV6				(ntohs(0x86dd))

#define ETH_PKT_TYPE_ARP				(ntohs(0x0806))
#define ETH_PKT_TYPE_EAP				(ntohs(0x888e))
#define ETH_PKT_LLC_HDR_LEN				sizeof(ieee802_2_llc_hdr_stru)
#define ETH_PKT_IPV4_HDR_LEN			sizeof(eth_ipv4_hdr_stru)

/**********************************************************************/
int proc_sniffer_read_01(struct ctl_table *ctl, int write,
				       void __user *buffer, size_t *lenp, loff_t *ppos);
int proc_sniffer_read_02(struct ctl_table *ctl, int write,
				       void __user *buffer, size_t *lenp, loff_t *ppos);
int proc_sniffer_read_03(struct ctl_table *ctl, int write,
				       void __user *buffer, size_t *lenp, loff_t *ppos);
int proc_sniffer_read_04(struct ctl_table *ctl, int write,
				       void __user *buffer, size_t *lenp, loff_t *ppos);
int proc_sniffer_read_05(struct ctl_table *ctl, int write,
				       void __user *buffer, size_t *lenp, loff_t *ppos);

int proc_sniffer_write_file(const char *header_buff, unsigned int header_len,
					const char *frame_buff, unsigned int frame_len, int flag_rx_tx);

#endif  /*end __HW_WIFI_SNIFFER_H*/
