
#include "system_common.h"
#include "drv_spiflash.h"
#include "soc_pin_mux.h"
#include "soc_top_reg.h"


#define SPIFLASH_TIMEOUT_COUNT	0xFFFFFFFF

#define SPIFLASH_PAGE_SIZE		0x100	//256B
#define SPIFLASH_SECTOR_SIZE	0x1000 	// 4KB
//#define SPIFLASH_MEM_LENGTH	(0x00400000) //4MB
#define SPIFLASH_MEM_LENGTH	(0x00200000)  //2MB

#define RESPONSE_SIZE				0x01
#define RESPONSE_FAIL				0xFF
#define RESPONSE_OK					0x00


#define SPIFLASH_CMD_WREN		0x06
#define SPIFLASH_CMD_WRSR		0x01
#define SPIFLASH_CMD_WRSR_2		0x31

#define SPIFLASH_CMD_RDSR		0x05
#define SPIFLASH_CMD_RDSR_Cfg	0x15

#define SPIFLASH_CMD_RDSR_H	0x35
#define SPIFLASH_CMD_SE		0x20
#define SPIFLASH_CMD_4PP		0x32
#define SPIFLASH_CMD_PP		0x02
#define SPIFLASH_CMD_RDID		0x9F
#define SPIFLASH_CMD_QUAD_READ	0xEB

#define SPIFLASH_CMD_OTP_SE 0x44
#define SPIFLASH_CMD_OTP_PP 0x42
#define SPIFLASH_CMD_OTP_RD 0x48

#define SPIFLASH_STATUS_WIP	0x01
#define SPIFLASH_STATUS_WEL	0x02

#define CLOSE   0x00


/* 0x20 - spi transfer control register*/
#define SPI_TRANSCTRL_RCNT(x)			(((x) & 0x1FF) << 0)
#define SPI_TRANSCTRL_WCNT(x)			(((x) & 0x1FF) << 12)
#define SPI_TRANSCTRL_DUALQUAD(x)		(((x) & 0x3) << 22)
#define SPI_TRANSCTRL_TRAMODE(x)		(((x) & 0xF) << 24)

#define SPI_TRANSCTRL_DUALQUAD_REGULAR	SPI_TRANSCTRL_DUALQUAD(0)
#define SPI_TRANSCTRL_DUALQUAD_DUAL		SPI_TRANSCTRL_DUALQUAD(1)
#define SPI_TRANSCTRL_DUALQUAD_QUAD		SPI_TRANSCTRL_DUALQUAD(2)

#define SPI_TRANSCTRL_TRAMODE_WRCON		SPI_TRANSCTRL_TRAMODE(0)	/* w/r at the same time */
#define SPI_TRANSCTRL_TRAMODE_WO		SPI_TRANSCTRL_TRAMODE(1)	/* write only		*/
#define SPI_TRANSCTRL_TRAMODE_RO		SPI_TRANSCTRL_TRAMODE(2)	/* read only		*/
#define SPI_TRANSCTRL_TRAMODE_WR		SPI_TRANSCTRL_TRAMODE(3)	/* write, read		*/
#define SPI_TRANSCTRL_TRAMODE_RW		SPI_TRANSCTRL_TRAMODE(4)	/* read, write		*/
#define SPI_TRANSCTRL_TRAMODE_WDR		SPI_TRANSCTRL_TRAMODE(5)	/* write, dummy, read	*/
#define SPI_TRANSCTRL_TRAMODE_RDW		SPI_TRANSCTRL_TRAMODE(6)	/* read, dummy, write	*/
#define SPI_TRANSCTRL_TRAMODE_NONE		SPI_TRANSCTRL_TRAMODE(7)	/* none data */
#define SPI_TRANSCTRL_TRAMODE_DW		SPI_TRANSCTRL_TRAMODE(8)	/* dummy, write	*/
#define SPI_TRANSCTRL_TRAMODE_DR		SPI_TRANSCTRL_TRAMODE(9)	/* dummy, read	*/

#define SPI_TRANSCTRL_CMD_EN			(1<<30)
#define SPI_TRANSCTRL_ADDR_EN			(1<<29)
#define SPI_TRANSCTRL_ADDR_FMT			(1<<28)
#define SPI_TRANSCTRL_TOKEN_EN			(1<<21)

#define SPI_TRANSCTRL_DUMMY_CNT_1		(0<<9)
#define SPI_TRANSCTRL_DUMMY_CNT_2		(1<<9)
#define SPI_TRANSCTRL_DUMMY_CNT_3		(2<<9)



/* 0x30 - spi control register */
#define SPI_CONTROL_SPIRST				BIT(0)
#define SPI_CONTROL_RXFIFORST			BIT(1)
#define SPI_CONTROL_TXFIFORST			BIT(2)


/* 0x34 - spi status register */
#define SPI_STATUS_BUSY					BIT(0)
#define SPI_STATUS_RXNUM(X)				(((X) >> 8) & 0x1FF)
#define SPI_STATUS_RXENPTY				BIT(14)
#define SPI_STATUS_TXFULL				BIT(23)

#define SPI_PREPARE_BUS(X)			\
	do{unsigned int spi_status = 0;				\
	do {								\
		spi_status = (X);	\
	} while(spi_status & SPI_STATUS_BUSY);}while(0)


#define SPI_CLEAR_FIFO(X)			X |= (SPI_CONTROL_RXFIFORST|SPI_CONTROL_TXFIFORST)

#define SPI_WAIT_RX_READY(X)		\
	do{unsigned int spi_status_r = 0;			\
	do {								\
		spi_status_r = (X);	\
	} while(spi_status_r & SPI_STATUS_RXENPTY);}while(0)

#define SPI_WAIT_TX_READY(X)		\
	do{unsigned int spi_status_t = 0;			\
	do {								\
		spi_status_t = (X);	\
	} while(spi_status_t & SPI_STATUS_TXFULL);}while(0)



/****************************************************************************
* 	                                        Types
****************************************************************************/
typedef struct _spi_regs {
			volatile unsigned int	edRer;		/* 0x00 		 - id and revision reg*/
			volatile unsigned int	rev1[3];		/* 0x04-0x0C - reserved reg */
			volatile unsigned int	transFmt;	/* 0x10 		 - spi transfer format reg */
			volatile unsigned int	directIO;	/* 0x14 		 - spi direct io control reg */
			volatile unsigned int	rev2[2];		/* 0x18-0x1C - reserved reg */
			volatile unsigned int	transCtrl;	/* 0x20 		 - spi transfer control reg */
			volatile unsigned int	cmd;		/* 0x24 		 - spi command reg */
			volatile unsigned int	addr;		/* 0x28 		 - spi address reg */
			volatile unsigned int	data;		/* 0x2C 		 - spi data reg */	
			volatile unsigned int	ctrl;			   /* 0x30			- spi control reg */
			volatile unsigned int	status; 	/* 0x34 		 - spi status reg */
			volatile unsigned int	intrEn; 	/* 0x38 		 - spi interrupt enable reg */
			volatile unsigned int	intrSt; 	/* 0x3C 		 - spi interrupt status reg */
			volatile unsigned int	timing; 	/* 0x40 		 - spi interface timing reg */
			volatile unsigned int	rev3[3];		/* 0x44-0x4C - reserved reg */
			volatile unsigned int	memCtrl;	/* 0x50 		 - spi memery access control reg */
			volatile unsigned int	rev4[3];		/* 0x54-0x5C - reserved reg */
			volatile unsigned int	stvSt;		/* 0x60 		 - spi slave status reg */
			volatile unsigned int	slvDataCnt; /* 0x64 		 - spi slave data count reg */
			volatile unsigned int	rev5[5];		/* 0x68-0x78  - spi status reg */
			volatile unsigned int	config; 	/* 0x7C 		 - configuration reg */
}spi_reg;

typedef struct {
	unsigned char cmd;				//flash SPI Command
	unsigned int transCtrl;			//SPI Transfer Control
}FLASH_CTRL_CMD;

typedef struct {
	FLASH_CTRL_CMD read;
	FLASH_CTRL_CMD program;
	FLASH_CTRL_CMD erase;
}FLASH_CMD;

typedef struct {
	unsigned int flashid;
	FLASH_CMD REGULAR;
	FLASH_CMD DUAL;
	FLASH_CMD QUAD;
}SPI_FLASH;


unsigned int Flash_ID;
SPI_FLASH * Flash_assign;
FLASH_CMD * Cmd_assign;
typedef struct _spi_dev {

	spi_reg * spiReg;

} spi_dev;


static spi_dev spiDev;


SPI_FLASH gflashobj[]=
{ //FlashID , CMD   ,	CmdEn      [30] 		|	AddrEn 	 [29] 					|	AddrFmt [28]
// 						TransMode  [27:24]		| 	DualQuad [23:22] 				|	TokenEn [21] 
//  					TokenValue [11] 		|	DummyCnt [10:9]
//XT25F08B_CMD
	{ 0x14400b, 
// XT25F08B_REGULAR	//read
		{	{ 0x0B 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN	 			| CLOSE  				  
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE    
					| CLOSE	     				| SPI_TRANSCTRL_DUMMY_CNT_1			},
					// write
			{ 0x02 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN				| CLOSE  				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE						| CLOSE   							},
			{ CLOSE	, CLOSE}
		},
// XT25F08B_DUAL	//read
		{	{ 0xBB 	, SPI_TRANSCTRL_CMD_EN	   	| SPI_TRANSCTRL_ADDR_EN	 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_RO 	| SPI_TRANSCTRL_DUALQUAD_DUAL		| SPI_TRANSCTRL_TOKEN_EN
					|CLOSE	     				| CLOSE								},
					// write
			{ 0x02 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN	 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE						| CLOSE								},
			{ CLOSE	,CLOSE}
		},
// XT25F08B_QUAD	//read
		{	{ 0xEB 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN	 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_QUAD		| SPI_TRANSCTRL_TOKEN_EN
					| CLOSE						| SPI_TRANSCTRL_DUMMY_CNT_2			},
					// write
			{ 0x38 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN	 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_QUAD		| CLOSE
					| CLOSE						| CLOSE								},
			{ CLOSE	,CLOSE}
		} 
	},
	
	{ 0x15400B, 	//XT25F16B_ID
		// XT25F16B_REGULAR //read
		{	{ 0x0B	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE    
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_1 		},
					// write
			{ 0x02	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE , CLOSE}
		},
		// XT25F16B_DUAL	//read
		{	{ 0xBB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_RO	| SPI_TRANSCTRL_DUALQUAD_DUAL		| SPI_TRANSCTRL_TOKEN_EN
					|CLOSE						| CLOSE 							},
					// write
			{ 0x02	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		},
		// XT25F16B_QUAD	//read
		{	{ 0xEB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_QUAD		| SPI_TRANSCTRL_TOKEN_EN
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_2 		},
			// write
			{ 0x32	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_QUAD		| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		} 
	},
//P25Q80H_CMD
    { 0x146085,		//P25Q80H_ID
// P25Q80H_REGULAR	//read
		{	{ 0x0B 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN	 			| CLOSE  				  
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE    
					| CLOSE	     				| SPI_TRANSCTRL_DUMMY_CNT_1			},
					// write
			{ 0x02 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN				| CLOSE  				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE						| CLOSE   							},
			{ CLOSE	,CLOSE}
		},
// P25Q80H_DUAL	//read
		{	{ 0xBB 	, SPI_TRANSCTRL_CMD_EN	   	| SPI_TRANSCTRL_ADDR_EN	 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_RO 	| SPI_TRANSCTRL_DUALQUAD_DUAL		| SPI_TRANSCTRL_TOKEN_EN
					|CLOSE	     				| CLOSE								},
					// write
			{ 0xA2 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN	 			| CLOSE   				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_DUAL		| CLOSE
					| CLOSE      				| CLOSE								},
			{ CLOSE	,CLOSE}
		},
// P25Q80H_QUAD	//read
		{	{ 0xEB 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN	 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_QUAD		| SPI_TRANSCTRL_TOKEN_EN
					| CLOSE						| SPI_TRANSCTRL_DUMMY_CNT_2			},
					// write
			{ 0x32 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN	 			| CLOSE 
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_QUAD		| CLOSE
					| CLOSE	     				| CLOSE   							},
			{ CLOSE	,CLOSE}
		} 
	},
//***********************************************20200312******************************************************************//
//
//P25Q16H_CMD
    { 0x156085,		//P25Q16H_ID
// P25Q16H_REGULAR	//read
		{	{ 0x0B 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN	 			| CLOSE  				  
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE    
					| CLOSE	     				| SPI_TRANSCTRL_DUMMY_CNT_1			},
					// write
			{ 0x02 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN				| CLOSE  				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE						| CLOSE   							},
			{ CLOSE	,CLOSE}
		},
// P25Q16H_DUAL	//read
		{	{ 0xBB 	, SPI_TRANSCTRL_CMD_EN	   	| SPI_TRANSCTRL_ADDR_EN	 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_RO 	| SPI_TRANSCTRL_DUALQUAD_DUAL		| SPI_TRANSCTRL_TOKEN_EN
					|CLOSE	     				| CLOSE								},
					// write
			{ 0xA2 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN	 			| CLOSE   				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_DUAL		| CLOSE
					| CLOSE      				| CLOSE								},
			{ CLOSE	,CLOSE}
		},
// P25Q16H_QUAD	//read
		{	{ 0xEB 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN	 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_QUAD		| SPI_TRANSCTRL_TOKEN_EN
					| CLOSE						| SPI_TRANSCTRL_DUMMY_CNT_2			},
					// write
			{ 0x32 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN	 			| CLOSE 
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_QUAD		| CLOSE
					| CLOSE	     				| CLOSE   							},
			{ CLOSE	,CLOSE}
		} 
	},


//
//*****************************************************************************************************************//
//
//	
//P25Q32H_CMD
	{ 0x166085, 	//P25Q80H_ID
// P25Q32H_REGULAR	//read
		{	{ 0x0B	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE    
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_1 		},
					// write
			{ 0x02	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		},
// P25Q32H_DUAL //read
		{	{ 0xBB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_RO	| SPI_TRANSCTRL_DUALQUAD_DUAL		| SPI_TRANSCTRL_TOKEN_EN
					|CLOSE						| CLOSE 							},
					// write
			{ 0xA2	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_DUAL		| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		},
// P25Q32H_QUAD //read
		{	{ 0xEB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_QUAD		| SPI_TRANSCTRL_TOKEN_EN
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_2 		},
					// write
			{ 0x32	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_QUAD		| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		} 
	},
//
//
//*****************************************************************************************************************//
//
//	

//GD25Q80C_CMD
    { 0x1440c8,		//GD25Q80C_ID
// GD25Q80C_REGULAR	//read
		{	{ 0x0B 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN	 			| CLOSE  				  
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE    
					| CLOSE	     				| SPI_TRANSCTRL_DUMMY_CNT_1			},
					// write
			{ 0x02 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN				| CLOSE  				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE						| CLOSE   							},
			{ CLOSE	, CLOSE}
		},
// GD25Q80C_DUAL	//read
		{	{ 0xBB 	, SPI_TRANSCTRL_CMD_EN	   	| SPI_TRANSCTRL_ADDR_EN	 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_RO 	| SPI_TRANSCTRL_DUALQUAD_DUAL		| SPI_TRANSCTRL_TOKEN_EN
					|CLOSE	     				| CLOSE								},
					// write
			{ 0x02 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN	 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE						| CLOSE								},
			{ CLOSE	,CLOSE}
		},
// GD25Q80C_QUAD	//read
		{	{ 0xEB 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN	 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_QUAD		| SPI_TRANSCTRL_TOKEN_EN
					| CLOSE						| SPI_TRANSCTRL_DUMMY_CNT_2			},
					// write
			{ 0x32 	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN	 			| CLOSE 
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_QUAD		| CLOSE
					| CLOSE						| CLOSE								},
			{ CLOSE	,CLOSE}
		} 
	},
	//ZB25VQ08_CMD
	{ 0x14605E, 	//ZB25VQ08_ID
		// ZB25VQ08_REGULAR 
		{	
			//read
			{ 0x0B	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE    
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_1 		},
			// write
			{ 0x02	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE , CLOSE}
		},
		// ZB25VQ08_DUAL	
		{	
			//read
			{ 0xBB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_RO	| SPI_TRANSCTRL_DUALQUAD_DUAL		| SPI_TRANSCTRL_TOKEN_EN
					|CLOSE						| CLOSE 							},
			// write
			{ 0x02	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		},
		// ZB25VQ08_QUAD	
		{	
			//read
			{ 0xEB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_QUAD		| SPI_TRANSCTRL_TOKEN_EN
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_2 		},
			// write
			{ 0x32	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_QUAD		| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		} 
	},
	//ZB25VQ16_CMD
	{ 0x15605E, 	//ZB25VQ16_ID
		// ZB25VQ16_REGULAR 
		{	
			//read
			{ 0x0B	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE    
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_1 		},
			// write
			{ 0x02	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE , CLOSE}
		},
		// ZB25VQ16_DUAL	
		{	
			//read
			{ 0xBB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_RO	| SPI_TRANSCTRL_DUALQUAD_DUAL		| SPI_TRANSCTRL_TOKEN_EN
					|CLOSE						| CLOSE 							},
			// write
			{ 0x02	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		},
		// ZB25VQ16_QUAD	
		{	
			//read
			{ 0xEB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_QUAD		| SPI_TRANSCTRL_TOKEN_EN
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_2 		},
			// write
			{ 0x32	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_QUAD		| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		} 
	},
	//
	//
	//*****************************************************************************************************************//
	//
	//
	//ZB25VQ32_CMD
	{ 0x16405E, 	//ZB25VQ032_ID
		// ZB25VQ032_REGULAR 
		{	
			//read
			{ 0x0B	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE    
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_1 		},
			// write
			{ 0x02	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE , CLOSE}
		},
		// ZB25VQ32_DUAL	
		{	
			//read
			{ 0xBB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_RO	| SPI_TRANSCTRL_DUALQUAD_DUAL		| SPI_TRANSCTRL_TOKEN_EN
					|CLOSE						| CLOSE 							},
			// write
			{ 0x02	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		},
		// ZB25VQ32_QUAD	
		{	
			//read
			{ 0xEB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_QUAD		| SPI_TRANSCTRL_TOKEN_EN
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_2 		},
			// write
			{ 0x32	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_QUAD		| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		} 
	},
	//
	//************************************************************************************************************//
	//
	//XM25QH80B_CMD
	{ 0x144020, 	//XM25QH80B_ID
		// XM25QH80B_REGULAR 
		{	
			//read
			{ 0x0B	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE    
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_1 		},
			// write
			{ 0x02	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE , CLOSE}
		},
		// XM25QH80B_DUAL	
		{	
			//read
			{ 0xBB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_RO	| SPI_TRANSCTRL_DUALQUAD_DUAL		| SPI_TRANSCTRL_TOKEN_EN
					|CLOSE						| CLOSE 							},
			// write
			{ 0x02	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		},
		// XM25QH80B_QUAD	
		{	
			//read
			{ 0xEB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_QUAD		| SPI_TRANSCTRL_TOKEN_EN
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_2 		},
			// write
			{ 0x32	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_QUAD		| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		} 
	},
	{ 0x154020, 	//XM25QH16B_ID
		// XM25QH16B_REGULAR 
		{	
			//read
			{ 0x0B	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE    
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_1 		},
			// write
			{ 0x02	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE , CLOSE}
		},
		// XM25QH16B_DUAL	
		{	
			//read
			{ 0xBB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_RO	| SPI_TRANSCTRL_DUALQUAD_DUAL		| SPI_TRANSCTRL_TOKEN_EN
					|CLOSE						| CLOSE 							},
			// write
			{ 0x02	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		},
		// XM25QH16B_QUAD	
		{	
			//read
			{ 0xEB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_QUAD		| SPI_TRANSCTRL_TOKEN_EN
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_2 		},
			// write
			{ 0x32	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_QUAD		| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		} 
	},
	
	{ 0x1540a1, 	//FM25QH16B_ID
		// FM25QH16B_REGULAR 
		{	
			//read
			{ 0x0B	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE    
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_1 		},
			// write
			{ 0x02	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE , CLOSE}
		},
		// FM25QH16B_DUAL	
		{	
			//read
			{ 0xBB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_RO	| SPI_TRANSCTRL_DUALQUAD_DUAL		| SPI_TRANSCTRL_TOKEN_EN
					|CLOSE						| CLOSE 							},
			// write
			{ 0x02	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		},
		// FM25QH16B_QUAD	
		{	
			//read
			{ 0xEB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_QUAD		| SPI_TRANSCTRL_TOKEN_EN
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_2 		},
			// write
			{ 0x32	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_QUAD		| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		} 
	},

	{ 0x1640ef, 	//W25Q32JV_ID
		// W25Q32JV_REGULAR 
		{	
			//read
			{ 0x0B	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE    
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_1 		},
			// write
			{ 0x02	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE , CLOSE}
		},
		// W25Q32JV_DUAL	
		{	
			//read
			{ 0xBB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_RO	| SPI_TRANSCTRL_DUALQUAD_DUAL		| SPI_TRANSCTRL_TOKEN_EN
					|CLOSE						| CLOSE 							},
			// write
			{ 0x02	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 				  
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_REGULAR	| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		},
		// W25Q32JV_QUAD	
		{	
			//read
			{ 0xEB	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| SPI_TRANSCTRL_ADDR_FMT 
					| SPI_TRANSCTRL_TRAMODE_DR	| SPI_TRANSCTRL_DUALQUAD_QUAD		| SPI_TRANSCTRL_TOKEN_EN
					| CLOSE 					| SPI_TRANSCTRL_DUMMY_CNT_2 		},
			// write
			{ 0x32	, SPI_TRANSCTRL_CMD_EN		| SPI_TRANSCTRL_ADDR_EN 			| CLOSE 
					| SPI_TRANSCTRL_TRAMODE_WO	| SPI_TRANSCTRL_DUALQUAD_QUAD		| CLOSE
					| CLOSE 					| CLOSE 							},
			{ CLOSE ,CLOSE}
		} 
	},

	
	{0}		
};
	

#ifdef MPW


int spi_bus_ready(unsigned int baseAddr)
{
	unsigned int i;

	for(i = 0; i < 1000; i++)
	{
		if((IN32(SPI2_BASE + 0x34) & SPI_STATUS_BUSY) == 0)
			return 1;
	}

	return 0;
}

int spi_fifo_reset(unsigned int baseAddr)
{
	unsigned int i;

	OUT32(SPI2_BASE+0x30, IN32(SPI2_BASE+0x30) |(SPI_CONTROL_RXFIFORST|SPI_CONTROL_TXFIFORST));

	for(i = 0; i < 1000; i++)
	{	
		if(IN32(SPI2_BASE+0x30) & (SPI_CONTROL_RXFIFORST|SPI_CONTROL_TXFIFORST))
			return 0;
	}

	return 1;
}

int spi_rx_ready(unsigned int baseAddr)
{
	unsigned int i;

	for(i = 0; i < 20000; i++)
	{
		if((IN32(SPI2_BASE+0x34) & SPI_STATUS_RXENPTY) == 0)
			return 1;
	}

	return 0;
}



int spiFlash_rdid(void)
{
	unsigned int value = 0;

	//wait spi-ilde
	if(!spi_bus_ready(SPI2_BASE))
		return -1;

	//clear rx & tx fifo
	if(spi_fifo_reset(SPI2_BASE))
		return -2;

	OUT32(SPI2_BASE+0x20, SPI_TRANSCTRL_CMD_EN |SPI_TRANSCTRL_TRAMODE_RO|SPI_TRANSCTRL_RCNT(2));
	OUT32(SPI2_BASE+0x24, SPIFLASH_CMD_RDID);

	//wait data ready
	if(!spi_rx_ready(SPI2_BASE))
		return -3;

	return IN32(SPI2_BASE+0x2C);
}


int spiFlash_rdsr(void)
{
	unsigned int value = 0;

	//wait spi-ilde
	if(!spi_bus_ready(SPI2_BASE))
		return -1;

	//clear rx & tx fifo
	if(spi_fifo_reset(SPI2_BASE))
		return -2;

	OUT32(SPI2_BASE+0x20, SPI_TRANSCTRL_CMD_EN |SPI_TRANSCTRL_TRAMODE_RO);
	OUT32(SPI2_BASE+0x24, SPIFLASH_CMD_RDSR);

	//wait data ready
	if(!spi_rx_ready(SPI2_BASE))
		return -3;

	return IN32(SPI2_BASE+0x2C);
}


int spiFlash_wren(void)
{
	unsigned int value = 0;

	//wait spi-ilde
	if(!spi_bus_ready(SPI2_BASE))
		return -1;

	//clear rx & tx fifo
	if(spi_fifo_reset(SPI2_BASE))
		return -2;

	OUT32(SPI2_BASE+0x20, SPI_TRANSCTRL_CMD_EN |SPI_TRANSCTRL_TRAMODE_NONE);
	OUT32(SPI2_BASE+0x24, SPIFLASH_CMD_WREN);
	return 0;
}


int spiFlash_se(unsigned int addr)
{
	unsigned int value = 0;

	//wait spi-ilde
	if(!spi_bus_ready(SPI2_BASE))
		return -1;

	//clear rx & tx fifo
	if(spi_fifo_reset(SPI2_BASE))
		return -2;

	OUT32(SPI2_BASE+0x20, SPI_TRANSCTRL_CMD_EN |SPI_TRANSCTRL_ADDR_EN|SPI_TRANSCTRL_TRAMODE_NONE);
	OUT32(SPI2_BASE+0x28, addr);	
	OUT32(SPI2_BASE+0x24, SPIFLASH_CMD_SE);
	return 0;
}



void spiFlash_init(unsigned int timing, unsigned int rdCmd)
{
	unsigned int value;

	//enable clk
	OUT32(CLK_EN_BASE, IN32(CLK_EN_BASE) |(CLK_SPI_FLASH |CLK_SPI_FLASH_AHB));

	//cfg pin
	value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<21));
	OUT32(SOC_PIN0_MUX_BASE, value |(3<<21));	/* MSPI GPIO7-DO MUX CFG */
	
	value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<24));
	OUT32(SOC_PIN0_MUX_BASE, value |(3<<24));	/* MSPI GPIO8-DI MUX CFG */

	value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<27));
	OUT32(SOC_PIN0_MUX_BASE, value |(3<<27));	/* MSPI GPIO9-HOLD MUX CFG */

	value = IN32(SOC_PIN1_MUX_BASE) & (~(7<<0));
	OUT32(SOC_PIN1_MUX_BASE, value |(3<<0));		/* MSPI GPIO10-WP MUX CFG */

	value = IN32(SOC_PIN1_MUX_BASE) & (~(7<<3));
	OUT32(SOC_PIN1_MUX_BASE, value |(3<<3));		/* MSPI GPIO11-CLK MUX CFG */
	
	value = IN32(SOC_PIN1_MUX_BASE) & (~(7<<6));
	OUT32(SOC_PIN1_MUX_BASE, value |(3<<6));		/* MSPI GPIO12-CS0 MUX CFG */

	//cfg timing
	value = IN32(SPI2_BASE+0x40) & 0xFFFFFF00;
	OUT32(SPI2_BASE+0x40, value |(timing&0xFF));

	//cfg read cmd
	value = IN32(SPI2_BASE+0x50) & 0xFFFFFFF0;
	OUT32(SPI2_BASE+0x50, value |(rdCmd&0x0F));

	//disable spi-int
	OUT32(SPI2_BASE+0x38, 0);
}

#else
int spi_bus_ready(unsigned int baseAddr);
int spi_fifo_reset(unsigned int baseAddr);
int spi_rx_ready(unsigned int baseAddr);
int spiFlash_rdid(void);
unsigned int spiFlash_rdsr(void);
void spiFlash_wren(void);
void spiFlash_se(unsigned int addr);
void spiFlash_init(unsigned int timing, unsigned int rdCmd);

#endif





static spi_dev spiDev;

//#pragma GCC optimize ("-O0")

int spiFlash_rdsr_common(int cmd)
{
	unsigned int value = 0;

	//wait spi-ilde
	if(!spi_bus_ready(SPI2_BASE))
		return -1;

	//clear rx & tx fifo
	if(spi_fifo_reset(SPI2_BASE))
		return -2;

	OUT32(SPI2_BASE+0x20, SPI_TRANSCTRL_CMD_EN |SPI_TRANSCTRL_TRAMODE_RO);
	OUT32(SPI2_BASE+0x24, cmd);

	//wait data ready
	if(!spi_rx_ready(SPI2_BASE))
		return -3;

	return IN32(SPI2_BASE+0x2C);
}

void spiFlash_wrsr_common(int cmd,int data_reg,int length)
{
	unsigned int j, data;
	spi_reg * pSpiReg = (spi_reg *)SPI2_BASE;
		
	for(j = 0; j<SPIFLASH_TIMEOUT_COUNT; j++)
	{
		spiFlash_wren();
		data = spiFlash_rdsr();
		if(data & SPIFLASH_STATUS_WEL)
			break;
	}
	
	if((data & SPIFLASH_STATUS_WEL) == 0)
	{
		system_printf("spiFlash_mode fail!1111\n");
	}	
	
	pSpiReg->transCtrl = SPI_TRANSCTRL_CMD_EN 
						|SPI_TRANSCTRL_TRAMODE_WO
						|SPI_TRANSCTRL_WCNT(length-1);
	pSpiReg->cmd = cmd;
			
	SPI_WAIT_TX_READY(pSpiReg->status);
	pSpiReg->data = data_reg; 
	for(j = 0; j<SPIFLASH_TIMEOUT_COUNT; j++)
	{
		data = spiFlash_rdsr();
		if((data & SPIFLASH_STATUS_WIP) == 0)
			break;
	}	
	if(data & SPIFLASH_STATUS_WIP)
	{
		system_printf("spiFlash_mode fail!\n");			
	}	
}
#if 1
int spiFlash_OTP_Read(int addr,int length,unsigned char *pdata)
{
	if(length <= 0)
	{
		system_printf("length error\n");
		return -3;
	}
	unsigned int flashID = spiFlash_rdid() & (~(0xFF<<24));
	if (flashID == 0x1440c8)//GD25Q80E
	{
		if(!((addr>=0x000000 && addr<=0x0003FF && (addr + length -1) <= 0x0003FF)
		   ||(addr>=0x001000 && addr<=0x0013FF && (addr + length -1) <= 0x0013FF)))
		{
			system_printf("addr or length error\n");
			return -3;
		}		
	}
	else if(flashID == 0x146085)//P25Q80H
	{
		unsigned int DP = spiFlash_rdsr_common(SPIFLASH_CMD_RDSR_Cfg) & BIT7;//512
		if (DP)
		{
			if(!((addr>=0x001000 && addr<=0x0011FF && (addr + length - 1) <= 0x0011FF)
		       ||(addr>=0x002000 && addr<=0x0021FF && (addr + length - 1) <= 0x0021FF)
		       ||(addr>=0x003000 && addr<=0x0031FF && (addr + length - 1) <= 0x0031FF)))
			{
				system_printf("addr or length error\n");
				return -3;
			}
		}
		else//256
		{
			if(!((addr>=0x001000 && addr<=0x0010FF && (addr + length - 1) <= 0x0010FF)
		       ||(addr>=0x002000 && addr<=0x0020FF && (addr + length - 1) <= 0x0020FF)
		       ||(addr>=0x003000 && addr<=0x0030FF && (addr + length - 1) <= 0x0030FF)))
			{
				system_printf("addr or length error\n");
				return -3;
			}
		}		
	}
	
	else if(flashID == 0x156085)//P25Q16H
	{
		unsigned int DP = spiFlash_rdsr_common(SPIFLASH_CMD_RDSR_Cfg) & BIT7;//512byte
		if (DP)
		{
			if(!((addr>=0x001000 && addr<=0x0011FF && (addr + length - 1) <= 0x0011FF)
		       ||(addr>=0x002000 && addr<=0x0021FF && (addr + length - 1) <= 0x0021FF)
		       ||(addr>=0x003000 && addr<=0x0031FF && (addr + length - 1) <= 0x0031FF)))
			{
				system_printf("addr or length error\n");
				return -3;
			}
		}
		else//256byte
		{
			if(!((addr>=0x001000 && addr<=0x0010FF && (addr + length - 1) <= 0x0010FF)
		       ||(addr>=0x002000 && addr<=0x0020FF && (addr + length - 1) <= 0x0020FF)
		       ||(addr>=0x003000 && addr<=0x0030FF && (addr + length - 1) <= 0x0030FF)))
			{
				system_printf("addr or length error\n");
				return -3;
			}
		}		
	}
	else if(flashID == 0X14400B)//XT25F08B
	{
		if(!(addr>=0x000000 && addr<=0x0003FF && (addr + length -1) <= 0x0003FF))
		{
			system_printf("addr or length error\n");
			return -3;
		}
	}
	else if(flashID == 0x14605E)//ZB25VQ80A
	{
		if(!((addr>=0x001000 && addr<=0x0010FF && (addr + length -1) <= 0x0010FF)
		   ||(addr>=0x002000 && addr<=0x0020FF && (addr + length -1) <= 0x0020FF)
		   ||(addr>=0x003000 && addr<=0x0030FF && (addr + length -1) <= 0x0030FF)))
		{
			system_printf("addr or length error\n");
			return -3;
		}		
	}
	else if(flashID == 0x15400b)//XT25F16B
	{
		if(!(addr>=0x000000 && addr<=0x0003FF && (addr + length -1) <= 0x0003FF))
		{
			system_printf("addr or length error\n");
			return -3;
		}
	}
	else
	{
		system_printf("flashID error:0x%x\n",flashID);
		return -3;
	}

	unsigned int *p_dst_buffer = (unsigned int *)pdata;
	
	//wait spi-ilde
	if(!spi_bus_ready(SPI2_BASE))
		return -1;

	//clear rx & tx fifo
	if(spi_fifo_reset(SPI2_BASE))
		return -2;

	OUT32(SPI2_BASE+0x20, SPI_TRANSCTRL_CMD_EN 
						 |SPI_TRANSCTRL_ADDR_EN 
						 |SPI_TRANSCTRL_TRAMODE_DR 
						 |SPI_TRANSCTRL_DUMMY_CNT_1
						 |SPI_TRANSCTRL_DUALQUAD_REGULAR
						 |SPI_TRANSCTRL_RCNT(length-1));
	OUT32(SPI2_BASE+0x28, addr);

	OUT32(SPI2_BASE+0x24, SPIFLASH_CMD_OTP_RD);
	
	
	unsigned int Rxcount = 0;
	unsigned int i;
	unsigned int data = IN32(SPI2_BASE+0x34);
	while( data & SPI_STATUS_BUSY )
	{           
		if(!(data & SPI_STATUS_RXENPTY))
	  	{
	        Rxcount = ((data & 0x00001f00)>>8) ;
	        for (i = 0; i < Rxcount; i++) 
			{
				*p_dst_buffer++ = IN32(SPI2_BASE+0x2c);
	        }
      	}
		data = IN32(SPI2_BASE+0x34);
    }
	Rxcount = ((data & 0x00001f00)>>8) ;
    for (i = 0; i < Rxcount; i++) 
	{
		*p_dst_buffer++ = IN32(SPI2_BASE+0x2c);
    }
	
	
	return 0;
}

/*
LB
0x01:lock Security Register #1 0x001000-0x0011FF
0x02:lock Security Register #2 0x002000-0x0021FF
0x03:lock Security Register #3 0x003000-0x0031FF
0x0F:lock all
*/
void spiFlash_OTP_Lock(int LB)
{
	unsigned int flashID = spiFlash_rdid() & (~(0xFF<<24));
	unsigned int data = spiFlash_rdsr_common(SPIFLASH_CMD_RDSR_H) & 0xFF;
	data = data<<8;
	if (flashID == 0x1440c8)//GD25Q80E
	{
		switch (LB)
		{
			case 0x01:
				data |= 0x0400;
				break;
			case 0x02:
				data |= 0x0800;
				break;
			case 0x0F:
				data |= 0x0C00;
				break;
			default:
				system_printf("parameter error\n");
				return;		
		}
		spiFlash_wrsr_common(SPIFLASH_CMD_WRSR, data, 2);
	}
	else if(flashID == 0x146085)//P25Q80HD
	{		
		switch (LB)
		{
			case 0x01:
				data |= 0x0800;
				break;
			case 0x02:
				data |= 0x1000;
				break;
			case 0x03:
				data |= 0x2000;
				break;
			case 0x0F:				
				data |= 0x3800;
				break;
			default:
				system_printf("parameter error\n");
				return;
		}
		spiFlash_wrsr_common(SPIFLASH_CMD_WRSR, data, 2);					
	}
	
	else if(flashID == 0x156085)//P25Q16HD
	{		
		switch (LB)
		{
			case 0x01:
				data |= 0x0800;
				break;
			case 0x02:
				data |= 0x1000;
				break;
			case 0x03:
				data |= 0x2000;
				break;
			case 0x0F:				
				data |= 0x3800;
				break;
			default:
				system_printf("parameter error\n");
				return;
		}
		spiFlash_wrsr_common(SPIFLASH_CMD_WRSR, data, 2);					
	}
	else if(flashID == 0x14605E)//ZB25VQ80A
	{
		switch (LB)
		{
			case 0x01:
				data |= 0x08;
				break;
			case 0x02:
				data |= 0x10;
				break;
			case 0x03:
				data |= 0x20;
				break;
			case 0x0F:				
				data |= 0x38;
				break;
			default:
				system_printf("parameter error\n");
				return;
		}
		spiFlash_wrsr_common(SPIFLASH_CMD_WRSR_2, data, 2);
	}
	else if(flashID == 0X14400B)//XT25F08B
	{
		data |= 0x0400;		
		spiFlash_wrsr_common(SPIFLASH_CMD_WRSR, data, 2);
	}
	else if(flashID == 0x15400b)//XT25F16B
	{		
		data |= 0x0400;		
		spiFlash_wrsr_common(SPIFLASH_CMD_WRSR, data, 2);
	}
	else
	{
		system_printf("flashID error:0x%x\n",flashID);
		return;
	}
//	spiFlash_wrsr_common(SPIFLASH_CMD_WRSR, data, 2);
}

int spiFlash_OTP_Se(unsigned int addr)
{
	unsigned int flashID = spiFlash_rdid() & (~(0xFF<<24));
	if (flashID == 0x1440c8)//GD25Q80E
	{
		if(!((addr>=0x000000 && addr<=0x0003FF) || (addr>=0x001000 && addr<=0x0013FF)))
		{
			system_printf("addr error\n");
			return -3;
		}		
	}
	else if(flashID == 0x146085)//P25Q80HD
	{
		if(!((addr>=0x001000 && addr<=0x0011FF)||(addr>=0x002000 && addr<=0x0021FF)||(addr>=0x003000 && addr<=0x0031FF)))
		{
			system_printf("addr error\n");
			return -3;
		}
	}
	
	else if(flashID == 0x156085)//P25Q16HD
	{
		if(!((addr>=0x001000 && addr<=0x0011FF)||(addr>=0x002000 && addr<=0x0021FF)||(addr>=0x003000 && addr<=0x0031FF)))
		{
			system_printf("addr error\n");
			return -3;
		}
	}
	else if(flashID == 0x14605E)//ZB25VQ80A
	{
		if(!((addr>=0x001000 && addr<=0x0010FF)||(addr>=0x002000 && addr<=0x0020FF)||(addr>=0x003000 && addr<=0x0030FF )))
		{
			system_printf("addr error\n");
			return -3;
		}		
	}
	
	else if(flashID == 0X14400B)//XT25F08B
	{
		if(!(addr>=0x000000 && addr<=0x0003FF))
		{
			system_printf("addr error\n");
			return -3;
		}
	}
	else if(flashID == 0x15400b)//XT25F16B
	{
		if(!(addr>=0x000000 && addr<=0x0003FF))
		{
			system_printf("addr error\n");
			return -3;
		}
	}
	else
	{
		system_printf("flashID error:0x%x\n",flashID);
		return -3;
	}

	unsigned int eraseSectorCnt, i, j, flag, data;
	unsigned int len = 512;

	flag = system_irq_save();
	for(j = 0; j<SPIFLASH_TIMEOUT_COUNT; j++)
	{
		spiFlash_wren();
		data = spiFlash_rdsr();
		if(data & SPIFLASH_STATUS_WEL)
			break;
	}

	if((data & SPIFLASH_STATUS_WEL) == 0)
	{
		system_irq_restore(flag);
		return -1;
	}

	//section erase
	if(!spi_bus_ready(SPI2_BASE))
		return -1;

	//clear rx & tx fifo
	if(spi_fifo_reset(SPI2_BASE))
		return -2;

	OUT32(SPI2_BASE+0x20, SPI_TRANSCTRL_CMD_EN |SPI_TRANSCTRL_ADDR_EN|SPI_TRANSCTRL_TRAMODE_NONE);
	OUT32(SPI2_BASE+0x28, addr);
	OUT32(SPI2_BASE+0x24, SPIFLASH_CMD_OTP_SE);

	for(j = 0; j<SPIFLASH_TIMEOUT_COUNT; j++)
	{
		data = spiFlash_rdsr();
		if((data & SPIFLASH_STATUS_WIP) == 0)
		{
			//system_printf("hal_spiflash_erase, %#x\n", data);
			break;
		}
	}

	system_irq_restore(flag);
		
	//system_printf("hal_spiflash_erase count: %d, ret: %#x\n", j, data);
	if(data & SPIFLASH_STATUS_WIP)
		return -2;
	
	return 0;
}

static void spiflash_OTP_Program(spi_reg * pSpiReg, unsigned int addr, unsigned char * buf, unsigned int length)
{
	if(length <= 0)
	{
			system_printf("length error\n");
			return;
	}

	unsigned int i, left, data;
	unsigned int flashID = spiFlash_rdid() & (~(0xFF<<24));
	if (flashID == 0x1440c8)//GD25Q80E
	{
		if(!((addr>=0x000000 && addr<=0x0003FF && ((addr + length -1) <= 0x0003FF))
		   ||(addr>=0x001000 && addr<=0x0013FF && ((addr + length -1) <= 0x0013FF))))
		{
			system_printf("addr or length error\n");
			return;
		}		
	}
	else if(flashID == 0x146085)//P25Q80HD
	{
		if(!((addr>=0x001000 && addr<=0x0011FF && ((addr + length - 1) <= 0x0011FF))
			||(addr>=0x002000 && addr<=0x0021FF && ((addr + length - 1) <= 0x0021FF))
			||(addr>=0x003000 && addr<=0x0031FF && ((addr + length - 1) <= 0x0031FF))))
		{
			system_printf("addr or length error\n");
			return;
		}
		if((addr>=0x001000 && addr<=0x0011FF && ((addr + length - 1) > 0x0010FF))
		|| (addr>=0x002000 && addr<=0x0021FF && ((addr + length - 1) > 0x0020FF))
		|| (addr>=0x003000 && addr<=0x0031FF && ((addr + length - 1) > 0x0030FF)))
		{
		
			spiFlash_wren();
			data = spiFlash_rdsr_common(SPIFLASH_CMD_RDSR_Cfg);
			spiFlash_wrsr_common(SPIFLASH_CMD_WRSR_2, (data | BIT7), 1);
			
//			data = spiFlash_rdsr_common(SPIFLASH_CMD_RDSR_Cfg);
//			system_printf("DP = 0x%x\n",(data));
		}			
	}
	
	else if(flashID == 0x156085)//P25Q16HD
	{
		if(!((addr>=0x001000 && addr<=0x0011FF && ((addr + length - 1) <= 0x0011FF))
			||(addr>=0x002000 && addr<=0x0021FF && ((addr + length - 1) <= 0x0021FF))
			||(addr>=0x003000 && addr<=0x0031FF && ((addr + length - 1) <= 0x0031FF))))
		{
			system_printf("addr or length error\n");
			return;
		}
		if((addr>=0x001000 && addr<=0x0011FF && ((addr + length - 1) > 0x0010FF))
		|| (addr>=0x002000 && addr<=0x0021FF && ((addr + length - 1) > 0x0020FF))
		|| (addr>=0x003000 && addr<=0x0031FF && ((addr + length - 1) > 0x0030FF)))
		{
		
			spiFlash_wren();
			data = spiFlash_rdsr_common(SPIFLASH_CMD_RDSR_Cfg);
			spiFlash_wrsr_common(SPIFLASH_CMD_WRSR_2, (data | BIT7), 1);
	
//			system_printf("DP = 0x%x\n",(data | BIT7));
		}			
	}
	else if(flashID == 0x14605E)//ZB25VQ80A
	{
		if(!((addr>=0x001000 && addr<=0x0010FF && (addr + length -1) <= 0x0010FF)
		   ||(addr>=0x002000 && addr<=0x0020FF && (addr + length -1) <= 0x0020FF)
		   ||(addr>=0x003000 && addr<=0x0030FF && (addr + length -1) <= 0x0030FF)))
		{
			system_printf("addr or length error\n");
			return;
		}		
	}
	else if(flashID == 0X14400B)//XT25F08B
	{			
		if(!((addr>=0x000000 && addr<=0x0000FF && ((addr + length -1) <= 0x0000FF))
		   ||(addr>=0x000100 && addr<=0x0001FF && ((addr + length -1) <= 0x0001FF))
		   ||(addr>=0x000200 && addr<=0x0002FF && ((addr + length -1) <= 0x0002FF))
		   ||(addr>=0x000300 && addr<=0x0003FF && ((addr + length -1) <= 0x0003FF))))
		{
			system_printf("addr or length error\n");
			return;
		}	
	}
	else if(flashID == 0x15400b)//XT25F16B
	{			
		if(!((addr>=0x000000 && addr<=0x0000FF && ((addr + length -1) <= 0x0000FF))
		   ||(addr>=0x000100 && addr<=0x0001FF && ((addr + length -1) <= 0x0001FF))
		   ||(addr>=0x000200 && addr<=0x0002FF && ((addr + length -1) <= 0x0002FF))
		   ||(addr>=0x000300 && addr<=0x0003FF && ((addr + length -1) <= 0x0003FF))))
		{
			system_printf("addr or length error\n");
			return;
		}	
	}
	else
	{
		system_printf("flashID error:0x%x\n",flashID);
		return;
	}	
	spiFlash_wren();

	
	SPI_PREPARE_BUS(pSpiReg->status);
	SPI_CLEAR_FIFO(pSpiReg->ctrl);
	
	pSpiReg->addr = addr;

	pSpiReg->transCtrl = SPI_TRANSCTRL_CMD_EN 
					|SPI_TRANSCTRL_ADDR_EN
					|SPI_TRANSCTRL_TRAMODE_WO
					|SPI_TRANSCTRL_DUALQUAD_REGULAR
					|SPI_TRANSCTRL_WCNT(length - 1);
	pSpiReg->cmd = SPIFLASH_CMD_OTP_PP;


	if((unsigned int)buf%4==0)
	{
		unsigned int *pdata = (unsigned int * )buf;
  		left = length/4;

		if(length%4)
			left++;

		for(i = 0; i< left; i++)
		{
			SPI_WAIT_TX_READY(pSpiReg->status);
			pSpiReg->data = *pdata++;
		}
	}
	else
	{
	
		while (length > 0) 
		{
			/* clear the data */
			data = 0;

			/* data are usually be read 32bits once a time */
			left = min(length, 4);

			for (i = 0; i < left; i++) 
			{
				data |= *buf++ << (i * 8);
				length--;
				
			}
			/* wait till TXFULL is deasserted */
			SPI_WAIT_TX_READY(pSpiReg->status);				
			pSpiReg->data = data;
			
		}
	}
}
int hal_spifiash_OTPWrite(unsigned int addr, unsigned int len, unsigned char * buf)
{
	unsigned int address, flag, j, length, data;
	spi_reg * spiReg = spiDev.spiReg;

	length = addr % SPIFLASH_PAGE_SIZE;

	if(len > (SPIFLASH_PAGE_SIZE - length))
		length = SPIFLASH_PAGE_SIZE - length;
	else
		length = len;
	
	address = addr;	

	do {
		flag = system_irq_save();

		for(j = 0; j<SPIFLASH_TIMEOUT_COUNT; j++)
		{
			spiFlash_wren();
			data = spiFlash_rdsr();
			//system_irq_restore(flag);
			if(data & SPIFLASH_STATUS_WEL)
				break;
		}

		if((data & SPIFLASH_STATUS_WEL) == 0)
		{
			system_irq_restore(flag);
			return -1;
		}

		//flag = system_irq_save();	
		spiflash_OTP_Program(spiReg, address, buf+(address - addr), length);
		//system_irq_restore(flag);

		len -= length;
		address += length;

		if(len > SPIFLASH_PAGE_SIZE)
			length = SPIFLASH_PAGE_SIZE;
		else
			length = len;

		for(j = 0; j<SPIFLASH_TIMEOUT_COUNT; j++)
		{
			//flag = system_irq_save();
			data = spiFlash_rdsr();
			//system_irq_restore(flag);
			if((data & SPIFLASH_STATUS_WIP) == 0)
			{
				break;
			}
		}

		system_irq_restore(flag);
		if(data & SPIFLASH_STATUS_WIP)
		{
			return -1;
		}
		
	}while(length);

	return 0;	
}
#endif
int spiFlash_rdsr_h(void)
{
	unsigned int value = 0;

	//wait spi-ilde
	if(!spi_bus_ready(SPI2_BASE))
		return -1;

	//clear rx & tx fifo
	if(spi_fifo_reset(SPI2_BASE))
		return -2;

	OUT32(SPI2_BASE+0x20, SPI_TRANSCTRL_CMD_EN |SPI_TRANSCTRL_TRAMODE_RO);
	OUT32(SPI2_BASE+0x24, SPIFLASH_CMD_RDSR_H);

	//wait data ready
	if(!spi_rx_ready(SPI2_BASE))
		return -3;

	return IN32(SPI2_BASE+0x2C);
}
void spiFlash_cmd(void)
{
	Flash_ID = spiFlash_rdid() & (~(0xFF<<24));
	for(Flash_assign = gflashobj ;Flash_assign->flashid; Flash_assign++) {
		if( Flash_assign->flashid == Flash_ID )
		{
			Cmd_assign = &(Flash_assign->QUAD);
			break;	
		}
	}
	if(Flash_assign->flashid != Flash_ID)
		system_printf("FlashID Error:0x%x\n",Flash_ID);	
}
void spiFlash_Qmode(void)
{
	unsigned int j, data;
	spi_reg * pSpiReg = (spi_reg *)SPI2_BASE;
	
	for(j = 0; j<SPIFLASH_TIMEOUT_COUNT; j++)
	{
		spiFlash_wren();
		data = spiFlash_rdsr();
		if(data & SPIFLASH_STATUS_WEL)
			break;
	}

	if((data & SPIFLASH_STATUS_WEL) == 0)
	{
		system_printf("spiFlash_mode fail!1111\n");
	}
	
	Flash_ID = spiFlash_rdid() & (~(0xFF<<24));

	data = spiFlash_rdsr();
	//add XT25F16b 0x15400b
	
	if(Flash_ID == 0x146085 || Flash_ID == 0x1440c8 || Flash_ID == 0x15400b || Flash_ID == 0x156085 || Flash_ID == 0x14400b)//P25Q80H & GD25Q80E & P25Q16H & XT25F08 & XT25F16
	{
		pSpiReg->transCtrl = SPI_TRANSCTRL_CMD_EN 
					|SPI_TRANSCTRL_TRAMODE_WO
					|SPI_TRANSCTRL_WCNT(1);
		pSpiReg->cmd = SPIFLASH_CMD_WRSR;
	
		SPI_WAIT_TX_READY(pSpiReg->status);
		pSpiReg->data = 0x0200;
	}
	/*
	else if(Flash_ID == 0x1540a1)
	{
		pSpiReg->transCtrl = SPI_TRANSCTRL_CMD_EN 
					|SPI_TRANSCTRL_TRAMODE_WO
					|SPI_TRANSCTRL_WCNT(1);
		pSpiReg->cmd = SPIFLASH_CMD_WRSR;
	
		SPI_WAIT_TX_READY(pSpiReg->status);
		pSpiReg->data = 0x0002;
	}
	*/
	else
	{
		pSpiReg->transCtrl = SPI_TRANSCTRL_CMD_EN 
					|SPI_TRANSCTRL_TRAMODE_WO
					|SPI_TRANSCTRL_WCNT(0);
		pSpiReg->cmd = SPIFLASH_CMD_WRSR_2;

		SPI_WAIT_TX_READY(pSpiReg->status);
		pSpiReg->data = 0x02;
		
  		
	}

	for(j = 0; j<SPIFLASH_TIMEOUT_COUNT; j++)
	{
		data = spiFlash_rdsr();
		if((data & SPIFLASH_STATUS_WIP) == 0)
			break;
	}	
	if(data & SPIFLASH_STATUS_WIP)
	{
		system_printf("spiFlash_mode fail!\n");
		
	}		
}


static void spiflash_program(spi_reg * pSpiReg, unsigned int addr, unsigned char * buf, unsigned int length)
{
	unsigned int i, left, data;

	SPI_PREPARE_BUS(pSpiReg->status);
	SPI_CLEAR_FIFO(pSpiReg->ctrl);

	pSpiReg->addr = addr;
		
	
#ifndef FPGA

#if 1
	/*
	pSpiReg->transCtrl = SPI_TRANSCTRL_CMD_EN 
					|SPI_TRANSCTRL_ADDR_EN
//					|SPI_TRANSCTRL_ADDR_FMT
					|SPI_TRANSCTRL_TRAMODE_WO
					|SPI_TRANSCTRL_DUALQUAD_QUAD
					|SPI_TRANSCTRL_WCNT(length - 1);
	pSpiReg->cmd = SPIFLASH_CMD_4PP;
	*/
	pSpiReg->transCtrl = Cmd_assign->program.transCtrl | SPI_TRANSCTRL_WCNT(length - 1);
	pSpiReg->cmd = Cmd_assign->program.cmd;
#else
	pSpiReg->transCtrl = SPI_TRANSCTRL_CMD_EN 
					|SPI_TRANSCTRL_ADDR_EN
					|SPI_TRANSCTRL_TRAMODE_WO
					|SPI_TRANSCTRL_WCNT(length - 1);

	system_printf("spiflash_program, 0x%x\n", pSpiReg->transCtrl);

	pSpiReg->cmd = SPIFLASH_CMD_PP;


#endif

#else
	/*
	pSpiReg->transCtrl = SPI_TRANSCTRL_CMD_EN 
					|SPI_TRANSCTRL_ADDR_EN
					|SPI_TRANSCTRL_TRAMODE_WO
					|SPI_TRANSCTRL_WCNT(length - 1);
	pSpiReg->cmd = SPIFLASH_CMD_PP;
	*/
	pSpiReg->transCtrl = Cmd_assign->program.transCtrl | SPI_TRANSCTRL_WCNT(length - 1);
	pSpiReg->cmd = Cmd_assign->program.cmd;
#endif

	if((unsigned int)buf%4==0)
	{
		unsigned int *pdata = (unsigned int * )buf;
  		left = length/4;

		if(length%4)
			left++;

		for(i = 0; i< left; i++)
		{
			SPI_WAIT_TX_READY(pSpiReg->status);
			pSpiReg->data = *pdata++;
		}
	}
	else
	{
		while (length > 0) 
		{
			/* clear the data */
			data = 0;

			/* data are usually be read 32bits once a time */
			left = min(length, 4);

			for (i = 0; i < left; i++) 
			{
				data |= *buf++ << (i * 8);
				length--;
			}
			/* wait till TXFULL is deasserted */
			SPI_WAIT_TX_READY(pSpiReg->status);				
			pSpiReg->data = data;
		}
	}
}


void spiflash_cmd_read(unsigned int addr, unsigned char * buf, unsigned int len)
{
	unsigned int flag, i, rx_num, data;
	unsigned int * rxBuf;
	spi_reg * spiReg = spiDev.spiReg;

	flag = system_irq_save();

	if(spi_fifo_reset(SPI2_BASE))
	{
		spiReg->ctrl = 1;
		system_printf("spiflash-warning: fifo reset!\n");
	}

	spiReg->addr = addr;
	spiReg->transCtrl = SPI_TRANSCTRL_CMD_EN     \
					| SPI_TRANSCTRL_ADDR_EN   \
					| SPI_TRANSCTRL_ADDR_FMT \
					| SPI_TRANSCTRL_TRAMODE_DR  \
					| SPI_TRANSCTRL_DUALQUAD_QUAD  \
					| SPI_TRANSCTRL_TOKEN_EN   \
					| SPI_TRANSCTRL_DUMMY_CNT_2 \
					| SPI_TRANSCTRL_RCNT(len -1);

	spiReg->cmd = SPIFLASH_CMD_QUAD_READ;


	if((unsigned int)buf%4==0)
	{
		rxBuf = (unsigned int *)buf;
		//SPI_WAIT_RX_READY(spiReg->status);

#if 1
		data = (len+3)/4;

		while((spiReg->status & SPI_STATUS_BUSY) == 1)
		{
			rx_num = ((spiReg->status) >> 8) & 0x1F;
			rx_num = MIN(rx_num , data);

			for(i=0; i<rx_num; i++)
			{
				*rxBuf++ = spiReg->data;
			}

			data -= rx_num;

			if(data == 0)
			{
				return;
			}
		}

		for(i=0; i<data; i++)
		{
			*rxBuf++ = spiReg->data;
		}
#else

		while(len > 0)
		{
			SPI_WAIT_RX_READY(spiReg->status);
			*rxBuf++ = spiReg->data;
			len -=4;
		}
#endif
	}
	else
	{
		while(len)
		{
			rx_num = MIN(4, len);
			SPI_WAIT_RX_READY(spiReg->status);
			data = spiReg->data;

			for(i=0; i<rx_num; i++)
			{
				*buf++ = (unsigned char)data;
				data >>= 8;
				len--;
			}
		}
	}
	system_irq_restore(flag);
}

void spiflash_read(unsigned int addr, unsigned char * buf, unsigned int len)
{
	unsigned int left_size =len, curr_size, offset = 0;

	while(left_size)
	{
		curr_size = MIN(0x200, left_size);
		spiflash_cmd_read(addr+offset, buf+offset, curr_size);
		offset += curr_size;
		left_size -= curr_size;
	}
}


int hal_spiflash_read(unsigned int addr, unsigned char * buf, unsigned int len)
{
#if 0

	spiflash_read(addr, buf, len);

#else
	unsigned int length;

	if(addr < SPIFLASH_MEM_LENGTH)
	{
		if((addr + len) > SPIFLASH_MEM_LENGTH)
		{
			length = SPIFLASH_MEM_LENGTH - addr;
		}
		else
		{
			length = len;
		}

		memcpy(buf, (void *)(addr+0x00C00000), length);

		if(length != len)
		{
			spiflash_read(addr + length, buf + length, len -length);
		}
	}
	else
	{
		spiflash_read(addr, buf, len);
	}
#endif
	return 0;
}



int hal_spifiash_write(unsigned int addr, unsigned char * buf, unsigned int len)
{
	unsigned int address, flag, j, length, data;
	spi_reg * spiReg = spiDev.spiReg;

	if(len == 0)
		return 0;

	length = addr % SPIFLASH_PAGE_SIZE;

	if(len > (SPIFLASH_PAGE_SIZE - length))
		length = SPIFLASH_PAGE_SIZE - length;
	else
		length = len;

	address = addr;

	
	
	do {
		flag = system_irq_save();

		for(j = 0; j<SPIFLASH_TIMEOUT_COUNT; j++)
		{
			spiFlash_wren();
			data = spiFlash_rdsr();
			//system_irq_restore(flag);
			if(data & SPIFLASH_STATUS_WEL)
				break;
		}

		if((data & SPIFLASH_STATUS_WEL) == 0)
		{
			system_irq_restore(flag);
			return -1;
		}

		//flag = system_irq_save();	
		spiflash_program(spiReg, address, buf+(address - addr), length);
		//system_irq_restore(flag);

		//system_printf("hal_spifiash_write, des: %#x, src: %#x, len: %d\n", address, buf+(address - addr), length);

		len -= length;
		address += length;

		if(len > SPIFLASH_PAGE_SIZE)
			length = SPIFLASH_PAGE_SIZE;
		else
			length = len;

		for(j = 0; j<SPIFLASH_TIMEOUT_COUNT; j++)
		{
			//flag = system_irq_save();
			data = spiFlash_rdsr();
			//system_irq_restore(flag);
			if((data & SPIFLASH_STATUS_WIP) == 0)
			{
				break;
			}
		}

		system_irq_restore(flag);
		if(data & SPIFLASH_STATUS_WIP)
		{
			return -1;
		}
		
	}while(length);

	return 0;	
}


int hal_spiflash_erase(unsigned int addr,  unsigned int len)
{
	unsigned int eraseSectorCnt, i, j, flag, data;

	if((addr%SPIFLASH_SECTOR_SIZE) || (len%SPIFLASH_SECTOR_SIZE))
		return -23;

	eraseSectorCnt = len /SPIFLASH_SECTOR_SIZE;

	for(i=0; i<eraseSectorCnt; i++)
	{
		flag = system_irq_save();
		for(j = 0; j<SPIFLASH_TIMEOUT_COUNT; j++)
		{
			spiFlash_wren();
			data = spiFlash_rdsr();
			if(data & SPIFLASH_STATUS_WEL)
				break;
		}

		if((data & SPIFLASH_STATUS_WEL) == 0)
		{
			system_irq_restore(flag);
			return -1;
		}

		//section erase
		spiFlash_se(addr + i * SPIFLASH_SECTOR_SIZE);

		for(j = 0; j<SPIFLASH_TIMEOUT_COUNT; j++)
		{
			data = spiFlash_rdsr();
			if((data & SPIFLASH_STATUS_WIP) == 0)
			{
				//system_printf("hal_spiflash_erase, %#x\n", data);
				break;
			}

		}

		system_irq_restore(flag);
		
		//system_printf("hal_spiflash_erase count: %d, ret: %#x\n", j, data);
		if(data & SPIFLASH_STATUS_WIP)
			return -2;
	}

	return 0;
}

void hal_spiflash_init(void)
{
#if 1
	spi_dev * pSpiDev = NULL;

	pSpiDev = &spiDev;
	pSpiDev->spiReg = (spi_reg *)SPI2_BASE;

#ifdef FPGA
	spiFlash_init(0, 4);
#else
	spiFlash_init(0xFF, 5);
#endif
	spiFlash_Qmode();
	spiFlash_cmd();


#else
	unsigned int value;
	spi_dev * pSpiDev = NULL;

	pSpiDev = &spiDev;
	pSpiDev->spiReg = (spi_reg *)SPI2_BASE;

	//enable clk
	OUT32(CLK_EN_BASE, IN32(CLK_EN_BASE) |(CLK_SPI_FLASH |CLK_SPI_FLASH_AHB));

	//cfg pin
	value = IN32(PIN0_MUX_BASE) & (~(7<<21));
	OUT32(PIN0_MUX_BASE, value |(3<<21));	/* MSPI GPIO7-DO MUX CFG */
	
	value = IN32(PIN0_MUX_BASE) & (~(7<<24));
	OUT32(PIN0_MUX_BASE, value |(3<<24));	/* MSPI GPIO8-DI MUX CFG */

	value = IN32(PIN0_MUX_BASE) & (~(7<<27));
	OUT32(PIN0_MUX_BASE, value |(3<<27));	/* MSPI GPIO9-HOLD MUX CFG */

	value = IN32(PIN1_MUX_BASE) & (~(7<<0));
	OUT32(PIN1_MUX_BASE, value |(3<<0));		/* MSPI GPIO10-WP MUX CFG */

	value = IN32(PIN1_MUX_BASE) & (~(7<<3));
	OUT32(PIN1_MUX_BASE, value |(3<<3));		/* MSPI GPIO11-CLK MUX CFG */
	
	value = IN32(PIN1_MUX_BASE) & (~(7<<6));
	OUT32(PIN1_MUX_BASE, value |(3<<6));		/* MSPI GPIO12-CS0 MUX CFG */

#ifdef MPW
	//wangc add for mpw xip function
	//cfg spi flash interface timing: the SCLK frequency is the same as the SPI source clock frequency
	pSpiDev->spiReg->timing |= 0xFF;
	//cfg spi flash  read-cmd-0x6B in quad mode
	pSpiDev->spiReg->memCtrl = (pSpiDev->spiReg->memCtrl & (~0xF)) | 0x5;
#else
	//wangc add for profpga xip function
	//cfg spi flash interface timing: SCLK period = 2 * (Period of the SPI clocks), if too high, there is a  problem in pro-fpga
	pSpiDev->spiReg->timing &= ~0xFF;
	//cfg spi flash  read-cmd-0xBB in dual mode
	pSpiDev->spiReg->memCtrl = (pSpiDev->spiReg->memCtrl & (~0xF)) | 0x4;
#endif

	pSpiDev->spiReg->intrEn = 0;
#endif
}

void hal_spiflash_exit(void)
{
	return;
}


#if 0
static int sf_partion(cmd_tbl_t *t, int argc, char *argv[])
{
	int ret = 0;
	unsigned int num;

	num = strtoul(argv[1], NULL, 0);
	hal_spiflash_erase(0x3000, 0x1000);
	//ret = hal_spiflash_erase((unsigned int)(512*1024), 0x1000);
	hal_spifiash_write((unsigned int)(0x3ffc), (unsigned char *)&num, 4);

	system_printf("sf_partion, %d\n", num);
	return CMD_RET_SUCCESS;
}
CMD(sfp,
    sf_partion,
    "sf_partion",
    "andes_partion select");
#endif

#if (!(defined AMT || defined SINGLE_BOARD_VER))
static int nv_erase(cmd_tbl_t *t, int argc, char *argv[])
{
	hal_spiflash_erase(0xFC000, 4*4096);
	return CMD_RET_SUCCESS;
}
CMD(nve,  nv_erase,  "nv_erase",  "nv partion erase");


#if 1
static int sf_otp_lock(cmd_tbl_t *t, int argc, char *argv[])
{
	unsigned int LB = strtoul(argv[1], NULL, 0);
		
	spiFlash_OTP_Lock(LB);
	
	return CMD_RET_SUCCESS;
}
	
	
CMD(otp_lock,
	sf_otp_lock,
	"otp_lock",
	"otp_lock LB");


static int sf_otp_se(cmd_tbl_t *t, int argc, char *argv[])
{
 	unsigned int addr = strtoul(argv[1], NULL, 0);
	
	spiFlash_OTP_Se(addr);

	return CMD_RET_SUCCESS;
}


CMD(otp_se,
    sf_otp_se,
    "otp_se",
    "otp_se addr");

//	PARTION_BASE

//unsigned char sfTest[256]={0};

unsigned char sfTest[1024]={0};

static int sf_otp_write(cmd_tbl_t *t, int argc, char *argv[])
{
 	unsigned int addr = strtoul(argv[1], NULL, 0);
	
	unsigned int length = strtoul(argv[2], NULL, 0);
	int i=0;
	for(i=0;i<length;++i)
	{
		sfTest[i] = i;
		
	}
	hal_spifiash_OTPWrite(addr, length, (unsigned char *)sfTest);
//	system_printf("hal_spifiash_write, des: 0x%x, len: %d\n", addr,length);
	
	return CMD_RET_SUCCESS;
}

CMD(otp_write,
    sf_otp_write,
    "sf_otp_write",
    "otp_write addr length");

static int sf_otp_read(cmd_tbl_t *t, int argc, char *argv[])
{
 	unsigned int addr = strtoul(argv[1], NULL, 0);
	
	unsigned int length = strtoul(argv[2], NULL, 0);

	memset( sfTest, 0, 512 );
	spiFlash_OTP_Read(addr, length, (unsigned char *)sfTest);
	
	int i=0;
	
	for(i=0;i<length;++i)
	{
		system_printf("data[%d] = %d\n",i,sfTest[i]);
//		system_printf("data[%x] = %x\n",addr+i,sfTest[i]);
	}
	return CMD_RET_SUCCESS;
}

CMD(otp_read,
    sf_otp_read,
    "sf_otp_read",
    "otp_read addr length");


#endif

#endif


#define F_ADDR  (unsigned int)(2*1024*1024)

#define F_SIZE	4096



static int sf_erase(cmd_tbl_t *t, int argc, char *argv[])
{
	int ret = 0;
	ret = hal_spiflash_erase(F_ADDR, 0x1000);
	system_printf("sf_erase, %d\n", ret);
	return CMD_RET_SUCCESS;
}

CMD(sferase, sf_erase, "sf_erase",  "sf_erase");




unsigned char lh_Test[F_SIZE]  =  {0};
#define       DATABUF_BASE	      (0x00200000+0x00024000 - 0x800 -0x400)
#define       UART0_BASE		  (0x00602000) /* Device base address */
#define       LOAD_MAX_SIZE		   0x400
#define       UART_BASE            UART0_BASE


void uart_write_upload(char *pBuf, int len)
{
	uart_data_write(UART_BASE, (unsigned char *)pBuf, len);	
}
void uart_read_upload(char *pBuf, int len)
{
	int i;
	
	for(i=0; i<len; i++)
	{
		while(uart_data_tstc(UART_BASE) == 0);
		pBuf[i] = uart_data_getc(UART_BASE);
	}
}


static int ram_read(cmd_tbl_t *t, int argc, char *argv[])
{
	int tempLen,j,offset = 0;
	uint32_t addr, size, i;
	char * ram_read_buf;
	char upload_buf;
	
    ram_read_buf = os_malloc(1024);
	addr = (uint32_t)strtoul(argv[1], NULL, 0);
	addr = WORD_ALIGN(addr);
	size = (atoi(argv[2]) < sizeof(uint32_t)) ? sizeof(uint32_t) : atoi(argv[2]);
	
	uart_set_intEnable(UART_BASE, UART_INT_DISABLE, UART_INT_DISABLE);

	do
	{
	
		tempLen = min(LOAD_MAX_SIZE, size);
		memcpy((char *)ram_read_buf,(unsigned char *) addr+offset, tempLen);
		uart_write_upload((char *) ram_read_buf, (unsigned int) tempLen);

		for(j = 0; j<SPIFLASH_TIMEOUT_COUNT; j++)
			{
				uart_read_upload(&upload_buf,RESPONSE_SIZE);
				if(upload_buf == RESPONSE_OK)
				break;
			}
		size -= tempLen;
		offset += tempLen;
	}while(size);
	
	uart_set_intEnable(UART_BASE, UART_INT_DISABLE, UART_INT_ENABLE);

//	system_printf("ramread addr = 0x%x, size= %d\n",addr,tempLen);
//	system_printf("now data  addr = 0x%x, lost size= %d\n",addr+tempLen,size-tempLen);
		
		
	return CMD_RET_SUCCESS;
}
CMD(ramread,
    ram_read,
    "read 32-bit value(s) from a ram memory location",
    "Ramread <address> <size in byte>");

#if 0
int hal_spiflash_chiperase(void)
{
	unsigned int  i, j, flag, data;
	unsigned int len = 512;

	flag = system_irq_save();
	for(j = 0; j<SPIFLASH_TIMEOUT_COUNT; j++)
	{
		spiFlash_wren();
		data = spiFlash_rdsr();
		if(data & SPIFLASH_STATUS_WEL)
			break;
	}

	if((data & SPIFLASH_STATUS_WEL) == 0)
	{
		system_irq_restore(flag);
		return -1;
	}

	//section erase
	if(!spi_bus_ready(SPI2_BASE))
		return -1;

	//clear rx & tx fifo
	if(spi_fifo_reset(SPI2_BASE))
		return -2;

	OUT32(SPI2_BASE+0x20, SPI_TRANSCTRL_CMD_EN |SPI_TRANSCTRL_TRAMODE_NONE);
//	OUT32(SPI2_BASE+0x28, 0x00);	
	OUT32(SPI2_BASE+0x24, 0x60);

	for(j = 0; j<SPIFLASH_TIMEOUT_COUNT; j++)
	{
		data = spiFlash_rdsr();
		if((data & SPIFLASH_STATUS_WIP) == 0)
		{
			//system_printf("hal_spiflash_erase, %#x\n", data);
			break;
		}
	}
	system_irq_restore(flag);
		
	//system_printf("hal_spiflash_erase count: %d, ret: %#x\n", j, data);
	if(data & SPIFLASH_STATUS_WIP)
		return -2;
	
	return 0;
}
static int sf_chiperase(cmd_tbl_t *t, int argc, char *argv[])
{
	int ret = 0;
	ret = hal_spiflash_chiperase();
	system_printf("sf_erase, %d\n", ret);
	return CMD_RET_SUCCESS;
}
CMD(sfchiperase, sf_erase, "sf_chiperase",  "sf_chiperase");


//char sfTest[F_SIZE]={0};

int	spiFlash_rd(cmd_tbl_t *t, int argc, char *argv[])
{unsigned int sp_id = 0;
	
	sp_id = spiFlash_rdid();
	
	system_printf("sp_id, %x\n", sp_id);
	
	return CMD_RET_SUCCESS;
}

CMD(sf_id,
	spiFlash_rd,
	"read 32-bit value(s) from a ram memory location",
	"Ramread <address> <size in byte>");

	static int sf_read(cmd_tbl_t *t, int argc, char *argv[])
	{unsigned int add_;
	 unsigned char * ram_buf;
	 int lh=1;
	 
		hal_spiflash_read(0x00, (unsigned char *)lh_Test, F_SIZE);
		for(lh=1;lh<=F_SIZE;lh++)
			{
				system_printf("%02x", lh_Test[lh-1]);
			if (lh%4 == 0)
				system_printf(" ");
			}
		system_printf("\n");
		return CMD_RET_SUCCESS;
	}
	CMD(sfread,
		sf_read,
		"read 32-bit value(s) from a memory location",
		"read <address> <size in byte>");
	
	
		static int sf_program(cmd_tbl_t *t, int argc, char *argv[])
		{
			int i, ret = 0;
		
			for(i=0; i<F_SIZE; i++)
				sfTest[i] = i;
		
			hal_spifiash_write(F_ADDR-64, (unsigned char *)sfTest, F_SIZE);
			system_printf("sf_program, %d\n", ret);
		
			return CMD_RET_SUCCESS;
		}
		
		CMD(sfprogram,
			sf_program,
			"read 32-bit value(s) from a memory location",
			"read <address> <size in byte>");

	


//*****************************************************************
static int memcopy_test(cmd_tbl_t *t, int argc, char *argv[])
{
	int ret = 0;
	ret = hal_spiflash_erase(F_ADDR, 0x1000);
	system_printf("sf_erase, %d\n", ret);
	return CMD_RET_SUCCESS;
}

CMD(sfmem, memcopy_test, "sf_memcopy",  "sf_memcopy");

static int sf_read(cmd_tbl_t *t, int argc, char *argv[])
{unsigned int add_;
 unsigned char * ram_buf;
 int lh=1;
 
	hal_spiflash_read(0x00, (unsigned char *)lh_Test, F_SIZE);
	for(lh=1;lh<=F_SIZE;lh++)
		{
			system_printf("%02x", lh_Test[lh-1]);
        if (lh%4 == 0)
			system_printf(" ");
	    }
	system_printf("\n");
	return CMD_RET_SUCCESS;
}
CMD(sfread,
    sf_read,
    "read 32-bit value(s) from a memory location",
    "read <address> <size in byte>");


****************************************************************//



//#define F_ADDR  (unsigned int)(512*1024)
#define F_ADDR  (unsigned int)(2*1024*1024)

#define F_SIZE	1024

char sfTest[F_SIZE]={0};

static int sf_erase(cmd_tbl_t *t, int argc, char *argv[])
{
	int ret = 0;
	ret = hal_spiflash_erase(F_ADDR, 0x1000);
	system_printf("sf_erase, %d\n", ret);
	return CMD_RET_SUCCESS;
}

CMD(sferase, sf_erase, "sf_erase",  "sf_erase");

static int sf_program(cmd_tbl_t *t, int argc, char *argv[])
{
	int i, ret = 0;

	for(i=0; i<F_SIZE; i++)
		sfTest[i] = i;

	hal_spifiash_write(F_ADDR-64, (unsigned char *)sfTest, F_SIZE);
	system_printf("sf_program, %d\n", ret);

	return CMD_RET_SUCCESS;
}


static int sf_read(cmd_tbl_t *t, int argc, char *argv[])
{
	hal_spiflash_read(F_ADDR-64, (unsigned char *)sfTest, F_SIZE);
	return CMD_RET_SUCCESS;
}

static int sf_rdsr(cmd_tbl_t *t, int argc, char *argv[])
{
	int data;
	data = spiFlash_rdsr();
	system_printf("sf_rdsr, %#x\n", data);
	data = spiFlash_rdsr_h();
	system_printf("sf_rdsr_h, %#x\n", data);
	return CMD_RET_SUCCESS;
}

static int sf_wren(cmd_tbl_t *t, int argc, char *argv[])
{
	spiFlash_wren();
	return CMD_RET_SUCCESS;
}

static int sf_rdid(cmd_tbl_t *t, int argc, char *argv[])
{
	int id = spiFlash_rdid();
	system_printf("sf_rdid, %#x\n", id);
	return CMD_RET_SUCCESS;
}

static int sf_qmode(cmd_tbl_t *t, int argc, char *argv[])
{
	unsigned int j, data;

	spi_reg * pSpiReg = (spi_reg *)SPI2_BASE;

	system_printf("sf_qmode entry!!\n");

	//spiFlash_wren();

	for(j = 0; j<SPIFLASH_TIMEOUT_COUNT; j++)
	{
		spiFlash_wren();
		data = spiFlash_rdsr();
		if(data & SPIFLASH_STATUS_WEL)
			break;
	}


	if((data & SPIFLASH_STATUS_WEL) == 0)
	{
		system_printf("spiFlash_mode fail!1111\n");
	}
	
	pSpiReg->transCtrl = SPI_TRANSCTRL_CMD_EN 
					|SPI_TRANSCTRL_TRAMODE_WO
					|SPI_TRANSCTRL_WCNT(1);
	pSpiReg->cmd = SPIFLASH_CMD_WRSR;
	SPI_WAIT_TX_READY(pSpiReg->status);
	pSpiReg->data = 0x0200;

	for(j = 0; j<SPIFLASH_TIMEOUT_COUNT; j++)
	{
		data = spiFlash_rdsr();
		if((data & SPIFLASH_STATUS_WIP) == 0)
			break;
	}
	
	if(data & SPIFLASH_STATUS_WIP)
	{
		system_printf("spiFlash_mode fail!\n");
	}


	
	return CMD_RET_SUCCESS;
}

CMD(sfq,
    sf_qmode,
    "read 32-bit value(s) from a memory location",
    "read <address> <size in byte>");




CMD(sfrdid,
    sf_rdid,
    "read 32-bit value(s) from a memory location",
    "read <address> <size in byte>");


CMD(sfwren,
    sf_wren,
    "read 32-bit value(s) from a memory location",
    "read <address> <size in byte>");


CMD(sfrdsr,
    sf_rdsr,
    "read 32-bit value(s) from a memory location",
    "read <address> <size in byte>");


CMD(sfread,
    sf_read,
    "read 32-bit value(s) from a memory location",
    "read <address> <size in byte>");

CMD(sfprogram,
    sf_program,
    "read 32-bit value(s) from a memory location",
    "read <address> <size in byte>");



#endif









