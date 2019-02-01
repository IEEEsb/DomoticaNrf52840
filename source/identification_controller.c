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

void identification_enroll_user(int id) {

  int enrollment_mode;
  //Enrollment mode
  if (id == -1) {
    enrollment_mode = TOUCH_SAVEMODE_DISABLE;
  } else {
    enrollment_mode = TOUCH_SAVEMODE_ENABLE;
  }
  //Enrollment process
  touch_enroll_start(id);
  printf("Press Finger (1)");
  wait_for_finger();
  touch_capture_finger();
  touch_enroll_1();
  printf("Release Finger (1)");
  wait_for_finger_release();
  printf("Press Finger (2)");
  wait_for_finger();
  touch_capture_finger();
  touch_enroll_2();
  printf("Release Finger (2)");
  wait_for_finger_release();
  printf("Press Finger (3)");
  wait_for_finger();
  touch_capture_finger();
  touch_enroll_3(enrollment_mode);
  printf("Release Finger (3)");
  wait_for_finger_release();

  //Verification process
  identification_check(id);
}

void identification_check(int id) {
  if (touch_check_enrolled(id) == TOUCH_ENROLLED) {
    printf("Success enrolled fingerprint (id %d)", id);
    return;
  }
  printf("Error not enrolled fingerprint (id %d)", id);
}