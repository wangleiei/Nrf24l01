#ifdef __cplusplus

extern "C" {

#endif
#include "Nrf24l01.h"
static NRF24L01_Mode NRF_Get_Mode(NRF24L01_STR*dev);
static uint8_t NRF_Get_freq(NRF24L01_STR*dev);
static uint8_t NRF_Write_Reg(NRF24L01_STR*dev, uint8_t reg, uint8_t value);
static uint8_t NRF_Read_Reg(NRF24L01_STR*dev, uint8_t reg);
static uint8_t NRF_Write_Buf(NRF24L01_STR*dev, uint8_t reg, uint8_t *pBuf, uint8_t uchars);
static uint8_t NRF_Read_Buf(NRF24L01_STR*dev, uint8_t reg, uint8_t *pBuf, uint8_t uchars);
static void Clear_RX_FIFO(NRF24L01_STR*dev);


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


void NRF24L01_TxMode(NRF24L01_STR*dev)
{
	uint8_t temp_u8;
	dev->nrf_ch = 0xff;
	Clear_RX_FIFO(dev);

	dev->nrf24l01ce_l();	

	NRF_Write_Buf(dev,TX_ADDR,dev->nrf_addr,5);      // 写寄存器指令+接收地址使能指令+接收地址+地址宽度
	NRF_Write_Buf(dev,RX_ADDR_P0,dev->nrf_addr,5);// 为了应答接收设备，接收通道0地址和发送地址相同	
	NRF_Write_Reg(dev,EN_AA,0);       //全部是普通模式，发送出去，就不管了 方便	

	// temp_u8 = NRF_Read_Reg(dev,EN_AA);
	NRF_Write_Reg(dev,EN_RXADDR,0x01);   // 使能接收通道0

	NRF_Write_Reg(dev,SETUP_RETR,0x0);	 //全部是普通模式，发送出去，就不管了 方便	

	NRF_Write_Reg(dev,RF_CH,2);         // 选择射频通道 固定通道2

	NRF_Write_Reg(dev,RX_PW_P0,dev->trx_pload_width);							 // 接收通道0选择和发送通道相同有效数据宽度

	temp_u8 = 0;
	if (dev->nrf_air_rate == 2)
	{
		temp_u8 |= 0x08;
	}else if(((dev->nrf_air_rate == 1))||((dev->nrf_air_rate == 250)))
	{
		temp_u8 |= 0x20;
	}

  	if (dev->nrf_trans_power == -12)
		temp_u8 |= 0x02;
	else if (dev->nrf_trans_power == -6)
		temp_u8 |= 0x04;
	else if (dev->nrf_trans_power == 0)
		temp_u8 |= 0x06;

  	NRF_Write_Reg(dev,RF_SETUP,temp_u8);    	
	NRF_Write_Reg(dev,CONFIG,0x7E);      // CRC使能，16位CRC校验，上电
	if (dev->enable_int == 1)
	{
		NRF_Write_Reg(dev,CONFIG,0x42);
	}else
	{
		NRF_Write_Reg(dev,CONFIG,0x72);
	}
	
	dev->nrf24l01csn_l();                                   
	dev->nrf24l01spirw(FLUSH_TX);// 用于清空FIFO，不然会出现意想不到的后果！ 
	dev->nrf24l01csn_h();
	// dev->nrf24l01delayms(1);//!!!!
	dev->nrf24l01ce_h();	 	
	// dev->nrf24l01delayms(700);//!!!
	dev->nrf_ch = NRF_Get_freq(dev);

	dev->Nrf24l01Mode = NRF24_TX;
}

void NRF24L01_RxMode(NRF24L01_STR*dev)
{
	uint8_t temp_u8;
	dev->nrf_ch = 0xff;
	dev->nrf24l01ce_l();	
	//dev->nrf24l01delayms(1);
	NRF_Write_Buf(dev,RX_ADDR_P0,dev->nrf_addr,5);// 为了应答接收设备，接收通道0地址和发送地址相同	
	NRF_Write_Reg(dev,EN_AA,0);       //全部是普通模式，发送出去，就不管了 方便	

	NRF_Write_Reg(dev,EN_RXADDR,0x01);   // 使能接收通道0 

	NRF_Write_Reg(dev,RF_CH,2);         // 选择射频通道 固定通道2

	NRF_Write_Reg(dev,RX_PW_P0,dev->trx_pload_width);// 接收通道0选择和发送通道相同有效数据宽度

	temp_u8 = 0;
  	if(dev->nrf_air_rate == 2){
		temp_u8 |= 0x08;
  	}
	else{
		temp_u8 |= 0x20;
	}
  	if (dev->nrf_trans_power == -12){
		temp_u8 |= 0x02;
  	}
	else if (dev->nrf_trans_power == -6){
		temp_u8 |= 0x04;
	}
	else if (dev->nrf_trans_power == 0){
		temp_u8 |= 0x06;
	}
	NRF_Write_Reg(dev,RF_SETUP,temp_u8);    // 数据传输率1Mbps，发射功率0dBm，低噪声放大器增益	
	NRF_Write_Reg(dev,CONFIG,0x77);      //，上电 	
	if(dev->enable_int == 1){
		NRF_Write_Reg(dev,CONFIG,0x33);
	}else{
		NRF_Write_Reg(dev,CONFIG,0x73);
	}
	dev->nrf24l01csn_l();                                   
	dev->nrf24l01spirw(FLUSH_RX);// 用于清空FIFO ！！关键！！不然会出现意想不到的后果！！！大家记住！！ 
	dev->nrf24l01csn_h();		                               
	
	//dev->nrf24l01delayms(1);
	dev->nrf24l01ce_h();	
	dev->nrf_ch = NRF_Get_freq(dev);

	dev->Nrf24l01Mode = NRF24_RX;
}

void NrfInit(NRF24L01_STR*dev,
	uint8_t (*nrf24l01spirw)(uint8_t data),
	void (*nrf24l01csn_h)(void),
	void (*nrf24l01csn_l)(void),
	void (*nrf24l01ce_h)(void),
	void (*nrf24l01ce_l)(void),
	uint8_t addr,
	uint8_t enable_int
	){
	dev->nrf_addr[0] = 0x23;
	dev->nrf_addr[1] = 0x45;
	dev->nrf_addr[2] = 0x11;
	dev->nrf_addr[3] = 0xae;
	dev->nrf_addr[4] = addr;
	dev->nrf_air_rate = 2;
	dev->nrf_trans_power = 0;
	dev->trx_pload_width = 8;
	dev->enable_int = enable_int;
	dev->nrf_ch = 2;

	dev->nrf24l01spirw = nrf24l01spirw;
	dev->nrf24l01csn_h = nrf24l01csn_h;
	dev->nrf24l01csn_l = nrf24l01csn_l;
	dev->nrf24l01ce_h = nrf24l01ce_h;
	dev->nrf24l01ce_l = nrf24l01ce_l;

	dev->Nrf24l01Mode = NRF24_IDLE;
}

// -2:发送超时，无法发送
// -1：处于掉电模式，需要初始化
// 0：长度超过发送长度
// >0:实际发送长度
int8_t NRF_TxPacket(NRF24L01_STR*dev, uint8_t *tx_buf,uint8_t len)
{	
	uint8_t Saft_Count_u8 = 0;
	if(NRF_Get_Mode(dev) == NRF24_POWER_DOWN){
		return -1;//掉电模式了
	}
	if (len > dev->trx_pload_width)
	{
		return 0;
	}
	NRF_Write_Reg(dev,NRFRegSTATUS,0x70);
	dev->nrf24l01ce_l();	
	NRF_Write_Buf(dev,WR_TX_PLOAD, tx_buf, len); // 装载数据
	dev->nrf24l01ce_h();// 置高CE，激发数据发送
	while((0 == (NRF_Read_Reg(dev,NRFRegSTATUS)&0x20))&&(Saft_Count_u8++<100)){//0是没有发送完
	}
	if(Saft_Count_u8 < 100){
		//清位
		NRF_Write_Reg(dev,NRFRegSTATUS,0x20);
	}else{ 
		NRF_Write_Reg(dev,NRFRegSTATUS,0x70);
		return -2;
	}
	dev->nrf24l01ce_l();                                    
	dev->nrf24l01csn_l();                                   
	dev->nrf24l01spirw(FLUSH_TX);// 清空FIFO，关键。不然会出现意想不到的后果！
	dev->nrf24l01csn_h();		                               
	dev->nrf24l01ce_h();	 
	return dev->trx_pload_width;
}
// -2:接受长度超过能提供的长度
// -1:无法接受数据，处于掉电模式
// 0：没有收到数据
// >0：数据长度
int8_t NRF_Receive_Data(NRF24L01_STR*dev, uint8_t *ptr,uint8_t len){
	uint8_t i_u8 = 0;
	if(NRF_Get_Mode(dev) == NRF24_POWER_DOWN){
		return -1;//掉电模式了
	}
	if(len > dev->trx_pload_width){
		return -2;
	}
	if(0 == (0x40&NRF_Read_Reg(dev,NRFRegSTATUS))){
		return 0;
	}else{

		NRF_Read_Buf(dev,RD_RX_PLOAD,ptr,dev->trx_pload_width);
		NRF_Write_Reg(dev,NRFRegSTATUS,0x70);//clear bit
		dev->nrf24l01csn_l();                                   
		dev->nrf24l01spirw(FLUSH_RX);// 清空FIFO，关键。不然会一直认为有接受，数据还是错误的
		dev->nrf24l01csn_h();		                               

		return dev->trx_pload_width;
	}
}
static uint8_t NRF_Write_Reg(NRF24L01_STR*dev, uint8_t reg, uint8_t value){
	uint8_t status;
	dev->nrf24l01csn_l();					      // 选通器件
	status = dev->nrf24l01spirw(NRF_WRITE_REG | reg);   
										  // 写寄存器地址
	dev->nrf24l01spirw(value);		      // 写数据
	dev->nrf24l01csn_h();						  // 禁止该器件
	return 	status;
}
static uint8_t NRF_Read_Reg(NRF24L01_STR*dev, uint8_t reg){
	uint8_t reg_val;
	dev->nrf24l01csn_l();					  // 选通器件 
	dev->nrf24l01spirw(NRF_READ_REG | reg);			      // 写寄存器地址 
	reg_val = dev->nrf24l01spirw(0);	  // 读取该寄存器返回数据 
	dev->nrf24l01csn_h();					  // 禁止该器件 
  	return 	reg_val;
}

static uint8_t NRF_Write_Buf(NRF24L01_STR*dev, uint8_t reg, uint8_t *pBuf, uint8_t uchars)
{
	uint8_t i;
	uint8_t status;
	dev->nrf24l01csn_l();				     /* 选通器件 */
	status = dev->nrf24l01spirw(NRF_WRITE_REG | reg);	   /* 写寄存器地址 */
	for(i=0; i<uchars; i++)
	{
		dev->nrf24l01spirw(pBuf[i]);		   /* 写数据 */
	}
	dev->nrf24l01csn_h();						 /* 禁止该器件 */
  return 	status;	
}
static uint8_t NRF_Read_Buf(NRF24L01_STR*dev, uint8_t reg, uint8_t *pBuf, uint8_t uchars)
{
	uint8_t i;
	uint8_t status;
	dev->nrf24l01csn_l();						/* 选通器件 */
	status = dev->nrf24l01spirw(NRF_READ_REG | reg);  	/* 写寄存器地址 */
	for(i=0; i<uchars; i++)
	{
		pBuf[i] = dev->nrf24l01spirw(0);  /* 读取返回数据 */ 	
	}
	dev->nrf24l01csn_h();						/* 禁止该器件 */
  return 	status;
}
/*得到通道频率*/
static uint8_t NRF_Get_freq(NRF24L01_STR*dev){
	return NRF_Read_Reg(dev,RF_CH);
}
/*得到当前工作模式*///
static NRF24L01_Mode NRF_Get_Mode(NRF24L01_STR*dev){
	uint8_t te = NRF_Read_Reg(dev,CONFIG);
	// uint8_t cepin = dev->nrf24l01cesta();
	if(0 == (te&0x02)){
		return NRF24_POWER_DOWN;
	}else{
		if(te&0x01){
			return NRF24_RX;
		}else{
			return NRF24_STB2;
		}
	}
}
static void Clear_RX_FIFO(NRF24L01_STR*dev)    
{
	dev->nrf24l01ce_l();                                    
	dev->nrf24l01csn_l();                                   
	dev->nrf24l01spirw(FLUSH_RX);                                  // 清空FIFO，关键。不然会出现意想不到的后果！
	dev->nrf24l01csn_h();		                               
	dev->nrf24l01ce_h();
}

#ifdef __cplusplus
}
#endif