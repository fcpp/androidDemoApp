// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file experiment_helper.hpp
 * @brief Experiment computing whether there is a traitor during an evacuation.
 */

#ifndef FCPP_COORDINATION_EXPERIMENT_HELPER_H_
#define FCPP_COORDINATION_EXPERIMENT_HELPER_H_

#include "lib/beautify.hpp"
#include "lib/coordination.hpp"
#include "lib/common/plot.hpp"
#include "lib/data.hpp"
#include "lib/option/aggregator.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Maximum X coordinate of the simulation area.
constexpr int hi_x = 120;
//! @brief Maximum Y coordinate of the simulation area.
constexpr int hi_y = 80;

//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {

//! @brief Tags used in the node storage.
namespace tags {
    //! @brief Color of the current node.
    struct node_color {};
    //! @brief Size of the current node.
    struct node_size {};
    //! @brief Shape of the current node.
    struct node_shape {};
}

//! @brief Export list for each experiment.
template <typename, bool> struct experiment_t;
//! @brief Storage list for each experiment.
template <typename, bool> struct experiment_s;
//! @brief Aggregator list for each experiment.
template <typename> struct experiment_a;
//! @brief Plot list for each experiment.
template <typename,typename> struct experiment_p;

//! @brief Export list for each experiment simulation.
template <typename, bool> struct experiment_simulation_t;
//! @brief Storage list for each experiment simulation.
template <typename, bool> struct experiment_simulation_s;
//! @brief Storage list for all experiment simulations.
using experiment_simulation_base_s = storage_list<
    tags::node_color,     color,
    tags::node_size,      double,
    tags::node_shape,     shape
>;

//! @brief Empty simulation logic.
GEN(T) void experiment_simulation(ARGS, T, common::bool_pack<false>) {}
//! @brief Export list for the empty simulation logic.
template <typename T>
struct experiment_simulation_t<T, false> : export_list<> {};
//! @brief Storage list for the empty simulation logic.
template <typename T>
struct experiment_simulation_s<T, false> : storage_list<> {};

} // namespace coordination

} // namespace fcpp

#endif  // FCPP_COORDINATION_EXPERIMENT_HELPER_H_
