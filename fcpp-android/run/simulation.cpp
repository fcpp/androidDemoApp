// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file simulation.cpp
 * @brief Runs a single execution of the spreading collection case study with a graphical user interface.
 */

#include <fstream>

#include "lib/configuration.hpp"

using namespace fcpp;

//! @brief Runs a sequence of experiments (empty case).
void run_experiment(common::type_sequence<>) {}

//! @brief Runs a sequence of experiments (general case).
template <typename E, typename... Es>
void run_experiment(common::type_sequence<E, Es...>) {
    {
        // The experiment name.
        std::string name = common::details::strip_namespaces(common::type_name<E>());
        // The plotter object.
        coordination::main_p<E> p;
        // Initialisation values.
        auto init_v = common::make_tagged_tuple<option::diameter, option::retain_time, option::round_period, option::name, option::plotter>(DIAMETER, RETAIN_TIME, ROUND_PERIOD, name, &p);
        // The network object type (interactive simulator with given options).
        using net_t = typename component::interactive_simulator<option::simulation<E>>::net;
        // Construct the network object.
        net_t network{init_v};
        // Run the simulation until exit.
        network.run();
        // Write plots.
        std::ofstream fout("../plot/" + name + ".asy");
        fout << plot::file(name, p.build());
    }
    // Pass to next experiment.
    run_experiment(common::type_sequence<Es...>{});
}

int main() {
    run_experiment(experiments{});
    return 0;
}
