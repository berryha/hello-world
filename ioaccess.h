#include "ast_type_def.h"

#define 	AST_MEM_ALLOC		1
#define     HW_RC4				1
#define     RC4_USE_INT         1
#define     USE_CMD_QUEUE       1
#define     WAIT_BLT_INT        0
#define     DOUBLE_BLT_DST      1
#define     WAIT_BULK_INT       1
#define     WAIT_JPEG_INT       1

//20151006 chance add for button shutdown
#define		WAIT_SHUTDOWN_TIME	5*1000000 // 5 secs
#define		WAIT_LOADDEFAULT_TIME	5*1000000 // 5 secs
enum _SCRACH_BOARD_INFO
{
	A0DEMO	= 0,    
	A0DEMO1,        
	A1DEMO,         
	A1DEMO24M,      
	A1L0,           
	A1AV,           
	A1UNKNOW,
	A1CRTAVGA
};    

#define SCRACH_BOARD_MASK     0x0F
#define SCRACH_CRT_A_ON_BIT   4
#define SCRACH_CRT_B_ON_BIT   5
#define SCRACH_CRT_C_ON_BIT   6
#define SCRACH_CRT_D_ON_BIT   7
#define SCRACH_HOTPLUG_BIT    8
#define SCRACH_CRT_MASK       0xF0
#define MMAVAIABLE_FLAG_BIT   24

#define RDPGFX_ENGINE_TIMEOUT       0xEFE7
#define RDPEVOR_ENGINE_TIMEOUT      0xEFE5

#define RDP_71_NOTIFY_FLAG          0xFFFF
#define RC4_HW_DECRYPTION           0
#define RC4_HW_ENCRYPTION           1

#define   AST3200_IOCTL_MAGIC  		0x3200

#define  SDRAM_BASE					0x80000000

#define  VIDEOMEM_BASE              0xC000000	// 
#define  VIDEOMEM_SIZE              0x6800000   // 104MB
#define  VIDEOMEM_MAP_SIZE			0x3800000	// 56 MB

#define HYSTY_RING_SIZE             0x800000    //8.0MB
#define HYSTY_GUARD_SIZE            0x400000    //4.0MB 
#define HYSTY_SYNC_SIZE             600000      //600k

#define MASK_FORMAT   0x7
#define YUV444_FORMAT	0
#define YUV420_FORMAT	1
#define RGB565_FORMAT	2
#define XRGB8888_FORMAT	3
#define RGB888_FORMAT	4
#define MIRROR_MODE     (1<<3)
#define FB_DISP         (1<<4)

#define MSG_EGFX (1<<0)
#define MSG_EVOR (1<<1)
#define MSG_H264 (1<<2)
#define MSG_JPEG (1<<3)

enum _CRT_IDX
{
	CRT_A = 0,
	CRT_B,
	CRT_C,
	CRT_D	
};

enum _CRT_REG
{
	CRT_A_REG = CRT_A<<8,
	CRT_B_REG = CRT_B<<8,
	CRT_C_REG = CRT_C<<8,
	CRT_D_REG = CRT_D<<8
};

enum _BREAK_LINE_HOTKEY
{
    BK_CTRL_PAUSE = 0,
    BK_CTRL_SHIFT_F1,
    BK_CTRL_SHIFT_F2,
    BK_CTRL_SHIFT_F3,
    BK_CTRL_SHIFT_F4,
    BK_CTRL_SHIFT_F5,
    BK_CTRL_SHIFT_F6,
    BK_CTRL_SHIFT_F7,
    BK_CTRL_SHIFT_F8,
    BK_CTRL_SHIFT_F9,
    BK_CTRL_SHIFT_F10,
    BK_CTRL_SHIFT_F11,
    BK_CTRL_SHIFT_F12,
};
//chance add for reset password
#define UNRESET_PASSWORD				0
#define RESET_PASSWORD					1
#define AST_DRIVER_LOADED               0
#define AST_DRIVER_UNLOAD               0xFF0000FF

#define SW_RECORD_BASE                  0x1e720000
#define POWERUP_CONTROL_OFFSET          0x0050  //use in u-boot ftgmac100.c
#define MM_AVALIABLE_OFFSET             0x0054  //use in u-boot cmd_logo.c
#define H264_FRAME_ID                   0x0060
#define POWER_STATE_OFFSET              0x0080
#define RFX_LOADED_OFFSET               0x0084
#define GPIO_LOADED_OFFSET              0x0088

#define POWER_CONNECTING				6
#define POWER_RESUME				    5
#define POWER_S3					    4
#define POWER_S2					    3
#define POWER_S1					    2
#define POWER_S0					    1
#define POWER_UNCONNECT				    0
                                        
#define  RELOCATE_OFFSET                0x380

#define  IOCTL_IO_READ			        0x1103
#define  IOCTL_IO_WRITE			        0x1104
#define  IOCTL_REAL_IO_READ		        0x1106
#define  IOCTL_REAL_IO_WRITE		    0x1107
#define  IOCTL_BIT_STREAM_BASE		    0x1108
#define  IOCTL_TX_BIT_STREAM		    0x1109
#define  IOCTL_GET_SOCKET		        0x1110
#define  IOCTL_TRIGGER			        0x1111
#define  IOCTL_USB20_MOUSE_PACKET	    0x1112
#define  IOCTL_MEMORY_OVER_READ		    0x1116
#define  IOCTL_MEMORY_OVER_WRITE	    0x1117
#define  IOCTL_IO_SCRATCH_SD		    0x1118
#define  IOCTL_IO_SCRATCH_USBHOST	    0x1119
#define  IOCTL_REMAP                    0x1105
#define  IOCTL_IO_SLEEP			        0x1124
#define  IOCTL_IO_POWEROFF				0x1125
#define  IOCTL_MEMORY_TEST              0x1126

#define  IOCTL_JPEG_RESET               0x112A
#define  IOCTL_JPEG_DECODE              0x112B
                                        
#define IOCTL_AST_GET_BOARD_INFO	    0x920
                                        
#define IOCTL_AST_FB_BASE			    0x922
#define IOCTL_AST_GETMAP_BASE           0x923
#define IOCTL_AST_VBIOSSETMODE    	    0x924
#define IOCTL_AST_SNEDTILESET   	    0x925
#define IOCTL_AST_RFXENGFIRE		    0x926
#define IOCTL_AST_EGFXCMD		        0x927
#define IOCTL_AST_ENGSTATUS			    0x928
#define IOCTL_AST_CLEAR_FRAME_CNT       0x929
#define IOCTL_AST_DEBUG_FLAG		    0x92A
#define IOCTL_GET_EDID                  0x92B
#define IOCTL_AST_FRAME_END             0x92C
#define IOCTL_AST_FRAME_BEGIN           0x92D
#define	IOCTL_AST_CONNECTION_CREATED    0x92E
#define	IOCTL_AST_CONNECTION_STABLE     0x92F                                        

#define  IOCTL_AST_IO_READ			    0x930
#define  IOCTL_AST_IO_WRITE			    0x931
#define IOCTL_AST_SETSOCREG			    0x932
#define AST3200_IOCTL_RLE_16_TO_16	    0x933
#define IOCTL_AST_RC4_CODEC			    0x934
#define IOCTL_AST_SETMEMDATA		    0x935
#define IOCTL_AST_GETMEMDATA		    0x936
#define IOCTL_AST_R_IDX_PORT		    0x937
#define IOCTL_AST_W_IDX_PORT		    0x938
#define IOCTL_AST_READ_PORT			    0x939		
#define IOCTL_AST_WRITE_PORT		    0x93A	
#define IOCTL_AST_SETCURSORPOS          0x93B
#define IOCTL_AST_SETCURSORPATTERN      0x93C
#define IOCTL_IO_ENABLE_CRT             0x93D
#define IOCTL_IO_SET_VIDEO_BASE         0x93E
#define IOCTL_DISABLE_CRT               0x93F
#define IOCTL_ENABLE_FB                 0x940
#define IOCTL_I2C_READ_PORT             0x941
#define IOCTL_I2C_WRITE_PORT            0x942
#define	IOCTL_FB_LINEAR_READ 		    0x943
#define IOCLT_BLT_16TO16			    0x944
#define IOCLT_BLT_SURF_TO_SURF		    0x945
#define IOCLT_COPY_MEM_SYNC			    0x946
#define IOCLT_COPY_MEM_ASYNC		    0x947
#define IOCLT_BLT_SURF_TO_SCREEN	    0x948
#define IOCTL_MONITOR_POSITION          0x949
#define IOCTL_ENABLE_CURSOR			    0x94A
#define IOCTL_DISABLE_CURSOR		    0x94B
#define AST3200_IOCTL_SURFACEOFFSET     0x94D
#define AST3200_IOCTL_CODEC_NON_ID	    0x94E
#define IOCTL_CAVIDEO_ENABLE            0x94F
#define IOCTL_CHECK_ENG_STATE           0x950
#define IOCTL_AST_READ_ENG_ADDR		    0x951
#define IOCTL_AST_S_G_ENG_DATA		    0x952
#define IOCTL_AST_S_G_ENG_RESET		    0x953
#define IOCTL_INIT_RFX_RING             0x954
#define IOCTL_AST_RDP_END			    0x955
#define IOCTL_GET_POWER_STATE		    0x956
#define IOCTL_WAIT_POWER_S0			    0x957
#define IOCTL_WAIT_POWER_S1			    0x958
#define IOCTL_WAIT_POWER_RESUME		    0x959
#define IOCTL_SET_PERFORMANCE           0x95A
#define IOCTL_GET_PERFORMANCE           0x95B
#define IOCTL_SET_POWER_STATE		    0x95C
#define IOCTL_IO_ENABLE_CRT_2           0x95D

#define IOCTL_GET_PREFER_MODE		    0x960
#define IOCTL_FLASH_ERASE               0x961
#define IOCTL_FLASH_WRITE               0x962
#define IOCTL_GET_FLASH_USER_INFO       0x963
#define IOCTL_GET_FLASH_WALLPAPER_DATA  0x964
#define IOCTL_WRITE_FLASH_USER_INFO     0x965
#define IOCTL_WRITE_FLASH_USER_DATA     0x966
#define IOCTL_WRITE_FLASH_WALLPAPER     0x967
#define IOCTL_CHECK_USB_HOTPLUG         0x968
#define IOCTL_SET_GPIO_O3				0x969
#define IOCTL_GET_GPIO_O3				0x96A
#define IOCTL_WAIT_GPIO_O3              0x96B
#define IOCTL_SET_GPIO_H0               0x96C
#define IOCTL_GET_GPIO_H0               0x96D
#define IOCTL_SET_SLEEP_MODE			0x96E
#define IOCTL_GET_SLEEP_MODE			0x96F
#define IOCTL_SET_WOL_MODE				0x970
#define IOCTL_GET_WOL_MODE				0x971
#define IOCTL_WATCHDOG_RESET            0x972
#define IOCTL_UPDATE_HISTRY_WRITE_PORT  0x978

#define IOCTL_SET_RING_BUFFER_BASE      0x97F
#define IOCTL_AST_BULK_DATA             0x980
#define IOCTL_DE_CLEAR_CODEC            0x981
#define IOCTL_SURFACE_TO_CACHE          0x982
#define IOCTL_GFX_CREATE_SURFACE        0x983
#define IOCTL_CACHE_TO_SURFACE          0x984
#define IOCTL_SURFACE_TO_SURFACE        0x985
#define IOCTL_DE_CAPROGRESSIVE          0x986
#define IOCTL_SURFACE_SOLIDFILL         0x987
#define IOCTL_MAP_SURFACE_OUTPUT        0x988
#define IOCTL_ENABLE_GPIO_F5		    0x989
#define IOCTL_EVOR_CTRL                 0x98A
#define IOCTL_EVOR_DATA                 0x98B
#define IOCTL_EGT_DATA                  0x98C

#define IO_MASK_WRITE 		1

#define AST3200_VCLK25_175  (0x0E << 27) | (0x15 << 22) | (0x03D << 13) | (0x01 << 8) | 0x40
#define AST3200_VCLK28_322  (0x0E << 27) | (0x15 << 22) | (0x031 << 13) | (0x00 << 8) | 0x3A
#define AST3200_VCLK31_5    (0x0E << 27) | (0x15 << 22) | (0x02F << 13) | (0x00 << 8) | 0x3E
#define AST3200_VCLK36      (0x0E << 27) | (0x15 << 22) | (0x023 << 13) | (0x00 << 8) | 0x35
#define AST3200_VCLK40      (0x0E << 27) | (0x15 << 22) | (0x01D << 13) | (0x00 << 8) | 0x31
#define AST3200_VCLK49_5    (0x0E << 27) | (0x15 << 22) | (0x01F << 13) | (0x00 << 8) | 0x41
#define AST3200_VCLK50      (0x0E << 27) | (0x15 << 22) | (0x017 << 13) | (0x00 << 8) | 0x31
#define AST3200_VCLK56_25   (0x0E << 27) | (0x15 << 22) | (0x01F << 13) | (0x00 << 8) | 0x4A
#define AST3200_VCLK65      (0x0E << 27) | (0x15 << 22) | (0x017 << 13) | (0x00 << 8) | 0x40
#define AST3200_VCLK75      (0x0E << 27) | (0x15 << 22) | (0x00F << 13) | (0x00 << 8) | 0x31
#define AST3200_VCLK78_75   (0x0E << 27) | (0x15 << 22) | (0x011 << 13) | (0x00 << 8) | 0x3A
#define AST3200_VCLK94_5    (0x0E << 27) | (0x15 << 22) | (0x00F << 13) | (0x00 << 8) | 0x3E
#define AST3200_VCLK108     (0x0E << 27) | (0x15 << 22) | (0x00B << 13) | (0x00 << 8) | 0x35
#define AST3200_VCLK135     (0x0E << 27) | (0x15 << 22) | (0x00A << 13) | (0x00 << 8) | 0x3D
#define AST3200_VCLK157_5   (0x0E << 27) | (0x15 << 22) | (0x00B << 13) | (0x00 << 8) | 0x4E
#define AST3200_VCLK162     (0x0E << 27) | (0x15 << 22) | (0x007 << 13) | (0x00 << 8) | 0x35
#define AST3200_VCLK154     (0x0E << 27) | (0x15 << 22) | (0x00B << 13) | (0x00 << 8) | 0x4C
#define AST3200_VCLK83_5    (0x0E << 27) | (0x15 << 22) | (0x016 << 13) | (0x00 << 8) | 0x4F
#define AST3200_VCLK106_5   (0x0E << 27) | (0x15 << 22) | (0x00F << 13) | (0x00 << 8) | 0x46
#define AST3200_VCLK146_25  (0x0E << 27) | (0x15 << 22) | (0x009 << 13) | (0x00 << 8) | 0x3C
#define AST3200_VCLK148_5   (0x0E << 27) | (0x15 << 22) | (0x009 << 13) | (0x00 << 8) | 0x3D
#define AST3200_VCLK71      (0x0E << 27) | (0x15 << 22) | (0x017 << 13) | (0x00 << 8) | 0x46
#define AST3200_VCLK88_75   (0x0E << 27) | (0x15 << 22) | (0x013 << 13) | (0x00 << 8) | 0x49
#define AST3200_VCLK119     (0x0E << 27) | (0x15 << 22) | (0x00F << 13) | (0x00 << 8) | 0x4E
#define AST3200_VCLK85_5    (0x0E << 27) | (0x15 << 22) | (0x00F << 13) | (0x00 << 8) | 0x38
#define AST3200_VCLK85_8    (0x0E << 27) | (0x15 << 22) | (0x012 << 13) | (0x00 << 8) | 0x40

typedef struct _SCRACH_DATA {
    unsigned int Reserved0;
    unsigned int PowerState;
}SCRACH_DATA, *PSCRACH_DATA;

typedef struct _IO_ACCESS_DATA {
    unsigned char Type;
    unsigned long Address;
    unsigned long Data;
    unsigned long Value;
    union
    {
        unsigned long I2CValue;
        unsigned long ExtValue;
    };
    unsigned long ExtData;
} IO_ACCESS_DATA, *PIO_ACCESS_DATA;

typedef struct _IO_CTRL_DATA {
    unsigned int Address;
    unsigned int Data;
    unsigned int Value;
} IO_CTRL_DATA, *PIO_CTRL_DATA;

typedef struct _IO_REGION_DATA {
	unsigned int    Size;
	unsigned int    Address;
	unsigned short  destLeft;
	unsigned short  destTop;
	unsigned short  destRight;
	unsigned short  destBottom;
	unsigned short  width;
	unsigned short  height;
	unsigned short  surfId;
	unsigned char*  pdata;
}IO_REGION_DATA, *PIO_REGION_DATA;

typedef struct _IO_CURSOR_POS {
    unsigned short Xpos;
    unsigned short Ypos;
    unsigned int   crtidx;
}IO_CURSOR_POS, *PIO_CURSOR_POS;    

typedef struct _JPEG_DECODE_DATA
{
    unsigned int    size;
    unsigned int    InAddr;
    unsigned int	OutAddr;
    short           width;
    short           height;
}JPEG_DECODE_DATA, JPEGIOINFO;

typedef struct _RLE_DECODE_DATA
{
    unsigned int    bs_addr;
    unsigned int    out_addr;
    unsigned int    Width;
    unsigned int    Height;
    unsigned int    Color;
    unsigned int    Length;
    unsigned int    bFlip;
}RLE_DECODE_DATA;

typedef struct _IO_BLT_DATA
{
    unsigned int    InAddr;
    unsigned int	OutAddr;
    short           Width;
    short           Height;
	short			StartX;
	short   		StartY;	
}IO_BLT_DATA, *PIO_BLT_DATA;

typedef struct _IO_BITBLT_DATA
{
    unsigned int    InAddr;
    unsigned int	OutAddr;
    unsigned short  Width;
    unsigned short  Height;
	unsigned short	SrcX;
	unsigned short 	SrcY;
	unsigned short	DstX;
	unsigned short	DstY;		
	unsigned short  SrcPitch;
	unsigned short  DstPitch;
}IO_BITBLT_DATA, *PIO_BITBLT_DATA;

typedef struct _IO_COPY_DATA
{
    unsigned int    InAddr;
    unsigned int	OutAddr;
    unsigned int    Size;
}IO_COPY_DATA, *PIO_COPY_DATA;

typedef struct _HWRC4KEY
{
    unsigned char data[16+256];
}HWRC4KEY;

typedef struct _RC4DEDATA
{
    unsigned int length;
    unsigned int *pData;
    unsigned int *pKey;    
    unsigned int encrypt;
}RC4DEDATA;

typedef struct _ENGS_READ_PTR
{
	unsigned int RC4BSAddr;     //1e6e3000
	unsigned int RFXBSAddr;		//1e6eB00C
	unsigned int BMPBSAddr;		//1e6eB404
	unsigned int RLEBSAddr;		//1e6eB30C
	unsigned int BulkBSAddr;	//1e6eB10C
	unsigned int H264BSAddr;	//chek sw  ptr
}ENGS_READ_PTR;

typedef struct _DEBULK_DATA
{
	unsigned int    outSize;
	void*           outAddr;
	unsigned int    inSize;
	void*           inData;
	unsigned int    EndFlag;
	unsigned int    FrameId;
	unsigned int    FrameCnt;
	unsigned int    HistryRead;
	unsigned int    HistryWrite;
}DEBULK_DATA;

typedef struct _DECLEAR_DATA
{
    unsigned int    StrAddr;
	unsigned short	SrcL;
	unsigned short 	SrcT;
    unsigned short  SrcR;
    unsigned short  SrcB;	
	unsigned int    Length;
	unsigned short  surfId;
}DECLEAR_DATA;

typedef struct _PREFER_MODE
{
    int    port;
    int    width;
    int    height;
}PREFER_MODE;

#define		HOR_NEGATIVE				1
#define		HOR_POSITIVE				0
#define		VER_NEGATIVE				1
#define		VER_POSITIVE				0

typedef struct _VESA_MODE
{
    USHORT    HorizontalTotal;
    USHORT    VerticalTotal;
    USHORT    HorizontalActive;
    USHORT    VerticalActive;
    BYTE      RefreshRate;
    USHORT    HSyncTime;
    USHORT    HBackPorch;
    USHORT    VSyncTime;
    USHORT    VBackPorch;
    USHORT    HLeftBorder;
    USHORT    HRightBorder;
    USHORT    VBottomBorder;
    USHORT	  VTopBorder;
	ULONG	  PixelClock;
	ULONG	  HorPolarity;
	ULONG	  VerPolarity;
    int       HorizontalShift;
    int       VerticalShift;
} VESA_MODE;

typedef struct _CRT_MODE
{
    int         crtidx;
    int         format;
    VESA_MODE   mode;
    int         offAudio;
    BYTE        Type;
}CRT_MODE;
