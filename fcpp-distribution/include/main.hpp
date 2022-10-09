// Copyright © 2022 Giorgio Audrito. All Rights Reserved.

#include <jni.h>
#include <stdint.h>

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Starts FCPP, returning a pointer to the storage.
extern void start(JNIEnv *env, jclass ap, int uid, int diam, int per, int end);

//! @brief Stops FCPP.
extern void stop();

//! @cond INTERNAL
#define __MAP0__(M)
#define __MAP1__(M,A)                               M(A)
#define __MAP2__(M,A,B)                             __MAP1__(M,A) __MAP1__(M,B)
#define __MAP3__(M,A,B,C)                           __MAP1__(M,A) __MAP2__(M,B,C)
#define __MAP4__(M,A,B,C,D)                         __MAP1__(M,A) __MAP3__(M,B,C,D)
#define __MAP5__(M,A,B,C,D,E)                       __MAP1__(M,A) __MAP4__(M,B,C,D,E)
#define __MAP6__(M,A,B,C,D,E,F)                     __MAP1__(M,A) __MAP5__(M,B,C,D,E,F)
#define __MAP7__(M,A,B,C,D,E,F,G)                   __MAP1__(M,A) __MAP6__(M,B,C,D,E,F,G)
#define __MAP8__(M,A,B,C,D,E,F,G,H)                 __MAP1__(M,A) __MAP7__(M,B,C,D,E,F,G,H)
#define __MAP9__(M,A,B,C,D,E,F,G,H,I)               __MAP1__(M,A) __MAP8__(M,B,C,D,E,F,G,H,I)
#define __MAPA__(M,A,B,C,D,E,F,G,H,I,J)             __MAP1__(M,A) __MAP9__(M,B,C,D,E,F,G,H,I,J)
#define __MAPB__(M,A,B,C,D,E,F,G,H,I,J,K)           __MAP1__(M,A) __MAPA__(M,B,C,D,E,F,G,H,I,J,K)
#define __MAPC__(M,A,B,C,D,E,F,G,H,I,J,K,L)         __MAP1__(M,A) __MAPB__(M,B,C,D,E,F,G,H,I,J,K,L)
#define __MAPD__(M,A,B,C,D,E,F,G,H,I,J,K,L,N)       __MAP1__(M,A) __MAPC__(M,B,C,D,E,F,G,H,I,J,K,L,N)
#define __MAPE__(M,A,B,C,D,E,F,G,H,I,J,K,L,N,O)     __MAP1__(M,A) __MAPD__(M,B,C,D,E,F,G,H,I,J,K,L,N,O)
#define __MAPF__(M,A,B,C,D,E,F,G,H,I,J,K,L,N,O,P)   __MAP1__(M,A) __MAPE__(M,B,C,D,E,F,G,H,I,J,K,L,N,O,P)
#define __MAPX__(M,A,B,C,D,E,F,G,H,I,J,K,L,N,O,P,X,...) X
//! @endcond

//! @brief Maps a macro to a variable number of arguments (up to 9), space separating the calls.
#define MACRO_MAP(...)                         __MAPX__(__VA_ARGS__, __MAPF__, __MAPE__,    \
                                                        __MAPD__, __MAPC__, __MAPB__,       \
                                                        __MAPA__, __MAP9__, __MAP8__,       \
                                                        __MAP7__, __MAP6__, __MAP5__,       \
                                                        __MAP4__, __MAP3__, __MAP2__,       \
                                                        __MAP1__, __MAP0__)(__VA_ARGS__)

#define DECL_GETSET(f)       \
    auto get_##f();         \
    template <typename T>   \
    void set_##f(T);

extern uint8_t get_max_msg_size();
extern uint16_t get_round_count();
extern bool get_im_weak();
extern bool get_some_weak();
extern uint8_t get_degree();

extern void set_round_count(int);

} // namespace fcpp
