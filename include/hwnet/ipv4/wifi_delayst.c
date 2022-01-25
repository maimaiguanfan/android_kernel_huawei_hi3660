

#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <net/tcp.h>
#include <linux/sysctl.h>
#include <hwnet/ipv4/wifi_delayst.h>
#include <trace/events/skb.h>
#include <linux/cryptohash.h>
#include <linux/inet.h>
#include <huawei_platform/chr/wbc_hw_hook.h>

u8 delayst_switch = 1;
/*for sysctl command,statistic configuration*/
char tcp_delay_filter[DELAY_FILTER_NAME_MAX] = {0};

/*store statistics results*/
DELAY_STAT_T Delay_S    = {{{0},{0},{0},{0},{0}},{0},{0}};
DELAY_STAT_T RcvDelay_S = {{{0},{0},{0},{0},{0}},{0},{0}};
DP_SETTINGS_T Settings  = {
    .dp_mode            = mode_stat,
    .print_interval     = 1000,
    .android_uid        = UID_MATCH_ALL,
    .dp_switch          = flag_auto,
};

struct static_key wifi_delay_statistic_key __read_mostly = STATIC_KEY_INIT_FALSE;

/*operatios for delayst_skb_cb for  skb->cb*/
skbdirect_enum skbprobe_get_direct(struct sk_buff *pskb)
{
    return DELAYST_SKB_CB(pskb)->pdirect;
}

void skbprobe_set_direct(struct sk_buff *pskb, skbdirect_enum direct)
{
    DELAYST_SKB_CB(pskb)->pdirect = direct;
}

skbtype_enum skbprobe_get_proto(struct sk_buff *pskb)
{
    return DELAYST_SKB_CB(pskb)->pproto;
}

void skbprobe_set_proto(struct sk_buff *pskb, skbtype_enum  proto)
{
    DELAYST_SKB_CB(pskb)->pproto = proto;
}

ktime_t skbprobe_get_skbtime(struct sk_buff *pskb,int time_index)
{
    return (DELAYST_SKB_CB(pskb)->ptime[time_index]);
}

__u32 skbprobe_get_skbuid(struct sk_buff *pskb)
{
    return (DELAYST_SKB_CB(pskb)->android_uid);
}

void skbprobe_copy_delayskbcb(DELAYSKB_CB_T *delayskbcb,struct sk_buff *pskb)
{
    if (IS_DIRECT(pskb, TP_SKB_DIRECT_SND)) {
        delayskbcb->android_uid = skbprobe_get_skbuid(pskb);
        delayskbcb->ptime[TP_SKB_SEND] = skbprobe_get_skbtime(pskb,TP_SKB_SEND);
        delayskbcb->ptime[TP_SKB_IP] = skbprobe_get_skbtime(pskb,TP_SKB_IP);
        delayskbcb->ptime[TP_SKB_HMAC_XMIT] = skbprobe_get_skbtime(pskb,TP_SKB_HMAC_XMIT);
        delayskbcb->ptime[TP_SKB_HMAC_TX] = skbprobe_get_skbtime(pskb,TP_SKB_HMAC_TX);
        delayskbcb->ptime[TP_SKB_DMAC] = skbprobe_get_skbtime(pskb,TP_SKB_DMAC);
    } else {
        delayskbcb->android_uid = skbprobe_get_skbuid(pskb);
        delayskbcb->ptime[TP_SKB_DMAC] = skbprobe_get_skbtime(pskb,TP_SKB_DMAC);
        delayskbcb->ptime[TP_SKB_HMAC_UPLOAD] = skbprobe_get_skbtime(pskb,TP_SKB_HMAC_UPLOAD);
        delayskbcb->ptime[TP_SKB_IP] = skbprobe_get_skbtime(pskb,TP_SKB_IP);
        delayskbcb->ptime[TP_SKB_RECV] = skbprobe_get_skbtime(pskb,TP_SKB_RECV);
    }
}

/*0: not match ,1:match*/
int is_uid_match(struct sk_buff *skb)
{
    if (0 == (DELAYST_SKB_CB(skb)->android_uid) ) {
        return 0;
    }
    if ((UID_MATCH_ALL == Settings.android_uid) ||
        ((DELAYST_SKB_CB(skb)->android_uid) == Settings.android_uid)) {
        return 1;
    } else {
        return 0;
    }
}

void skbprobe_record_first(struct sk_buff *skb, u32 type)
{
    if (type > (TP_SKB_MAX_ENTRY - 1)) {
        return;
    }
    memset(DELAYST_SKB_CB(skb)->ptime, 0, sizeof(ktime_t)*TP_SKB_MAX_ENTRY);
    DELAYST_SKB_CB(skb)->ptime[type] = ktime_get_real();
    DELAYST_SKB_CB(skb)->pdirect = ((type == 0) ? (TP_SKB_DIRECT_SND) : (TP_SKB_DIRECT_RCV));
    DELAYST_SKB_CB(skb)->android_uid = UID_MATCH_ALL;
}

void skbprobe_record_time(struct sk_buff *skb, int index)
{
    if (index < 0 || index >= TP_SKB_MAX_ENTRY) {
        return;
    }
    DELAYST_SKB_CB(skb)->ptime[index] = ktime_get_real();
}

void skbprobe_record_proto(struct sk_buff *skb, __u8 n)
{
	skbprobe_set_proto(skb,n);
}

u32 skbprobe_get_latency(struct sk_buff *skb, int t1, int t2)
{
    if (t1 < 0 || t2 < 0 || t1 >= TP_SKB_MAX_ENTRY || t2 >= TP_SKB_MAX_ENTRY) {
        return 0;
    }
    return (ktime_to_us(skbprobe_get_skbtime(skb,t2)) - ktime_to_us(skbprobe_get_skbtime(skb,t1)));
 }

u32 skbprobe_get_latency_from_delayst(DELAYSKB_CB_T *delayskb, int t1, int t2)
{
    if (t1 < 0 || t2 < 0 || t1 >= TP_SKB_MAX_ENTRY || t2 >= TP_SKB_MAX_ENTRY) {
        return 0;
    }
    return (ktime_to_us(delayskb->ptime[t2]) - ktime_to_us(delayskb->ptime[t1]));
 }

int delay_record_get_interval_index(uint32_t uDelay)
{
    const int threshold_values[4] ={50,200, 1000, 10000};
    int j = 0;
    for (j=0; j<4; ++j) {
        if (uDelay < threshold_values[j]) {
            break;
        }
    }
    return j;
}
void delay_record_send(DELAY_STAT_T *delay, struct sk_buff *skb)
{
    int i=0;
    uint32_t uDelay = 0;
    for (i=0; i<4; ++i) {
        uDelay = skbprobe_get_latency(skb,i,i+1);
        delay->T_TotalDelay[i+1] += uDelay;
        delay->T_TotalPkts[i+1]  += 1;
        delay->T_gap[i+1][delay_record_get_interval_index(uDelay)] ++;
    }
}

void delay_record_receive(DELAY_STAT_T *delay, struct sk_buff *skb)
{
    int i=0;
    uint32_t uDelay = 0;
    for (i=3; i>0; --i) {
        uDelay = skbprobe_get_latency(skb,i,i-1);
        delay->T_TotalDelay[i-1] += uDelay;
        delay->T_TotalPkts[i-1]  += 1;
        delay->T_gap[i-1][delay_record_get_interval_index(uDelay)] ++;
    }
}

/*combine funtion for network layer*/
void delay_record_ip_combine(struct sk_buff *skb, skbdirect_enum direct)
{
    struct iphdr *iph= NULL;
    int index =TP_SKB_HMAC_UPLOAD;

    delay_flow_ctl(skb);
    iph = ip_hdr(skb);
    if (NULL == iph) {
        return;
    }
    if (iph->saddr == in_aton("127.0.0.1") || iph->daddr == in_aton("127.0.0.1")) {
        memset(DELAYST_SKB_CB(skb)->ptime, 0, sizeof(ktime_t)*TP_SKB_MAX_ENTRY);
        return;
    }
    if (direct == TP_SKB_DIRECT_SND) {
        index = TP_SKB_SEND;
    }
    if (IS_NEED_RECORD_DELAY(skb,index)) {
        skbprobe_record_time(skb, TP_SKB_IP);
    }
}

/*determine whether we should record the packet or not in receive direction,filter function*/
void delay_record_rcv_combine(struct sk_buff *skb,struct sock *sk,skbtype_enum type)
{
    DELAYST_SKB_CB(skb)->android_uid = sk->sk_uid.val;
    if (IS_NEED_RECORD_DELAY(skb, TP_SKB_IP)  && is_uid_match(skb)) {
        skbprobe_record_time(skb, TP_SKB_RECV);
        skbprobe_record_proto(skb,type);
        delay_record_gap(skb);
    }
}

/*determine whether we should record the packet or not in send direction,filter function*/
void  delay_record_snd_combine(struct sk_buff *skb)
{
    if (IS_NEED_RECORD_DELAY(skb,TP_SKB_HMAC_TX)  &&  is_uid_match(skb)) {
        skbprobe_record_time(skb, TP_SKB_DMAC);
        delay_record_gap(skb);
    }
}

void delay_record_gap(struct sk_buff *skb)
{
    if (IS_DIRECT(skb, TP_SKB_DIRECT_SND)) {
        delay_print_time_exception(skb,TP_SKB_SEND,TP_SKB_DMAC);
        delay_record_send(&Delay_S, skb);
    } else {
        delay_print_time_exception(skb,TP_SKB_DMAC,TP_SKB_RECV);
        delay_record_receive(&RcvDelay_S, skb);
    }
    if (Settings.dp_mode == mode_stat) {
        if (0 != delayst_switch) {
            delay_record_print_combine(skb);
        }
    } else {
        trace_skb_latency(skb);
    }
}

/*record first timestamp*/
void delay_record_first_combine(struct sock *sk, struct sk_buff *skb , skbdirect_enum direct , skbtype_enum type)
{
    skbprobe_record_first(skb, TP_SKB_SEND);
    skbprobe_record_proto(skb, type);
    DELAYST_SKB_CB(skb)->android_uid = sk->sk_uid.val;
}

/*print delay time*/
void delay_record_print_combine(struct sk_buff *skb)
{
    if (!delayst_switch) {
         return;
    }
    if (IS_DIRECT(skb, TP_SKB_DIRECT_RCV)) {
        if (PACKET_IS_ENOUGH_FOR_PRINT(RcvDelay_S, TP_SKB_RECV)) {
            printk("SumPkts record : %u ,"
                "Rcv Delay [<50us, <200us, <1ms, <10ms, >=10ms]"
                "driver:[%u, %u, %u, %u, %u ]"
                "ip:[%u, %u, %u, %u, %u ]"
                "transport:[%u, %u, %u, %u, %u ]"
                "Average: driver  %u, ip %u, transport %u",
                RcvDelay_S.T_TotalPkts[TP_SKB_RECV],
                GET_UPLOAD_ALL, GET_RCV_IP_ALL,GET_RECV_ALL,
                GET_AVG_UPLOAD,  GET_RCV_AVG_IP,  GET_AVG_RECV);
                memset(&RcvDelay_S,0,sizeof(DELAY_STAT_T));
        }
    } else {
        if (PACKET_IS_ENOUGH_FOR_PRINT(Delay_S, TP_SKB_DMAC)) {
            printk("SumPkts record in driver: %u ,"
                "Send Delay [<50us, <200us, <1ms, <10ms, >=10ms]"
                "transport:[%u, %u, %u, %u, %u ]"
                "ip:[%u, %u, %u, %u, %u ]"
                "hmac:[%u, %u, %u, %u, %u ]"
                "driver:[%u, %u, %u, %u, %u ]"
                "Average: transport %u, ip %u,hmac %u,driver %u",
                Delay_S.T_TotalPkts[TP_SKB_DMAC],
                GET_SND_IP_ALL, GET_XMIT_ALL, GET_TX_ALL,GET_DMAC_ALL,
                GET_SND_AVG_IP, GET_AVG_XMIT,GET_AVG_TX,GET_AVG_DMAC);
                memset(&Delay_S,0,sizeof(DELAY_STAT_T));
        }
    }
}

/*for flow control ,based on pps*/
void delay_flow_ctl(struct sk_buff *skb)
{
    static u32 total_pkt = 0;
    static u8 delay_cnt = 0;
    static s64 t_begin  =  0;
    s64 kt = 0;
    if (!IS_DELAY_SWITCH_AUTO) {
        return;
    }
    kt = ktime_to_us(ktime_get_real());
    if (0 == t_begin) {
        t_begin = kt;
    }
    total_pkt += 1;
    if ((kt - t_begin) > 1000000) {                                                /*statistics time is over 1s */ 
        if ((total_pkt > DELAY_FLOW_THRESHOLD &&   delayst_switch) ||
            (total_pkt < DELAY_FLOW_THRESHOLD &&  (!delayst_switch))) {      /*overflow/lowflow */
            delay_cnt++;
            if (delay_cnt >= 3) {                  /*overflow/lowflow for at least 3 times */
                delayst_switch = !delayst_switch;
                delay_cnt = 0;
            }
        } else {                                               /*not overflow/lowflow,count again*/
            delay_cnt = 0;
        }
        total_pkt = 0;
        t_begin =kt;
    }
    return ;
}

/* for time delay exception */
void delay_print_time_exception(struct sk_buff *skb, int t1, int t2)
{
    static u8 time_delay_exceed_count_tx = 0;
    static u8 time_delay_exceed_count_rx = 0;
    u64 uDelay = 0;
    static DELAYSKB_CB_T tx_buff;
    static DELAYSKB_CB_T rx_buff;
    DELAY_CHR_REPROT_T delay_excp_chr_report = {0};

    uDelay = skbprobe_get_latency(skb,t1,t2);
    if (IS_DIRECT(skb, TP_SKB_DIRECT_SND)) {
    if ((uDelay >  7*DELAY_NORMAL_TIME) && (uDelay < 100*DELAY_NORMAL_TIME)) {                                     /*bigger than 7 times normal delay*/
        time_delay_exceed_count_tx++;
        if (time_delay_exceed_count_tx >=10) {
            skbprobe_copy_delayskbcb(&tx_buff,skb);
        }
    } else {
        if (time_delay_exceed_count_tx >=10) {
            delay_excp_chr_report.data_direct = TP_SKB_DIRECT_SND;
            delay_excp_chr_report.exception_cnt = time_delay_exceed_count_tx;
            delay_excp_chr_report.driver_delay = skbprobe_get_latency_from_delayst(&tx_buff, TP_SKB_HMAC_TX, TP_SKB_DMAC);
            delay_excp_chr_report.hmac_delay = skbprobe_get_latency_from_delayst(&tx_buff, TP_SKB_HMAC_XMIT, TP_SKB_HMAC_TX);
            delay_excp_chr_report.ip_delay = skbprobe_get_latency_from_delayst(&tx_buff, TP_SKB_IP, TP_SKB_HMAC_XMIT);
            delay_excp_chr_report.transport_delay = skbprobe_get_latency_from_delayst(&tx_buff, TP_SKB_SEND, TP_SKB_IP);
            delay_excp_chr_report.android_uid = tx_buff.android_uid;
            wifi_kernel_delay_report(&delay_excp_chr_report);
            printk( "time_delay_exception! TX: exception_count = %d, uid = %u, transport_delay %u, ip_delay %u,hmac_delay %u,driver_delay %u,send_time:%lld,ip_time:%lld", 
                    time_delay_exceed_count_tx, tx_buff.android_uid, delay_excp_chr_report.transport_delay, delay_excp_chr_report.ip_delay, delay_excp_chr_report.hmac_delay,
                    delay_excp_chr_report.driver_delay,ktime_to_us(tx_buff.ptime[TP_SKB_SEND]),ktime_to_us(tx_buff.ptime[TP_SKB_IP])); 
        }
        time_delay_exceed_count_tx = 0;
    }
    } else {
        if ((uDelay > 7*DELAY_NORMAL_TIME) && (uDelay < 100*DELAY_NORMAL_TIME)) {
            time_delay_exceed_count_rx++;
            if(time_delay_exceed_count_rx >=10) {
                skbprobe_copy_delayskbcb(&rx_buff,skb);
            }
        } else {
            if (time_delay_exceed_count_rx >=10) {
                delay_excp_chr_report.data_direct = TP_SKB_DIRECT_RCV;
                delay_excp_chr_report.exception_cnt = time_delay_exceed_count_rx;
                delay_excp_chr_report.driver_delay = skbprobe_get_latency_from_delayst(&rx_buff, TP_SKB_DMAC, TP_SKB_HMAC_UPLOAD);
                delay_excp_chr_report.ip_delay = skbprobe_get_latency_from_delayst(&rx_buff, TP_SKB_HMAC_UPLOAD, TP_SKB_IP);
                delay_excp_chr_report.transport_delay = skbprobe_get_latency_from_delayst(&rx_buff, TP_SKB_IP, TP_SKB_RECV);
                delay_excp_chr_report.android_uid = rx_buff.android_uid;
                wifi_kernel_delay_report(&delay_excp_chr_report);
                printk("time_delay_exception! RX: exception_count = %d, uid = %u,  driver_delay  %u, ip_delay  %u,transport_delay %u,DMAC_time:%lld,UPLOAD_time:%lld",
                        time_delay_exceed_count_rx , rx_buff.android_uid,delay_excp_chr_report.driver_delay,delay_excp_chr_report.ip_delay,delay_excp_chr_report.transport_delay,
                        ktime_to_us(rx_buff.ptime[TP_SKB_DMAC]),ktime_to_us(rx_buff.ptime[TP_SKB_HMAC_UPLOAD]));
            }
            time_delay_exceed_count_rx = 0;
        }
    }
    return;
}


int delay_set_wifi_command(char *val)
{
    char *clone = NULL, *sub_cmd = NULL;
    int err = 0;
    unsigned int uid = 0, interval = 0;
    const char  flag_names[][20]    = {"flag_off", "flag_on", "flag_auto"};
    const char  mode_names[][20] = {"mode_stat", "mode_trace"};

    if (!val)
        return -EFAULT; /* Bad address */
    clone = kstrdup(val, GFP_USER);
    if (!clone) {
        return -EFAULT;
    }

    switch (clone[0])
    {
    case 'f':
        if (strcmp(clone,"flag_on") == 0) {
            Settings.dp_switch = flag_on;
            static_key_enable(&wifi_delay_statistic_key);
            delayst_switch = 1;
        } else if (strcmp(clone,"flag_off") == 0) {
            Settings.dp_switch = flag_off;
            static_key_disable(&wifi_delay_statistic_key);
            delayst_switch = 0;
        } else if (strcmp(clone,"flag_auto") == 0) {
            Settings.dp_switch = flag_auto;
            delayst_switch = 1;
        }
        break;
    case 'm':
        if (strcmp(clone,"mode_stat") == 0) {
            Settings.dp_mode = mode_stat;
        } else if(strcmp(clone,"mode_trace") == 0) {
            Settings.dp_mode = mode_trace;
        }
        break;
    case 'u':
        sub_cmd = strchr((const char *)clone, '_');
        if (NULL == sub_cmd) {
            return -EFAULT;
        }
        sub_cmd++;
        err = kstrtouint((const char *)sub_cmd, 10, &uid);
        if (!err) {
            Settings.android_uid = uid;
            CLEAN_DELAY_RECORD;
        }
        break;
    case 'p':
        sub_cmd = strchr((const char *)clone, '_');
        if (NULL == sub_cmd) {
            return -EFAULT;
        }
        sub_cmd++;
        err = kstrtouint((const char *)sub_cmd, 10, &interval);
        if (!err) {
            Settings.print_interval = interval;
            CLEAN_DELAY_RECORD;
        }
        break;
    case 'd':
        CLEAN_DELAY_RECORD;
        break;
    case 'c':
        printk("delay_command configure:delay_switch = %s,dealy_mode = %s,print_interval = %u,android_uid = %u",
                flag_names[Settings.dp_switch],mode_names[Settings.dp_mode],Settings.print_interval,Settings.android_uid);
        break;
    default:
        break;
    }
    if (clone)
        kfree(clone);
    return 0;
}

int proc_wifi_delay_command(struct ctl_table *ctl, int write, void __user *buffer,
                 size_t *lenp, loff_t *ppos)
{
    char val[DELAY_FILTER_NAME_MAX];
    struct ctl_table tbl = {
        .data = val,
        .maxlen = DELAY_FILTER_NAME_MAX,
    };
    int err = 0;
    if (write) {
        err = proc_dostring(&tbl, write, buffer, lenp, ppos);
        if (!err) {
            err = delay_set_wifi_command(val);
        }
    }
    return err;
}

