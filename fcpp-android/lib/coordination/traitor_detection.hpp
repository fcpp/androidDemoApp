// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file traitor_detection.hpp
 * @brief Experiment computing whether there is a traitor during an evacuation.
 */

#ifndef FCPP_COORDINATION_TRAITOR_DETECTION_H_
#define FCPP_COORDINATION_TRAITOR_DETECTION_H_

#include "lib/coordination/experiment_helper.hpp"
#include "lib/coordination/slcs.hpp"
#include "lib/coordination/past_ctl.hpp"
#include "lib/coordination/tracker.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Tag for the vulnerability detection experiment.
struct traitor_detection {};


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {

//! @brief Tags used in the node storage.
namespace tags {
    //! @brief The expected time of an evacuation.
    struct evacuation_time {};
    //! @brief To which evacuation group an individual belongs.
    struct evacuation_group {};
    //! @brief Whether the evacuation should be done already.
    struct evacuation_done {};
    //! @brief Whether the individual is currently in an homogenous group.
    struct homogeneous_group {};
    //! @brief Whether there is no traitor in the group.
    struct traitor_free {};
    //! @brief Whether I am a traitor.
    struct is_traitor {};
}


//! @brief Simulation logic of the traitor detection experiment.
FUN void experiment_simulation(ARGS, traitor_detection, common::bool_pack<true>) { CODE
    using namespace tags;
    node.storage(evacuation_time{}) = 60; // 1 minute of evacuation time
    bool& it = node.storage(is_traitor{}) = node.uid == 0;
    bool& eg = node.storage(evacuation_group{}) = constant(CALL, node.next_int(1));
    bool& hg = node.storage(homogeneous_group{});
    bool& tf = node.storage(traitor_free{});
    vec<2> low = {0, 0};
    vec<2> hi = {hi_x, hi_y};
    vec<2> room_low = 0.3 * hi;
    vec<2> room_hi  = 0.7 * hi;
    if (counter(CALL) == 1)
        node.position() = random_rectangle_target(CALL, room_low, room_hi);
    vec<2> target = random_rectangle_target(CALL, eg^it ? low : room_hi, eg^it ? room_low : hi);
    follow_target(CALL, constant(CALL, target), 1.4, node.storage(round_period{}));
    node.storage(node_size{}) = tf ? 1.4 : 2;
    node.storage(node_shape{}) = eg ? shape::tetrahedron : shape::sphere;
    node.storage(node_color{}) = it ? color(BLACK) : tf ? hg ? color(GREEN) : color(GOLD) : color(RED);
}
//! @brief Export list for the simulation logic of the traitor detection experiment.
template <>
struct experiment_simulation_t<traitor_detection, true> : export_list<
    constant_t<vec<2>>, constant_t<int>, counter_t<>
> {};
//! @brief Storage list for the simulation logic of the traitor detection experiment.
template <>
struct experiment_simulation_s<traitor_detection, true> : storage_list<
    experiment_simulation_base_s,
    tags::is_traitor,   bool
> {};


//! @brief Computes whether there is a node with only one connected neighbour at a given time.
GEN(S) void experiment(ARGS, traitor_detection, S) { CODE
    using namespace tags;
    using namespace logic;
    bool& eg = node.storage(evacuation_group{});
    bool& ed = node.storage(evacuation_done{}) = node.current_time() > node.storage(evacuation_time{});
    bool& hg = node.storage(homogeneous_group{}) = (eg <= G(CALL, eg)) & ((not eg) <= G(CALL, not eg));
    node.storage(traitor_free{}) = AH(CALL, ed <= hg);
    experiment_simulation(CALL, traitor_detection{}, S{});
}
//! @brief Export list for the traitor detection experiment.
template <bool simulation>
struct experiment_t<traitor_detection, simulation> : export_list<
    experiment_simulation_t<traitor_detection, simulation>,
    past_ctl_t, slcs_t
> {};
//! @brief Storage list for the traitor detection experiment.
template <bool simulation>
struct experiment_s<traitor_detection, simulation> : storage_list<
    experiment_simulation_s<traitor_detection, simulation>,
    past_ctl_s, slcs_s,
    tags::evacuation_time,    short,
    tags::evacuation_group,   bool,
    tags::evacuation_done,    bool,
    tags::homogeneous_group,  bool,
    tags::traitor_free,       bool
> {};
//! @brief Aggregator list for the traitor detection experiment.
template <>
struct experiment_a<traitor_detection> : storage_list<
    tags::evacuation_group,     aggregator::mean<double>,
    tags::evacuation_done,      aggregator::mean<double>,
    tags::homogeneous_group,    aggregator::mean<double>,
    tags::traitor_free,         aggregator::mean<double>
> {};
//! @brief Plot list for the traitor detection experiment.
template <typename A>
struct experiment_p<traitor_detection,A> : public plot::split<plot::time, plot::values<A, common::type_sequence<>, tags::evacuation_done, tags::homogeneous_group, tags::traitor_free>> {};

} // namespace coordination

} // namespace fcpp

#endif  // FCPP_COORDINATION_TRAITOR_DETECTION_H_
