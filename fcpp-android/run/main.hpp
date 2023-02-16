// Copyright © 2022 Giorgio Audrito. All Rights Reserved.

#include <jni.h>
#include <stdint.h>

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Starts FCPP.
extern void start(JNIEnv *env, jclass apclass, int uid, char const* experiment);

//! @brief Stops FCPP.
extern void stop();

extern char* get_storage();

extern char* get_string(char const*);
extern double get_double(char const*);
extern int get_int(char const*);
extern int get_bool(char const*);

extern void set_string(char const*, char const*);
extern void set_double(char const*, double);
extern void set_int(char const*, int);
extern void set_bool(char const*, bool);

// DEPRECATED
extern void start(JNIEnv *env, jclass apclass, int uid);
extern char* get_nbr_lags();
extern uint16_t get_round_count();
extern uint8_t get_max_msg_size();
extern bool get_im_weak();
extern bool get_some_weak();
extern uint8_t get_min_uid();
extern float get_global_clock();
extern uint8_t get_hop_dist();
extern uint8_t get_degree();

extern uint8_t get_diameter();
extern void set_diameter(uint8_t);

extern float get_retain_time();
extern void set_retain_time(float);

extern float get_round_period();
extern void set_round_period(float);

extern void set_position_latlong(double, double);
extern void set_position_accuracy(float);
} // namespace fcpp
