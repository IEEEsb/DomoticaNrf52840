/*
 * identification.c
 *
 *  Created on: 4 sept. 2018
 *      Author: jcala
 */

#include "identification_controller.h"
#include "SEGGER_RTT.h"
#include "nrf_delay.h"
#include "touch.h"

static void wait_for_finger() {
  while (!touch_is_press_finger()) {
    nrf_delay_ms(100);
  }
}

static void wait_for_finger_release() {
  while (touch_is_press_finger()) {
    nrf_delay_ms(100);
  }
}

void gt511c3_reset_data(void) {
  touch_delete_all_fingerprints();
}

void gt511c3_initialize(void) {
  touch_open(0);
  touch_bakcklight(1);
}

void gt511c3_deinitialize(void) {
  touch_close();
}

void gt511c3_enroll_user(int id) {

  int enrollment_mode;
  //Enrollment mode
  if (id == -1) {
    enrollment_mode = TOUCH_SAVEMODE_DISABLE;
  } else {
    enrollment_mode = TOUCH_SAVEMODE_ENABLE;
  }
  //Enrollment process
  printf("BEGINING ENROLLMENT \n");
  touch_enroll_start(id);
  printf("Press Finger (1) \n");
  wait_for_finger();
  touch_capture_finger();
  touch_enroll_1();
  printf("Release Finger (1) \n");
  wait_for_finger_release();
  printf("Press Finger (2) \n");
  wait_for_finger();
  touch_capture_finger();
  touch_enroll_2();
  printf("Release Finger (2) \n");
  wait_for_finger_release();
  printf("Press Finger (3) \n");
  wait_for_finger();
  touch_capture_finger();
  touch_enroll_3(enrollment_mode);
  printf("Release Finger (3) \n");
  wait_for_finger_release();
  printf("ENDING ENROLLMENT \n");
  //Verification process
  gt511c3_is_id_enrolled(id);
}

int gt511c3_is_id_enrolled(int id) {
  if (touch_check_enrolled(id) == TOUCH_ENROLLED) {
    printf("Success enrolled fingerprint (id %d) \n", id);
    return GT511C3_IS_ENROLLED;
  }
  printf("Error not enrolled fingerprint (id %d) \n", id);
  return GT511C3_NOT_ENROLLED;
}

int gt511c3_identify_finger(){
  printf("READY TO IDENTIFY \n");
  printf("Press Finger \n");
  wait_for_finger();
  touch_capture_finger();
  if(touch_identify() == TOUCH_IDENTIFICATION_CORRECT){
    printf("CORRECTLY INDETIFIED \n");
    printf("Release Finger \n");
    return GT511C3_IDENTIFICATION_OK;
  }
  printf("Release Finger \n");
  return GT511C3_IDENTIFICATION_ERROR;
}