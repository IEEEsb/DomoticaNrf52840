#ifndef IDENTIFICATION_CONTROLLER_H_
#define IDENTIFICATION_CONTROLLER_H_

void identification_enroll_user(int id);
void identification_check();
void gt511c3_reset_data(void);
void gt511c3_initialize(void);
void gt511c3_deinitialize(void);

#endif /* IDENTIFICATION_H_ */