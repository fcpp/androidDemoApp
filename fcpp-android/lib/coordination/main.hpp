// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file main.hpp
 * @brief Main aggregate function, calling a given experiment.
 */

#ifndef FCPP_COORDINATION_MAIN_H_
#define FCPP_COORDINATION_MAIN_H_

#include <cassert>

#include "lib/component/base.hpp"
#include "lib/coordination/tracker.hpp"
#include "lib/coordination/vulnerability_detection.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {

//! @brief Tags used in the node storage.
namespace tags {
    //! @brief GPS latitude.
    struct position_latitude {};
    //! @brief GPS longitude.
    struct position_longitude {};
    //! @brief GPS accuracy.
    struct position_accuracy {};
}

//! @brief Templated main aggregate function.
template <typename tag>
struct main {
    template <typename node_t>
    void operator()(node_t& node, times_t) {
        using namespace tags;
        experiment(CALL, tag{});
        tracker(CALL);
    }
};
//! @brief Export list for the main aggregate function.
template <typename tag>
FUN_EXPORT main_t = export_list<experiment_t<tag>, tracker_t>;
//! @brief Storage list for the main aggregate function.
template <typename tag>
FUN_EXPORT main_s = storage_list<experiment_s<tag>, tracker_s,
    component::tags::uid,     device_t,
    tags::position_latitude,  double,
    tags::position_longitude, double,
    tags::position_accuracy,  float
>;

} // namespace coordination

} // namespace fcpp

#endif  // FCPP_COORDINATION_MAIN_H_