#ifdef __cplusplus

extern "C" {

#endif
#ifndef _NRF24L01_H_
#define _NRF24L01_H_

#include <stdint.h>

typedef enum NRF24L01_Mode { NRF24_RX,NRF24_TX,NRF24_STB2,NRF24_STB1,NRF24_POWER_DOWN,NRF24_IDLE}NRF24L01_Mode;
typedef struct NRF24L01_STR{

	// int(*nrf24l01cesta)(void);
	// 固定成10字节//只能是8？
	uint8_t trx_pload_width;//发送数据最大字节长度，8-64 
	// 固定0
	int8_t nrf_trans_power;//发送功率最大值-18->-18dBm -12->-12dBm -6->-6dBm 0->0dBm 可选有0，-6，-12，-18
	// 固定2
	uint8_t nrf_air_rate;//发送载波频率2M和1M,25khz 可选有1和2 250
	
	uint8_t enable_int;//使用中断吗，1表示使用接受中断，其他值表示不使用中断
	// 固定 0x23,0x45,0x11,0xae,0x??
	uint8_t nrf_addr[5];//nrf地址，5个字节
	// 固定2
	uint8_t nrf_ch;

	NRF24L01_Mode Nrf24l01Mode;
	/*spi要求，时钟平时为低，在下降沿传输数据能读能写*/
	uint8_t (*nrf24l01spirw)(uint8_t data);
	void (*nrf24l01delayms)(uint32_t data);//可以不用
	void (*nrf24l01csn_h)(void);
	void (*nrf24l01csn_l)(void);
	void (*nrf24l01ce_h)(void);
	void (*nrf24l01ce_l)(void);
}NRF24L01_STR;
void NrfInit(NRF24L01_STR*dev,
	uint8_t (*nrf24l01spirw)(uint8_t data),
	void (*nrf24l01csn_h)(void),
	void (*nrf24l01csn_l)(void),
	void (*nrf24l01ce_h)(void),
	void (*nrf24l01ce_l)(void),
	uint8_t addr,
	uint8_t enable_int
	);

// -2:接受长度超过能提供的长度
// -1:无法接受数据，处于掉电模式
// 0：没有收到数据
// >0：数据长度
int8_t NRF_Receive_Data(NRF24L01_STR*dev, uint8_t *ptr,uint8_t len);
// -2:发送超时，无法发送
// -1：处于掉电模式，需要初始化
// 0：长度超过发送长度
// >0:实际发送长度
int8_t NRF_TxPacket(NRF24L01_STR*dev, uint8_t *tx_buf,uint8_t len);

void NRF24L01_TxMode(NRF24L01_STR*dev);
void NRF24L01_RxMode(NRF24L01_STR*dev);

//*************************************指令宏定义*******************************************
#endif

#ifdef __cplusplus
}
#endif