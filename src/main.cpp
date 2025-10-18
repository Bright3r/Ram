#include <chrono>
#include <functional>
#include <numeric>
#include <string>
#include <unordered_set>

#define MAXN (17*4)
#include "EdgeColoredUndirectedGraph.h"
#include "GraphUtils.h"


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

	// Create marked subset coloring for all possible subsets
	for (auto k = 1; k <= 16; ++k)
	{
		for (const auto& combo : choose(16, k))
		{
			for (const auto& t : ts)
			{
				// Create t with 17 vertices, 4 colors
				EdgeColoredUndirectedGraph t_prime(17, 4);
				for (auto i = 0; i < 16; ++i)
					for (auto j = i+1; j < 16; ++j)
						t_prime.setEdge(i, j, t.getEdge(i, j));

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
		}

		std::printf("Finished k=%d\n", static_cast<int>(k));
	}

	for (auto k = 1; k <= 16; ++k)
	{
		std::printf(
			"Found %d colorings for S=%d.\n",
			static_cast<int>(canons[k].size()),
			static_cast<int>(k)
		);
	}

	writeGraphsToFile("graphs/62/upsilon1.adj", graphs);
}


void prop2() noexcept
{
	auto upsilon1 = load_bulk("graphs/62/upsilon1.adj", 17, 4);
	std::vector<EdgeColoredUndirectedGraph> ts = { make_T1(), make_T2() };

	std::vector<std::unordered_set<std::string>> canons(17);
	std::vector<EdgeColoredUndirectedGraph> graphs;
	int progress = 0;
	for (const auto& g : upsilon1)
	{
		// Get marked vertices of the graph
		std::unordered_set<size_t> marked;
		for (auto v = 0; v < 16; ++v)
		{
			if (g.getEdge(v, 16) == 4)
			{
				marked.insert(v);
			}
		}
		std::vector<size_t> marked_v(marked.begin(), marked.end());
		auto k = marked_v.size();

		for (const auto& t : ts)
		{
			// Base graph for overlapping g and t
			auto num_vertices = 32 - k;
			EdgeColoredUndirectedGraph overlap_base(num_vertices, 4);

			// Copy t's colors onto base graph
			for (auto i = 0; i < 16; ++i)
				for (auto j = i+1; j < 16; ++j)
					overlap_base.setEdge(i, j, t.getEdge(i, j));

			// Map unmarked vertices of g onto vertices of the base graph
			std::vector<size_t> g_map(16, -1);
			int v_map = 16;
			for (auto v = 0; v < 16; ++v)
			{
				if (!marked.contains(v))
				{
					g_map[v] = v_map;
					++v_map;
				}
			}

			// Copy g's unmarked colors onto base graph
			for (auto i = 0; i < 16; ++i)
			{
				if (marked.contains(i)) continue;
				for (auto j = i+1; j < 16; ++j)
				{
					if (marked.contains(j)) continue;

					overlap_base.setEdge(g_map[i], g_map[j], g.getEdge(i, j));
				}
			}


			// Backtrack over all possible mappings of marked vertices onto vertices of t
			std::vector<int> map_g_to_t(16, -1);
			std::vector<bool> used_t(16, false);
			std::function<void(int)> dfs = [&](int depth)
			{
				// Full graph constructed
				if (depth == k)
				{
					auto overlap = overlap_base;

					// Copy marked vertices of g into overlap
					for (auto vm_g : marked_v)
					{
						// Find corresponding vertex in t
						auto vm_t = map_g_to_t[vm_g];

						// Copy the outgoing edges from marked vertices in g
						// as outgoing edges from vertices of t in overlap
						for (auto v = 0; v < 16; ++v)
						{
							auto v_g = g_map[v];
							if (v_g == -1) continue;

							overlap.setEdge(vm_t, v_g, g.getEdge(vm_g, v));
						}
					}

					// Canonize
					auto canon = canonize(overlap);
					if (!canons[k].contains(canon))
					{
						canons[k].insert(canon);
						graphs.push_back(overlap);
					}
					return;
				}

				// Find a vertex in t that we can map to the marked vertex in g
				auto v_g = marked_v[depth];
				for (auto v_t = 0; v_t < 16; ++v_t)
				{
					if (used_t[v_t]) continue;

					// Check that edges from previously mapped vertices
					// to new marked vertex agrees with colors in g and mapped t
					bool ok = true;
					for (auto prev = 0; prev < depth; ++prev)
					{
						auto v_g_prev = marked_v[prev];
						auto v_t_prev = map_g_to_t[v_g_prev];
						if (v_t_prev == -1) continue;
						if (g.getEdge(v_g, v_g_prev) != t.getEdge(v_t, v_t_prev))
						{
							ok = false;
							break;
						}
					}

					if (!ok) continue;

					// Found valid mapping
					map_g_to_t[v_g] = v_t;
					used_t[v_t] = true;

					// Recurse then continue
					dfs(depth + 1);
					used_t[v_t] = false;
					map_g_to_t[v_g] = -1;
				}
			};

			// Run algo on current graph
			dfs(0);
		}

		std::printf("Finished g%d\n", progress++);
	}

	// Output number of embeddings
	for (auto k = 1; k <= 16; ++k)
	{
		std::printf(
			"Found %d embeddings for k=%d\n",
			static_cast<int>(canons[k].size()),
			k
		);
	}

	// Save graphs
	writeGraphsToFile("graphs/62/upsilon2.adj", graphs);
}

int main(int argc, char** argv)
{
	prop2();
	
	return 0;
}

