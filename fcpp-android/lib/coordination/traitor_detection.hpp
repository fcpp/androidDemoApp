// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file traitor_detection.hpp
 * @brief Experiment computing whether there is a traitor during an evacuation.
 */

#ifndef FCPP_COORDINATION_TRAITOR_DETECTION_H_
#define FCPP_COORDINATION_TRAITOR_DETECTION_H_

#include "lib/beautify.hpp"
#include "lib/coordination.hpp"
#include "lib/coordination/slcs.hpp"
#include "lib/coordination/past_ctl.hpp"


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
}

//! @brief Export list for each experiment.
template <typename> struct experiment_t;
//! @brief Storage list for each experiment.
template <typename> struct experiment_s;


//! @brief Computes whether there is a node with only one connected neighbour at a given time.
FUN void experiment(ARGS, traitor_detection) { CODE
    using namespace tags;
    using namespace logic;
    bool& eg = node.storage(evacuation_group{});
    bool& ed = node.storage(evacuation_done{}) = node.current_time() > node.storage(evacuation_time{});
    bool& hg = node.storage(homogeneous_group{}) = (eg <= G(CALL, eg)) and ((not eg) <= G(CALL, not eg));
    node.storage(traitor_free{}) = S(CALL, hg, ed);
}
//! @brief Export list for the vulnerability detection experiment.
template <>
struct experiment_t<traitor_detection> : export_list<
    past_ctl_t, slcs_t
> {};
//! @brief Storage list for the vulnerability detection experiment.
template <>
struct experiment_s<traitor_detection> : storage_list<
    past_ctl_s, slcs_s,
    tags::evacuation_time,    short,
    tags::evacuation_group,   bool,
    tags::evacuation_done,    bool,
    tags::homogeneous_group,  bool,
    tags::traitor_free,       bool
> {};

} // namespace coordination

} // namespace fcpp

#endif  // FCPP_COORDINATION_TRAITOR_DETECTION_H_
