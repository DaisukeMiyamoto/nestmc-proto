#include <json/json.hpp>

#include <common_types.hpp>
#include <cell.hpp>
#include <fvm_multicell.hpp>
#include <model.hpp>
#include <recipe.hpp>
#include <simple_sampler.hpp>
#include <util/rangeutil.hpp>

#include "../test_common_cells.hpp"
#include "convergence_test.hpp"
#include "trace_analysis.hpp"
#include "validation_data.hpp"

void validate_soma(nest::mc::backend_policy backend) {
    using namespace nest::mc;

    cell c = make_cell_soma_only();
    add_common_voltage_probes(c);
    domain_decomposition decomp(singleton_recipe{c}, {1u, backend});
    model m(singleton_recipe{c}, decomp);

    float sample_dt = .025f;
    sampler_info samplers[] = {{"soma.mid", {0u, 0u}, simple_sampler(sample_dt)}};

    nlohmann::json meta = {
        {"name", "membrane voltage"},
        {"model", "soma"},
        {"sim", "nestmc"},
        {"units", "mV"},
        {"backend_policy", to_string(backend)}
    };

    convergence_test_runner<float> runner("dt", samplers, meta);
    runner.load_reference_data("numeric_soma.json");

    float t_end = 100.f;

    // use dt = 0.05, 0.02, 0.01, 0.005, 0.002,  ...
    double max_oo_dt = std::round(1.0/g_trace_io.min_dt());
    for (double base = 100; ; base *= 10) {
        for (double multiple: {5., 2., 1.}) {
            double oo_dt = base/multiple;
            if (oo_dt>max_oo_dt) goto end;

            m.reset();
            float dt = float(1./oo_dt);
            runner.run(m, dt, t_end, dt, {});
        }
    }
end:

    runner.report();
    runner.assert_all_convergence();
}
