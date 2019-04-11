/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : ionAlloc.c
* Description :
* History :
*   Author  : xyliu <xyliu@allwinnertech.com>
*   Date    : 2016/04/13
*   Comment :
*
*
*/


/*
 * ion_alloc.c
 *
 * john.fu@allwinnertech.com
 *
 * ion memory allocate
 *
 */

//#define CONFIG_LOG_LEVEL    OPTION_LOG_LEVEL_DETAIL
#define LOG_TAG "ionAlloc"

#include <cutils/log.h>

#include "memory/ionMemory/ionAlloc.h"
#include "memory/ionMemory/ionAllocList.h"
#include "memory/ionMemory/ionAllocEntry.h"
#include "memory/sc_interface.h"

#include <sys/ioctl.h>
#include <errno.h>
//#include "veAdapter.h"
//#include "veInterface.h"

#define DEBUG_ION_REF 0   //just for H3 ION memery info debug
#define ION_ALLOC_ALIGN    SZ_4k
#define DEV_NAME                     "/dev/ion"
#define ION_IOC_SUNXI_POOL_INFO        10

#define UNUSA_PARAM(param) (void)param

//----------------------
#if DEBUG_ION_REF==1
    int   cdx_use_mem = 0;
    typedef struct ION_BUF_NODE_TEST
    {
        unsigned int addr;
        int size;
    } ion_buf_node_test;

    #define ION_BUF_LEN  50
    ion_buf_node_test ion_buf_nodes_test[ION_BUF_LEN];
#endif
//----------------------

    struct sunxi_pool_info {
        unsigned int total;     //unit kb
        unsigned int free_kb;  // size kb
        unsigned int free_mb;  // size mb
    };

struct user_iommu_param {
    int            fd;
    unsigned int   iommu_addr;
};

typedef struct BUFFER_NODE
{
    struct aw_mem_list_head i_list;
    unsigned long phy;        //phisical address
    unsigned long vir;        //virtual address
    unsigned int size;        //buffer size
    unsigned int tee;      //
    unsigned long user_virt;//
    ion_fd_data_t fd_data;
    unsigned char alloc_by_ve;
    struct user_iommu_param iommu_buffer;
}buffer_node;

typedef struct ION_ALLOC_CONTEXT
{
    int                    fd;            // driver handle
    struct aw_mem_list_head    list;        // buffer list
    int                    ref_cnt;    // reference count
    unsigned int           phyOffset;
}ion_alloc_context;

ion_alloc_context    *g_alloc_context_cam = NULL;
pthread_mutex_t      g_mutex_alloc_cam = PTHREAD_MUTEX_INITIALIZER;

unsigned long CdcIonGetPhyAdrVe(int nIonFd, uintptr_t handle,
                                            unsigned char bAllocByVe)
{
    int ret = 0;

    struct aw_ion_custom_info custom_data;
    sunxi_phys_data   phys_data;
    memset(&phys_data, 0, sizeof(sunxi_phys_data));
    custom_data.aw_cmd = ION_IOC_SUNXI_PHYS_ADDR;
    phys_data.handle = (aw_ion_user_handle_t)handle;
    custom_data.aw_arg = (unsigned long)&phys_data;
    ret = ioctl(nIonFd, AW_MEM_ION_IOC_CUSTOM, &custom_data);
    if(ret) {
        ALOGE("ION_IOC_CUSTOM err, ret %d\n", ret);
        return 0;
    }
    ALOGV("CdcIonGetPhyAdrVe:%x, handle:%d\n",phys_data.phys_addr, handle);

    return (unsigned long)phys_data.phys_addr;
}

/*funciton begin*/
int ion_cam_alloc_open()
{
    ALOGD("begin ion_cam_alloc_open \n");

    pthread_mutex_lock(&g_mutex_alloc_cam);
    if (g_alloc_context_cam != NULL)
    {
        ALOGV("ion allocator has already been created \n");
        goto SUCCEED_OUT;
    }

    g_alloc_context_cam = (ion_alloc_context*)malloc(sizeof(ion_alloc_context));
    if (g_alloc_context_cam == NULL)
    {
        ALOGE("create ion allocator failed, out of memory \n");
        goto ERROR_OUT;
    }
    else
    {
        ALOGV("pid: %d, g_alloc_context_cam = %p \n", getpid(), g_alloc_context_cam);
    }

    memset((void*)g_alloc_context_cam, 0, sizeof(ion_alloc_context));
#if 0
    int type = VE_OPS_TYPE_NORMAL;
    VeOpsS* veOps = GetVeOpsS(type);
    if(veOps == NULL)
    {
        ALOGE("get ve ops failed");
        goto ERROR_OUT;
    }
    VeConfig mVeConfig;
    memset(&mVeConfig, 0, sizeof(VeConfig));
    mVeConfig.nDecoderFlag = 1;

    void* pVeopsSelf = CdcVeInit(veOps,&mVeConfig);
    if(pVeopsSelf == NULL)
    {
        ALOGE("init ve ops failed");
        CdcVeRelease(veOps, pVeopsSelf);
        goto ERROR_OUT;
    }
    g_alloc_context_cam->phyOffset = CdcVeGetPhyOffset(veOps, pVeopsSelf);
    ALOGD("** phy offset = %x",g_alloc_context_cam->phyOffset);
    CdcVeRelease(veOps, pVeopsSelf);
#endif
/* Readonly should be enough. */
    g_alloc_context_cam->fd = open(DEV_NAME, O_RDONLY, 0);

    if (g_alloc_context_cam->fd <= 0)
    {
        ALOGE("open %s failed \n", DEV_NAME);
        goto ERROR_OUT;
    }

#if DEBUG_ION_REF==1
    cdx_use_mem = 0;
    memset(&ion_buf_nodes_test, sizeof(ion_buf_nodes_test), 0);
    ALOGD("ion_open, cdx_use_mem=[%dByte].", cdx_use_mem);
    ion_cam_alloc_get_total_size();
#endif

    AW_MEM_INIT_LIST_HEAD(&g_alloc_context_cam->list);

SUCCEED_OUT:
    g_alloc_context_cam->ref_cnt++;
    pthread_mutex_unlock(&g_mutex_alloc_cam);
    return 0;

ERROR_OUT:
    if (g_alloc_context_cam != NULL
        && g_alloc_context_cam->fd > 0)
    {
        close(g_alloc_context_cam->fd);
        g_alloc_context_cam->fd = 0;
    }

    if (g_alloc_context_cam != NULL)
    {
        free(g_alloc_context_cam);
        g_alloc_context_cam = NULL;
    }

    pthread_mutex_unlock(&g_mutex_alloc_cam);
    return -1;
}

void ion_cam_alloc_close()
{
    struct aw_mem_list_head * pos, *q;

    ALOGV("ion_cam_alloc_close \n");

    pthread_mutex_lock(&g_mutex_alloc_cam);
    if (--g_alloc_context_cam->ref_cnt <= 0)
    {
        ALOGV("pid: %d, release g_alloc_context_cam = %p \n", getpid(), g_alloc_context_cam);

        aw_mem_list_for_each_safe(pos, q, &g_alloc_context_cam->list)
        {
            buffer_node * tmp;
            tmp = aw_mem_list_entry(pos, buffer_node, i_list);
            ALOGV("ion_cam_alloc_close del item phy= 0x%lx vir= 0x%lx, size= %d \n", \
                tmp->phy, tmp->vir, tmp->size);
            aw_mem_list_del(pos);
            free(tmp);
        }
#if DEBUG_ION_REF==1
        ALOGD("ion_close, cdx_use_mem=[%d MB]", cdx_use_mem/1024/1024);
        ion_cam_alloc_get_total_size();
#endif
        close(g_alloc_context_cam->fd);
        g_alloc_context_cam->fd = 0;

        free(g_alloc_context_cam);
        g_alloc_context_cam = NULL;
    }
    else
    {
        ALOGV("ref cnt: %d > 0, do not free \n", g_alloc_context_cam->ref_cnt);
    }
    pthread_mutex_unlock(&g_mutex_alloc_cam);

    //--------------
#if DEBUG_ION_REF==1
    int i = 0;
    int counter = 0;
    for(i=0; i<ION_BUF_LEN; i++)
    {
        if(ion_buf_nodes_test[i].addr != 0 || ion_buf_nodes_test[i].size != 0){

            ALOGE("ion mem leak????  addr->[0x%x], leak size->[%dByte]", \
                ion_buf_nodes_test[i].addr, ion_buf_nodes_test[i].size);
            counter ++;
        }
    }

    if(counter != 0)
    {
        ALOGE("my god, have [%d]blocks ion mem leak.!!!!", counter);
    }
    else
    {
        ALOGD("well done, no ion mem leak.");
    }
#endif
    //--------------
    return ;
}

// return virtual address: 0 failed
void* ion_cam_alloc_palloc(int size, int chose,int * pFd)
{
    aw_ion_allocation_info_t alloc_data;
    ion_fd_data_t fd_data;
    struct ion_handle_data handle_data;

    int rest_size = 0;
    unsigned long addr_phy = 0;
    unsigned long addr_vir = 0;
    buffer_node * alloc_buffer = NULL;
    int ret = 0;

    ALOGD("ion_cam_alloc_palloc,chose:%d\n",chose);
    pthread_mutex_lock(&g_mutex_alloc_cam);

    if (g_alloc_context_cam == NULL)
    {
        ALOGE("ion_cam_alloc do not opened, should call ion_cam_alloc_open() \
            before ion_cam_alloc_alloc(size) \n");
        goto ALLOC_OUT;
    }

    if(size <= 0)
    {
        ALOGE("can not alloc size 0 \n");
        goto ALLOC_OUT;
    }
    ALOGD("size:%d",size);
    alloc_data.aw_len = (size_t)size;
    alloc_data.aw_align = ION_ALLOC_ALIGN ;
    //ALOGD("aw_len:%d,size:%d",alloc_data.aw_len,size);

#ifdef CONF_USE_IOMMU
    alloc_data.aw_heap_id_mask = AW_ION_SYSTEM_HEAP_MASK | AW_ION_CARVEOUT_HEAP_MASK;
#else
    alloc_data.aw_heap_id_mask = AW_ION_DMA_HEAP_MASK | AW_ION_CARVEOUT_HEAP_MASK;
#endif
    alloc_data.flags = AW_ION_CACHED_FLAG | AW_ION_CACHED_NEEDS_SYNC_FLAG;

    ret = ioctl(g_alloc_context_cam->fd, AW_MEM_ION_IOC_ALLOC, &alloc_data);
    if (ret)
    {
        ALOGE("ION_IOC_ALLOC error \n");
        goto ALLOC_OUT;
    }

    /* get dmabuf fd */
    fd_data.handle = alloc_data.handle;
    if(chose == DMA_BUFFER_FD)
    {
        ret = ioctl(g_alloc_context_cam->fd, AW_MEM_ION_IOC_MAP, &fd_data);
        if(ret)
        {
            ALOGE("ION_IOC_MAP err, ret %d, dmabuf fd 0x%08x\n", ret, (unsigned int)fd_data.aw_fd);
            goto ALLOC_OUT;
        }
        if (*pFd < 0) {
            ALOGE("dma ioctl returned negative fd\n");
            goto ALLOC_OUT;
        }

       *pFd = fd_data.aw_fd;

       ALOGD("DMA_BUFFER_FD :%d\n", *pFd);
    }
    else if (chose == SHARE_BUFFER_FD)
    {
        ret = ioctl(g_alloc_context_cam->fd, AW_MEMION_IOC_SHARE, &fd_data);
        /*
        if (ret < 0)
            return ret;
        */
        *pFd = fd_data.aw_fd;
        if (*pFd < 0) {
            ALOGE("share ioctl returned negative fd\n");
            goto ALLOC_OUT;
        }
        //ALOGD("SHARE_BUFFER_FD :%d\n", *pFd);
    }else
    {
        //ALOGD("debug flag!\n");
    }

    /* mmap to user */
    addr_vir = (unsigned long)mmap(NULL, alloc_data.aw_len, \
        PROT_READ|PROT_WRITE, MAP_SHARED, fd_data.aw_fd, 0);
    if((unsigned long)MAP_FAILED == addr_vir)
    {
        ALOGE("mmap err, ret %d\n", (unsigned int)addr_vir);
        addr_vir = 0;
        goto ALLOC_OUT;
    }

    alloc_buffer = (buffer_node *)malloc(sizeof(buffer_node));
    if (alloc_buffer == NULL)
    {
        ALOGE("malloc buffer node failed");

        /* unmmap */
        ret = munmap((void*)addr_vir, alloc_data.aw_len);
        if(ret) {
            ALOGE("munmap err, ret %d\n", ret);
        }

        /* close dmabuf fd */
        close(fd_data.aw_fd);

        /* free buffer */
        handle_data.handle = alloc_data.handle;
        ret = ioctl(g_alloc_context_cam->fd, AW_MEM_ION_IOC_FREE, &handle_data);

        if(ret) {
            ALOGE("ION_IOC_FREE err, ret %d\n", ret);
        }

        addr_phy = 0;
        addr_vir = 0;        // value of MAP_FAILED is -1, should return 0

        goto ALLOC_OUT;
    }
    alloc_buffer->alloc_by_ve = 1;

#ifndef CONF_USE_IOMMU
    addr_phy = CdcIonGetPhyAdrVe(g_alloc_context_cam->fd, alloc_data.handle,
                                                    alloc_buffer->alloc_by_ve);
#endif
    if(!addr_phy)
    {
        ALOGE("get phy addr error\n");
        goto ALLOC_OUT;
    }

    alloc_buffer->phy     = addr_phy;
    alloc_buffer->vir     = addr_vir;
    alloc_buffer->user_virt = addr_vir;
    alloc_buffer->size    = size;
    alloc_buffer->fd_data.handle = fd_data.handle;
    alloc_buffer->fd_data.aw_fd = fd_data.aw_fd;

    //ALOGD("alloc succeed, addr_phy: 0x%lx, addr_vir: 0x%lx, size: %d", addr_phy, addr_vir, size);

    aw_mem_list_add_tail(&alloc_buffer->i_list, &g_alloc_context_cam->list);

    //------start-----------------
#if DEBUG_ION_REF==1
    cdx_use_mem += size;
    ALOGD("++++++cdx_use_mem = [%d MB], increase size->[%d B], addr_vir=[0x%x], addr_phy=[0x%x]", \
        cdx_use_mem/1024/1024, size, addr_vir, addr_phy);
    int i = 0;
    for(i=0; i<ION_BUF_LEN; i++)
    {
        if(ion_buf_nodes_test[i].addr == 0 && ion_buf_nodes_test[i].size == 0){
            ion_buf_nodes_test[i].addr = addr_vir;
            ion_buf_nodes_test[i].size = size;
            break;
        }
    }

    if(i>= ION_BUF_LEN){
        ALOGE("error, ion buf len is large than [%d]", ION_BUF_LEN);
    }
#endif
//--------------------------------

ALLOC_OUT:
    pthread_mutex_unlock(&g_mutex_alloc_cam);
    return (void*)addr_vir;
}

void ion_cam_alloc_pfree(void * pbuf)
{
    int flag = 0;
    unsigned long addr_vir = (unsigned long)pbuf;
    buffer_node * tmp;
    int ret;
    struct ion_handle_data handle_data;

    ALOGD("ion_cam_alloc_pfree");
    if (0 == pbuf)
    {
        ALOGE("can not free NULL buffer \n");
        return ;
    }

    pthread_mutex_lock(&g_mutex_alloc_cam);

    if (g_alloc_context_cam == NULL)
    {
        ALOGE("ion_cam_alloc do not opened, should call ion_cam_alloc_open() \
            before ion_cam_alloc_alloc(size) \n");
        return ;
    }

    aw_mem_list_for_each_entry(tmp, &g_alloc_context_cam->list, i_list)
    {
        if (tmp->vir == addr_vir)
        {
            ALOGD("ion_cam_alloc_free item phy= 0x%lx vir= 0x%lx, size= %d \n", \
                tmp->phy, tmp->vir, tmp->size);
#if 0
            /*unmap user space*/
        #ifdef CONF_USE_IOMMU
            ALOGV("ion_cam_alloc_pfree: fd:%d, iommu_addr:%x\n",
                                        tmp->iommu_buffer.fd,
                                        tmp->iommu_buffer.iommu_addr);
            CdcIonFreeIommuAddr((void *)&tmp->iommu_buffer, veOps, pVeopsSelf);
        #endif
#endif
            if (munmap((void *)(tmp->user_virt), tmp->size) < 0)
            {
                ALOGE("munmap 0x%p, size: %d failed \n", (void*)addr_vir, tmp->size);
            }

            /*close dma buffer fd*/
            close(tmp->fd_data.aw_fd);
            /* free buffer */
            handle_data.handle = tmp->fd_data.handle;

            ret = ioctl(g_alloc_context_cam->fd, AW_MEM_ION_IOC_FREE, &handle_data);
            if (ret)
            {
                ALOGV("TON_IOC_FREE failed \n");
            }

            aw_mem_list_del(&tmp->i_list);
            free(tmp);

            flag = 1;

            //------start-----------------
#if DEBUG_ION_REF==1
            int i = 0;
            for(i=0; i<ION_BUF_LEN; i++)
            {
                if(ion_buf_nodes_test[i].addr == addr_vir && ion_buf_nodes_test[i].size > 0){

                    cdx_use_mem -= ion_buf_nodes_test[i].size;
                    ALOGV("--------cdx_use_mem = [%d MB], reduce size->[%d B]",\
                        cdx_use_mem/1024/1024, ion_buf_nodes_test[i].size);
                    ion_buf_nodes_test[i].addr = 0;
                    ion_buf_nodes_test[i].size = 0;

                    break;
                }
            }

            if(i>= ION_BUF_LEN){
                ALOGE("error, ion buf len is large than [%d]", ION_BUF_LEN);
            }
#endif
            //--------------------------------

            break;
        }
    }

    if (0 == flag)
    {
        ALOGE("ion_cam_alloc_free failed, do not find virtual address: 0x%lx \n", addr_vir);
    }

    pthread_mutex_unlock(&g_mutex_alloc_cam);
    return ;
}

void* ion_cam_alloc_vir2phy_cpu(void * pbuf)
{
    int flag = 0;
    unsigned long addr_vir = (unsigned long)pbuf;
    unsigned long addr_phy = 0;
    buffer_node * tmp;

    if (0 == pbuf)
    {
        // ALOGV("can not vir2phy NULL buffer \n");
        return 0;
    }

    pthread_mutex_lock(&g_mutex_alloc_cam);

    aw_mem_list_for_each_entry(tmp, &g_alloc_context_cam->list, i_list)
    {
        if (addr_vir >= tmp->vir
            && addr_vir < tmp->vir + tmp->size)
        {
            addr_phy = tmp->phy + addr_vir - tmp->vir;
            // ALOGV("ion_cam_alloc_vir2phy phy= 0x%08x vir= 0x%08x \n", addr_phy, addr_vir);
            flag = 1;
            break;
        }
    }

    if (0 == flag)
    {
        ALOGE("ion_cam_palloc_vir2phy failed, do not find virtual address: 0x%lx \n", addr_vir);
    }

    pthread_mutex_unlock(&g_mutex_alloc_cam);

    return (void*)addr_phy;
}

void* ion_cam_alloc_phy2vir_cpu(void * pbuf)
{
    int flag = 0;
    unsigned long addr_vir = 0;
    unsigned long addr_phy = (unsigned long)pbuf;
    buffer_node * tmp;

    if (0 == pbuf)
    {
        ALOGE("can not phy2vir NULL buffer \n");
        return 0;
    }

    pthread_mutex_lock(&g_mutex_alloc_cam);

    aw_mem_list_for_each_entry(tmp, &g_alloc_context_cam->list, i_list)
    {
        if (addr_phy >= tmp->phy
            && addr_phy < tmp->phy + tmp->size)
        {
            addr_vir = tmp->vir + addr_phy - tmp->phy;
            flag = 1;
            break;
        }
    }

    if (0 == flag)
    {
        ALOGE("ion_cam_alloc_phy2vir failed, do not find physical address: 0x%lx \n", addr_phy);
    }

    pthread_mutex_unlock(&g_mutex_alloc_cam);

    return (void*)addr_vir;
}

void* ion_cam_alloc_vir2phy_ve(void * pbuf)
{
    ALOGV("**11 phy offset = %x",g_alloc_context_cam->phyOffset);

    return (void*)((unsigned long)ion_cam_alloc_vir2phy_cpu(pbuf) - g_alloc_context_cam->phyOffset);
}

void* ion_cam_alloc_phy2vir_ve(void * pbuf)
{
    ALOGV("**22 phy offset = %x",g_alloc_context_cam->phyOffset);

    return (void*)((unsigned long)ion_cam_alloc_phy2vir_cpu(pbuf) - g_alloc_context_cam->phyOffset);
}

//#ifdef CONF_KERNEL_VERSION_3_10
void ion_cam_alloc_flush_cache(void* startAddr, int size)
{
    sunxi_cache_range range;
    int ret;

    /* clean and invalid user cache */
    range.start = (unsigned long)startAddr;
    range.end = (unsigned long)startAddr + size;
    ret = ioctl(g_alloc_context_cam->fd, ION_IOC_SUNXI_FLUSH_RANGE, &range);
    if (ret)
    {
        ALOGE("ION_IOC_SUNXI_FLUSH_RANGE failed \n");
    }

    return;
}
/*
//#else
void ioncam__alloc_flush_cache(void* startAddr, int size)
{
    sunxi_cache_range range;
    struct aw_ion_custom_info custom_data;
    int ret;

    // clean and invalid user cache
    range.start = (unsigned long)startAddr;
    range.end = (unsigned long)startAddr + size;

    custom_data.aw_cmd = ION_IOC_SUNXI_FLUSH_RANGE;
    custom_data.aw_arg = (unsigned long)&range;

    ret = ioctl(g_alloc_context_cam->fd, AW_MEM_ION_IOC_CUSTOM, &custom_data);
    if (ret)
    {
        ALOGE("ION_IOC_CUSTOM failed \n");
    }

    return;
}
//#endif
*/

void ion_cam_flush_cache_all()
{
#if 0
    CEDARC_UNUSE(ion_flush_cache_all);
#endif
    ioctl(g_alloc_context_cam->fd, ION_IOC_SUNXI_FLUSH_ALL, 0);
}

void* ion_cam_alloc_alloc_drm(int size)
{
    aw_ion_allocation_info_t alloc_data;
    ion_fd_data_t fd_data;
    struct ion_handle_data handle_data;
    struct aw_ion_custom_info custom_data;
    sunxi_phys_data   phys_data, tee_data;

    int rest_size = 0;
    unsigned long addr_phy = 0;
    unsigned long addr_vir = 0;
    unsigned long addr_tee = 0;
    buffer_node * alloc_buffer = NULL;
    int ret = 0;

    pthread_mutex_lock(&g_mutex_alloc_cam);

    if (g_alloc_context_cam == NULL)
    {
        ALOGE("ion_cam_alloc do not opened, should call ion_cam_alloc_open() \
            before ion_cam_alloc_alloc(size) \n");
        goto ALLOC_OUT;
    }

    if(size <= 0)
    {
        ALOGV("can not alloc size 0 \n");
        goto ALLOC_OUT;
    }

    /*alloc buffer*/
    alloc_data.aw_len = size;
    alloc_data.aw_align = ION_ALLOC_ALIGN ;
    alloc_data.aw_heap_id_mask = AW_ION_SECURE_HEAP_MASK;
    alloc_data.flags = AW_ION_CACHED_FLAG | AW_ION_CACHED_NEEDS_SYNC_FLAG;
    ret = ioctl(g_alloc_context_cam->fd, AW_MEM_ION_IOC_ALLOC, &alloc_data);
    if (ret)
    {
        ALOGE("ION_IOC_ALLOC error %s \n", strerror(errno));
        goto ALLOC_OUT;
    }

    /* get dmabuf fd */
    fd_data.handle = alloc_data.handle;
    ret = ioctl(g_alloc_context_cam->fd, AW_MEM_ION_IOC_MAP, &fd_data);
    if(ret)
    {
        ALOGE("ION_IOC_MAP err, ret %d, dmabuf fd 0x%08x\n", ret, (unsigned int)fd_data.aw_fd);
        goto ALLOC_OUT;
    }

    /* mmap to user */
    addr_vir = (unsigned long)mmap(NULL, alloc_data.aw_len, \
        PROT_READ|PROT_WRITE, MAP_SHARED, fd_data.aw_fd, 0);
    if((unsigned long)MAP_FAILED == addr_vir)
    {
        //ALOGE("mmap err, ret %d\n", (unsigned int)addr_vir);
        addr_vir = 0;
        goto ALLOC_OUT;
    }

    /* get phy address */
    memset(&phys_data, 0, sizeof(phys_data));
    phys_data.handle = alloc_data.handle;
    phys_data.size = size;
    custom_data.aw_cmd = ION_IOC_SUNXI_PHYS_ADDR;
    custom_data.aw_arg = (unsigned long)&phys_data;

    ret = ioctl(g_alloc_context_cam->fd, AW_MEM_ION_IOC_CUSTOM, &custom_data);
    if(ret) {
        ALOGE("ION_IOC_CUSTOM err, ret %d\n", ret);
        addr_phy = 0;
        addr_vir = 0;
        goto ALLOC_OUT;
    }

    addr_phy = phys_data.phys_addr;
#if(ADJUST_ADDRESS_FOR_SECURE_OS_OPTEE)
    memset(&tee_data, 0, sizeof(tee_data));
    tee_data.handle = alloc_data.handle;
    tee_data.size = size;
    custom_data.aw_cmd = ION_IOC_SUNXI_TEE_ADDR;
    custom_data.aw_arg = (unsigned long)&tee_data;
    ret = ioctl(g_alloc_context_cam->fd, AW_MEM_ION_IOC_CUSTOM, &custom_data);
    if(ret) {
        ALOGE("ION_IOC_CUSTOM err, ret %d\n", ret);
        addr_phy = 0;
        addr_vir = 0;
        goto ALLOC_OUT;
    }
    addr_tee = tee_data.phys_addr;
#else
    addr_tee = addr_vir;
#endif
    alloc_buffer = (buffer_node *)malloc(sizeof(buffer_node));
    if (alloc_buffer == NULL)
    {
        ALOGE("malloc buffer node failed");

        /* unmmap */
        ret = munmap((void*)addr_vir, alloc_data.aw_len);
        if(ret) {
            ALOGE("munmap err, ret %d\n", ret);
        }

        /* close dmabuf fd */
        close(fd_data.aw_fd);

        /* free buffer */
        handle_data.handle = alloc_data.handle;
        ret = ioctl(g_alloc_context_cam->fd, AW_MEM_ION_IOC_FREE, &handle_data);

        if(ret) {
            ALOGE("ION_IOC_FREE err, ret %d\n", ret);
        }

        addr_phy = 0;
        addr_vir = 0;        // value of MAP_FAILED is -1, should return 0

        goto ALLOC_OUT;
    }


    alloc_buffer->size        = size;
    alloc_buffer->phy         = addr_phy;
    alloc_buffer->user_virt = addr_vir;
    alloc_buffer->vir         = addr_tee;
    alloc_buffer->tee         = addr_tee;
    alloc_buffer->fd_data.handle = fd_data.handle;
    alloc_buffer->fd_data.aw_fd = fd_data.aw_fd;

    aw_mem_list_add_tail(&alloc_buffer->i_list, &g_alloc_context_cam->list);

    ALLOC_OUT:

    pthread_mutex_unlock(&g_mutex_alloc_cam);

    return (void*)addr_tee;
}

//return total meminfo with MB
int ion_cam_alloc_get_total_size()
{
    int ret = 0;

    int ion_fd = open(DEV_NAME, O_WRONLY);

    if (ion_fd < 0) {
        ALOGE("open ion dev failed, cannot get ion mem.");
        goto err;
    }

    struct sunxi_pool_info binfo = {
        .total   = 0,    // mb
        .free_kb  = 0, //the same to free_mb
        .free_mb = 0,
    };

    struct aw_ion_custom_info cdata;

    cdata.aw_cmd = ION_IOC_SUNXI_POOL_INFO;
    cdata.aw_arg = (unsigned long)&binfo;
    ret = ioctl(ion_fd,AW_MEM_ION_IOC_CUSTOM, &cdata);
    if (ret < 0){
        ALOGE("Failed to ioctl ion device, errno:%s\n", strerror(errno));
        goto err;
    }

    ALOGD(" ion dev get free pool [%d MB], total [%d MB]\n", binfo.free_mb, binfo.total / 1024);
    ret = binfo.total;
err:
    if(ion_fd > 0){
        close(ion_fd);
    }
    return ret;
}

int ion_cam_alloc_memset(void* buf, int value, size_t n)
{
    memset(buf, value, n);
    return -1;
}

int ion_cam_alloc_copy(void* dst, void* src, size_t n)
{
    memcpy(dst, src, n);
    return -1;
}

int ion_cam_alloc_read(void* dst, void* src, size_t n)
{
    memcpy(dst, src, n);
    return -1;
}

int ion_cam_alloc_write(void* dst, void* src, size_t n)
{
    memcpy(dst, src, n);
    return -1;
}

int ion_cam_alloc_setup()
{
    return -1;
}

int ion_cam_alloc_shutdown()
{
    return -1;
}

struct ScCamMemOpsS _ionCamMemOpsS =
{
    open_cam:                 ion_cam_alloc_open,
    close_cam:                 ion_cam_alloc_close,
    total_size_cam:         ion_cam_alloc_get_total_size,
    palloc_cam:             ion_cam_alloc_palloc,
    pfree_cam:                ion_cam_alloc_pfree,
    flush_cache_cam:        ion_cam_alloc_flush_cache,
    ve_get_phyaddr_cam:     ion_cam_alloc_vir2phy_ve,
    ve_get_viraddr_cam:     ion_cam_alloc_phy2vir_ve,
    cpu_get_phyaddr_cam:    ion_cam_alloc_vir2phy_cpu,
    cpu_get_viraddr_cam:    ion_cam_alloc_phy2vir_cpu,
    mem_set_cam:            ion_cam_alloc_memset,
    mem_cpy_cam:            ion_cam_alloc_copy,
    mem_read_cam:            ion_cam_alloc_read,
    mem_write_cam:            ion_cam_alloc_write,
    setup_cam:                ion_cam_alloc_setup,
    shutdown_cam:            ion_cam_alloc_shutdown,
    palloc_secure_cam:      ion_cam_alloc_alloc_drm
};

struct ScCamMemOpsS* __GetIonCamMemOpsS()
{ALOGE("------------------------------__GetIonCamMemOpsS------------------------------------------");
    ALOGD("*** get __GetIonCamMemOpsS ***");
    return &_ionCamMemOpsS;
}
