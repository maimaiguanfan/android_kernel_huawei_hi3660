

#ifndef __MEM_TRACE_H__
#define __MEM_TRACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(_PRE_MEM_TRACE)
#include "oal_types.h"

/**
 @brief ���ڴ����ģ��ĳ�ʼ���ӿ�?
 @param[in] none
 @return    oal_void
 @note  ����Ҫ���ٵ��ڴ�ڵ㿪ʼ���ж�̬����֮ǰ��ִ��
*/
extern oal_void mem_trace_init(oal_void);

/**
 @brief ���ڴ����ģ���ȥ��ʼ���ӿ�
 @param[in] none
 @return    oal_void
 @note  ���ڵ�ǰ��ʵ����ģ�鱾����ڴ���Դ���Ǿ�̬�����
        ���Դ˽ӿ������κ����ã���������˸���ģ�鱾��ʹ�õ��ڴ���Դ
        ��Ҫ��̬���룬�������ô˽ӿ����ͷ�������ڴ���Դ
*/
extern oal_void mem_trace_exit(void);

/**
 @brief ��ʾ�ڴ����ģ���и��ٵ��Ľڵ���Ϣ
 @param[in] ul_mode     ģʽ0:��������������Ϣ����ʾ����ڵǼǵ������ڴ�ڵ���Ϣ
                        ģʽ1:��ʾ������ڵǼǵ��ڴ�ڵ�����
                        ģʽ2:��ʾ��ǰ���д��ڵǼ�״̬�Ľڵ�����

 @param[in] ul_fileid   ������ڴ�������Ϣ:�ļ���
 @param[in] ul_linenum  ������ڴ�������Ϣ:�к�
 @return    oal_void
 @note  ��ǰ��ʵ���ǲ���printk�ķ�ʽ��������Ϣ���ָ��û���
*/
extern oal_void mem_trace_info_show(oal_uint32 ul_mode,
                                    oal_uint32 ul_fileid,
                                    oal_uint32 ul_line);
#ifdef _PRE_SKB_TRACE
/**
 @brief �Ǽ���Ҫ�����ٵ��ڴ�ڵ㣬�����ڴ�ڵ�ĵ�ַ��������ڴ�������Ϣ(�ļ���+�к�)
 @param[in] ul_addr     �ڴ�ڵ��Ӧ�ĵ�ַ
 @param[in] ul_fileid   ������ڴ�������Ϣ:�ļ���
 @param[in] ul_linenum  ������ڴ�������Ϣ:�к�
 @return    oal_void
 @note  �����ڽ��ĳһ�ض�����ʱ����Ҫ�޸���Ӧ���ڴ�����ӿڻ����ڴ���ٵ����
        ������Ҫ����ĳһ��ģ�鶯̬�����skb����ô��Ҫͳһ�Ը�ģ��ʹ�õ�skb��̬����ӿڽ���ͳһ�ĺ����:

     _oal_netbuf_alloc()��ԭ����ʵ�ֽӿڣ�ֻ���޸���һ�½ӿ�����

     #ifdef _PRE_SKB_TRACE
     #define oal_netbuf_alloc(size,l_reserve,l_align)        \
     ({\
         oal_netbuf_stru *pst_netbuf;\
         pst_netbuf = _oal_netbuf_alloc(size,l_reserve,l_align);\
         if(pst_netbuf)\
         {\
             mem_trace_add_node((oal_size_t)pst_netbuf, THIS_FILE_ID, (oal_uint32)__LINE__);\
         }\
         pst_netbuf;\
     })
     #else
     #define oal_netbuf_alloc(size,l_reserve,l_align) _oal_netbuf_alloc(size,l_reserve,l_align)
     #endif
*/
extern oal_void __mem_trace_add_node(oal_ulong   ul_addr,
                                     oal_uint32  ul_fileid,
                                     oal_uint32  ul_linenum);

/**
 @brief ע�������ٵ��ڴ�ڵ�
 @param[in] ul_addr     �ڴ�ڵ��Ӧ�ĵ�ַ
 @return    oal_void
 @note  ĳģ����Ϊ���ڴ�ڵ㲻��Ҫ�������ٵ�ʱ����Ҫ���ô˽ӿ�ע�������ڴ�ڵ㡣
        ͨ�����ڴ汻�ͷ�ʱ�����ڴ�ڵ㴫�ݸ���һ��ģ���ˡ�

        �����ڽ��ĳһ�ض�����ʱ����Ҫ�޸���Ӧ���ڴ��ͷŽӿڻ����ڴ洫�ݸ���һ��ģ��ĳ���
        ������Ҫ����ĳһ��ģ���ͷŵ�skb����ô��Ҫͳһ�Ը�ģ��ʹ�õ�skb�ͷŽӿڽ���ͳһ�ĺ����:

     _oal_netbuf_free()��ԭ����ʵ�ֽӿڣ�ֻ���޸���һ�½ӿ�����

     #ifdef _PRE_SKB_TRACE
     #define oal_netbuf_free(pst_netbuf)        \
     ({\
         mem_trace_delete_node((oal_size_t)pst_netbuf);\
         _oal_netbuf_free(pst_netbuf);\
     })
     #else
     #define oal_netbuf_free(pst_netbuf) _oal_netbuf_free(pst_netbuf)
     #endif
*/
extern oal_void __mem_trace_delete_node(oal_ulong   ul_addr,
                                        oal_uint32  ul_fileid,
                                        oal_uint32  ul_linenum);


/**
 @brief �����ٵ��ڴ�ڵ��̽�룬�����ڳ�������ʱ���ٸ��ڴ�ڵ����̽�⵽��λ��
 @param[in] ul_addr     �ڴ�ڵ��Ӧ�ĵ�ַ
 @param[in] ul_fileid   ������ڴ�������Ϣ:�ļ���
 @param[in] ul_linenum  ������ڴ�������Ϣ:�к�
 @return    oal_void
 @note  ͨ���ڽ��ĳһ��ʵ������ʱ���ڳ��������в�ͬ�ĵط����ô˽ӿ�����̽��ĳ�ڴ�ڵ��Ƿ����е���λ�ã�
        �Ӷ�����ȷ���������̣���������Ķ�λ

        �����ڽ���ڴ�й¶ʱ��ͨ��mem_trace_info_show�Ĵ�ӡ�����Ե�֪��Щ�ڴ�ڵ�û�б��ͷŵ���Ҳ����֪��
        ��Щ���������ڴ�ڵ�û���ͷŵ�����ô�����ڸ���ڿ�ʼ������������̽�룬�Ӷ����Ը����ڴ�������ʱ
        ��󵽴��λ��
*/
extern oal_void __mem_trace_probe(oal_ulong  ul_addr,
                                  oal_uint32 ul_probe_fileid,
                                  oal_uint32 ul_probe_line);



#define mem_trace_add_node(ul_addr) \
        __mem_trace_add_node(ul_addr, THIS_FILE_ID, __LINE__)
#define mem_trace_delete_node(ul_addr) \
        __mem_trace_delete_node(ul_addr, THIS_FILE_ID, __LINE__)
#define mem_trace_probe(ul_addr) \
        __mem_trace_probe(ul_addr, THIS_FILE_ID, __LINE__)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
#endif

