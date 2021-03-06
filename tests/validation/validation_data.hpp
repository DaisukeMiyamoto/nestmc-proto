#pragma once

#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <json/json.hpp>

#include <simple_sampler.hpp>
#include <util/path.hpp>

namespace nest {
namespace mc {

/*
 * Class manages input (loading and parsing) of JSON
 * reference traces, and output of saved traces from
 * the validation tests.
 *
 * Global object has paths initalised by the test
 * main() and will write all saved traces to a single
 * output JSON file, if specified.
 */

class trace_io {
public:
    // Try to find the data directory on construction.

    trace_io() {
        datadir_ = find_datadir();
    }

    void clear_traces() {
        jtraces_ = nlohmann::json::array();
    }

    void write_traces() {
        if (out_) {
            out_ << jtraces_;
            out_.close();
        }
    }

    void save_trace(const std::string& label, const trace_data& data, const nlohmann::json& meta);
    void save_trace(const std::string& abscissa, const std::string& label, const trace_data& data, const nlohmann::json& meta);
    std::map<std::string, trace_data> load_traces(const util::path& name);

    // common flags, options set by driver

    void set_verbose(bool v) { verbose_flag_ = v; }
    bool verbose() const { return verbose_flag_; }

    void set_max_ncomp(int ncomp) { max_ncomp_ = ncomp; }
    int max_ncomp() const { return max_ncomp_; }

    void set_min_dt(float dt) { min_dt_ = dt; }
    float min_dt() const { return min_dt_; }

    void set_datadir(const util::path& dir) { datadir_ = dir; }

    void set_output(const util::path& file) {
        out_.open(file);
        if (!out_) {
            throw std::runtime_error("unable to open file for writing");
        }
    }

    // Write traces on exit.

    ~trace_io() {
        if (out_) {
            write_traces();
        }
    }

private:
    util::path datadir_;
    std::ofstream out_;
    nlohmann::json jtraces_ = nlohmann::json::array();
    bool verbose_flag_ = false;
    int max_ncomp_ = 100;
    float min_dt_ = 0.001f;

    // Returns value of NMC_DATADIR environment variable if set,
    // otherwise make a 'best-effort' search for the data directory,
    // starting with NMC_DATADIR preprocessor define if defined and
    // if the directory exists, or else try './validation/data'
    // and '../validation/data'.
    static util::path find_datadir();
};

extern trace_io g_trace_io;

} // namespace mc
} // namespace nest
