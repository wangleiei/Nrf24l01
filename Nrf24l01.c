#include "Nrf24l01.h"
static uint8_t NRF_Write_Reg(NRF24L01_STR*base, uint8_t reg, uint8_t value);
static uint8_t NRF_Read_Reg(NRF24L01_STR*base, uint8_t reg);
static uint8_t NRF_Write_Buf(NRF24L01_STR*base, uint8_t reg, uint8_t *pBuf, uint8_t uchars);
static uint8_t NRF_Read_Buf(NRF24L01_STR*base, uint8_t reg, uint8_t *pBuf, uint8_t uchars);
static void Clear_RX_FIFO(NRF24L01_STR*base);


//***************************************NRF24L01寄存器指令*******************************************************
#define NRF_READ_REG    0x00  // 读寄存器指令
#define NRF_WRITE_REG   0x20 	// 写寄存器指令
#define R_RX_PL_WID   	0x60
#define RD_RX_PLOAD     0x61  // 读取接收数据指令
#define WR_TX_PLOAD     0xA0  // 写待发数据指令
#define FLUSH_TX        0xE1 	// 清除发送 FIFO指令
#define FLUSH_RX        0xE2  // 清除接收 FIFO指令
#define REUSE_TX_PL     0xE3  // 定义重复装载数据指令
//*************************************SPI(nRF24L01)寄存器地址****************************************************
#define CONFIG          0x00  // 配置收发状态，CRC校验模式以及收发状态响应方式
#define EN_AA           0x01  // 自动应答功能设置
#define EN_RXADDR       0x02  // 可用信道设置
#define SETUP_AW        0x03  // 收发地址宽度设置
#define SETUP_RETR      0x04  // 自动重发功能设置
#define RF_CH           0x05  // 工作频率设置
#define RF_SETUP        0x06  // 发射速率、功耗功能设置
#define NRFRegSTATUS    0x07  // 状态寄存器
#define OBSERVE_TX      0x08  // 发送监测功能
#define RPD             0x09  // 
#define RX_ADDR_P0      0x0A  // 频道0接收数据地址
#define RX_ADDR_P1      0x0B  // 频道1接收数据地址
#define RX_ADDR_P2      0x0C  // 频道2接收数据地址
#define RX_ADDR_P3      0x0D  // 频道3接收数据地址
#define RX_ADDR_P4      0x0E  // 频道4接收数据地址
#define RX_ADDR_P5      0x0F  // 频道5接收数据地址
#define TX_ADDR         0x10  // 发送地址寄存器
#define RX_PW_P0        0x11  // 接收频道0接收数据长度
#define RX_PW_P1        0x12  // 接收频道1接收数据长度
#define RX_PW_P2        0x13  // 接收频道2接收数据长度
#define RX_PW_P3        0x14  // 接收频道3接收数据长度
#define RX_PW_P4        0x15  // 接收频道4接收数据长度
#define RX_PW_P5        0x16  // 接收频道5接收数据长度
#define FIFO_STATUS     0x17  // FIFO栈入栈出状态寄存器设置


//*************************************************************************************************
// 功能描述 : 写寄存器
// 输入参数 : reg寄存器地址，value写入的值
// 返回参数 : status写入后状态
// 说    明 : 
// 修改时间 : 
// 修改内容 : 
//*************************************************************************************************
static uint8_t NRF_Write_Reg(NRF24L01_STR*base, uint8_t reg, uint8_t value){
	uint8_t status;
	base->nrf24l01csn_l();					      // 选通器件
	status = base->nrf24l01spirw(NRF_WRITE_REG base,| reg);   
										  // 写寄存器地址
	base->nrf24l01spirw(value);		      // 写数据
	base->nrf24l01csn_h();						  // 禁止该器件
	return 	status;
}
//*************************************************************************************************
// 功能描述 : 读寄存器
// 输入参数 : reg寄存器地址
// 返回参数 : 寄存器值
// 说    明 : 
// 修改时间 : 
// 修改内容 : 
//*************************************************************************************************
static uint8_t NRF_Read_Reg(NRF24L01_STR*base, uint8_t reg){
	uint8_t reg_val;
	base->nrf24l01csn_l();					  // 选通器件 
	base->nrf24l01spirw(reg);			      // 写寄存器地址 
	reg_val = base->nrf24l01spirw(0);	  // 读取该寄存器返回数据 
	base->nrf24l01csn_h();					  // 禁止该器件 
  	return 	reg_val;
}

//*************************************************************************************************
// 功能描述 : 缓冲区数据写入寄存器
// 输入参数 : reg寄存器地址，*pBuf待写入数据地址，uchars数据量
// 返回参数 : 写入状态
// 说    明 : 
// 修改时间 : 
// 修改内容 : 
//*************************************************************************************************
static uint8_t NRF_Write_Buf(NRF24L01_STR*base, uint8_t reg, uint8_t *pBuf, uint8_t uchars)
{
	uint8_t i;
	uint8_t status;
	base->nrf24l01csn_l();				     /* 选通器件 */
	status = base->nrf24l01spirw(NRF_WRITE_REG base,| reg);	   /* 写寄存器地址 */
	for(i=0; i<uchars; i++)
	{
		base->nrf24l01spirw(pBuf[i]);		   /* 写数据 */
	}
	base->nrf24l01csn_h();						 /* 禁止该器件 */
  return 	status;	
}

//*************************************************************************************************
// 功能描述 : 读寄存器数据存入缓冲区
// 输入参数 : reg寄存器地址，*pBuf读出的数据暂存地址，uchars数据量
// 返回参数 : 读取状态
// 说    明 : 
// 修改时间 : 
// 修改内容 : 
//*************************************************************************************************
static uint8_t NRF_Read_Buf(NRF24L01_STR*base, uint8_t reg, uint8_t *pBuf, uint8_t uchars)
{
	uint8_t i;
	uint8_t status;
	base->nrf24l01csn_l();						/* 选通器件 */
	status = base->nrf24l01spirw(reg);  	/* 写寄存器地址 */
	for(i=0; i<uchars; i++)
	{
		pBuf[i] = base->nrf24l01spirw(0);  /* 读取返回数据 */ 	
	}
	base->nrf24l01csn_h();						/* 禁止该器件 */
  return 	status;
}

//*************************************************************************************************
// 功能描述 : 发送缓冲区数据
// 输入参数 : *tx_buf发送缓冲区地址
// 返回参数 : Keine
// 说    明 : 
// 修改时间 :
// 修改内容 : 
//*************************************************************************************************
uint8_t NRF_TxPacket(NRF24L01_STR*base, uint8_t *tx_buf,uint8_t len)
{	
	uint8_t i_u8 = 0;
	uint8_t Saft_Count_u8 = 0;
	if(NRF_Get_Mode() == NRF_POWER_DOWN){
		return 1;//掉电模式了
	}
	if (len > base->base->trx_pload_width)
	{
		return 1;
	}
	NRF_Write_Reg(base,NRFRegSTATUS,0x70);
	base->nrf24l01ce_l();	
	NRF_Write_Buf(base,WR_TX_PLOAD, tx_buf, len); // 装载数据
	base->nrf24l01ce_h();// 置高CE，激发数据发送
	while((0 == (NRF_Read_Reg(base,NRFRegSTATUS)&0x20))&&(Saft_Count_u8++<100)){//0是没有发送完
	}
	if(Saft_Count_u8 < 100){
		//清位
		NRF_Write_Reg(base,NRFRegSTATUS,0x20);
		i_u8  = 0;
	}else{ 
		NRF_Write_Reg(base,NRFRegSTATUS,0x70);
		return 1;
	}
	base->nrf24l01ce_l();                                    
	base->nrf24l01csn_l();                                   
	base->nrf24l01spirw(FLUSH_TX);// 清空FIFO，关键。不然会出现意想不到的后果！
	base->nrf24l01csn_h();		                               
	base->nrf24l01ce_h();	 
	return i_u8;
}
/*NRF接受函数*/
uint8_t NRF_Receive_Data(NRF24L01_STR*base, uint8_t *ptr,uint8_t len){
	uint8_t i_u8 = 0;
	if(NRF_Get_Mode() == NRF_POWER_DOWN){
		return 1;//掉电模式了
	}
	if(len<base->trx_pload_width){return 1;}
	if(0 == (0x40&NRF_Read_Reg(base,NRFRegSTATUS))){
		return 1;
	}else{
		// base->nrf24l01ce_l();
		NRF_Read_Buf(base,RD_RX_PLOAD,ptr,base->trx_pload_width);
		NRF_Write_Reg(base,NRFRegSTATUS,0x70);//clear bit
		base->nrf24l01csn_l();                                   
		base->nrf24l01spirw(FLUSH_RX);// 清空FIFO，关键。不然会一直认为有接受，数据还是错误的
		base->nrf24l01csn_h();		                               
		// base->nrf24l01ce_h();
		return 0;	
	}
}
//*************************************************************************************************
// 功能描述 : 检查能不能读写寄存器
// 输入参数 : Keine
// 返回参数 : 初始化结果 
// 说    明 : 
// 修改时间 : 
// 修改内容 : 
//*************************************************************************************************
uint8_t NRF_Test(NRF24L01_STR*base){}
 
/*得到通道频率*/
uint8_t NRF_Get_freq(NRF24L01_STR*base){
	return NRF_Read_Reg(base,RF_CH);
}
/*得到当前工作模式*/// enum NRF_Mode { NRF_RX,NRF_TX,NRF_STB2,NRF_STB1,NRF_POWER_DOWN };
uint8_t NRF_Get_Mode(NRF24L01_STR*base){
	uint8_t te = NRF_Read_Reg(base,CONFIG);
	uint8_t cepin = base->nrf24l01cesta;
	if(0 == (te&0x02)){
		return NRF_POWER_DOWN;
	}else{
		if(0 == cepin){
			return NRF_STB1;
		}
		if((1 == cepin)&&(0 == (te&0x01))){
			return NRF_STB2;
		}
		if((1 == cepin)&&(1 == (te&0x01))){
			return NRF_RX;
		}
		return 1;
	}
}
//*************************************************************************************************
// 功能描述 : 清除接收FIFO
// 输入参数 : Keine
// 返回参数 : Keine
// 说    明 : 发送模式切换为接收模式前先清除接收FIFO 
// 修改时间 : 
// 修改内容 : 
//*************************************************************************************************
static void Clear_RX_FIFO(NRF24L01_STR*base)    
{
	base->nrf24l01ce_l();                                    
	base->nrf24l01csn_l();                                   
	base->nrf24l01spirw(FLUSH_RX);                                  // 清空FIFO，关键。不然会出现意想不到的后果！
	base->nrf24l01csn_h();		                               
	base->nrf24l01ce_h();
}

//*************************************************************************************************
// 功能描述 : 配置成TX_PLOAD_WIDTH Bytes发射数据 
// 输入参数 : ch 通道号
// 返回参数 : Keine
// 说    明 : 发送读卡结果
// 修改时间 : 2017.5.23
// 修改内容 : 
//*************************************************************************************************
void NRF24L01_TxMode(NRF24L01_STR*base,uint8_t ch)
{
	uint8_t temp_u8;
	Clear_RX_FIFO(base);

	base->nrf24l01ce_l();	
	// base->nrf24l01delayms(1);
	NRF_Write_Buf(base,TX_ADDR,base->nrf_addr,5);      // 写寄存器指令+接收地址使能指令+接收地址+地址宽度
	NRF_Write_Buf(base,RX_ADDR_P0,base->nrf_addr,5);// 为了应答接收设备，接收通道0地址和发送地址相同	
	NRF_Write_Reg(base,EN_AA,0);       //全部是普通模式，发送出去，就不管了 方便	
	// base->nrf24l01delayms(1);
	// temp_u8 = NRF_Read_Reg(base,EN_AA);
	NRF_Write_Reg(base,EN_RXADDR,0x01);   // 使能接收通道0

	NRF_Write_Reg(base,SETUP_RETR,0x0);	 //全部是普通模式，发送出去，就不管了 方便	

	NRF_Write_Reg(base,RF_CH,ch);         // 选择射频通道

	NRF_Write_Reg(base,RX_PW_P0,base->trx_pload_width);							 // 接收通道0选择和发送通道相同有效数据宽度

	temp_u8 = 0;
	if (base->nrf_air_rate == 2)
	{
		temp_u8 |= 0x08;
	}else if(((base->nrf_air_rate == 1))||((base->nrf_air_rate == 250)))
	{
		temp_u8 |= 0x20;
	}

  	if (nrf_trans_power == -12)
		temp_u8 |= 0x02;
	else if (nrf_trans_power == -6)
		temp_u8 |= 0x04;
	else if (nrf_trans_power == 0)
		temp_u8 |= 0x06;

  	NRF_Write_Reg(base,RF_SETUP,temp_u8);    	
	NRF_Write_Reg(base,CONFIG,0x7E);      // CRC使能，16位CRC校验，上电
	if (base->enable_int == 1)
	{
		NRF_Write_Reg(base,CONFIG,0x42);
	}else
	{
		NRF_Write_Reg(base,CONFIG,0x72);
	}
	
	base->nrf24l01csn_l();                                   
	base->nrf24l01spirw(FLUSH_TX);                              // 用于清空FIFO，不然会出现意想不到的后果！ 
	base->nrf24l01csn_h();
	base->nrf24l01delayms(1);
	base->nrf24l01ce_h();	 	
	base->nrf24l01delayms(700);
}

//*************************************************************************************************
// 功能描述 : 接收指令函数
// 输入参数 : ch 通道号
// 返回参数 : Keine
// 说    明 : 接收动作遥控指令专用
// 修改时间 : 
// 修改内容 : 
//*************************************************************************************************
void NRF24L01_RxMode(NRF24L01_STR*base,uint8_t ch)
{
	uint8_t temp_u8;
	base->nrf24l01ce_l();	
	base->nrf24l01delayms(1);
	NRF_Write_Buf(base,RX_ADDR_P0,base->nrf_addr,5);// 为了应答接收设备，接收通道0地址和发送地址相同	
	NRF_Write_Reg(base,EN_AA,0);       //全部是普通模式，发送出去，就不管了 方便	

	NRF_Write_Reg(base,EN_RXADDR,0x01);   // 使能接收通道0 

	NRF_Write_Reg(base,RF_CH,ch);         // 选择射频通道

	NRF_Write_Reg(base,RX_PW_P0,base->trx_pload_width);// 接收通道0选择和发送通道相同有效数据宽度

	temp_u8 = 0;
  	if(base->nrf_air_rate == 2){
		temp_u8 |= 0x08;
  	}
	else{
		temp_u8 |= 0x20;
	}
  	if base->nrf_trans_power == -12{
		temp_u8 |= 0x02;
  	}
	else if base->nrf_trans_power == -6{
		temp_u8 |= 0x04;
	}
	else if base->nrf_trans_power == 0{
		temp_u8 |= 0x06;
	}
	NRF_Write_Reg(base,RF_SETUP,temp_u8);    // 数据传输率1Mbps，发射功率0dBm，低噪声放大器增益	
	NRF_Write_Reg(base,CONFIG,0x77);      //，上电 	
	if(base->enable_int == 1){
		NRF_Write_Reg(base,CONFIG,0x33);
	}else{
		NRF_Write_Reg(base,CONFIG,0x73);
	}
	base->nrf24l01csn_l();                                   
	base->nrf24l01spirw(FLUSH_RX);// 用于清空FIFO ！！关键！！不然会出现意想不到的后果！！！大家记住！！ 
	base->nrf24l01csn_h();		                               
	
	base->nrf24l01delayms(1);
	base->nrf24l01ce_h();	
}