#include <net/sock.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/list.h>
#include "../../emcom_netlink.h"
#include "../../emcom_utils.h"
#include <huawei_platform/emcom/smartcare/fi/fi_utils.h>

static char hyxd_data[] = {0x00, 0x00, 0x00, 0x00, 0x51, 0x00, 0x40, 0x00};


uint32_t fi_get_srtt(uint32_t prev_srtt, uint32_t cur_rtt, int alpha)
{
	if (prev_srtt == 0)
	{
		prev_srtt = cur_rtt;
	}

	return (uint32_t)(((FI_PERCENT - alpha) * prev_srtt +
	                   (alpha * cur_rtt)) / FI_PERCENT);
}


void fi_rtt_send(uint32_t appid)
{
	fi_report_rtt report = {0};
	fi_app_info *appinfo = g_fi_ctx.appinfo + appid;
	fi_gamectx *gamectx = g_fi_ctx.gamectx + appid;

	/* ��û���rtt ���� ��Ϸ���˳� */
	if (!(gamectx->final_rtt) || !(appinfo->valid))
	{
		return;
	}

	/* �ϱ�rtt��native */
	if ((appinfo->switchs & FI_SWITCH_RTT) != 0)
	{
		report.uid = appinfo->uid;
		report.rtt = gamectx->final_rtt;
		report.apptype = FI_APP_TYPE_GAME;

		emcom_send_msg2daemon(NETLINK_EMCOM_KD_SMARTCARE_FI_REPORT_RTT, &report, sizeof(report));

		FI_LOGD(" : FI send rtt to daemon, cmd=%d, rtt=%u, uid=%u."
		        " (btrtt=%d, btsrtt=%d, hbrtt=%d, hbsrtt=%d)",
				NETLINK_EMCOM_KD_SMARTCARE_FI_REPORT_RTT, report.rtt, report.uid,
				gamectx->rtt, gamectx->battlertt, gamectx->hbrtt, gamectx->hbsrtt);
	}

	return;
}


void fi_rtt_status(uint32_t appid, uint32_t status)
{
	fi_app_info *appinfo;
	fi_report_status report = {0};

	/* �ϱ���Ϸ״̬ */
	appinfo = g_fi_ctx.appinfo + appid;
	if ((appinfo->switchs & FI_SWITCH_STATUS) != 0)
	{
		report.uid = appinfo->uid;
		report.status = status;
		report.apptype = FI_APP_TYPE_GAME;

		emcom_send_msg2daemon(NETLINK_EMCOM_KD_SMARTCARE_FI_REPORT_APP_STATUS, &report, sizeof(report));

		FI_LOGD(" : FI send status to daemon, appid=%u, uid=%u, status=%u.", appid, report.uid, status);
	}

	return;
}


void fi_reset_gamectx(fi_gamectx *gamectx)
{
	fi_gamectx tmpctx;
	fi_mpctx *mpctx;

	mpctx = g_fi_ctx.mpctx + gamectx->appid;

	memcpy(&tmpctx, gamectx, sizeof(fi_gamectx));
	memset(gamectx, 0, sizeof(fi_gamectx));
	memset(mpctx, 0, sizeof(fi_mpctx));

	/* ��������ֵ */
	gamectx->appid = tmpctx.appid;
	gamectx->battle_flow_port = tmpctx.battle_flow_port;
	gamectx->cliip = tmpctx.cliip;
	gamectx->hb_flow_port = tmpctx.hb_flow_port;
}


void fi_rtt_battle_pktnum_cal(fi_gamectx *gamectx, fi_app_info *appinfo)
{
	/* ����Ϸ�ں�̨�ڼ䣬��ͳ������û�б��ĵ�ʱ�� */
	if (appinfo->appstatus != GAME_SDK_STATE_BACKGROUND)
	{
		/* ��¼û�����б�����������ʱ�� */
		if (gamectx->uplinkpktnum == 0)
		{
			gamectx->nouplinkpkt++;
		}
		else
		{
			gamectx->nouplinkpkt = 0;
		}

		/* ��¼û�����б�����������ʱ�� */
		if (gamectx->downlinkpktnum == 0)
		{
			gamectx->nodownlinkpkt++;
		}
		else
		{
			gamectx->nodownlinkpkt = 0;
		}
	}

	/* ͨ��pre***pktnum���ж���Ϸ״̬����Ϊ�����һ����ı������� */
	gamectx->preuplinkpktnum = gamectx->uplinkpktnum;
	gamectx->predownlinkpktnum = gamectx->downlinkpktnum;
	gamectx->uplinkpktnum = 0;
	gamectx->downlinkpktnum = 0;
}


void fi_rtt_timer(void)
{
	int i;
	fi_gamectx  *gamectx;
	fi_app_info *appinfo;

	for (i = 0; i < FI_APPID_MAX; i++)
	{
		gamectx = g_fi_ctx.gamectx + i;
		if (!FI_APPID_VALID(gamectx->appid))
		{
			continue;
		}

		/* �õ�app������, validΪ��˵����Ϸδ���� */
		appinfo = g_fi_ctx.appinfo + gamectx->appid;
		if (appinfo->valid == FI_FALSE)
		{
			continue;
		}

		/* �����ս��������һ������� */
		fi_rtt_battle_pktnum_cal(gamectx, appinfo);

		FI_LOGD(" : FI timer appid=%d, uplinkpps=%u, nouplinkpkt=%u,"
		        " downlinkpps=%u, nodownlinkpkt=%u.",
		        i, gamectx->preuplinkpktnum, gamectx->nouplinkpkt,
		        gamectx->predownlinkpktnum, gamectx->nodownlinkpkt);

		/* ��Ϸ��û���rtt���߲��ڶ�ս�� */
		if (!(gamectx->updatetime) || !FI_BATTLING(gamectx->appstatus))
		{
			continue;
		}

		/* �ж���ս�Ƿ���� */
		if (fi_rtt_judge_battle_stop(gamectx, appinfo) == FI_TRUE)
		{
			continue;
		}

		/* �ϱ�rtt */
		fi_rtt_send(gamectx->appid);
	}

	return;
}


uint32_t fi_rtt_get_le_u32(uint8_t *data)
{
	uint32_t value = 0;

	value = data[3];
	value = (value << 8) + data[2];
	value = (value << 8) + data[1];
	value = (value << 8) + data[0];

	return value;
}


uint32_t fi_rtt_get_be_u32(uint8_t *data)
{
	uint32_t value = 0;

	value = data[0];
	value = (value << 8) + data[1];
	value = (value << 8) + data[2];
	value = (value << 8) + data[3];

	return value;
}


void fi_rtt_update_integral(fi_gamectx *gamectx, int32_t newrtt, int64_t curms)
{
	int64_t cursec = FI_MS2SEC(curms);
	int32_t avgrtt;

	if (curms <= gamectx->updatetime)
	{
		return;
	}

	if (cursec == FI_MS2SEC(gamectx->updatetime))
	{
		gamectx->rtt_integral += gamectx->rtt * (curms - gamectx->updatetime);
	}
	else
	{
		gamectx->rtt_integral += gamectx->rtt * (cursec * FI_MS - gamectx->updatetime);
		avgrtt = gamectx->rtt_integral / FI_MS / (cursec - FI_MS2SEC(gamectx->updatetime));

		/* ���б��ĺ��ٵĳ���: ���ֶ�ս֮��, ��ɫ����, ��Ϸ�ں�̨ */
		/* ���б��ĺ���ʱ���б���ͨ��Ҳ����, RTT����Ļ᲻׼, ������ʱ������RTT */
		if ((gamectx->preuplinkpktnum >= FI_BATTLE_ONGOING) || (avgrtt < gamectx->battlertt))
		{
			gamectx->srtt = fi_get_srtt(gamectx->srtt, FI_MIN(avgrtt, FI_MAX_ORG_RTT), FI_SRTT_VAR);
			gamectx->battlertt = FI_MIN(gamectx->srtt, FI_MAX_RTT);
		}

		gamectx->rtt_integral = gamectx->rtt * (curms - cursec * FI_MS);
	}

	gamectx->updatetime = curms;
	gamectx->rtt = FI_MIN(newrtt, FI_MAX_ORG_RTT);

	return;
}


void fi_rtt_reset_study(fi_pkt *pktinfo, fi_flowctx *flowctx)
{
	flowctx->studystarttime = pktinfo->msec;
	flowctx->uppktnum = 0;
	flowctx->downpktnum = 0;
	flowctx->uppktbytes = 0;
}


int fi_rtt_study_by_port(fi_pkt *pktinfo, fi_flowctx *flowctx, fi_gamectx *gamectx)
{
	uint16_t srvport = FI_SRVPORT(pktinfo);

	if (srvport == gamectx->battle_flow_port)
	{
		flowctx->flowtype = FI_FLOWTYPE_BATTLE;
		FI_LOGD(" : FI learned a battle flow %u,%u by port %u.",
		        pktinfo->sport, pktinfo->dport, gamectx->battle_flow_port);

		/* �����µĶ�ս�����ж��Ƿ�Ϊ�������� */
		fi_rtt_judge_reconn(pktinfo, gamectx);
		gamectx->cliip = FI_CLIIP(pktinfo);

		return FI_SUCCESS;
	}
	else if (srvport == gamectx->hb_flow_port)
	{
		flowctx->flowtype = FI_FLOWTYPE_HB;
		FI_LOGD(" : FI learned a heartbeat flow %u,%u by port %u.",
		        pktinfo->sport, pktinfo->dport, gamectx->hb_flow_port);
		return FI_SUCCESS;
	}
	else
	{
		return FI_FAILURE;
	}

	return FI_FAILURE;
}


void fi_rtt_judge_reconn(fi_pkt *pktinfo, fi_gamectx *gamectx)
{
	fi_app_info *appinfo = g_fi_ctx.appinfo + gamectx->appid;
	int reconn = FI_FALSE;

	FI_LOGD(" : FI nouplinkpkt=%u, nodownlinkpkt=%u, appstatus=%u, cliip=%08x.",
		    gamectx->nouplinkpkt, gamectx->nodownlinkpkt,
		    gamectx->appstatus, gamectx->cliip);

	/* ���ڶ�ս�в����������� */
	if (!FI_BATTLING(gamectx->appstatus))
	{
		return;
	}

	/* ��ֹ����Ƶ���ı��������� */
	if (pktinfo->msec - gamectx->reconntime < FI_RECONN_LIMIT)
	{
		return;
	}

	do
	{
		/* �������wifi-lte��·�л����򲻹���ǰ̨���Ǻ�̨������������ */
		if ((gamectx->cliip != FI_CLIIP(pktinfo)) || (gamectx->final_rtt > FI_RECONN_THRESH))
		{
			reconn = FI_TRUE;
			break;
		}

		/* ��Ϸ���ں�̨����������������ȴû�����б��ģ�����Ϊ�Ƕ������� */
		if ((appinfo->appstatus != GAME_SDK_STATE_BACKGROUND) &&
		    (gamectx->nouplinkpkt < FI_NO_DOWNLINK_TIME) &&
		    (gamectx->nodownlinkpkt >= FI_NO_DOWNLINK_TIME))
		{
			reconn = FI_TRUE;
			break;
		}
	} while (0);

	if (reconn)
	{
		fi_rtt_status(gamectx->appid, FI_STATUS_BATTLE_RECONN);
		gamectx->reconntime = pktinfo->msec;
	}

	return;
}


void fi_rtt_judge_qqfc(fi_pkt *pktinfo, fi_flowctx *flowctx, fi_gamectx *gamectx)
{
	int64_t  timediff;      /* msec */
	uint16_t up_pps;

	timediff = FI_MAX(pktinfo->msec - flowctx->studystarttime, 1);

	up_pps = flowctx->uppktnum * FI_MS / timediff;
	FI_LOGD(" : FI judge qqfc, up: %u/s, down: %u/s, flow: %u,%u, times: %u",
	        up_pps, flowctx->downpktnum, pktinfo->sport, pktinfo->dport, flowctx->btflow_times);

	/* ʶ��QQ�ɳ��Ķ�ս�� */
	if ((up_pps >= FI_QQFC_UP_PPS_MIN) && (up_pps <= FI_QQFC_UP_PPS_MAX))
	{
		flowctx->btflow_times++;
	}
	else
	{
		flowctx->btflow_times = 0;
	}

	/* ���ö�ս����� */
	if (flowctx->btflow_times >= FI_BATTLE_FLOW_CON)
	{
		flowctx->flowtype = FI_FLOWTYPE_BATTLE;
		gamectx->battle_flow_port = FI_SRVPORT(pktinfo);
		gamectx->cliip = FI_CLIIP(pktinfo);
		FI_LOGD(" : FI learned qqfc battle flow %u,%u, battle port %u.",
			    pktinfo->sport, pktinfo->dport, gamectx->battle_flow_port);
	}

	return;
}


int fi_rtt_judge_hb(fi_pkt *pktinfo, fi_flowctx *flowctx, fi_gamectx *gamectx)
{
	uint32_t timediff;      /* msec->sec */
	int ret = FI_FAILURE;

	/* ��һ������, ��¼��ʼѧϰ��ʱ��󷵻� */
	if (flowctx->hbstudytime == 0)
	{
		flowctx->hbstudytime = (uint32_t)(pktinfo->msec);
		return FI_FAILURE;
	}

	/* �ռ�20������֮��ʼ�ж��Ƿ�Ϊ������ */
	if (flowctx->hbdownnum < FI_HB_STUDY_NUM)
	{
		return FI_FAILURE;
	}

	timediff = ((uint32_t)(pktinfo->msec) - flowctx->hbstudytime) / FI_MS;

	FI_LOGD(" : FI judge hb, hbupnum: %u, hbdownnum: %u, time: %u, flow: %u,%u",
	        flowctx->hbupnum, flowctx->hbdownnum, timediff, pktinfo->sport, pktinfo->dport);

	/* ���������ж�������: �����б��ĸ�����ֵ, pps, ���ļ�� */
	if ((FI_ABS_SUB(flowctx->hbupnum, flowctx->hbdownnum) < FI_HB_PKT_SUB) &&
	    (flowctx->hbupnum <= (FI_HB_MAX_PPS * timediff)) &&
	    (timediff <= (FI_HB_MAX_TIME_GAP * flowctx->hbupnum)))
	{
		/* ���������Ϊ������ */
		flowctx->flowtype = FI_FLOWTYPE_HB;
		gamectx->hb_flow_port = FI_SRVPORT(pktinfo);
		FI_LOGD(" : FI learned a heartbeat flow %u,%u, heartbeat port %u.",
		        pktinfo->sport, pktinfo->dport, gamectx->hb_flow_port);
		ret = FI_SUCCESS;
	}
	/* �ú����������������У�飬�������������� */
	else
	{
		flowctx->flowtype = FI_FLOWTYPE_INIT;
		gamectx->hb_flow_port = 0;
		FI_LOGD(" : FI heartbeat flow verification failed, flow: %u,%u.",
		        pktinfo->sport, pktinfo->dport);
	}

	/* һ��ͳ�ƽ���֮�����ͳ����Ϣ */
	flowctx->hbstudytime = 0;
	flowctx->hbupnum = 0;
	flowctx->hbdownnum = 0;

	return ret;
}


void fi_rtt_judge_battle_flow(fi_pkt *pktinfo, fi_flowctx *flowctx, fi_gamectx *gamectx)
{
	int64_t  timediff;      /* msec */
	uint32_t avgpktlen;
	uint16_t uppktpersec;
	uint16_t downpktpersec;

	timediff = FI_MAX(pktinfo->msec - flowctx->studystarttime, 1);

	uppktpersec = flowctx->uppktnum * FI_MS / timediff;
	downpktpersec = flowctx->downpktnum * FI_MS / timediff;
	avgpktlen = flowctx->uppktbytes / FI_MAX(flowctx->uppktnum, 1);

	FI_LOGD(" : FI judge battle: up: %u/s, down: %u/s, avg_pkt_len: %u, times: %u, flow: %u,%u",
	        uppktpersec, downpktpersec, avgpktlen, flowctx->btflow_times, pktinfo->sport, pktinfo->dport);

	if ((uppktpersec >= FI_BATTLE_UP_PKT_PER_SEC) &&
	    (downpktpersec >= FI_BATTLE_DOWN_PKT_PER_SEC) &&
	    (avgpktlen < FI_AUDIO_BATTLE_BOUNDARY))
	{
		flowctx->btflow_times++;
	}
	else
	{
		flowctx->btflow_times = 0;
	}

	/* ���������Ϊ��ս�� */
	if (flowctx->btflow_times >= FI_BATTLE_FLOW_CON)
	{
		flowctx->flowtype = FI_FLOWTYPE_BATTLE;
		gamectx->battle_flow_port = FI_SRVPORT(pktinfo);
		gamectx->cliip = FI_CLIIP(pktinfo);
		FI_LOGD(" : FI learned a battle flow %u,%u, battle port %u.",
			    pktinfo->sport, pktinfo->dport, gamectx->battle_flow_port);
	}

	return;
}


void fi_rtt_judge_battle_start(fi_pkt *pktinfo, fi_flowctx *flowctx, fi_gamectx *gamectx)
{
	int64_t  timediff;      /* msec */
	uint16_t uppktpersec;
	uint16_t downpktpersec;

	timediff = FI_MAX(pktinfo->msec - flowctx->studystarttime, 1);
	uppktpersec = flowctx->uppktnum * FI_MS / timediff;
	downpktpersec = flowctx->downpktnum * FI_MS / timediff;

	/* ��Ҫ���Ƕ�ս�м���Ϣ��״̬ */
	if ((FI_MIN(uppktpersec, downpktpersec) >= FI_BATTLE_UP_PKT_PER_SEC) &&
	    (FI_MAX(uppktpersec, downpktpersec) >= FI_BATTLE_DOWN_PKT_PER_SEC))
	{
		flowctx->battle_times++;
	}
	else
	{
		flowctx->battle_times = 0;
	}

	/* ������������������ж�Ϊ��ս��ʼ */
	if (flowctx->battle_times >= FI_BATTLE_START_THRESH)
	{
		gamectx->appstatus |= FI_STATUS_BATTLE_START;
		FI_LOGD(" : FI learned battle start status by flow %u,%u.",
		        pktinfo->sport, pktinfo->dport);

		/* �ϱ���Ϸ״̬: ��ս��ʼ */
		fi_rtt_status(gamectx->appid, FI_STATUS_BATTLE_START);
	}

	return;
}


int fi_rtt_judge_battle_stop(fi_gamectx *gamectx, fi_app_info *appinfo)
{
	/* �����Ϸ�ں�̨���򲻱���ս������ֱ����Ϸ��ɱ�����ߵ���ǰ̨ */
	if (appinfo->appstatus == GAME_SDK_STATE_BACKGROUND)
	{
		return FI_FALSE;
	}

	/* QQ�ɳ�ʹ�õ������߼� */
	if (gamectx->appid == FI_APPID_QQFC)
	{
		/* �ж�qqfc�Ƿ����: ����n�������е�û���� */
		if (gamectx->nodownlinkpkt > (gamectx->nouplinkpkt + FI_APP_DISCONN))
		{
			gamectx->applocalstatus |= FI_STATUS_DISCONN;
			FI_LOGD(" : FI disconn, nouplinkpkt=%u, nodownlinkpkt=%u.",
				    gamectx->nouplinkpkt, gamectx->nodownlinkpkt);
		}

		/* ���ִ�������˵���������״̬ */
		if (gamectx->preuplinkpktnum >= FI_QQFC_UP_PPS_MIN)
		{
			gamectx->applocalstatus = 0;
			FI_LOGD(" : FI reconn, preuplinkpktnum=%u, predownlinkpktnum=%u.",
				    gamectx->preuplinkpktnum, gamectx->predownlinkpktnum);
		}

		/* ���ߵ�ʱ��С���趨����ֵ��������ս���� */
		if (FI_DISCONN(gamectx->applocalstatus) &&
		    (gamectx->nouplinkpkt < FI_APP_DISCONN_STOP))
	    {
			return FI_FALSE;
	    }
	}

	/* ��ս������һ��ʱ��û�������б�������Ϊ��ս���� */
	if ((gamectx->nouplinkpkt >= FI_BATTLE_STOP_THRESH) &&
	    (gamectx->nodownlinkpkt >= FI_BATTLE_STOP_THRESH))
	{
		/* ���Ͷ�ս������Ϣ */
		fi_rtt_status(gamectx->appid, FI_STATUS_BATTLE_STOP);

		/* ������ڼ���rtt�Ļ���, ������Ϸ״̬Ϊ��սδ��ʼ */
		fi_flow_lock();
		fi_reset_gamectx(gamectx);
		fi_flow_unlock();

		return FI_TRUE;
	}

	return FI_FALSE;
}


void fi_rtt_judge_flow_type(fi_pkt *pktinfo, fi_flowctx *flowctx, fi_gamectx *gamectx)
{
	do
	{
		/* ͨ����ʷ�˿ڽ����������ж� */
		if (fi_rtt_study_by_port(pktinfo, flowctx, gamectx) == FI_SUCCESS)
		{
			break;
		}

		/* ̫��ʱ��ûѧ�����������������Ϊunknown */
		if (pktinfo->msec - flowctx->flowstarttime > FI_MAX_STUDY_TIME)
		{
			flowctx->flowtype = FI_FLOWTYPE_UNKNOWN;
			break;
		}

		switch (gamectx->appid)
		{
			/* QQ�ɳ�ʹ�õ�����ѧϰ���� */
			case FI_APPID_QQFC:
			{
				fi_rtt_judge_qqfc(pktinfo, flowctx, gamectx);
				break;
			}
			case FI_APPID_HYXD:
			case FI_APPID_BH3:
			{
				/* do nothing */
				break;
			}
			default:
			{
				/* �ж��Ƿ�Ϊ��ս�� */
				fi_rtt_judge_battle_flow(pktinfo, flowctx, gamectx);
				break;
			}
		}
	} while (0);

	return;
}


void fi_rtt_flow_study(fi_pkt *pktinfo, fi_flowctx *flowctx, fi_gamectx *gamectx)
{
	/* ��������ѧϰ����Ҫ��tcp */
	if (pktinfo->proto == FI_IPPROTO_TCP)
	{
		return;
	}

	/* �ռ�ͳ����Ϣ */
	if (pktinfo->dir == FI_DIR_UP)
	{
		flowctx->hbupnum++;
		flowctx->uppktnum++;
		flowctx->uppktbytes += pktinfo->len;
	}
	else
	{
		/* �����������п��ܲ������������ģ��������һ�� */
		if (pktinfo->len < FI_HB_PKT_LEN)
		{
			flowctx->hbdownnum++;
		}
		flowctx->downpktnum++;
	}

	/* �ж��Ƿ�Ϊ������ */
	if (flowctx->flowtype == FI_FLOWTYPE_INIT)
	{
		if (fi_rtt_judge_hb(pktinfo, flowctx, gamectx) == FI_SUCCESS)
		{
			/* ���Ϊ�������Ͳ�����ս�����ж� */
			return;
		}
	}

	/* ��ս��ÿ���n�����һ���ж�, ����n��ֱ�ӷ��� */
	if (pktinfo->msec - flowctx->studystarttime < FI_STUDY_INTERVAL)
	{
		return;
	}

	/* �ж������� */
	if (flowctx->flowtype == FI_FLOWTYPE_INIT)
	{
		fi_rtt_judge_flow_type(pktinfo, flowctx, gamectx);
	}

	/* �ж϶�ս�Ƿ�ʼ */
	if (!FI_BATTLING(gamectx->appstatus))
	{
		/* QQ�ɳ�������3 ʶ�����ս����ζ�Ŷ�ս��ʼ */
		if ((gamectx->appid == FI_APPID_QQFC) ||
		    (gamectx->appid == FI_APPID_BH3))
		{
			if (flowctx->flowtype == FI_FLOWTYPE_BATTLE)
			{
				gamectx->appstatus |= FI_STATUS_BATTLE_START;
				FI_LOGD(" : FI learned battle start status by battle flow %u,%u.",
						pktinfo->sport, pktinfo->dport);

				/* �ϱ���Ϸ״̬: ��ս��ʼ */
				fi_rtt_status(gamectx->appid, FI_STATUS_BATTLE_START);
			}
		}
		else
		{
			fi_rtt_judge_battle_start(pktinfo, flowctx, gamectx);
		}
	}

	/* ÿ���n�����ͳ����Ϣ */
	fi_rtt_reset_study(pktinfo, flowctx);

	return;
}


int fi_rtt_cal_tcprtt(fi_pkt *pktinfo, fi_flowctx *flowctx, fi_gamectx *gamectx)
{
	int32_t rawrtt = 0;   /* ����seq/ack�������ԭʼ��rtt */
	int32_t newrtt = 0;   /* ��rawrtt�Ļ����Ͼ���min max��������rtt */
	char *logstr = "";

	/* ���б��ģ�������¼seq */
	if (pktinfo->dir == FI_DIR_UP)
	{
		/* seq����0˵����һ�����������Ѿ�����, ���¿�ʼһ���µ����� */
		if (flowctx->seq == 0)
		{
			/* �и��صı��ĲŻ���ack */
			if (pktinfo->len > 0)
			{
				/* ��¼�����յ���ack�͵�ǰʱ��� */
				flowctx->seq = pktinfo->seq + pktinfo->len;
				flowctx->uppkttime = pktinfo->msec;
				FI_LOGD(" : FI save seq, seq=%u, nextseq=%u.",
				        pktinfo->seq, flowctx->seq);
			}
			return FI_SUCCESS;
		}

		/* �ܳ�ʱ��û���յ�ack���������б��ĸ���rtt */
		if (pktinfo->msec - gamectx->updatetime < FI_UPPKT_WAIT)
		{
			return FI_FAILURE;
		}

		/* ��׳�Կ���, ̫��ʱ��û���յ�ack, ����ѡȡһ������ */
		if ((flowctx->uppkttime > 0) &&
		    (pktinfo->msec - flowctx->uppkttime > FI_ACK_MAX_WAIT))
		{
			FI_LOGD(" : FI reset tcprtt ctx.");
			flowctx->seq = 0;
			flowctx->uppkttime = 0;
			return FI_FAILURE;
		}

		/* ��ʱ��û���յ�ack, ���rtt�������� */
		newrtt = FI_RANGE(pktinfo->msec - flowctx->uppkttime, FI_MIN_RTT, FI_MAX_ORG_RTT);
		logstr = "uplink pkt";
	}
	else
	{
		/* �ж��Ƿ�Ϊ��Ч��ack */
		if (!(flowctx->uppkttime) || (pktinfo->ack - flowctx->seq > (uint32_t)FI_ACK_MAX))
		{
			return FI_FAILURE;
		}

		/* ����ʱ���ֵ����ԭʼrtt */
		rawrtt = pktinfo->msec - flowctx->uppkttime;
		gamectx->rawrtt = fi_get_srtt(gamectx->rawrtt, rawrtt, FI_SRTT_VAR);
		newrtt = FI_RANGE(rawrtt, FI_MIN_RTT, FI_MAX_ORG_RTT);

		FI_LOGD(" : FI get ack, appid=%u, rawrtt=%d, seq=%u, ack=%u, flow %u,%u.",
		        gamectx->appid, rawrtt, flowctx->seq, pktinfo->ack,
		        pktinfo->sport, pktinfo->dport);

		/* ��һ��rtt�����ѽ���������״̬��׼����һ�� */
		flowctx->seq = 0;
		flowctx->uppkttime = 0;
		logstr = "ack";
	}

	/* ��������rtt����Ҫ��ԭʼrtt����ƽ�� */
	fi_rtt_update_integral(gamectx, newrtt, pktinfo->msec);

	FI_LOGD(" : FI update rtt by tcp %s, appid=%u, newrtt=%d, battlertt=%d.",
			logstr, gamectx->appid, newrtt, gamectx->battlertt);

	return FI_SUCCESS;
}


int fi_rtt_cal_mptcp(fi_pkt *pktinfo, fi_flowctx *flowctx, fi_gamectx *gamectx)
{
	int32_t rawrtt = 0;   /* ����seq/ack�������ԭʼ��rtt */
	int32_t newrtt = 0;   /* ��rawrtt�Ļ����Ͼ���min max��������rtt */
	fi_mpctx *mpctx = g_fi_ctx.mpctx + gamectx->appid;

	/* ���б��ģ�������¼seq */
	if (pktinfo->dir == FI_DIR_UP)
	{
		/* ���µ�ǰseq�Ѿ��������ˣ�������ش����ļ���rtt */
		mpctx->preseq = FI_MAX(mpctx->preseq, pktinfo->seq);

		/* seq����0˵����һ�����������Ѿ�����, ���¿�ʼһ���µ����� */
		if ((mpctx->seq == 0) && (pktinfo->len > 0) &&
		    ((pktinfo->seq >= mpctx->preseq) ||
		    (FI_ABS_SUB(pktinfo->seq, mpctx->preseq) > FI_RETRANS_PKT)))
		{
			/* ��¼�����յ���ack�͵�ǰʱ��� */
			mpctx->seq = pktinfo->seq + pktinfo->len;
			mpctx->uppkttime = pktinfo->msec;
			mpctx->preseq = mpctx->seq;
			FI_LOGD(" : FI save seq, seq=%u, nextseq=%u, preseq=%u.",
			        pktinfo->seq, mpctx->seq, mpctx->preseq);
		}

		/* ��׳�Կ���, ̫��ʱ��û���յ�ack, ����ѡȡһ������ */
		if ((mpctx->uppkttime > 0) &&
		    (pktinfo->msec - mpctx->uppkttime > FI_ACK_MAX_WAIT))
		{
			FI_LOGD(" : FI reset mptcp ctx.");
			memset(mpctx, 0, sizeof(fi_mpctx));
		}

		return FI_SUCCESS;
	}
	else
	{
		/* ���µ�ǰseq�Ѿ��������ˣ�������ش����ļ���rtt */
		mpctx->preseq = FI_MAX(mpctx->preseq, pktinfo->ack);

		/* �ж��Ƿ�Ϊ��Ч��ack */
		if (!(mpctx->uppkttime) || (pktinfo->ack - mpctx->seq > (uint32_t)FI_ACK_MAX))
		{
			return FI_FAILURE;
		}

		/* ����ʱ���ֵ����ԭʼrtt */
		rawrtt = pktinfo->msec - mpctx->uppkttime;
		gamectx->rawrtt = fi_get_srtt(gamectx->rawrtt, rawrtt, FI_SRTT_VAR);
		newrtt = FI_RANGE(rawrtt, FI_MIN_RTT, FI_MAX_ORG_RTT);

		FI_LOGD(" : FI get ack, rawrtt=%d, seq=%u, ack=%u, flow %u,%u.",
		        rawrtt, mpctx->seq, pktinfo->ack, pktinfo->sport, pktinfo->dport);

		/* ��һ��rtt�����ѽ���������״̬��׼����һ�� */
		mpctx->seq = 0;
		mpctx->uppkttime = 0;
	}

	/* ��������rtt����Ҫ��ԭʼrtt����ƽ�� */
	fi_rtt_update_integral(gamectx, newrtt, pktinfo->msec);

	FI_LOGD(" : FI update rtt by mptcp, appid=%u, newrtt=%d, battlertt=%d.",
			gamectx->appid, newrtt, gamectx->battlertt);

	return FI_SUCCESS;
}


int fi_rtt_cal_hyxd(fi_pkt *pktinfo, fi_flowctx *flowctx, fi_gamectx *gamectx)
{
	int32_t newrtt = 0;

	/* ��Ҫ���ʸ��أ�������bufdatalen */
	if (pktinfo->bufdatalen < FI_HYXD_MIN_LEN)
	{
		return FI_FAILURE;
	}

	/* ���б��ģ�ȡ��seq��¼���� */
	if (pktinfo->dir == FI_DIR_UP)
	{
		/* ɸѡЯ�����ݵ����б���(�Ǵ�ack����) */
		if (memcmp(pktinfo->data, hyxd_data, sizeof(hyxd_data)))
		{
			return FI_FAILURE;
		}

		/* ���϶�ս����ǣ��Ժ�ͨ���ñ�ǹ��˱��� */
		flowctx->flowtype = FI_FLOWTYPE_BATTLE;

		/* ȡ��seq�ţ�ֻ��¼һ��seq�� */
		if (flowctx->seq == 0)
		{
			flowctx->seq = fi_rtt_get_le_u32(pktinfo->data + FI_HYXD_SEQ_OFFSET);
			flowctx->uppkttime = pktinfo->msec;
		}

		flowctx->uppktnum++;

		/* ����ʱ��û�����б���ʱ�������б���������rtt */
		if ((pktinfo->msec - flowctx->uppkttime < FI_UPPKT_WAIT) ||
		    (flowctx->uppktnum <= FI_HB_LOST_TOLERANCE))
		{
			return FI_FAILURE;
		}

		newrtt = FI_RANGE(pktinfo->msec - flowctx->uppkttime, FI_MIN_RTT, FI_MAX_ORG_RTT);
	}
	else
	{
		uint32_t ack;

		/* û���Ѽ�¼��seq, ���޷����seq - ack */
		if (flowctx->seq == 0)
		{
			return FI_FAILURE;
		}

		/* ȡ��ack�� */
		ack = fi_rtt_get_le_u32(pktinfo->data + FI_HYXD_ACK_OFFSET);
		if (ack - flowctx->seq > (uint32_t)FI_ACK_MAX)
		{
			return FI_FAILURE;
		}

		/* ����ԭʼrtt */
		newrtt = FI_RANGE(pktinfo->msec - flowctx->uppkttime, FI_MIN_RTT, FI_MAX_ORG_RTT);

		/* ��һ��rtt�����ѽ���������״̬��׼����һ�� */
		flowctx->seq = 0;
		flowctx->uppkttime = 0;
		flowctx->uppktnum = 0;
	}

	/* ��������rtt����Ҫ��ԭʼrtt����ƽ�� */
	fi_rtt_update_integral(gamectx, newrtt, pktinfo->msec);

	FI_LOGD(" : FI update hyxd rtt, appid=%u, rtt=%d.",
	        gamectx->appid, gamectx->battlertt);

	return FI_SUCCESS;
}


int fi_rtt_cal_battle(fi_pkt *pktinfo, fi_flowctx *flowctx, fi_gamectx *gamectx)
{
	char *logstr = "";

	/* ���б��� */
	if (pktinfo->dir == FI_DIR_UP)
	{
		/* Wait for downlink data to start. */
		if (gamectx->downlast == 0)
		{
			return FI_FAILURE;
		}

		/* Use uplink pkt as a timer */
		if (pktinfo->msec - gamectx->downlast < FI_UPPKT_WAIT)
		{
			return FI_FAILURE;
		}

		/* For too long, there is no uplink pkt, update RTT. */
		gamectx->rtt = FI_RANGE(pktinfo->msec - gamectx->downlast,
		                        FI_MIN_RTT, FI_MAX_ORG_RTT);
		logstr = "uplink pkt";
	}
	else
	{
		/* Save the timestamp of the first downlink pkt. */
		if (gamectx->downlast == 0)
		{
			gamectx->downlast = pktinfo->msec;
			return FI_FAILURE;
		}

		/* Calculate RTT by adjacent downlink pkts */
		gamectx->rtt = FI_RANGE((pktinfo->msec - gamectx->downlast),
		                         FI_MIN_RTT, FI_MAX_ORG_RTT);
		gamectx->downlast = pktinfo->msec;
		logstr = "downlink pkt";
	}

	/* ��������rtt����Ҫ��ԭʼrtt����ƽ�� */
	fi_rtt_update_integral(gamectx, gamectx->rtt, pktinfo->msec);

	FI_LOGD(" : FI update rtt by battle flow %s, appid=%u, rtt=%d.",
	        logstr, gamectx->appid, gamectx->battlertt);

	return FI_SUCCESS;
}


void fi_rtt_cal_hb(fi_pkt *pktinfo, fi_flowctx *flowctx, fi_gamectx *gamectx)
{
	/* ���б��ļ���ʱ��� */
	if (pktinfo->dir == FI_DIR_UP)
	{
		/* ͳ�����������б��ĵĸ��� */
		flowctx->hbupnum++;

		/* һ���������б�����, �������һ�����б��ĵ�ʱ�� */
		flowctx->uppkttime = pktinfo->msec;
	}
	/* ���б��ĸ���ʱ����Ĳ�ֵ����rtt */
	else
	{
		flowctx->hbdownnum++;

		/* ǰ��û���������� */
		if (flowctx->uppkttime == 0)
		{
			return;
		}

		/* �յ�������Ӧ */
		gamectx->hbrtt = FI_RANGE(pktinfo->msec - flowctx->uppkttime, FI_MIN_RTT, FI_MAX_RTT);
		gamectx->hbsrtt = fi_get_srtt(gamectx->hbsrtt, gamectx->hbrtt, FI_HB_SRTT_VAR);
		FI_LOGD(" : FI update rtt by heartbeat, appid=%u, hbrtt=%d, hbsrtt=%d.",
				gamectx->appid, gamectx->hbrtt, gamectx->hbsrtt);

		/* ���ּ������ */
		flowctx->uppkttime = 0;
	}

	/* У���������Ƿ���ȷ */
	fi_rtt_judge_hb(pktinfo, flowctx, gamectx);
	if (flowctx->flowtype != FI_FLOWTYPE_HB)
	{
		gamectx->hbrtt= 0;
		gamectx->hbsrtt = 0;
	}

	return;
}


void fi_rtt_cal_uu(fi_pkt *pktinfo, fi_flowctx *flowctx, fi_gamectx *gamectx)
{
	static uint32_t uu_seq[FI_UU_CACHE_NUM] = {0};
	static int64_t  uu_time[FI_UU_CACHE_NUM] = {0};
	static uint32_t curcache = 0;
	uint32_t i;
	int32_t newrtt = 0;

	/* UUֻʹ��tcp���� */
	if (pktinfo->proto != FI_IPPROTO_TCP)
	{
		return;
	}

	/* UU��������ͬ��tcprtt���ǣ�����ʹ�ø���Ϊ0�ı��� */
	if (pktinfo->len == 0)
	{
		return;
	}

	/* ���б��ģ�������¼seq��ʱ��� */
	if (pktinfo->dir == FI_DIR_UP)
	{
		uu_seq[curcache] = pktinfo->seq + pktinfo->len;
		uu_time[curcache] = pktinfo->msec;
		curcache = (curcache + 1) % FI_UU_CACHE_NUM;
		return;
	}

	/* ʣ�µľ������б��ģ��������л��棬����rtt */
	for (i = 0; i < FI_UU_CACHE_NUM; i++)
	{
		/* ��Ч�Ļ���, ��Ч��ack */
		if (uu_seq[curcache] && (pktinfo->ack - uu_seq[curcache] < (uint32_t)FI_ACK_MAX))
		{
			/* ���һ����ĵ�rtt�϶���֮ǰ��С������֮ǰ�� */
			newrtt = FI_RANGE(pktinfo->msec - uu_time[curcache], FI_MIN_RTT, FI_MAX_ORG_RTT);
			uu_seq[curcache] = 0;
			uu_time[curcache] = 0;
		}

		curcache = (curcache + 1) % FI_UU_CACHE_NUM;
	}

	if (newrtt == 0)
	{
		return;
	}

	/* ��������rtt����Ҫ��ԭʼrtt����ƽ�� */
	gamectx->rtt = newrtt;
	gamectx->srtt = fi_get_srtt(gamectx->srtt, newrtt, FI_SRTT_VAR);
	gamectx->battlertt = FI_MIN(gamectx->srtt, FI_MAX_RTT);

	FI_LOGD(" : FI update uu rtt, newrtt=%d, battlertt=%d.", newrtt, gamectx->battlertt);

	return;
}


void fi_rtt_cal_bh3(fi_pkt *pktinfo, fi_flowctx *flowctx, fi_gamectx *gamectx)
{
	fi_flow_bh3 *bh3cache;
	uint32_t newrtt = 0;
	uint16_t seq = 0;
	uint16_t ack = 0;
	uint16_t verify = 0;
	int i;

	/* ���б��ģ�ȡ��seq��¼���� */
	if (pktinfo->dir == FI_DIR_UP)
	{
		/* ��seq�ı��ĳ��ȹ̶�Ϊ12, ���ʸ�����Ҫ��bufdatalen */
		if (pktinfo->bufdatalen != FI_BH3_UP_LEN)
		{
			return;
		}

		/* �̶�λ�ó��ֹؼ��� */
		if (pktinfo->data[FI_BH3_KEY_OFFSET_UP] != FI_BH3_KEY_WORD)
		{
			return;
		}

		/* ��ȡseq��verify */
		seq = ntohs(*(uint16_t *)(pktinfo->data + FI_BH3_SEQ_OFFSET_UP));
		verify = ntohs(*(uint16_t *)(pktinfo->data + FI_BH3_VRF_OFFSET_UP));

		/* ���ָ����seq, �򸲸�С��seq, �������ȵ�seq����������ҵط����� */
		for (i = 0; i < FI_BH3_SEQ_CACHE_NUM; i++)
		{
			bh3cache = flowctx->flow_bh3 + i;
			if (seq > bh3cache->seq)
			{
				bh3cache->seq = seq;
				bh3cache->verify = verify;
				bh3cache->time = (uint32_t)pktinfo->msec;
				FI_LOGD(" : FI bh3 uplink, seq=%04x, verify=%04x, time=%u.",
				        seq, verify, bh3cache->time);
				break;
			}
		}

		/* ��������seq�ȵ�ǰseqС����Ŀ */
		for (i++; (i < FI_BH3_SEQ_CACHE_NUM) && (flowctx->flow_bh3[i].seq > 0); i++)
		{
			flowctx->flow_bh3[i].seq = 0;
		}

		return;
	}
	else
	{
		/* ��seq�ı��ĳ��ȹ̶�Ϊ14, ���ʸ�����Ҫ��bufdatalen */
		if (pktinfo->bufdatalen != FI_BH3_DOWN_LEN)
		{
			return;
		}

		/* �̶�λ�ó��ֹؼ��� */
		if (pktinfo->data[FI_BH3_KEY_OFFSET_DOWN] != FI_BH3_KEY_WORD)
		{
			return;
		}

		/* ��ȡack��verify */
		ack = ntohs(*(uint16_t *)(pktinfo->data + FI_BH3_ACK_OFFSET_DOWN));
		verify = ntohs(*(uint16_t *)(pktinfo->data + FI_BH3_VRF_OFFSET_DOWN));

		FI_LOGD(" : FI bh3 downlink, ack=%04x, verify=%04x.", ack, verify);

		/* �ڻ���������Ե�seq */
		for (i = 0; i < FI_BH3_SEQ_CACHE_NUM; i++)
		{
			bh3cache = flowctx->flow_bh3 + i;
			if ((ack == bh3cache->seq) && (verify == bh3cache->verify))
			{
				newrtt = (uint32_t)pktinfo->msec - bh3cache->time;
				bh3cache->seq = 0;
				FI_LOGD(" : FI bh3 downlink, ack=%04x, verify=%04x, rawrtt=%u.",
				        ack, verify, newrtt);
				break;
			}
		}

		/* û�ҵ� */
		if (i == FI_BH3_SEQ_CACHE_NUM)
		{
			return;
		}

		/* û��ר�ŵĶ�ս��ʶ���㷨��������϶�ս����� */
		flowctx->flowtype = FI_FLOWTYPE_BATTLE;

		/* ��ԭʼrtt�������� */
		newrtt = FI_RANGE(newrtt, FI_MIN_RTT, FI_MAX_ORG_RTT);

		/* ֱ�Ӽ���srtt, ��ʹ�û���ƽ�� */
		gamectx->rtt = newrtt;
		gamectx->srtt = fi_get_srtt(gamectx->srtt, newrtt, FI_SRTT_VAR);
		gamectx->battlertt = FI_MIN(gamectx->srtt, FI_MAX_RTT);
		gamectx->updatetime = pktinfo->msec;

		FI_LOGD(" : FI bh3, rtt=%d, srtt=%d, btrtt=%d.",
				gamectx->rtt, gamectx->srtt, gamectx->battlertt);
	}

	return;
}


void fi_rtt_amend(fi_gamectx *gamectx, fi_gamectx *uuctx)
{
	switch (gamectx->appid)
	{
		case FI_APPID_CJZC:
		case FI_APPID_QJCJ:
		{
			/* ʹ������rtt������rtt�������� */
			if (gamectx->hbsrtt)
			{
				gamectx->final_rtt = FI_RANGE(gamectx->battlertt +
				    gamectx->hbsrtt - FI_RTT_BASE, FI_MIN_RTT, FI_MAX_RTT);
			}
			else
			{
				gamectx->final_rtt = gamectx->battlertt;
			}

			break;
		}
		case FI_APPID_QQFC:
		{
			/* �������ʹ��UU������UU��rtt�������� */
			if ((gamectx->rawrtt < FI_GAME_UU_RTT) && (uuctx->battlertt > 0))
			{
				gamectx->final_rtt = uuctx->battlertt + FI_UU_BASE_RTT;
			}
			else
			{
				gamectx->final_rtt = gamectx->battlertt;
			}
			break;
		}
		default:
		{
			gamectx->final_rtt = gamectx->battlertt;
			break;
		}
	}

	return;
}


int fi_rtt_record_battle(fi_pkt *pktinfo, fi_gamectx *gamectx)
{
	/* ���б��� */
	if (pktinfo->dir == FI_DIR_UP)
	{
		gamectx->uplinkpktnum++;
	}
	else
	{
		gamectx->downlinkpktnum++;
	}

	return FI_SUCCESS;
}


int fi_rtt_para_check(fi_pkt *pktinfo, fi_flowctx *flowctx, uint32_t appid)
{
	static int64_t  prepkttime = 0;

	if (!pktinfo || !flowctx || !FI_APPID_VALID(appid))
	{
		FI_LOGD(" : FI parameter error, appid=%u.", appid);
		return FI_FAILURE;
	}

	/* ����ʱ����� */
	if (pktinfo->msec < prepkttime)
	{
		FI_LOGD(" : FI pkt timestamp error.");
		return FI_FAILURE;
	}

	/* ��¼����ʼʱ�� */
	if (flowctx->flowstarttime == 0)
	{
		flowctx->flowstarttime = pktinfo->msec;
	}

	/* ��¼��ʼѧϰ��ʱ�� */
	if (flowctx->studystarttime == 0)
	{
		flowctx->studystarttime = pktinfo->msec;
	}

	return FI_SUCCESS;
}


int fi_rtt_filter_pkt(fi_pkt *pktinfo, uint32_t appid)
{
	int ret = FI_SUCCESS;

	/* ��ͬ����Ϸ��ע�ı������Ͳ�ͬ */
	switch (appid)
	{
		/* QQ�ɳ���Ҫtcp��udp */
		case FI_APPID_QQFC:
		{
			break;
		}
		/* UU����������Ҫtcp */
		case FI_APPID_UU:
		{
			if (pktinfo->proto == FI_IPPROTO_UDP)
			{
				ret = FI_FAILURE;
			}
			break;
		}
		default:
		{
			if (pktinfo->proto == FI_IPPROTO_TCP)
			{
				ret = FI_FAILURE;
			}
			break;
		}
	}

	return ret;
}


int fi_rtt_entrance(fi_pkt *pktinfo, fi_flowctx *flowctx, uint32_t appid)
{
	fi_gamectx *gamectx;

	/* ��ǰδ֧��uu��Ѹ�Σ������ⷢ������������������������ֱ�ӷ��� */
	if (g_fi_ctx.appinfo[FI_APPID_UU].valid || g_fi_ctx.appinfo[FI_APPID_XUNYOU].valid)
	{
		return FI_FAILURE;
	}

	/* ������� */
	if (fi_rtt_para_check(pktinfo, flowctx, appid) != FI_SUCCESS)
	{
		return FI_FAILURE;
	}

	/* ������Ϸ���ͶԱ��Ľ���ɸѡ(�Ƿ���ҪTCP����) */
	if (fi_rtt_filter_pkt(pktinfo, appid) != FI_SUCCESS)
	{
		return FI_FAILURE;
	}

	gamectx = g_fi_ctx.gamectx + appid;
	gamectx->appid = appid;

	/* UU������ֱ�Ӳ���tcp rtt, �ҽ���ֻ����һ���� */
	if (appid == FI_APPID_UU)
	{
		fi_rtt_cal_uu(pktinfo, flowctx, gamectx);
		return FI_SUCCESS;
	}

	/* �����ͼ���Ϸ״̬��ѧϰ */
	if ((flowctx->flowtype == FI_FLOWTYPE_INIT) ||
	    !FI_BATTLING(gamectx->appstatus))
	{
		fi_rtt_flow_study(pktinfo, flowctx, gamectx);
	}

	/* ͳ�ƶ�ս��ÿ�������б��ĸ��� */
	if ((flowctx->flowtype == FI_FLOWTYPE_BATTLE) ||
	    (FI_SRVPORT(pktinfo) == gamectx->battle_flow_port))
	{
		fi_rtt_record_battle(pktinfo, gamectx);
	}

	/* ����3ֱ�ӳ��Լ���rtt */
	if (gamectx->appid == FI_APPID_BH3)
	{
		fi_rtt_cal_bh3(pktinfo, flowctx, gamectx);
		fi_rtt_amend(gamectx, g_fi_ctx.gamectx + FI_APPID_UU);
		return FI_SUCCESS;
	}

	/* ������Ϸ����ս��ʼ֮��Ų���rtt */
	if (!FI_BATTLING(gamectx->appstatus))
	{
		return FI_SUCCESS;
	}

	/* ��ͬ����Ϸ���ò�ͬ�Ĳ��㷽�� */
	switch (appid)
	{
		case FI_APPID_HYXD:
		{
			fi_rtt_cal_hyxd(pktinfo, flowctx, gamectx);
			break;
		}
		case FI_APPID_QQFC:
		{
			/* QQ�ɳ�ʹ��TCP��ս������rtt */
			if ((pktinfo->proto == FI_IPPROTO_TCP) &&
			    (FI_SRVPORT(pktinfo) == gamectx->battle_flow_port))
			{
				if (pktinfo->mptcp)
				{
					fi_rtt_cal_mptcp(pktinfo, flowctx, gamectx);
				}
				else
				{
					fi_rtt_cal_tcprtt(pktinfo, flowctx, gamectx);
				}
			}

			break;
		}
		default:
		{
			/* ���ݶ�ս������rtt */
			if (flowctx->flowtype == FI_FLOWTYPE_BATTLE)
			{
				fi_rtt_cal_battle(pktinfo, flowctx, gamectx);
			}

			/* �̼�ս����ȫ��������Ҫ������������ */
			if (flowctx->flowtype == FI_FLOWTYPE_HB)
			{
				fi_rtt_cal_hb(pktinfo, flowctx, gamectx);
			}

			break;
		}
	}

	/* ������rtt��uurtt������rtt���и�������, �����յ�rtt��ֵ */
	fi_rtt_amend(gamectx, g_fi_ctx.gamectx + FI_APPID_UU);

	return FI_SUCCESS;
}

