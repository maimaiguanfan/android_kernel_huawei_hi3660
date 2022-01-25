
 
 
#include "message.h"


static HI_VOID message_stop(msg_queue_s *queue)
{
    unsigned long flags;
    struct list_head *list, *tmp;

    /* flush msg list */
    spin_lock_irqsave(&queue->lock, flags);
    queue->stop = 1;
    if (!list_empty(&queue->head)) 
    {
        OmxPrint(OMX_WARN, "msg queue not empty, flush it!\n");
        list_for_each_safe(list, tmp, &queue->head) 
        {
            list_del(list);
            list_add_tail(list, &queue->free);
        }
    }
    spin_unlock_irqrestore(&queue->lock, flags);

    wake_up_interruptible(&queue->wait);
}

static HI_U32 message_pending(msg_queue_s *queue)
{
    HI_U32 has_msg = 0;
    HI_U32 stopped = 0;
    unsigned long flags;

    if (HI_NULL == queue)
    {   
       OmxPrint(OMX_FATAL, "%s: queue = NULL.\n", __func__);
       return -EINVAL;/*lint !e570 */
    }
    
    spin_lock_irqsave(&queue->lock, flags);
    has_msg = !list_empty(&queue->head);
    stopped = queue->stop;
    spin_unlock_irqrestore(&queue->lock, flags);

    return (HI_U32)(has_msg || stopped);
}

HI_S32  message_queue(msg_queue_s *queue, HI_U32 msgcode, HI_U32 status, HI_VOID *priv)
{
    unsigned long flags;
    msg_data_s *msg;
    if (HI_NULL == queue)
    {
        OmxPrint(OMX_FATAL, "%s: invalid param: queue=NULL\n", __func__);
        return -EINVAL;
    }

    spin_lock_irqsave(&queue->lock, flags);
    if (queue->stop) 
    {
        spin_unlock_irqrestore(&queue->lock, flags);
        OmxPrint(OMX_FATAL, "%s: msg queue stopped.\n", __func__);
        return 0;
    }

    if (list_empty(&queue->free))
    {
        spin_unlock_irqrestore(&queue->lock, flags);
        OmxPrint(OMX_FATAL, "%s: no free space in list.\n", __func__);
        return -ENOMEM;
    }

    msg = list_first_entry(&queue->free, msg_data_s, list);
    list_del(&msg->list);
    msg->msg_info.status_code = status;
    msg->msg_info.msgcode     = msgcode;

    if (priv)
    {
        if (VDEC_EVT_REPORT_IMG_SIZE_CHG == msgcode)
        {
            memcpy(&msg->msg_info.msgdata, priv, sizeof(OMXVDEC_IMG_SIZE));/* unsafe_function_ignore: memcpy  */
        }
        else if(VDEC_EVT_REPORT_SEQ_INFO_CHG == msgcode)
        {
            memcpy(&msg->msg_info.msgdata, priv, sizeof(OMXVDEC_SEQ_INFO));/* unsafe_function_ignore: memcpy  */
        }
        else if (VDEC_EVT_REPORT_COLOR_ASPECTS_CHG == msgcode)
        {
            memcpy(&msg->msg_info.msgdata,  priv, sizeof(OMXVDEC_COLOUR_DESCRIPTION_INFO_S));
        }
        else
        {
            memcpy(&msg->msg_info.msgdata, priv, sizeof(OMXVDEC_BUF_DESC));/* unsafe_function_ignore: memcpy  */
        }
    }

    list_add_tail(&msg->list, &queue->head);
    spin_unlock_irqrestore(&queue->lock, flags);

    wake_up_interruptible(&queue->wait);

    return 0;
}

HI_S32 message_dequeue(msg_queue_s *queue, OMXVDEC_MSG_INFO *pmsg_info)
{
    unsigned long flags;
    msg_data_s *msg = HI_NULL;
    HI_S32 ret;

    if (HI_NULL == queue)
    {   
        OmxPrint(OMX_FATAL, "%s: invalid param: queue=NULL(0x%pK)\n", __func__, queue);
        return -EINVAL;
    }

    if (HI_NULL == pmsg_info)
    {
        OmxPrint(OMX_FATAL, "%s: invalid param: pmsg_info=NULL(0x%pK)\n", __func__, pmsg_info);
        goto shutdown;
    }

    //ret = wait_event_interruptible(queue->wait, msg_pending(queue));
    ret = wait_event_interruptible_timeout(queue->wait, (message_pending(queue)), (msecs_to_jiffies(100))); /*lint !e665 !e666*/

    if (queue->stop)
    {
        OmxPrint(OMX_WARN, "%s: wait msg failed, msg queue stopped.\n", __func__);
        goto shutdown; 
    }

    if (ret <= 0)
    {
        if (ret < 0)
        {
            OmxPrint(OMX_WARN, "%s: wait msg interrupted.\n", __func__);
        }
        else
        {
            OmxPrint(OMX_VER, "%s: wait msg timeout.\n", __func__); 
        }
        return -EAGAIN;
    }

    spin_lock_irqsave(&queue->lock, flags);
    if (queue->stop) 
    {
        spin_unlock_irqrestore(&queue->lock, flags);
        OmxPrint(OMX_INFO, "%s: msg queue stopped.\n", __func__);
        goto shutdown;
    }

    if (!list_empty(&queue->head)) 
    {
        msg = list_first_entry(&queue->head, msg_data_s, list);
        list_del(&msg->list);
        memcpy(pmsg_info, &msg->msg_info, sizeof(OMXVDEC_MSG_INFO));/* unsafe_function_ignore: memcpy  */
        list_add(&msg->list, &queue->free);
    }
    spin_unlock_irqrestore(&queue->lock, flags);
    
    return HI_SUCCESS;

shutdown:
    queue->quit = 1;
    
    return -ESHUTDOWN;
}

msg_queue_s* message_queue_init(HI_S32 chan_id, HI_U32 max_msg_num)
{
    HI_S32  ret;
    HI_U32  nqueues = 0;
    HI_CHAR buf_name[20];
    MEM_BUFFER_S mem_buf;
    msg_queue_s *queue = HI_NULL;
    msg_data_s  *data  = HI_NULL;

    /* alloc mem for message queue & data */
    memset(&mem_buf, 0, sizeof(MEM_BUFFER_S));/* unsafe_function_ignore: memset  */
    mem_buf.u8IsCached = 1;
    mem_buf.u32Size = sizeof(msg_queue_s) + sizeof(msg_data_s)*max_msg_num;
    snprintf(buf_name, sizeof(buf_name), "Chan%d_MSG", chan_id);/* unsafe_function_ignore: snprintf  */
    ret = VDEC_MEM_AllocAndMap(buf_name, OMXVDEC_ZONE, &mem_buf);
    if (ret != HI_SUCCESS)
    {
        OmxPrint(OMX_FATAL, "%s alloc memory for queue failed.\n", __func__);
        return HI_NULL;
    }
    memset(mem_buf.pStartVirAddr, 0, mem_buf.u32Size);/* unsafe_function_ignore: memset  */
    
    queue = (msg_queue_s *)mem_buf.pStartVirAddr;
    data  = (msg_data_s *)((HI_U8 *)mem_buf.pStartVirAddr + sizeof(msg_queue_s));

    memcpy(&queue->mem_buf, &mem_buf, sizeof(MEM_BUFFER_S));/* unsafe_function_ignore: memcpy  */

    spin_lock_init(&queue->lock);
    INIT_LIST_HEAD(&queue->head);
    INIT_LIST_HEAD(&queue->free);
    init_waitqueue_head(&queue->wait);
    queue->stop = 0;
    queue->quit = 0;

    /* add node into data list */
    nqueues = max_msg_num;
    for (; nqueues; data++, nqueues--)
    {
        list_add(&data->list, &queue->free);
    }
    queue->msg_num = max_msg_num;

    return queue;
}

HI_VOID message_queue_deinit(msg_queue_s *queue)
{
    HI_U32 slp_cnt = 0;

    if (NULL == queue)
    {
        OmxPrint(OMX_FATAL, "%s: queue = NULL.\n", __func__);
        return;
    }

    message_stop(queue);

    while(!queue->quit && slp_cnt<10)
    {
        msleep(10);
        slp_cnt++;
    }

    VDEC_MEM_UnmapAndRelease(&queue->mem_buf);

    return;
}
