#include <unistd.h>
#include <string.h>
#include "NseMsg.h"
#include "cam_api.h"
#include <sys/time.h>

#include "slb_api.h"
//#define __DEBUG

#define OPCODE_LC               0x40
#define OPCODE_LD5              0x100
#define OPCODE_LD2              0x1
#define OPCODE_LD3              0x2
#define OPCODE_NOP              0x0

#if 1
typedef unsigned char       u8;
typedef char                s8;
typedef unsigned short      u16;
typedef short               s16;
typedef unsigned int        u32;
typedef int                 s32;
typedef unsigned long long  u64;
typedef long long           s64;
#endif

#define SWAP64(a)                                                                                 \
            {                                                                                     \
                a = ((a >> 8)  & 0x00ff00ff00ff00ffull) | ((a << 8  ) & (0xff00ff00ff00ff00ull)); \
                a = ((a >> 16) & 0x0000ffff0000ffffull) | ((a << 16 ) & (0xffff0000ffff0000ull)); \
                a = ((a >> 32) & 0x00000000ffffffffull) | ((a << 32 ) & (0xffffffff00000000ull)); \
            }

#define SWAP32(a)                           a = (((((a)>>24) & 0xff) << 0)  |\
                                                 ((((a)>>16) & 0xff) << 8)  |\
                                                 ((((a)>>8 ) & 0xff) << 16) |\
                                                 ((((a)>>0 ) & 0xff) << 24))

#define SWAP16(a)                           a = (((((a)>>8) & 0xff) << 0)  |\
                                                 ((((a)>>0) & 0xff) << 8))

TcError NseMsg_Create(NseMsg **msg)
{
    return 0;
}

TcError NseMsg_Destroy(NseMsg *msg)
{
    return 0;
}


static void __hexdump(const void *buff, int size);
static void __hexdump(const void *buff, int size)
{
    int i;

    if ((!buff) || (size <= 0)) {
        printf("(null)\n");
        return;
    }

    printf("0x");
    for (i = 0; i < size; i++) {
        printf("%02X", ((const uint8_t *)buff)[i]);
    }
    printf("\n");
}

static void swap80bit(uint8_t *buff)
{
    uint8_t tmp = 0;
    int i;

    for (i = 0; i <= 4;i ++) {
        tmp       = buff[i];
        buff[i]   = buff[9-i];
        buff[9-i] = tmp;
    }
}

TcError NseSendMsg(u32 opcode, u32 profile, u32 sbAddr, u32 width, u8* data, u32 dataLen, u8 *response, u32 responseLen)
{
    u8 dataIn[10];
    u32 dataInLen = 10;
    u8 maskIn[10];
    u32 maskInLen = 10;

    //return 0;
    
#ifdef __DEBUG
    printf("%s: data\n", __func__);
    __hexdump(data, dataLen);
#endif

    if(opcode == OPCODE_LD2) {
        /*cam_write_reg(cam_addr[26:16], 11'b01000000000);    // Write RA_CAM_ADDR_H and RA_CAM_ADD_L

        cam_write_entry(RA_CAM_DATA addr, data[4], 0);            // Write RA_CAM_DATA  
        cam_write_entry(RA_CAM_DATA addr, data[3], 0);  
        cam_write_entry(RA_CAM_DATA addr, data[2], 0);  
        cam_write_entry(RA_CAM_DATA addr, data[1], 0);  
        cam_write_entry(RA_CAM_DATA addr, data[0], 0);
        
        cam_write_entry(RA_CAM_DATA_MASK addr, 0, data[9]);            // Write RA_CAM_DATA_MASK
        cam_write_entry(RA_CAM_DATA_MASK addr, 0, data[8]);
        cam_write_entry(RA_CAM_DATA_MASK addr, 0, data[7]);
        cam_write_entry(RA_CAM_DATA_MASK addr, 0, data[6]);
        cam_write_entry(RA_CAM_DATA_MASK addr, 0, data[5]);

        cam_write_reg(RA_CAM_INST, 10'b1000000001);              // Write RA_CAM_INST  
        */

        memset(dataIn, 0, dataInLen);                              
        memcpy((void *)((u8 *)dataIn), (u8 *)data, dataInLen);    
        memset(maskIn, 0, maskInLen);                                  
        memcpy((void *)((u8 *)maskIn), (u8 *)data + 10, maskInLen); 

        swap80bit((void *)dataIn);
        swap80bit((void *)maskIn);
        
#ifdef __DEBUG
        printf("%s: dataIn\n", __func__);
        __hexdump(dataIn, sizeof(dataIn));

        printf("%s: maskIn\n", __func__);
        __hexdump(maskIn, sizeof(maskIn));
#endif

        cam_write_entry(0, dataIn, maskIn);
        //_exit(0);
    } 
    else if(opcode == 0) {
        //cam_write_entry(0, 0, 0);
    }
   return 0;
}


TcError NseMsg_SendNOP(NseMsg *msg, int portId)
{
    u32 opcode    = OPCODE_NOP;
    u32 profile   = 0;
    u32 sbAddr    = 0;
    u32 width     = 0;
    u8  data[10]; 
    u32 dataLen   = 20;
    u8 response[16];
    u32 responseLen = 16;

    memset(data,0,dataLen);

    NseSendMsg(opcode, profile, sbAddr, width, data, dataLen, response, responseLen);

    return 0;
}


static inline uint64_t tv_to_timestamp_usec(const struct timeval tv)
{
    return (tv.tv_sec * (1000lu * 1000) + tv.tv_usec);
}

   
TcError NseMsg_SendLD2(NseMsg *msg, int portId, u8 devId, const void *dataIn, int dataInLen)
{
    u32 opcode = OPCODE_LD2;
    u32 profile = 0;
    u32 sbAddr = 0;
    u32 width = 0;
    u8 data[20];
    u32 dataLen = 20;
    u8 response[16];
    u32 responseLen = 16;

    static int first = 0;

    memset(data, 0, dataLen);                                     // No need
    memcpy((void *)((u8 *)data), (void *)dataIn, dataInLen);      // No need 

#if 0
    {
        int i;
        for (i = 0; i < 20; i++) {
            data[i] = i;
        }
    }
#endif
    
    // Not sure whether byte swap needed
    // Swap format is that: before - ab cd, after -  cd ab, the basic unit is 16bit
    // for(i=0; i<10; i++)
    //    SWAP16(data[i]);    

#if 0
    if (0 == first) {
        int i;
        struct timeval tv;
        uint64_t timestamp_last;
        uint64_t timestamp_curr;
        gettimeofday(&tv, NULL);
        timestamp_last = tv_to_timestamp_usec(tv);
        for (i = 0; i < 10 * 10000; i++) {
            NseSendMsg(opcode, profile, sbAddr, width, (u8*)data, dataLen, response, responseLen);
        }
        gettimeofday(&tv, NULL);
        timestamp_curr = tv_to_timestamp_usec(tv);
        printf("%ld-%ld=%ld usec\n", timestamp_curr, timestamp_last,
            timestamp_curr - timestamp_last);
        first = 1;
    } else {
        NseSendMsg(opcode, profile, sbAddr, width, (u8*)data, dataLen, response, responseLen);
    }
#else
        NseSendMsg(opcode, profile, sbAddr, width, (u8*)data, dataLen, response, responseLen);
#endif

    //printf("%s: press any key to go!\n", __func__);
    //getchar(); 
    return 0;
}


TcError NseMsg_SendLD3(NseMsg*msg, int portId, u8 devId, u32 addr, void *resp, u16 *respSize)
{ 
    return TcE_Not_Implement;
}


TcError NseMsg_SendLD5(NseMsg* msg, int portId, u8 profileId, u16 sbAddr, u8 width, const void* dataIn, u32 dataInLen)
{
    return TcE_Not_Implement;
}


TcError NseMsg_SendLC(NseMsg *msg, int portId, u8 profile, u16 sbAddr, u8 width, const void *dataIn, u32 dataInLen, u8* readies, u8* matches, u32 *prioOut, void *adValue[4], u8 adwidth[4], u8 *eccErr)
{
    return TcE_Not_Implement;
}


TcError
NseMsg_SendMdio(
    NseMsg* msg,
    unsigned char    operation,
    unsigned char    mpId,
    unsigned char    dev,
    unsigned short   regAddr,
    unsigned short*  regData
    )
{
    return 0;
}

