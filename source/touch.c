#include "touch.h"
#include "nrfx_uart.h"
#include <stdint.h>
#include <stdlib.h>

#define COMMAND_PACKET_LEN 12
#define REPONSE_PACKET_LEN 12


#define ACK 0x30
#define NACK 0x31

#define CMOSLED_CMD 0x12

#define ENROLLMENTCOUNT_CMD 0x20
#define CHECKENROLLMENT_CMD 0x21
#define ENROLLSTART_CMD 0x22
#define ENROLL1_CMD 0x23
#define ENROLL2_CMD 0x24
#define ENROLL3_CMD 0x25
#define ISPRESSFINGER_CMD 0x26

#define DELETE_ALL_FINGERPRINTS_CMD 0x41

#define CAPTUREFINGERPRINT_CMD 0x60


extern nrfx_uart_t huart1;

static uint8_t command_packet[12];
static uint8_t response_packet[12];



/****************************
*	Custom Data Types       *
*****************************/
typedef struct __attribute__((packed)) dev_info{
	uint32_t header;
	uint32_t FirmwareVersion;
	uint32_t IsoAreaMaxSize;
	uint8_t DeviceSerialNumber[16];
	uint16_t checksum;
}dev_info;

/****************************
*   RX/TX functions    		*
*****************************/
void touch_rcv(uint8_t* packet, int len,int timeout){
	nrfx_uart_rx(&huart1,packet,len);
}

void touch_send(uint8_t* packet, int len){
	nrfx_uart_tx(&huart1,packet,len);
}
/****************************
*   Aux functions    		*
*****************************/

uint16_t get_checksum(uint8_t *package, int len)
{
    int i = 0;
    uint16_t checksum = 0;
    for (; i < len - 2; i++)
    {
        checksum += *(package + i);
    }
    return checksum;
}

void create_command_package(uint32_t param, uint16_t command,uint8_t* package)
{
    uint16_t checksum;
    package[0] = 0x55;
    package[1] = 0xAA;
    package[2] = 0x01;
    package[3] = 0x00;
    package[4] = param & 0xFF;
    package[5] = (param >> 8) & 0xFF;
    package[6] = (param >> 16) & 0xFF;
    package[7] = (param >> 24) & 0xFF;
    package[8] = command & 0xFF;
    package[9] = (command >> 8) & 0xFF;
    checksum = get_checksum(package, COMMAND_PACKET_LEN);
    package[10] = (checksum)&0xFF;
    package[11] = (checksum >> 8) & 0xFF;
}

int rcv_ack(uint8_t* packet, int len,int timeout){
	uint16_t calc_checksum;
	uint16_t recv_checksum;
	touch_rcv(packet,len,timeout);
	recv_checksum = packet[len - 2]  | packet[len - 1] << 8;
	calc_checksum = get_checksum(packet,len);
	if(recv_checksum != calc_checksum){
		return UART_CHECKSUM_ERR;
	}else if(packet[8] != ACK){
		return UART_NACK_ERR;
	}
	return UART_OK;
}

void print_rcv(uint8_t *package, int len)
{
    int i;
    printf("RCV: ");
    for (i = 0; i < len; i++)
        printf("%x ", package[i] & 0xff);
    printf("\n");
}
void print_snd(uint8_t *package, int len)
{
    int i;
    printf("SND: ");
    for (i = 0; i < len; i++)
        printf("%x ", package[i] & 0xff);
    printf("\n");
}

/****************************
*   Operations       		*
*****************************/
void touch_open(int verbose){
	dev_info* extra_info =  (dev_info*)malloc(sizeof(dev_info));
	if(!verbose){
		create_command_package(0,1,command_packet);
		touch_send(command_packet,COMMAND_PACKET_LEN);
		if(rcv_ack(response_packet,REPONSE_PACKET_LEN,1000) != UART_OK){
			//Protocolo de errores
			free(extra_info);
			return;
		}
	}else{
		create_command_package(0,1,command_packet);
		touch_send(command_packet,COMMAND_PACKET_LEN);
		if(rcv_ack(response_packet,REPONSE_PACKET_LEN,1000) != UART_OK){
			//Protocolo de errores
			free(extra_info);
			return;
		}
		touch_rcv((uint8_t*)extra_info,sizeof(dev_info),2000);
	}
}

void touch_bakcklight(int value){
	create_command_package(value, CMOSLED_CMD, command_packet);
	touch_send(command_packet,COMMAND_PACKET_LEN);
	if(rcv_ack(response_packet,REPONSE_PACKET_LEN,1000) != UART_OK){
		//Error Handling
		return;
	}
	if(value != 0){
		printf("Touch: Backlight ON");
	}else{
		printf("Touch: Backlight OFF");
	}
}

int touch_is_press_finger(){
	create_command_package(1, ISPRESSFINGER_CMD, command_packet);
	touch_send(command_packet,COMMAND_PACKET_LEN);
	if(rcv_ack(response_packet,REPONSE_PACKET_LEN,1000) != UART_OK){
		//Error Handling
		return -1;
	}
	if(response_packet[4] == 0 && response_packet[5] == 0 && response_packet[6] == 0 && response_packet[7] == 0){
		printf("Touch: Finger Press");
		return 1;
	}
	printf("Touch: No Finger Press");
	return 0;
}

void touch_enroll_start(int id){
	//if poss > 200 no save mode
	if(id >= 200){
		id = -1;
	}
	create_command_package(id, ENROLLSTART_CMD, command_packet);
	touch_send(command_packet,COMMAND_PACKET_LEN);
	if(rcv_ack(response_packet,REPONSE_PACKET_LEN,1000) != UART_OK){
		//Error Handling
		return;
	}
}

void touch_enroll_1(){
	//Non zero argument to capture best image posible
	create_command_package(1, ENROLL1_CMD, command_packet);
	touch_send(command_packet,COMMAND_PACKET_LEN);
	if(rcv_ack(response_packet,REPONSE_PACKET_LEN,1000) != UART_OK){
		//Error Handling
		return;
	}
}
void touch_enroll_2(){
	//Non zero argument to capture best image posible
	create_command_package(1, ENROLL2_CMD, command_packet);
	touch_send(command_packet,COMMAND_PACKET_LEN);
	if(rcv_ack(response_packet,REPONSE_PACKET_LEN,1000) != UART_OK){
		//Error Handling
		return;
	}
}

//if no save mode extra data would be sent
void touch_enroll_3(int no_save_mode){
	uint8_t* extra_data;
	//Non zero argument to capture best image posible
	create_command_package(1, ENROLL3_CMD, command_packet);
	touch_send(command_packet,COMMAND_PACKET_LEN);
	if(rcv_ack(response_packet,REPONSE_PACKET_LEN,1000) != UART_OK){
		//Error Handling
		return;
	}
	if(!no_save_mode){
		return;
	}
	//TODO: Delete Magic Number
	extra_data = (uint8_t*)malloc(498);
	//TODO: Calc real accurate timeout
	touch_rcv(extra_data,498,10000);
	//TODO: Info handling
	if(extra_data[1]){
		printf("s");
	}
	return;
}

void touch_capture_finger(){
	//Non zero argument to capture best image posible
	create_command_package(1, CAPTUREFINGERPRINT_CMD, command_packet);
	touch_send(command_packet,COMMAND_PACKET_LEN);
	if(rcv_ack(response_packet,REPONSE_PACKET_LEN,1000) != UART_OK){
		//Error Handling
		return;
	}
}

int touch_check_enrolled(int id){
	int ack_reponse_code;
	//Non zero argument to capture best image posible
	create_command_package(id, CHECKENROLLMENT_CMD, command_packet);
	touch_send(command_packet,COMMAND_PACKET_LEN);
	ack_reponse_code = rcv_ack(response_packet,REPONSE_PACKET_LEN,1000);
	switch(ack_reponse_code){
	case UART_OK:
		return 1;
	case UART_NACK_ERR:
		return 0;
	default:
		return -1;
	}
}

int touch_enrolled_count(){
	int ack_reponse_code;
	//Non zero argument to capture best image posible
	create_command_package(0, ENROLLMENTCOUNT_CMD, command_packet);
	touch_send(command_packet,COMMAND_PACKET_LEN);
	ack_reponse_code = rcv_ack(response_packet,REPONSE_PACKET_LEN,1000);
	switch(ack_reponse_code){
	case UART_OK:
		return 1;
	case UART_NACK_ERR:
		return 0;
	default:
		return -1;
	}
}

void touch_delete_all_fingerprints(){
	create_command_package(0, DELETE_ALL_FINGERPRINTS_CMD, command_packet);
	touch_send(command_packet,COMMAND_PACKET_LEN);
	if(rcv_ack(response_packet,REPONSE_PACKET_LEN,1000) != UART_OK){
		//Error Handling
		return;
	}
}
