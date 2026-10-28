/* app_types.h - types used by more than one module */
#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define ARM_N_JOINTS 6

/* joint index = servo index = position in joint_vec_t */
typedef enum {
  J1_BASE = 0,
  J2_SHOULDER,
  J3_ELBOW,
  J4_WRIST_ROLL,
  J5_WRIST_PITCH,
  J6_GRIPPER
} joint_id_t;

/* joint angles in degrees, in the servo's own 0..180 frame */
typedef struct {
  float deg[ARM_N_JOINTS];
} joint_vec_t;

/* tool target in the robot frame, mm (same frame the pi uses) */
typedef struct {
  float x_mm;
  float y_mm;
  float z_mm;
  float pitch_deg;   /* tool pitch */
} cart_pose_t;

typedef enum {
  SPEED_FINE = 0,
  SPEED_FAST
} speed_level_t;

typedef enum {
  MODE_VOICE = 0,
  MODE_CARTESIAN,
  MODE_JOINT_GROUP,
  MODE_SINGLE_JOINT,
  MODE_COUNT
} arm_mode_t;

/* objects the pi can find; names match the pi protocol */
typedef enum {
  OBJ_BLUE_BOX = 0,
  OBJ_GREEN_BOX,
  OBJ_RED_BOX,
  OBJ_BLACK_BOX,
  OBJ_BOX_COUNT,               /* boxes only (voice words, colours) */
  OBJ_HAND = OBJ_BOX_COUNT,    /* palm centre from the pi, may move */
  OBJ_STATION,                 /* free drop spot on the station, from the pi */
  OBJ_COUNT,
  OBJ_NONE = 0xFF
} object_id_t;

#endif /* APP_TYPES_H */
