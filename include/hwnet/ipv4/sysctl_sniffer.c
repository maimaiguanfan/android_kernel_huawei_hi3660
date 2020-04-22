/*
*    An implementation for wifi sysctl interface for wifi sniffer.
*    Author                       :    Ding Guozhi, Li Renjie.
*    Date                         :    20th Sept,2018
*    Functions                    :    save packet sniffer data as pcap files.
*
*
*    Modification of kernel       :    add /proc/sys/net/ipv4 directory entry
*
*
*
*/
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/jump_label.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/statfs.h>
#include "sysctl_sniffer.h"

#ifdef CONFIG_HW_SNIFFER
/*******************************************************************************/
proc_file_config_struct g_configs = {0,IEEE80211_DATA_PAYLOAD_LEN, PCAP_FILE_NUM, PCAP_FILE_LEN, {0},{NULL},{0}};
static DEFINE_SPINLOCK(proc_wrlock);
struct static_key wifi_sniffer_enable __read_mostly = STATIC_KEY_INIT_FALSE;

/*******************************************************************************/
static void proc_set_cur_file_index_to_first(void) //change to inline?
{
	g_configs.cur_file = 0;
}

static void proc_init_file_content(int file_index, int file_len)
{
	memset(g_configs.buffer[file_index], 0, file_len);
	g_configs.curpos[file_index]    = 0;
	g_configs.file_full[file_index] = 0;
}

static int proc_sniffer_reset_buff(int old_file_num, int new_file_num, int new_file_len)
{
	int i;
	for (i = 0; i < old_file_num; i++)
	{
		if (SNIFFER_FILE_BUFFER(i))
		{
			vfree(SNIFFER_FILE_BUFFER(i));
			SNIFFER_FILE_BUFFER(i) = NULL;
		}
	}

	for (i = 0; i < new_file_num; i++)
	{
		SNIFFER_FILE_BUFFER(i) = (char *)vmalloc(new_file_len);
		if (!SNIFFER_FILE_BUFFER(i))
		{
			return -ENOMEM;
		}
		proc_init_file_content(i, new_file_len);
	}

	proc_set_cur_file_index_to_first();
	return 0;
}

static void proc_sniffer_init_buff(void)
{
	proc_sniffer_reset_buff( g_configs.max_file_num, g_configs.max_file_num, g_configs.max_file_len);
}

static void proc_cur_file_move_to_next(void)
{
	g_configs.file_full[g_configs.cur_file] = 1;
	g_configs.cur_file = (g_configs.cur_file + 1) % g_configs.max_file_num;
}

static void proc_select_file_to_save_packet(unsigned int packet_len)
{
	/* current file is not full, and will be full if write one more packet */
	if  (  (!SNIFFER_IS_CUR_FULL && !SNIFFER_ENOUGH_TO_SAVE(packet_len) )
		|| (SNIFFER_IS_CUR_FULL) )
	{
		proc_cur_file_move_to_next();
		proc_init_file_content(g_configs.cur_file,g_configs.max_file_len);
	}
}

static void proc_increase_cur_file_pos(int len)
{
    g_configs.curpos[g_configs.cur_file] += len;
}

static int proc_write_pcap_hdr_for_empty_file (void)
{
	oam_pcap_filehdr_stru *pcap_filehdr =
			(oam_pcap_filehdr_stru *)SNIFFER_CUR_FILE_BUFFER;
 	if (!SNIFFER_IS_CUR_FILE_EMPTY)
	{
		return 0;
	}

	pcap_filehdr->magic			= PCAP_PKT_HDR_MAGIC;
	pcap_filehdr->version_major = PCAP_PKT_HDR_VER_MAJ;
	pcap_filehdr->version_minor	= PCAP_PKT_HDR_VER_MIN;
	pcap_filehdr->thiszone 		= PCAP_PKT_HDR_THISZONE;
	pcap_filehdr->sigfigs 		= PCAP_PKT_HDR_SIGFIGS;
	pcap_filehdr->snaplen 		= PCAP_PKT_HDR_SNAPLEN;
	pcap_filehdr->linktype 		= PCAP_PKT_HDR_LINKTYPE;

	proc_increase_cur_file_pos(sizeof(oam_pcap_filehdr_stru));
	return sizeof(oam_pcap_filehdr_stru);
}

static void proc_update_packet_pcap_header(char *file_buffer, int packet_len)
{
	struct timeval tv;
	oam_pcap_pkthdr_stru  	st_pcap_pkthdr ={{0,0},0,0};

	do_gettimeofday(&tv);
	st_pcap_pkthdr.time.GMTtime   = tv.tv_sec;
	st_pcap_pkthdr.time.microTime = tv.tv_usec;
	st_pcap_pkthdr.caplen         = packet_len;
	st_pcap_pkthdr.len 	          = packet_len;

	memcpy(file_buffer, (char *)&st_pcap_pkthdr, sizeof(oam_pcap_pkthdr_stru));
}

/* filter ARP/DHCP/DNS/EAP/ICMP for packet trimming  */
static int proc_packet_get_trim_flag(const char * p_llc_hdr )
{
	int loop = 0, port_loop = 0;
	const ST_PACKET_FILTER packet_filter[] = {
		{ETH_PKT_TYPE_IPV4, ETH_PKT_TYPE_IPV4_UDP,  4, {{ETH_PKT_TYPE_IPV4_UDP_DHCP_A, ETH_PKT_TYPE_IPV4_UDP_DHCP_B},
		                                                {ETH_PKT_TYPE_IPV4_UDP_DHCP_B, ETH_PKT_TYPE_IPV4_UDP_DHCP_A},
													    {ETH_PKT_TYPE_IPV4_UDP_DNS,    PORT_ANY},
													    {PORT_ANY,                     ETH_PKT_TYPE_IPV4_UDP_DNS},
													    {PORT_ANY,PORT_ANY}}  },
		{ETH_PKT_TYPE_IPV4, ETH_PKT_TYPE_IPV4_ICMP, 0, {{PORT_ANY,PORT_ANY}} },
		{ETH_PKT_TYPE_ARP , PROTOCOL_ANY,           0, {{PORT_ANY,PORT_ANY}} },
		{ETH_PKT_TYPE_EAP , PROTOCOL_ANY,           0, {{PORT_ANY,PORT_ANY}} }
	};
	const int types_count = sizeof(packet_filter)/(sizeof(packet_filter[0]));

	if(ETH_PKT_TYPE_IPV6 == GET_LLC_TYPE(p_llc_hdr))
	{
		return TRIM_NOT_NEEDED;
	}
	for (loop = 0; loop < types_count; loop++)
	{
		if (   packet_filter[loop].llc_type == GET_LLC_TYPE(p_llc_hdr)  &&
			   GET_IPV4_PROTOCOL_TYPE(p_llc_hdr) == (packet_filter[loop].protocol_type & GET_IPV4_PROTOCOL_TYPE(p_llc_hdr)) )
		{
			if (0 == packet_filter[loop].port_count)
			{
				return TRIM_NOT_NEEDED;
			}
			for (port_loop = 0; port_loop < packet_filter[loop].port_count; port_loop++)
			{
			   if ( GET_IPV4_SRC_PORT(p_llc_hdr) == (GET_IPV4_SRC_PORT(p_llc_hdr) & GET_FILTER_SPORT(loop, port_loop))  &&
				    GET_IPV4_DST_PORT(p_llc_hdr) == (GET_IPV4_DST_PORT(p_llc_hdr) & GET_FILTER_DPORT(loop, port_loop))  )
			   {
				   return TRIM_NOT_NEEDED;
			   }
			}
		}
	}
	return TRIM_NEEDED;
}

static int proc_trim_packet_len(const char *header_buff, int *p_header_len,
					const char *frame_buff, int *p_frame_len, int packet_len_trim)
{
	int trim_flag = 1;
	char * p_llc_hdr = NULL;
	mac_header_control_stru * p_frame_type = NULL;

	if (!header_buff)
	{
		*p_header_len = 0;
		p_frame_type = (mac_header_control_stru *)frame_buff;
		p_llc_hdr = (char *)(frame_buff + IEEE80211_MAC_HDR_LEN);
	}
	else
	{
		p_frame_type = (mac_header_control_stru *)header_buff;
		p_llc_hdr = (char *)frame_buff;
	}

	if (IEEE80211_FRAME_TYPE_DATA == p_frame_type->bit_type)
	{
		trim_flag = proc_packet_get_trim_flag(p_llc_hdr);
		if(TRIM_NOT_NEEDED == trim_flag)
		{
			return p_frame_type->bit_type;
		}
		if ((*p_frame_len + *p_header_len) > (packet_len_trim + IEEE80211_MAC_HDR_LEN))
		{
			*p_frame_len =  packet_len_trim + IEEE80211_MAC_HDR_LEN - *p_header_len;
		}
	}
	return p_frame_type->bit_type;
}


static void proc_set_rx_packet_to_no_proection(int flag_rx_tx, int frame_type, char *dst_buff)
{
	if ((DIRECTION_RX == flag_rx_tx) && (IEEE80211_FRAME_TYPE_DATA == frame_type))
	{
       ((mac_header_control_stru *)dst_buff)->bit_protected_frame = 0;
	}

	if ((DIRECTION_TX == flag_rx_tx) && (IEEE80211_FRAME_TYPE_DATA == frame_type))
	{
	   ((mac_header_control_stru *)dst_buff)->bit_sub_type = IEEE80211_FRAME_SUBTYPE_QOS;
	}

}

static void proc_write_packet_to_file(char *dst_buff, const char *header_buff, int header_len, const char *frame_buff, int frame_len)
{
	proc_update_packet_pcap_header(dst_buff, frame_len + header_len);
	if (header_buff)
	{
		memcpy(dst_buff + sizeof(oam_pcap_pkthdr_stru), header_buff, header_len);
	}
	memcpy(dst_buff + sizeof(oam_pcap_pkthdr_stru) + header_len, frame_buff, frame_len);
}

int proc_sniffer_write_file(const char *header_buff, unsigned int header_len,
					const char *frame_buff, unsigned int frame_len, int flag_rx_tx)
{
	char *dst_buff = NULL;
	int frame_type;

	if (!SNIFFER_START)
	{
		return 0;
	}

	if (!frame_buff || (frame_len + header_len)<(26+4) )
	{
		return -1;
	}

	/* TODO: what if a large AMSDU frame? */
	if ((frame_len + header_len) >= 1700)
	{
		return -1;
	}

	spin_lock_bh(&proc_wrlock);
	if (!SNIFFER_CUR_FILE_BUFFER)
	{
		proc_sniffer_init_buff();
	}

    frame_type = proc_trim_packet_len(header_buff, &header_len,
						frame_buff, &frame_len, SNIFFER_MAX_PACKET_LEN);

	proc_select_file_to_save_packet(sizeof(oam_pcap_pkthdr_stru) + frame_len + header_len);
    proc_write_pcap_hdr_for_empty_file();

	dst_buff = SNIFFER_CUR_FILE_BUFF_POS;
	proc_increase_cur_file_pos(sizeof(oam_pcap_pkthdr_stru) + frame_len + header_len);
    proc_write_packet_to_file(dst_buff, header_buff, header_len, frame_buff, frame_len);
	proc_set_rx_packet_to_no_proection(flag_rx_tx, frame_type, dst_buff + sizeof(oam_pcap_pkthdr_stru));

	spin_unlock_bh(&proc_wrlock);
	return (sizeof(oam_pcap_pkthdr_stru) + frame_len + header_len);
}

/*****************************************************************/
extern int proc_handle_commands(int file_index, char __user *buffer);

int proc_sniffer_read(int file_index, int write, char __user *buffer,
			   size_t *lenp, loff_t *ppos)
{
	size_t len;
	char *file_data = SNIFFER_FILE_BUFFER(file_index);
	int file_pos    = SNIFFER_FILE_POS(file_index);
	CtlCmdStru		Cmd2Parse;

	if(copy_from_user(&Cmd2Parse, buffer, sizeof(CtlCmdStru)))
	{
		return -EFAULT;
	}

	if ((sizeof(CtlCmdHdrStru) == *lenp) && (CMD_BUFF_MAGIC_NUM == Cmd2Parse.CtlCmdHdr.iMagicNum) )
	{
		return proc_handle_commands(file_index, buffer);
	}

	if (!file_data || !*lenp)
	{
		*lenp = 0;
		return 0;
	}

	if (write)
	{
		return 0;
	}

	/* can not read data behind the current file_content_pos */
	if (*ppos >= file_pos)
	{
		*lenp = 0;
		return 0;
	}

	len  = file_pos - *ppos ;
	if (len > *lenp)
	{
		len = *lenp;
	}

	if (copy_to_user(buffer, file_data+ *ppos, len))
	{
		return -EFAULT;
	}

	*lenp = len;
	*ppos += len;

	return 0;
}

/* return 0 , if the userBuffer is a valid command buffer. */
int proc_handle_commands(int file_index, char __user *buffer)
{
	static int iSniffMode 	= -1;	/* 0 sniffer my own packets, 1 sniffer all packets */
	static int iChannel 	= -1;	/* channel number */
	static int iBandWidth 	= -1;	/* 20/40/80/160 */
	static int iPrototype 	= -1;	/* a/b/g/n/ac/ax(1~6 respectively) */
	int iCurFileLen	        = -1;	/* current file length */
	int old_file_num;
	int old_file_len;

	CtlCmdStru Cmd2Parse;
	if(copy_from_user(&Cmd2Parse, buffer, sizeof(CtlCmdStru)))
	{
		return -EFAULT;
	}

	if((file_index < 0) || (file_index >= PCAP_FILE_NUM))
	{
		printk(KERN_ERR "{%s:%d} ERROR : Invalid file_index (%d) \n",
				__FUNCTION__, __LINE__, file_index);
		return -1;
	}

	if ((Cmd2Parse.CtlCmdHdr.iCmdID <= CMD_BUFF_CMDID_TOP)
			|| (Cmd2Parse.CtlCmdHdr.iCmdID >= CMD_BUFF_CMDID_BUTTOM))
	{
		printk(KERN_ERR "{%s:%d} ERROR : Invalid Command ID (0x%08x) \n",
				__FUNCTION__, __LINE__, Cmd2Parse.CtlCmdHdr.iCmdID);
		return -1;
	}

	/* TODO: GET all parameters */
	if (CMD_BUFF_RDWR_READ == Cmd2Parse.CtlCmdHdr.iRdWr)
	{
		printk(KERN_ERR "{%s:%d} TODO : GET all paras \n", __FUNCTION__, __LINE__);
		return 0;
	}


	if ((Cmd2Parse.CtlCmdHdr.iParaNum < 0) || (Cmd2Parse.CtlCmdHdr.iParaNum > CMD_BUF_MAX_PARAM))
	{
		printk(KERN_ERR "{%s:%d} ERROR : Invalid Para Num (0x%08x) \n",
				__FUNCTION__, __LINE__, Cmd2Parse.CtlCmdHdr.iParaNum);
		return -1;
	}

	/* SET all parameters */
	switch (Cmd2Parse.CtlCmdHdr.iCmdID)
	{
		case CMD_BUFF_CMDID_SNIFFER_ENABLE:
			if (Cmd2Parse.iParas[0])
			{
				if (!SNIFFER_CUR_FILE_BUFFER)
				{
					proc_sniffer_init_buff();
				}
			    static_key_enable(&wifi_sniffer_enable);
			}
			else
			{
				static_key_disable(&wifi_sniffer_enable);
			}
			break;

		case CMD_BUFF_CMDID_SNIFFER_MODE:
			iSniffMode = Cmd2Parse.iParas[0];
			break;

		case CMD_BUFF_CMDID_CHANNDEL:
			iChannel = Cmd2Parse.iParas[0];
			break;

		case CMD_BUFF_CMDID_BANDWIDTH:
			iBandWidth = Cmd2Parse.iParas[0];
			break;

		case CMD_BUFF_CMDID_PROTOTYPE:
			iPrototype = Cmd2Parse.iParas[0];
			break;

		case CMD_BUFF_CMDID_CURFILELEN:
			iCurFileLen = SNIFFER_FILE_POS(file_index);
			if(copy_to_user((char __user *)&(((CtlCmdStru *)buffer)->iParas[0]),
								(char *)&iCurFileLen, sizeof(iCurFileLen)))
			{
				return -1;
			}
			break;

		case CMD_BUFF_CMDID_FILESPEC:
			/* allow config file number and file length only when capture switch is off */
			if(SNIFFER_START)
			{
				printk(KERN_ERR "{%s:%d} ERROR : Should Stop Capturing First! \n", __FUNCTION__, __LINE__);
				return -1;
			}
			if((Cmd2Parse.iParas[0] < PCAP_FILE_NUM_MIN) || (Cmd2Parse.iParas[0] > PCAP_FILE_NUM_MAX))
			{
				printk(KERN_ERR "{%s:%d} ERROR : Invalid Parameter max_file_num (%d) \n",
							__FUNCTION__, __LINE__, Cmd2Parse.iParas[0]);
				return -1;
			}
			if((Cmd2Parse.iParas[1] < PCAP_FILE_LEN_MIN) || (Cmd2Parse.iParas[1] > PCAP_FILE_LEN_MAX))
			{
				printk(KERN_ERR "{%s:%d} ERROR : Invalid Parameter g_max_file_len (%d) \n",
							__FUNCTION__, __LINE__, Cmd2Parse.iParas[1]);
				return -1;
			}
			old_file_num = SNIFFER_MAX_FILE_NUM;
			old_file_len = SNIFFER_MAX_FILE_LEN;
			SNIFFER_MAX_FILE_NUM = Cmd2Parse.iParas[0];
			SNIFFER_MAX_FILE_LEN = Cmd2Parse.iParas[1];
			proc_sniffer_reset_buff(old_file_num, SNIFFER_MAX_FILE_NUM, SNIFFER_MAX_FILE_LEN);
			break;
		case CMD_BUFF_CMDID_DATAPAYLOAD:
			/* allow config 802.11 payload length only when capture switch is off */
			if(SNIFFER_START)
			{
				printk(KERN_ERR "{%s:%d} ERROR : Should Stop Capturing First! \n", __FUNCTION__, __LINE__);
				return -1;
			}
			SNIFFER_MAX_PACKET_LEN = Cmd2Parse.iParas[0];
			break;
		default:
			printk(KERN_ERR "{%s:%d} ERROR : Invalid Command ID (0x%08x) \n",
						__FUNCTION__, __LINE__, Cmd2Parse.CtlCmdHdr.iCmdID);
			return -1;
	}

	return 0;
}


int proc_sniffer_read_01(struct ctl_table *ctl, int write,
				       void __user *buffer, size_t *lenp, loff_t *ppos)
{
	return  proc_sniffer_read(0, FILE_READ, (char __user *)buffer, lenp, ppos);
}

int proc_sniffer_read_02(struct ctl_table *ctl, int write,
				       void __user *buffer, size_t *lenp, loff_t *ppos)
{
	return  proc_sniffer_read(1, FILE_READ, (char __user *)buffer, lenp, ppos);
}

int proc_sniffer_read_03(struct ctl_table *ctl, int write,
				       void __user *buffer, size_t *lenp, loff_t *ppos)
{
	return  proc_sniffer_read(2, FILE_READ, (char __user *)buffer, lenp, ppos);
}

int proc_sniffer_read_04(struct ctl_table *ctl, int write,
				       void __user *buffer, size_t *lenp, loff_t *ppos)
{
	return  proc_sniffer_read(3, FILE_READ, (char __user *)buffer, lenp, ppos);
}

int proc_sniffer_read_05(struct ctl_table *ctl, int write,
				       void __user *buffer, size_t *lenp, loff_t *ppos)
{
	return  proc_sniffer_read(4, FILE_READ, (char __user *)buffer, lenp, ppos);
}

#endif
