#ifndef TOUCH_H_
#define TOUCH_H_

//TODO: ERROR HANDLER
enum TOUCH_STATE{
	TOUCH_OK,
};

enum UART_ERR{
	UART_OK,
	UART_CHECKSUM_ERR,
	UART_NACK_ERR
};

void touch_open(int verbose);
void touch_bakcklight(int value);
int touch_is_press_finger();
void touch_enroll_start();
void touch_enroll_1();
void touch_enroll_2();
void touch_enroll_3(int no_save_mode);
void touch_capture_finger();
int touch_check_enrolled(int id);
int touch_enrolled_count();
void touch_delete_all_fingerprints();
#endif /* TOUCH_H_ */