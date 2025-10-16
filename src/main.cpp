#include <numeric>
#define MAXN (17*4)
#include "EdgeColoredUndirectedGraph.h"
#include "GraphUtils.h"

#include <string>
#include <unordered_set>

using namespace Ram;

EdgeColoredUndirectedGraph make_T1() noexcept
{
	return load_adj("graphs/T1.adj", 16, 3);
}

EdgeColoredUndirectedGraph make_T2() noexcept
{
	return load_adj("graphs/T2.adj", 16, 3);
}

std::vector<std::vector<int>> permute(int n, int k) noexcept
{
	std::vector<std::vector<int>> res;
	std::vector<int> items(n);
	std::iota(items.begin(), items.end(), 0);

	std::vector<int> combo;
	std::vector<bool> select(n, false);
	std::fill(select.begin(), select.begin() + k, true);

	do
	{
		combo.clear();
		for (auto i = 0; i < n; ++i)
		{
			if (select[i]) combo.emplace_back(items[i]);
		}

		// generate all permutations of current combo
		std::sort(combo.begin(), combo.end());
		do
		{
			res.emplace_back(combo);
		} while (std::next_permutation(combo.begin(), combo.end()));
	} while (std::prev_permutation(select.begin(), select.end()));

	return res;
}

std::vector<std::vector<int>> choose(int n, int k) noexcept
{
	std::vector<std::vector<int>> res;
	std::vector<int> items(n);
	std::iota(items.begin(), items.end(), 0);

	std::vector<int> combo;
	std::vector<bool> select(n, false);
	std::fill(select.begin(), select.begin() + k, true);

	do
	{
		combo.clear();
		for (auto i = 0; i < n; ++i)
		{
			if (select[i]) combo.emplace_back(items[i]);
		}
		res.emplace_back(combo);
	} while (std::prev_permutation(select.begin(), select.end()));

	return res;
}
void prop1() noexcept
{
	std::vector<EdgeColoredUndirectedGraph> ts = { make_T1(), make_T2() };

	std::vector<EdgeColoredUndirectedGraph> graphs;
	std::vector<std::unordered_set<std::string>> canons(17);
	for (const auto& t : ts)
	{
		// Create t with 17 vertices, 4 colors
		EdgeColoredUndirectedGraph t_prime(17, 4);
		for (auto i = 0; i < 16; ++i)
			for (auto j = i+1; j < 16; ++j)
				t_prime.setEdge(i, j, t.getEdge(i, j));

		// Create marked subset coloring for all possible subsets
		for (auto k = 1; k <= 16; ++k)
		{
			for (const auto& combo : choose(16, k))
			{
				// Mark the subset
				auto g = t_prime;
				for (auto v_marked : combo)
				{
					// Create color 4 edge to 17th vertex
					g.setEdge(16, v_marked, 4);
				}

				// Canonize
				auto canon = canonize(g);
				if (!canons[k].contains(canon))
				{
					canons[k].insert(canon);
					graphs.push_back(g);
				}
			}

			std::printf("Finished k=%d\n", static_cast<int>(k));
		}
	}

	for (auto s = 1; s <= 16; ++s)
	{
		std::printf(
			"Found %d colorings for S=%d.\n",
			static_cast<int>(canons[s].size()),
			static_cast<int>(s)
		);
	}

	writeGraphsToFile("graphs/62/upsilon1.adj", graphs);
}

int main(int argc, char** argv)
{
	prop1();
	
	return 0;
}

