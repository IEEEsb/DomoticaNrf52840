#include "touch.h"
#include "nrfx_uart.h"
#include "SEGGER_RTT.h"
#include <stdint.h>
#include <stdlib.h>

/****************************
*	LENGTH DEFINES       *
*****************************/

#define COMMAND_PACKET_LEN 12
#define REPONSE_PACKET_LEN 12

/****************************
*	RESPONSE DEFINES       *
*****************************/

#define ACK 0x30
#define NACK 0x31

/****************************
*	NACK CODES DEFINES       *
*****************************/
#define  NACK_DB_IS_FULL 0x1009
#define  NACK_DB_IS_EMPTY 0x100A
#define  NACK_INVALID_POS 0x1003
#define  NACK_IS_ALREADY_USED 0x1005
#define  NACK_ENROLL_FAILED 0x100D
#define  NACK_BAD_FINGER 0x100C
#define  NACK_IDENTIFY_FAILED 0x1007
/****************************
*	CMD CODES DEFINES       *
*****************************/
#define OPEN_CMD 0x01
#define CLOSE_CMD 0x02
#define CMOSLED_CMD 0x12

#define ENROLLMENTCOUNT_CMD 0x20
#define CHECKENROLLMENT_CMD 0x21
#define ENROLLSTART_CMD 0x22
#define ENROLL1_CMD 0x23
#define ENROLL2_CMD 0x24
#define ENROLL3_CMD 0x25
#define ISPRESSFINGER_CMD 0x26

#define DELETE_ALL_FINGERPRINTS_CMD 0x41

#define IDENTIFY_CMD 0x51

#define CAPTUREFINGERPRINT_CMD 0x60

extern nrfx_uart_t huart1;

static uint8_t command_packet[12];
static uint8_t response_packet[12];

/****************************
*	Custom Data Types       *
*****************************/
typedef struct __attribute__((packed)) dev_info {
  uint32_t header;
  uint32_t FirmwareVersion;
  uint32_t IsoAreaMaxSize;
  uint8_t DeviceSerialNumber[16];
  uint16_t checksum;
} dev_info;

/****************************
*   RX/TX functions    		*
*****************************/
void touch_rcv(uint8_t *packet, int len, int timeout) {
  nrfx_uart_rx(&huart1, packet, len);
}

void touch_send(uint8_t *packet, int len) {
  nrfx_uart_tx(&huart1, packet, len);
}
/****************************
*   Aux functions    		*
*****************************/

uint16_t get_checksum(uint8_t *package, int len) {
  int i = 0;
  uint16_t checksum = 0;
  for (; i < len - 2; i++) {
    checksum += *(package + i);
  }
  return checksum;
}

void create_command_package(uint32_t param, uint16_t command, uint8_t *package) {
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

int rcv_ack(uint8_t *packet, int len, int timeout) {
  uint16_t calc_checksum;
  uint16_t recv_checksum;
  touch_rcv(packet, len, timeout);
  recv_checksum = packet[len - 2] | packet[len - 1] << 8;
  calc_checksum = get_checksum(packet, len);
  if (recv_checksum != calc_checksum) {
    return UART_CHECKSUM_ERR;
  } else if (packet[8] != ACK) {
    return UART_NACK_ERR;
  }
  return UART_OK;
}

void print_rcv(uint8_t *package, int len) {
  int i;
  printf("RCV: ");
  for (i = 0; i < len; i++)
    printf("%x ", package[i] & 0xff);
  printf("\n");
}
void print_snd(uint8_t *package, int len) {
  int i;
  printf("SND: ");
  for (i = 0; i < len; i++)
    printf("%x ", package[i] & 0xff);
  printf("\n");
}

void get_nack_error_code(uint8_t *package, int len,uint32_t* nack_code){
  if(len != 12 || package[8] != NACK){
    printf("ERROR THIS IS NOT NACK \n");
    nack_code = NULL;
  }
  *(nack_code) = (package[4] & 0xFF) | ((package[5] & 0xFF) << 8) | ((package[6] & 0xFF) << 16) | ((package[7] & 0xFF) << 24);
}

void process_nack(uint8_t* package, int len){
  uint32_t nack_code;
  get_nack_error_code(package,len,&nack_code);
  switch(nack_code){
    case NACK_ENROLL_FAILED:
      printf("ERROR NACK ENROLL FAILED \n");
      return;
    case NACK_BAD_FINGER:
      printf("ERROR BAD FINGER FAILED \n");
      return;
    case NACK_IDENTIFY_FAILED:
      printf("ERROR INDENTIFY FAILED \n");
      return;
     case NACK_DB_IS_EMPTY:
      printf("ERROR DB IS EMPTY \n");
      return;
     case NACK_DB_IS_FULL:
      printf("ERROR DB IS FULL \n");
      return;
  }
}

uint32_t get_reponse_parameter(uint8_t* package, int len){
  if(len != 12 || package[8] != ACK){
    printf("ERROR THIS IS NOT ACK \n");
  }
  return (package[4] & 0xFF) | ((package[5] & 0xFF) << 8) | ((package[6] & 0xFF) << 16) | ((package[7] & 0xFF) << 24);
}

/****************************
*   Operations       		*
*****************************/
void touch_open(int verbose) {
  dev_info *extra_info = (dev_info *)malloc(sizeof(dev_info));
  if (!verbose) {
    create_command_package(0, OPEN_CMD, command_packet);
    touch_send(command_packet, COMMAND_PACKET_LEN);
    if (rcv_ack(response_packet, REPONSE_PACKET_LEN, 1000) != UART_OK) {
      printf("ERROR OPENNING CONNECTION TO TOUCH \n");
      free(extra_info);
      return;
    }
  } else {
    create_command_package(1, OPEN_CMD, command_packet);
    touch_send(command_packet, COMMAND_PACKET_LEN);
    if (rcv_ack(response_packet, REPONSE_PACKET_LEN, 1000) != UART_OK) {
      printf("ERROR OPENNING CONNECTION TO TOUCH \n");
      free(extra_info);
      return;
    }
    touch_rcv((uint8_t *)extra_info, sizeof(dev_info), 2000);
  }
  printf("CONNECTION OPENNED TO TOUCH \n");
}

void touch_close(void){
  create_command_package(0, CLOSE_CMD, command_packet);
  touch_send(command_packet, COMMAND_PACKET_LEN);
  if (rcv_ack(response_packet, REPONSE_PACKET_LEN, 1000) != UART_OK) {
    printf("ERROR CLOSING CONNECTION TO TOUCH \n");
    return;
  }
  printf("CONNECTION TO TOUCH CLOSED \n");
}

void touch_bakcklight(int value) {
  create_command_package(value, CMOSLED_CMD, command_packet);
  touch_send(command_packet, COMMAND_PACKET_LEN);
  if (rcv_ack(response_packet, REPONSE_PACKET_LEN, 1000) != UART_OK) {
    //Error Handling
    return;
  }
  if (value != 0) {
    printf("Touch: Backlight ON \n");
  } else {
    printf("Touch: Backlight OFF \n");
  }
}

int touch_is_press_finger() {
  create_command_package(1, ISPRESSFINGER_CMD, command_packet);
  touch_send(command_packet, COMMAND_PACKET_LEN);
  if (rcv_ack(response_packet, REPONSE_PACKET_LEN, 1000) != UART_OK) {
    //Error Handling
    return -1;
  }
  if (response_packet[4] == 0 && response_packet[5] == 0 && response_packet[6] == 0 && response_packet[7] == 0) {
    //printf("Touch: Finger Press");
    return 1;
  }
  //printf("Touch: No Finger Press");
  return 0;
}

void touch_enroll_start(int id) {
  //if poss > 200 no save mode
  if (id >= 200) {
    id = -1;
  }
  create_command_package(id, ENROLLSTART_CMD, command_packet);
  touch_send(command_packet, COMMAND_PACKET_LEN);
  if (rcv_ack(response_packet, REPONSE_PACKET_LEN, 1000) != UART_OK) {
    //Error Handling
    return;
  }
}

void touch_enroll_1() {
  //Non zero argument to capture best image posible
  create_command_package(1, ENROLL1_CMD, command_packet);
  touch_send(command_packet, COMMAND_PACKET_LEN);
  if (rcv_ack(response_packet, REPONSE_PACKET_LEN, 1000) != UART_OK) {
    process_nack(response_packet,REPONSE_PACKET_LEN);
    printf("Error in enrollment 1 \n");
    return;
  }
}
void touch_enroll_2() {
  //Non zero argument to capture best image posible
  create_command_package(1, ENROLL2_CMD, command_packet);
  touch_send(command_packet, COMMAND_PACKET_LEN);
  if (rcv_ack(response_packet, REPONSE_PACKET_LEN, 1000) != UART_OK) {
    process_nack(response_packet,REPONSE_PACKET_LEN);
    printf("Error in enrollment 2 \n");
    return;
  }
}

//if no save mode extra data would be sent
void touch_enroll_3(int enrollment_mode) {
  uint8_t *extra_data;
  //Non zero argument to capture best image posible
  create_command_package(1, ENROLL3_CMD, command_packet);
  touch_send(command_packet, COMMAND_PACKET_LEN);
  if (rcv_ack(response_packet, REPONSE_PACKET_LEN, 1000) != UART_OK) {
    process_nack(response_packet,REPONSE_PACKET_LEN);
    printf("Error in enrollment 3 \n");
    return;
  }
  if (enrollment_mode == TOUCH_SAVEMODE_ENABLE) {
    return;
  }
  //TODO: Delete Magic Number
  extra_data = (uint8_t *)malloc(498);
  //TODO: Calc real accurate timeout
  touch_rcv(extra_data, 498, 10000);
  //TODO: Info handling
  if (extra_data[1]) {
    printf("s");
  }
  free(extra_data);
  return;
}

void touch_capture_finger() {
  //Non zero argument to capture best image posible
  create_command_package(1, CAPTUREFINGERPRINT_CMD, command_packet);
  touch_send(command_packet, COMMAND_PACKET_LEN);
  if (rcv_ack(response_packet, REPONSE_PACKET_LEN, 1000) != UART_OK) {
    //Error Handling
    return;
  }
}

int touch_check_enrolled(int id) {
  int ack_response_code;
  //Non zero argument to capture best image posible
  create_command_package(id, CHECKENROLLMENT_CMD, command_packet);
  touch_send(command_packet, COMMAND_PACKET_LEN);
  ack_response_code = rcv_ack(response_packet, REPONSE_PACKET_LEN, 1000);
  if(ack_response_code == UART_OK){
    return TOUCH_ENROLLED;
  }
  return TOUCH_NOT_ENROLLED;
}

int touch_enrolled_count() {
  create_command_package(0, ENROLLMENTCOUNT_CMD, command_packet);
  touch_send(command_packet, COMMAND_PACKET_LEN);
  if (rcv_ack(response_packet, REPONSE_PACKET_LEN, 1000) != UART_OK) {
    process_nack(response_packet,REPONSE_PACKET_LEN);
    printf("error getting enrolled count \n");
    return;
  }
  return get_reponse_parameter(response_packet,REPONSE_PACKET_LEN);
}

void touch_delete_all_fingerprints() {
  create_command_package(0, DELETE_ALL_FINGERPRINTS_CMD, command_packet);
  touch_send(command_packet, COMMAND_PACKET_LEN);
  if (rcv_ack(response_packet, REPONSE_PACKET_LEN, 1000) != UART_OK) {
    process_nack(response_packet,REPONSE_PACKET_LEN);
    printf("Error in deleting all fingerprints \n");
    return;
  }
}

int touch_identify(){
  create_command_package(0, IDENTIFY_CMD, command_packet);
  touch_send(command_packet, COMMAND_PACKET_LEN);
  if (rcv_ack(response_packet, REPONSE_PACKET_LEN, 1000) != UART_OK) {
    process_nack(response_packet,REPONSE_PACKET_LEN);
    printf("Indetification Error \n");
    return TOUCH_IDENTIFICATION_ERROR;
  }
  return TOUCH_IDENTIFICATION_CORRECT;
}