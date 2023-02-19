// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file friend_finding.hpp
 * @brief Experiment allowing to search for friends.
 */

#ifndef FCPP_COORDINATION_FRIEND_FINDING_H_
#define FCPP_COORDINATION_FRIEND_FINDING_H_

#include "lib/coordination/experiment_helper.hpp"
#include "lib/coordination/tracker.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Tag for the friend finding experiment.
struct friend_finding {};


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {

//! @brief Tags used in the node storage.
namespace tags {
}


//! @brief Simulation logic of the friend finding experiment.
FUN void experiment_simulation(ARGS, friend_finding, common::bool_pack<true>) { CODE
    using namespace tags;
    node.storage(node_size{}) = 1;
    node.storage(node_shape{}) = shape::sphere;
    node.storage(node_color{}) = color(BLACK);
}
//! @brief Export list for the simulation logic of the friend finding experiment.
template <>
struct experiment_simulation_t<friend_finding, true> : export_list<
> {};
//! @brief Storage list for the simulation logic of the friend finding experiment.
template <>
struct experiment_simulation_s<friend_finding, true> : storage_list<
    experiment_simulation_base_s
> {};


//! @brief Allows users to search for friends.
GEN(S) void experiment(ARGS, friend_finding, S) { CODE
    using namespace tags;
    experiment_simulation(CALL, friend_finding{}, S{});
}
//! @brief Export list for the friend finding experiment.
template <bool simulation>
struct experiment_t<friend_finding, simulation> : export_list<
    experiment_simulation_t<friend_finding, simulation>
> {};
//! @brief Storage list for the friend finding experiment.
template <bool simulation>
struct experiment_s<friend_finding, simulation> : storage_list<
    experiment_simulation_s<friend_finding, simulation>
> {};
//! @brief Aggregator list for the friend finding experiment.
template <>
struct experiment_a<friend_finding> : storage_list<
> {};
//! @brief Plot list for the friend finding experiment.
template <typename A>
struct experiment_p<friend_finding,A> : public plot::split<plot::time, plot::value<plot::time>> {};

} // namespace coordination

} // namespace fcpp

#endif  // FCPP_COORDINATION_FRIEND_FINDING_H_
