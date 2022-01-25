

#ifndef __HW_FI_UTILS_H__
#define __HW_FI_UTILS_H__

#include <linux/spinlock_types.h>
#include <linux/netfilter_ipv4.h>

#define FI_TIMER_INTERVAL           1           /* ��ʱ��ʱ���� */
#define FI_HB_STUDY_NUM             20          /* �ռ�20������Ȼ���ж��Ƿ�Ϊ������ */
#define FI_HB_PKT_SUB               4           /* �ж�������: �����б��ĸ���������n�� */
#define FI_HB_MAX_PPS               2           /* �ж�������: ÿ�뱨�ĸ������ܴ��ڸ�ֵ */
#define FI_HB_MAX_TIME_GAP          3           /* �ж�������: ������������� */
#define FI_HB_LOST_TOLERANCE        2           /* �������������̶� */
#define FI_HB_PKT_LEN               100         /* �������ĵ���󳤶� */
#define FI_PKT_LOST_PUNISH          50          /* punish = lost_ptk_num * 50 */

#define FI_BATTLE_UP_PKT_PER_SEC    3           /* �ж϶�ս��, ���б�����СƵ�� */
#define FI_BATTLE_DOWN_PKT_PER_SEC  8           /* �ж϶�ս��, ���б�����СƵ�� */
#define FI_BATTLE_START_THRESH      2           /* ���������������ٴβ����ս��ʼ */
#define FI_AUDIO_BATTLE_BOUNDARY    120         /* ��ս����������ƽ�ֱ��ĳ��ȵķֽ��� */

#define FI_SRTT_VAR                 25          /* SRTTƽ��ϵ�� */
#define FI_HB_SRTT_VAR              25          /* ����SRTTƽ��ϵ�� */
#define FI_STUDY_INTERVAL           (1*1000)    /* ÿ��ѧϰ��ʱ����  */
#define FI_MAX_STUDY_TIME           (5*60*1000) /* ������ѧϰ��ʱ�䲻����5min */
#define FI_MIN_IP_PKT_LEN           20
#define FI_BATTLE_STOP_THRESH       5           /* ����������û�ж�ս��������Ϊ��ս���� */
#define FI_BATTLE_FLOW_CON          4           /* �ж϶�ս��: �������������ս�������� */
#define FI_NO_DOWNLINK_TIME         3           /* ����������û�����б��ģ������ж϶������� */
#define FI_RECONN_LIMIT             (10*1000)   /* �����������ļ������С��10s */
#define FI_RECONN_THRESH            180         /* �����ؽ�������rtt���ڸ�ֵ���������� */

#define FI_QQFC_PKT_AVG_LEN         400         /* �ж϶�ս��: QQ�ɳ����б���ƽ������ */
#define FI_QQFC_UP_PPS_MIN          14          /* �ж϶�ս��: QQ�ɳ�������С����Ƶ�� */
#define FI_QQFC_UP_PPS_MAX          16          /* �ж϶�ս��: QQ�ɳ����������Ƶ�� */
#define FI_APP_DISCONN              5           /* �ж�QQ�ɳ��Ƿ���� */
#define FI_APP_DISCONN_STOP         30          /* ���߶�����֮�󱨶�ս���� */

#define FI_HYXD_MIN_LEN             20          /* ��Ұ�ж���С���ĳ��� */
#define FI_HYXD_SEQ_OFFSET          12          /* ��Ұ�ж�seq�ֶ��ڱ����е�ƫ�� */
#define FI_HYXD_ACK_OFFSET          16          /* ��Ұ�ж�ack�ֶ��ڱ����е�ƫ�� */
#define FI_UPPKT_WAIT               200         /* �����б�����Ϊ200ms�Ķ�ʱ�� */
#define FI_ACK_MAX                  60000U      /* �����ж�ack��ת */
#define FI_RETRANS_PKT              2000        /* �����жϱ����ش� */
#define FI_ACK_MAX_WAIT             (10*1000)   /* �ȴ�ack���ʱ��10s */
#define FI_TCP_OPT_HDR_LEN          2           /* tcpѡ���ײ��ĳ��� */
#define FI_TCP_OPT_MPTCP            30          /* tcpѡ������ mptcp */
#define FI_MPTCP_DSS_MINLEN         8           /* mptcp dss��С���� */
#define FI_MPTCP_SUBTYPE_DSS        2           /* mptcp dssѡ������ */
#define FI_GAME_UU_RTT              10          /* ���һ����Ϸ��ʹ��uu����ô�������rttС��10ms */
#define FI_UU_BASE_RTT              25          /* UU rtt�Ļ����ϼ���һ��base */
#define FI_UU_CACHE_NUM             8           /* UU rtt�Ļ����ϼ���һ��base */

#define FI_BH3_SEQ_CACHE_NUM        4           /* ����3: ��������б��ĵĸ��� */
#define FI_BH3_KEY_WORD             0xFF        /* ����3: �ؼ��� */
#define FI_BH3_KEY_OFFSET_UP        9           /* ����3: ���б��Ĺؼ��ֵ�ƫ�� */
#define FI_BH3_KEY_OFFSET_DOWN      7           /* ����3: ���б��Ĺؼ��ֵ�ƫ�� */
#define FI_BH3_SEQ_OFFSET_UP        10          /* ����3: ���б�����ȡseq��λ�� */
#define FI_BH3_ACK_OFFSET_DOWN      8           /* ����3: ���б�����ȡack��λ�� */
#define FI_BH3_VRF_OFFSET_UP        2           /* ����3: ���б�����ȡverify��λ�� */
#define FI_BH3_VRF_OFFSET_DOWN      12          /* ����3: ���б�����ȡverify��λ�� */
#define FI_BH3_UP_LEN               12          /* ����3: ���д�seq���ĳ��� */
#define FI_BH3_DOWN_LEN             14          /* ����3: ���д�ack���ĳ��� */

#define FI_BATTLE_ONGOING           4           /* �ж϶�ս�Ƿ��ڽ��У����б���Ƶ�ʲ�����4��/s */
#define FI_BATTLE_START_PORT_MIN    1025        /* ͨ���˿�ɸѡ��ս���ģ��˿�С��1025�Ĳ����� */

#define FI_IPPROTO_UDP              17
#define FI_IPPROTO_TCP              6
#define FI_IP_VER_4                 4
#define FI_IP_HDR_LEN_BASE          4
#define FI_TCP_HDR_LEN_BASE         4
#define FI_LOOP_ADDR                127

#define FI_MS                       1000        /* ����ת�� */
#define FI_PERCENT                  100         /* �ٷֱȵ�λת�� */
#define FI_MAX_ORG_RTT              700         /* limit the original value of rtt */
#define FI_MIN_RTT                  40          /* limit the min value of rtt */
#define FI_MAX_RTT                  500         /* limit the max value of rtt */
#define FI_RTT_BASE                 50          /* limit the min value of rtt */

#define FI_APPID_NULL               0
#define FI_APPID_HYXD               1           /* ��Ұ�ж�appid */
#define FI_APPID_WZRY               2           /* ������ҫappid */
#define FI_APPID_CJZC               3           /* �̼�ս��appid */
#define FI_APPID_QJCJ               4           /* ȫ������appid */
#define FI_APPID_CYHX               5           /* ��Խ����appid */
#define FI_APPID_QQFC               6           /* QQ�ɳ�appid */
#define FI_APPID_BH3                7           /* ����3appid */
#define FI_APPID_UU                 11          /* UU������ */
#define FI_APPID_XUNYOU             12          /* Ѹ�μ����� */
#define FI_APPID_MAX                16          /* appid���� */

#define FI_TRUE                     1
#define FI_FALSE                    0
#define FI_FAILURE                  -1
#define FI_SUCCESS                  0

#define FI_FLOW_TABLE_SIZE          0x10                        /* hashͰ�Ĺ�ģ 16 */
#define FI_FLOW_TABLE_MASK          (FI_FLOW_TABLE_SIZE - 1)    /* hashͰ������ */
#define FI_FLOW_AGING_TIME          60000                       /* �����ϻ�ʱ��, ms, 1����*/
#define FI_FLOW_NODE_LIMIT          64                          /* ������ģ������ */

#define GAME_SDK_STATE_DEFAULT      0                          /* �ϲ��·�����Ϸ״̬ */
#define GAME_SDK_STATE_BACKGROUND   3                          /* �ϲ��·�����Ϸ״̬: ��Ϸת���̨ */
#define GAME_SDK_STATE_FOREGROUND   4                          /* �ϲ��·�����Ϸ״̬: ��Ϸת��ǰ̨ */
#define GAME_SDK_STATE_DIE          5                          /* �ϲ��·�����Ϸ״̬: ��Ϸ�˳� */
#define FI_STATUS_BATTLE_START      1                          /* FI�ϱ�����Ϸ״̬: ��ս��ʼ */
#define FI_STATUS_BATTLE_STOP       0                          /* FI�ϱ�����Ϸ״̬: ��ս���� */
#define FI_STATUS_BATTLE_RECONN     0x10                       /* FI�ϱ�����Ϸ״̬: �������� */
#define FI_STATUS_DISCONN           0x01                       /* �ڲ�ά������Ϸ״̬: ��Ϸ���� */

#define FI_APP_TYPE_GAME            1

#define FI_SWITCH_STATUS            0x80000000                 /* FI�ϱ�����Ϸ״̬�Ŀ��� */
#define FI_SWITCH_RTT               0x40000000                 /* FI�ϱ���rtt�Ŀ��� */

#define FI_DEBUG                    1
#define FI_INFO                     1

#define FI_APP_NAME_WZRY           "com.tencent.tmgp.sgame"     /* app����: ������ҫ */
#define FI_APP_NAME_CJZC           "com.tencent.tmgp.pubgmhd"   /* app����: �̼�ս�� */
#define FI_APP_NAME_QJCJ           "com.tencent.tmgp.pubgm"     /* app����: ȫ������ */
#define FI_APP_NAME_HYXD_HW        "com.netease.hyxd.huawei"    /* app����: ��Ұ�ж� */
#define FI_APP_NAME_HYXD           "com.netease.hyxd"           /* app����: ��Ұ�ж� */
#define FI_APP_NAME_CYHX           "com.tencent.tmgp.cf"        /* app����: ��Խ���� */
#define FI_APP_NAME_QQFC           "com.tencent.tmgp.speedmobile"/* app����: QQ�ɳ� */
#define FI_APP_NAME_BH3            "com.miHoYo.bh3.huawei"      /* app����: ����3 */
#define FI_APP_NAME_BH3_2          "com.miHoYo.bh3.qihoo"       /* app����: ����3 */
#define FI_APP_NAME_BH3_3          "com.miHoYo.bh3.uc"          /* app����: ����3 */
#define FI_APP_NAME_BH3_4          "com.tencent.tmgp.bh3"       /* app����: ����3 */
#define FI_APP_NAME_BH3_5          "com.miHoYo.enterprise.NGHSoD" /* app����: ����3 */
#define FI_APP_NAME_UU             "com.netease.uu"             /* app����: UU������ */
#define FI_APP_NAME_XUNYOU         "cn.wsds.gamemaster"         /* app����: Ѹ�μ����� */

/* mptcp DSS(Data Sequence Signal)������Ϣ�Ľṹ, 2 bytes */
typedef struct fi_mptcp_dss_t
{
#if defined(__BIG_ENDIAN_BITFIELD)
	uint16_t     subtype : 4;
	uint16_t     rev1 : 4;
	uint16_t     rev2 : 3;
	uint16_t     fin : 1;
	uint16_t     seq8 : 1;      /* seq�Ƿ�Ϊ8�ֽ�, Ĭ��4�ֽ� */
	uint16_t     seqpre : 1;    /* �Ƿ����seq */
	uint16_t     ack8 : 1;      /* ack�Ƿ�Ϊ8�ֽ�, Ĭ��4�ֽ� */
	uint16_t     ackpre : 1;    /* �Ƿ����ack */
#elif defined(__LITTLE_ENDIAN_BITFIELD)
	uint16_t     rev1 : 4;
	uint16_t     subtype : 4;
	uint16_t     ackpre : 1;
	uint16_t     ack8 : 1;
	uint16_t     seqpre : 1;
	uint16_t     seq8 : 1;
	uint16_t     fin : 1;
	uint16_t     rev2 : 3;
#else
#error	"Adjust your <asm/byteorder.h> defines"
#endif
} fi_mptcp_dss;

/* �յ����ϲ���Ϣ: ��Ϸapp���� */
typedef struct fi_msg_applaunch_t
{
	uint32_t    uid;            /* ��Ϸ�������ֻ��е�uid */
	uint32_t    switchs;        /* ���ֿ��� bit:0 �Ƿ��ϱ���Ϸ״̬�仯; bit:1 �Ƿ��ϱ�rtt */
	char        appname[0];     /* ��Ϸ���Ƶ��ַ��� */
} fi_msg_applaunch;

/* �յ����ϲ���Ϣ: ��Ϸapp״̬�仯 */
typedef struct fi_msg_appstatus_t
{
	uint32_t    uid;            /* ��Ϸ�������ֻ��е�uid */
	uint32_t    appstatus;      /* ��Ϸ��״̬����Ҫ��ע��Ϸ�˳� */
	char        appname[0];     /* ��Ϸ���Ƶ��ַ��� */
} fi_msg_appstatus;

/* ���͵�daemon�����ݽṹ: rtt */
typedef struct fi_report_rtt_t
{
	uint32_t uid;
	uint32_t apptype;
	uint32_t rtt;
} fi_report_rtt;

/* ���͵�daemon�����ݽṹ: ��Ϸ״̬�仯 */
typedef struct fi_report_status_t
{
	uint32_t uid;
	uint32_t apptype;
	uint32_t status;
} fi_report_status;

typedef struct fi_pkt_t
{
	uint8_t     *data;          /* payload data */

	uint16_t    len;            /* ���ص��ܳ��ȣ�����bufdatalen + �����������еĸ��صĳ��� */
	uint16_t    bufdatalen;     /* ���������ռ��еĸ��صĳ��� */
	uint16_t    sport;          /* network byte order, big-endian */
	uint16_t    dport;          /* network byte order, big-endian */

	uint8_t     proto;          /* tcp or udp */
	uint8_t     dir : 2;        /* SA_DIR_UP or SA_DIR_DOWN */
	uint8_t     mptcp : 1;      /* mptcp or not */
	uint8_t     rev : 5;
	uint8_t     rev2[6];

	uint32_t    sip;            /* network byte order, big-endian */
	uint32_t    dip;            /* network byte order, big-endian */

	uint32_t    seq;            /* tcp seq num */
	uint32_t    ack;            /* tcp ack num */

	/* only for netdelay calculate */
	int64_t     msec;           /* time stamp of this pkt, millisecond */
} fi_pkt;

/* ����3ÿ����˽�е�����: ��¼���б��ĵ����к� */
typedef struct fi_flow_bh3_t
{
	uint16_t    seq;                /* ���ĵ����к� */
	uint16_t    verify;             /* У���� */
	uint32_t    time;               /* ʱ��� */
} fi_flow_bh3;

/* ÿ��������б������rtt��صĻ��� */
typedef struct fi_flowctx_st
{
	uint8_t     flowtype;           /* hb flow, nonhb flow, init */
	uint8_t     appid;
	uint8_t     battle_times;       /* ���������ս��ʼ�������Ĵ��� */
	uint8_t     btflow_times;       /* ���������ս���������Ĵ��� */
	uint16_t    uppktnum;           /* ���ڶ�ս���ж�: ���б��ĸ��� */
	uint16_t    downpktnum;         /* ���ڶ�ս���ж�: ���б��ĸ��� */

	uint16_t    hbupnum;            /* �����������ж�: ���б��ĸ��� */
	uint16_t    hbdownnum;          /* �����������ж�: ���б��ĸ��� */
	uint32_t    hbstudytime;        /* �����������ж�: ��ʼ����������ѧϰ��ʱ��, ms */

	uint32_t    uppktbytes;
	uint32_t    seq;

	int64_t     studystarttime;     /* ÿ����ѧϰ��ʼ��ʱ��, ms */

	int64_t     flowstarttime;      /* time stamp of the first pkt of this flow, ms */
	int64_t     uppkttime;          /* ���б��ĵ�ʱ��� */

	union
	{                                                    /* �ض���Ϸ���˽������ */
		fi_flow_bh3 flow_bh3[FI_BH3_SEQ_CACHE_NUM];      /* ����3˽�е����� */
	};
} fi_flowctx;

/* ����� */
typedef struct fi_flow_node_t
{
	struct list_head list;

	uint32_t    sip;
	uint32_t    dip;

	uint16_t    sport;
	uint16_t    dport;
	uint32_t    updatetime;

	fi_flowctx  flowctx;
} fi_flow_node;

/* ����ı�ͷ */
typedef struct fi_flow_head_t
{
	struct list_head list;
} fi_flow_head;

/* ������ص����ݽṹ */
typedef struct fi_flow_t
{
	fi_flow_head    flow_table[FI_FLOW_TABLE_SIZE];    /* ����ı�ͷ */
	atomic_t        nodenum;                           /* �����нڵ������ */
	spinlock_t      flow_lock;                         /* ������� */
} fi_flow;

#define FI_FLOW_SAME(a, b)    \
( \
	((a)->sip == (b)->sip && \
	(a)->dip == (b)->dip && \
	(a)->sport == (b)->sport && \
	(a)->dport == (b)->dport) \
	|| \
	((a)->sip == (b)->dip && \
	(a)->dip == (b)->sip && \
	(a)->sport == (b)->dport && \
	(a)->dport == (b)->sport) \
)

enum fi_pkt_dir
{
	FI_DIR_ANY = 0,
	FI_DIR_UP,
	FI_DIR_DOWN,
	FI_DIR_MAX
};

/* ��ģ�鱣���app״̬ */
typedef struct fi_app_info_t {
	uint32_t        uid;
	uint16_t        appid;
	uint8_t         valid;          /* ͨ���ñ��λ��ʾuid�Ƿ���Ч */
	uint8_t         rev;
	uint32_t        appstatus;      /* app��ǰ��ǰ��״̬��ǰ��̨�ȵ� */
	uint32_t        switchs;        /* ���ֿ���,  */
} fi_app_info;

/* rtt cache data for every game */
typedef struct fi_gamectx_st
{
	uint8_t     appid;
	uint8_t     appstatus;
	uint8_t     applocalstatus;     /* FI�ڲ������һЩ����״̬ */
	uint8_t     preuplinkpktnum;    /* ��ս����һ�����б��ĵĸ��� */
	uint8_t     predownlinkpktnum;  /* ��ս����һ�����б��ĵĸ��� */
	uint8_t     uplinkpktnum;       /* ��¼��ս����ǰһ�����б��ĵĸ��� */
	uint8_t     downlinkpktnum;     /* ��¼��ս����ǰһ�����б��ĵĸ��� */
	uint8_t     nouplinkpkt;        /* ��¼û�����б�����������ʱ�� */

	uint8_t     nodownlinkpkt;      /* ��¼û�����б�����������ʱ�� */
	uint8_t     rev[3];
	int16_t     hbrtt;              /* ͨ�����������rtt */
	int16_t     hbsrtt;             /* ͨ�����������srtt */

	uint16_t    battle_flow_port;
	uint16_t    hb_flow_port;
	uint32_t    cliip;              /* �˲��ip��ַ */
	int32_t     rawrtt;             /* ͨ��seq/ack�������ԭʼ��rtt, �����ж���Ϸ�Ƿ񱻴��� */
	int32_t     rtt;                /* ͨ����ս�������rtt */
	int32_t     srtt;               /* ͨ����ս�������srtt */
	int32_t     battlertt;          /* ͨ����ս�����������rtt */
	int32_t     final_rtt;          /* �ۺ϶�ս������֮���rtt */

	int64_t     downlast;           /* time stamp of last down pkt, ms */
	int64_t     updatetime;         /* time stamp */
	int64_t     rtt_integral;
	int64_t     reconntime;         /* ������������ʱ�� */
} fi_gamectx;

typedef struct fi_mpctx_t
{
	uint32_t    seq;                /* ��������ڼ���rtt��seq */
	uint32_t    preseq;             /* mptcpͨ����ǰseq�Ѿ����е������� */
	int64_t     uppkttime;          /* ��¼seq�ı��ĵ�ʱ��� */
} fi_mpctx;

/* fiģ�������� */
typedef struct fi_ctx_t
{
	struct timer_list tm;                       /* timer */
	struct mutex    appinfo_lock;               /* ����appinfo���� */
	struct mutex    nf_mutex;                   /* ����nfע����� */
	uint32_t        memused;                    /* ģ��ʹ�õ��ڴ�ͳ�� */

	uint8_t         enable;                     /* ģ��ʹ�ܱ�� */
	uint8_t         appidmin;                   /* ��¼��С��appid */
	uint8_t         appidmax;                   /* ��¼����appid */
	uint8_t         nf_exist;                   /* �Ƿ��Ѿ����ڹ��Ӻ��� */

	fi_app_info     appinfo[FI_APPID_MAX];      /* ��������appid-uid, ��appid���� */
	fi_gamectx      gamectx[FI_APPID_MAX];      /* ÿ����Ϸ����rtt�Ļ���, ��appid���� */
	fi_mpctx        mpctx[FI_APPID_MAX];        /* mptcp�����±����·������Ϣ */
} fi_ctx;

typedef enum
{
	FI_FLOWTYPE_INIT,
	FI_FLOWTYPE_HB,
	FI_FLOWTYPE_BATTLE,
	FI_FLOWTYPE_UNKNOWN
} fi_rtt_flowtype_enum;


#define FI_LOGD(fmt, ...) \
	do { \
		if (FI_DEBUG) { \
			hwlog_info("%s"fmt"\n", __func__, ##__VA_ARGS__); \
		} \
	} while (0)

#define FI_LOGI(fmt, ...) \
	do { \
		if (FI_INFO) { \
			hwlog_info("%s"fmt"\n", __func__, ##__VA_ARGS__); \
		} \
	} while (0)


#define FI_LOGE(fmt, ...) \
	do { \
			hwlog_err("%s"fmt"\n", __func__, ##__VA_ARGS__); \
	} while (0)

#define FI_MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define FI_MAX(a,b) (((a) >= (b)) ? (a) : (b))
#define FI_RANGE(v,l,r) ((v) < (l) ? (l) : ((v) > (r) ? (r) : (v)))
#define FI_ABS_SUB(a, b) (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))
#define FI_SRVPORT(p) (((p)->dir == FI_DIR_UP) ? (p)->dport : (p)->sport)
#define FI_CLIPORT(p) (((p)->dir == FI_DIR_UP) ? (p)->sport : (p)->dport)
#define FI_CLIIP(p) (((p)->dir == FI_DIR_UP) ? (p)->sip : (p)->dip)
#define FI_MS2SEC(ms) ((ms) / FI_MS)
#define FI_APPID_VALID(id) ((id) < FI_APPID_MAX && (id) > FI_APPID_NULL)
#define FI_HAS_NO_APPID(ctx) ((ctx.appidmin == 0) && (ctx.appidmax == 0))
#define FI_BATTLING(s) (((s) & FI_STATUS_BATTLE_START) != 0)
#define FI_DISCONN(s) (((s) & FI_STATUS_DISCONN) != 0)

extern fi_ctx g_fi_ctx;
extern uint32_t fi_flow_node_num(void);
extern uint32_t fi_flow_hash(uint32_t sip, uint32_t dip, uint32_t sport, uint32_t dport);
extern fi_flow_node *fi_flow_get(fi_pkt *pktinfo, fi_flow_head *head, uint32_t addflow);
extern fi_flow_head *fi_flow_header(uint32_t index);
extern void  fi_rtt_timer(void);
extern void  fi_flow_init(void);
extern void  fi_flow_age(void);
extern void  fi_flow_clear(void);
extern void  fi_flow_lock(void);
extern void  fi_flow_unlock(void);
extern void  fi_free(void *ptr, uint32_t size);
extern void *fi_malloc(uint32_t size);
extern void  fi_rtt_status(uint32_t appid, uint32_t status);
extern void  fi_rtt_judge_reconn(fi_pkt *pktinfo, fi_gamectx *gamectx);
extern int   fi_rtt_judge_battle_stop(fi_gamectx *gamectx, fi_app_info *appinfo);
extern void  fi_rtt_cal_bh3(fi_pkt *pktinfo, fi_flowctx *flowctx, fi_gamectx *gamectx);
extern int   fi_rtt_entrance(fi_pkt *pktinfo, fi_flowctx *flowctx, uint32_t appid);
unsigned int fi_hook_out(void *priv, struct sk_buff *skb, const struct nf_hook_state *state);
unsigned int fi_hook_in(void *priv, struct sk_buff *skb, const struct nf_hook_state *state);

#endif /* __HW_FI_UTILS_H__ */
