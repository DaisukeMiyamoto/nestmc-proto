#include "../gtest.h"

#include <domain_decomposition.hpp>
#include <backends.hpp>

using namespace nest::mc;

namespace {

// dummy recipe type for testing
class test_recipe: public recipe {
public:
    test_recipe(cell_size_type s): size_(s)
    {}

    cell_size_type num_cells() const override {
        return size_;
    }

    util::unique_any get_cell_description(cell_gid_type) const override {
        return {};
    }
    cell_kind get_cell_kind(cell_gid_type) const override {
        return cell_kind::cable1d_neuron;
    }

    cell_count_info get_cell_count_info(cell_gid_type) const override {
        return {0, 0, 0};
    }
    std::vector<cell_connection> connections_on(cell_gid_type) const override {
        return {};
    }

private:
    cell_size_type size_;
};

}

TEST(domain_decomposition, one_cell_groups)
{
    group_rules rules{1, backend_policy::use_multicore};

    unsigned num_cells = 10;
    domain_decomposition decomp(test_recipe(num_cells), rules);

    EXPECT_EQ(0u, decomp.cell_begin());
    EXPECT_EQ(num_cells, decomp.cell_end());

    EXPECT_EQ(num_cells, decomp.num_local_cells());
    EXPECT_EQ(num_cells, decomp.num_global_cells());
    EXPECT_EQ(num_cells, decomp.num_local_groups());

    // cell group indexes are monotonically increasing
    for (unsigned i=0u; i<num_cells; ++i) {
        auto g = decomp.get_group(i);
        EXPECT_LT(g.begin, g.end);
        EXPECT_EQ(g.end-g.begin, 1u);
    }

    // check that local gid are identified as local
    for (auto i=0u; i<num_cells; ++i) {
        EXPECT_TRUE(decomp.is_local_gid(i));
    }
    EXPECT_FALSE(decomp.is_local_gid(num_cells));
}

TEST(domain_decomposition, multi_cell_groups)
{
    unsigned num_cells = 10;

    // test group sizes from 1 up to 1 more than the total number of cells
    for (unsigned group_size=1u; group_size<=num_cells+1; ++group_size) {
        group_rules rules{group_size, backend_policy::use_multicore};
        domain_decomposition decomp(test_recipe(num_cells), rules);

        EXPECT_EQ(0u, decomp.cell_begin());
        EXPECT_EQ(num_cells, decomp.cell_end());

        EXPECT_EQ(num_cells, decomp.num_local_cells());
        EXPECT_EQ(num_cells, decomp.num_global_cells());

        unsigned num_groups = decomp.num_local_groups();

        // check that cell group indexes are monotonically increasing
        unsigned total_cells = 0;
        std::vector<cell_size_type> bounds{decomp.cell_begin()};
        for (auto i=0u; i<num_groups; ++i) {
            auto g = decomp.get_group(i);
            auto size = g.end-g.begin;

            // assert that size of the group:
            //   is nonzero
            EXPECT_LT(g.begin, g.end);
            //   is no larger than group_size
            EXPECT_TRUE(size<=group_size);

            // Check that the start of this group matches the end of
            // the preceding group.
            EXPECT_EQ(bounds.back(), g.begin);
            bounds.push_back(g.end);

            total_cells += size;
        }
        // check that the sum of the group sizes euqals the total number of cells
        EXPECT_EQ(total_cells, num_cells);

        // check that local gid are identified as local
        for (auto i=0u; i<num_cells; ++i) {
            EXPECT_TRUE(decomp.is_local_gid(i));
            auto group_id = decomp.local_group_from_gid(i);
            EXPECT_TRUE(group_id);
            EXPECT_LT(*group_id, num_groups);
        }
        EXPECT_FALSE(decomp.is_local_gid(num_cells));
        EXPECT_FALSE(decomp.local_group_from_gid(num_cells));
    }
}
