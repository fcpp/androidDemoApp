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
extern char* get_nbr_lags();

} // namespace fcpp
