#ifdef __cplusplus

extern "C" {

#endif
#ifndef _NRF24L01_H_
#define _NRF24L01_H_

#include <stdint.h>

typedef struct NRF24L01_STR{

	uint8_t (*nrf24l01spirw)(uint8_t data);/*spi要求，时钟平时为低，在下降沿传输数据能读能写*/
	void (*nrf24l01delayms)(uint32_t data);
	void (*nrf24l01csn_h)(void);
	void (*nrf24l01csn_l)(void);
	void (*nrf24l01ce_h)(void);
	void (*nrf24l01ce_l)(void);
	int(*nrf24l01cesta)(void);
	uint8_t trx_pload_width;//发送数据最大字节长度，8-64
	int8_t nrf_trans_power;//发送功率最大值-18->-18dBm -12->-12dBm -6->-6dBm 0->0dBm 可选有0，-6，-12，-18
	uint8_t nrf_air_rate;//发送载波频率2M和1M,25khz 可选有1和2 250
	uint8_t enable_int;//使用中断吗，1表示使用接受中断，其他值表示不使用中断
	uint8_t nrf_addr[5];//nrf地址，5个字节
	
}NRF24L01_STR;
enum NRF_Mode { NRF_RX,NRF_TX,NRF_STB2,NRF_STB1,NRF_POWER_DOWN };


uint8_t NRF_TxPacket(NRF24L01_STR*base, uint8_t *tx_buf,uint8_t len);
uint8_t NRF_Receive_Data(NRF24L01_STR*base, uint8_t *ptr,uint8_t len);
// uint8_t NRF_Get_freq(NRF24L01_STR*base);
// uint8_t NRF_Get_Mode(NRF24L01_STR*base);
void NRF24L01_TxMode(NRF24L01_STR*base,uint8_t ch);
void NRF24L01_RxMode(NRF24L01_STR*base,uint8_t ch);

//*************************************指令宏定义*******************************************
#endif

#ifdef __cplusplus
}
#endif