#ifndef IDENTIFICATION_CONTROLLER_H_
#define IDENTIFICATION_CONTROLLER_H_

enum GT511C3_ENROLLED{
  GT511C3_IS_ENROLLED = 0,
  GT511C3_NOT_ENROLLED
};

enum GT511C3_IDENTIFICATION{
  GT511C3_IDENTIFICATION_OK = 0,
  GT511C3_IDENTIFICATION_ERROR
};


void gt511c3_enroll_user(int id);
int gt511c3_identify_finger();
void gt511c3_reset_data(void);
void gt511c3_initialize(void);
void gt511c3_deinitialize(void);
int gt511c3_is_id_enrolled(int id);
#endif /* IDENTIFICATION_H_ */