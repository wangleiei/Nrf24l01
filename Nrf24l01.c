#ifdef __cplusplus

extern "C" {

#endif
#include "Nrf24l01.h"
static uint8_t NRF_Get_freq(NRF24L01_STR*dev);
static uint8_t NRF_Write_Reg(NRF24L01_STR*dev, uint8_t reg, uint8_t value);
static uint8_t NRF_Read_Reg(NRF24L01_STR*dev, uint8_t reg);
static uint8_t NRF_Write_Buf(NRF24L01_STR*dev, uint8_t reg, uint8_t *pBuf, uint8_t uchars);
static uint8_t NRF_Read_Buf(NRF24L01_STR*dev, uint8_t reg, uint8_t *pBuf, uint8_t uchars);
static void Clear_RX_FIFO(NRF24L01_STR*dev);
static void sta_(NRF24L01_STR*dev,uint8_t sta);

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
#define STATUS          0x07  // 状态寄存器
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

#define TRX_PLOAD_WIDTH 8
void NrfInit(NRF24L01_STR*dev,
	uint8_t (*nrf24l01spirw)(uint8_t data),
	void (*nrf24l01csn_h)(void),
	void (*nrf24l01csn_l)(void),
	void (*nrf24l01ce_mode_h)(void),
	void (*nrf24l01ce_mode_l)(void),
	void (*delay1ms)(void),
	uint8_t addr,
	uint8_t enable_int
	){

	dev->nrf_addr[0] = 0x23;
	dev->nrf_addr[1] = 0x45;
	// dev->nrf_addr[2] = 0x11;
	// dev->nrf_addr[3] = 0xae;
	dev->nrf_addr[2] = addr;
	// dev->nrf_air_rate = 2;
	// dev->nrf_trans_power = 0;
	// TRX_PLOAD_WIDTH = 8;
	dev->enable_int = enable_int;
	dev->nrf_ch = 0;

	dev->nrf24l01spirw = nrf24l01spirw;
	dev->nrf24l01csn_h = nrf24l01csn_h;
	dev->nrf24l01csn_l = nrf24l01csn_l;
	dev->nrf24l01ce_mode_h = nrf24l01ce_mode_h;
	dev->nrf24l01ce_mode_l = nrf24l01ce_mode_l;
	dev->delay1ms = delay1ms;

	// 待机模式，掉电模式都可以设置配置寄存器
	dev->nrf24l01ce_mode_l();

	// 功率设置 2Mbps 0dBm
	NRF_Write_Reg(dev,RF_SETUP,0x0f);

	NRF_Write_Reg(dev,RF_CH,2);// 选择射频通道 固定通道2
	dev->nrf_ch = 0;//
	NRF_Write_Reg(dev,SETUP_AW,0x01);//3字节地址宽度

	NRF_Write_Buf(dev,TX_ADDR,dev->nrf_addr,3);//发送地址

	NRF_Write_Buf(dev,RX_ADDR_P1,dev->nrf_addr,3);//接受地址
	NRF_Write_Buf(dev,RX_ADDR_P0,dev->nrf_addr,3);//ack地址
	// 通道会在后面开启
	NRF_Write_Reg(dev,RX_PW_P0,0);//不使用通道
	NRF_Write_Reg(dev,RX_PW_P1,0);//不使用通道
	NRF_Write_Reg(dev,RX_PW_P2,0);//不使用通道
	NRF_Write_Reg(dev,RX_PW_P3,0);//不使用通道
	NRF_Write_Reg(dev,RX_PW_P4,0);//不使用通道

	// 自动重发，750us，15次，
	// 在1mbps，2mbps下500us有足够时间接受ACK携带的数据，目前不支持ack携带数据
	NRF_Write_Reg(dev,SETUP_RETR,0x2F);

	// 清理状态
	NRF_Write_Reg(dev,STATUS,0xf0);	
	NRF_Write_Reg(dev,STATUS,0x00);	
	NRF_Write_Reg(dev,EN_RXADDR,0x00);//不允许接受数据通道，
	// 待机模式1,无数据传输
	NRF_Write_Reg(dev,CONFIG,0x02);

	dev->nrf_ch = NRF_Read_Reg(dev,RF_CH);
	if(2 != dev->nrf_ch){
		dev->Nrf24l01Mode = NRF24_ERROR;
		return;
	}
	// dev->nrf24l01ce_mode_l();

	// dev->plos_cnt = 0;
	dev->auto_retry_trans_cnt = 0;
	dev->packet_trans_ber = 0.0;
	dev->Nrf24l01Mode = NRF24_STB1;
}
void NRF24L01_TxMode(NRF24L01_STR*dev){
	if(NRF24_ERROR == dev->Nrf24l01Mode){return ;}

	/*注意这里4个寄存器配置是特别设置，不用通道1，只用0*/
	NRF_Write_Reg(dev,RX_PW_P1,0);//不使用通道1
	// 发送完会自动进入接受ack模式，需要EN_AA的数据通道0设置1
	//只允许接受数据通道0启用,为了接受ACK
	NRF_Write_Reg(dev,EN_RXADDR,0x01);
	NRF_Write_Reg(dev,EN_AA,0x01);
	NRF_Write_Reg(dev,RX_PW_P0,TRX_PLOAD_WIDTH);

	dev->nrf24l01ce_mode_l();
	dev->Nrf24l01Mode = NRF24_STB1;
	// 不中断，8位CRC，上电，发送置位，待机模式1，发送函数会进入发送模式
	NRF_Write_Reg(dev,CONFIG,0x7A);

	dev->Nrf24l01Mode = NRF24_TX;
}
void NRF24L01_RxMode(NRF24L01_STR*dev){
	if(NRF24_ERROR == dev->Nrf24l01Mode){return ;}
	// 待机模式，掉电模式都可以设置配置寄存器
	dev->nrf24l01ce_mode_l();
	dev->Nrf24l01Mode = NRF24_STB1;
	// 不中断，8位CRC，上电，接受模式
	NRF_Write_Reg(dev,CONFIG,0x7B);

	NRF_Write_Reg(dev,EN_RXADDR,0x02);//只允许接受数据通道1启用	
	NRF_Write_Reg(dev,EN_AA,0x02);//启动自动ack回应
	NRF_Write_Reg(dev,RX_PW_P1,TRX_PLOAD_WIDTH);//设置通道接受数据长度
	NRF_Write_Reg(dev,RX_PW_P0,0);//不使用通道0

	dev->nrf24l01ce_mode_h();
	dev->delay1ms();//130us后nrf进入检测无线
	dev->Nrf24l01Mode = NRF24_RX;
}

// 1:发送成功
// -1:发送失败，不是发送模式
// -2:发送失败，超过最多发送上限
// -3:发送失败,MCU和NRF无法通信
int8_t NRF_TxPacket(NRF24L01_STR*dev, uint8_t *tx_buf,uint8_t len){

	uint8_t sta = 0;
	uint8_t count = 0;
	uint8_t obs = 0;

	if(dev->Nrf24l01Mode != NRF24_TX){
		return -1;
	}
	if(NRF24_ERROR == dev->Nrf24l01Mode){return - 3;}
	dev->nrf24l01ce_mode_l();
	dev->Nrf24l01Mode = NRF24_STB1;	
	NRF_Write_Buf(dev,WR_TX_PLOAD, tx_buf, TRX_PLOAD_WIDTH); // 装载数据
	dev->nrf24l01ce_mode_h();// 置高CE，激发数据发送，
	dev->delay1ms();
	dev->nrf24l01ce_mode_l();
	dev->Nrf24l01Mode = NRF24_TX;	

	// 自动进入接受模式，需要判断ack，最多发送次数,12ms之后达到最多发送次数
	while(count < 14){
		sta = NRF_Read_Reg(dev,STATUS);
		sta_(dev,sta);
		
		obs = NRF_Read_Reg(dev,OBSERVE_TX);

		// dev->plos_cnt = (obs&0xf0)>>4;
		dev->auto_retry_trans_cnt = 0x0f&obs;
		dev->packet_trans_ber = dev->auto_retry_trans_cnt/15.0;

		if(sta & 0x20){//接受到ACK
			// 会自动清除TX fifo
			NRF_Write_Reg(dev,STATUS,0x20);
			return 1;
		}else if(0x10 & sta){//发送失败
			// 需要清理MX_RT中断源
			NRF_Write_Reg(dev,STATUS,0x10);

			dev->nrf24l01ce_mode_l();                                    
			dev->nrf24l01csn_l();                                   
			dev->nrf24l01spirw(FLUSH_TX);
			dev->nrf24l01csn_h();		                               
			dev->nrf24l01ce_mode_h();				
			return -2;
		}
		dev->delay1ms();
		count++;
	}
	if(count == 14){
		return -2;
	}
}
// 0：没有收到数据
// >0：数据长度
// -3:发送失败,MCU和NRF无法通信
int8_t NRF_Receive_Data(NRF24L01_STR*dev, uint8_t *ptr,uint8_t len){
	static uint8_t recch = 0;
	uint8_t temp = 0;
	uint8_t sta = 0;
	if(dev->Nrf24l01Mode != NRF24_RX){
		return 0;
	}
	if(NRF24_ERROR == dev->Nrf24l01Mode){return - 3;}

	sta = NRF_Read_Reg(dev,STATUS);
	sta_(dev,sta);
	if(sta & 0x40){
		temp = (sta & 0x0e)>>1;
		if(0x07 == temp){
			dev->rx_fifo = RX_FIFO_EMPTY;
		}else if(0x06 == temp){
			dev->rx_fifo = RX_FIFO_NOT_USED;
		}else{
			dev->rx_fifo = temp;
		}

		dev->nrf24l01ce_mode_l();//进入待机1,只有待机模式下可以配置config
		dev->Nrf24l01Mode = NRF24_STB1;
		// 按照手册例，进入待机模式之后，读取数据，虽然手册也写了任何模式都可读取
		NRF_Read_Buf(dev,RD_RX_PLOAD,ptr,TRX_PLOAD_WIDTH);
		dev->nrf24l01csn_l();                                   
		dev->nrf24l01spirw(FLUSH_RX);
		dev->nrf24l01csn_h();		

		NRF_Write_Reg(dev,STATUS,0x40);//清理状态
		// 重新进入 不中断，8位CRC，上电，接受模式
		NRF_Write_Reg(dev,CONFIG,0x7B);
		dev->nrf24l01ce_mode_h();
		dev->Nrf24l01Mode = NRF24_RX;
		return TRX_PLOAD_WIDTH;
	}
	return 0;
}
static void sta_(NRF24L01_STR*dev,uint8_t sta){
	uint8_t temp = 0;
	temp = (sta & 0x0e)>>1;
	if(0x07 == temp){
		dev->rx_fifo = RX_FIFO_EMPTY;
	}else if(0x06 == temp){
		dev->rx_fifo = RX_FIFO_NOT_USED;
	}else{
		dev->rx_fifo = temp;
	}
	if(sta & 0x01){
		dev->tx_fifo = TX_FIFO_FULL;
	}else{
		dev->tx_fifo = TX_FIFO_NO_FULL;
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
// static void Clear_RX_FIFO(NRF24L01_STR*dev)    
// {
// 	dev->nrf24l01ce_mode_l();                                    
// 	dev->nrf24l01csn_l();                                   
// 	dev->nrf24l01spirw(FLUSH_RX);                                  // 清空FIFO，关键。不然会出现意想不到的后果！
// 	dev->nrf24l01csn_h();		                               
// 	dev->nrf24l01ce_mode_h();
// }

#ifdef __cplusplus
}
#endif