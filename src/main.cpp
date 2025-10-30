#include <functional>
#include <numeric>
#include <string>
#include <unordered_set>

#define MAXN (32*4)
#include "EdgeColoredUndirectedGraph.h"
#include "GraphUtils.h"


using namespace Ram;

EdgeColoredUndirectedGraph make_T1() noexcept
{
	return loadBulk("graphs/T1.adj")[0];
}

EdgeColoredUndirectedGraph make_T2() noexcept
{
	return loadBulk("graphs/T2.adj")[0];
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
	auto upsilon1 = loadBulk("graphs/62/upsilon1.adj");
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
			std::vector<int> g_map(16, -1);
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

					// Create u and v of attaching set
					overlap.addVertex();
					overlap.addVertex();
					auto u_attach = overlap.num_vertices - 2;
					auto v_attach = overlap.num_vertices - 1;
					overlap.setEdge(u_attach, v_attach, 4);

					// Copy marked vertices of g into overlap
					for (auto vm_g : marked_v)
					{
						// Find corresponding vertex in t
						auto vm_t = map_g_to_t[vm_g];

						// Add edge between marked vertices of g with u and v of attaching set
						overlap.setEdge(u_attach, vm_t, 4);
						overlap.setEdge(v_attach, vm_t, 4);

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


std::vector<EdgeColoredUndirectedGraph> embed(
	const EdgeColoredUndirectedGraph& subgraph,
	const EdgeColoredUndirectedGraph& graph) noexcept
{
	std::vector<EdgeColoredUndirectedGraph> embeddings;

	// Cannot embed subgraph into smaller graph
	if (subgraph.num_vertices > graph.num_vertices) return embeddings;

	std::vector<int> map_sub_to_main(subgraph.num_vertices, -1);
	std::vector<bool> used_main(graph.num_vertices, false);
	std::function<void(int)> VF2_dfs = [&](int depth)
	{
		// Full embedding found
		if (depth == subgraph.num_vertices)
		{
			// Construct overlapping subgraph
			EdgeColoredUndirectedGraph overlap(graph.num_vertices, subgraph.max_color);

			// Copy vertices of subgraph into overlap
			for (auto i = 0; i < subgraph.num_vertices; ++i)
			{
				// Find corresponding vertex in main graph
				auto vi_main = map_sub_to_main[i];

				// Copy the edges from vertices in subgraph
				// as edges from vertices of main graph in overlap
				for (auto j = i+1; j < subgraph.num_vertices; ++j)
				{
					auto vj_main = map_sub_to_main[j];
					overlap.setEdge(vi_main, vj_main, subgraph.getEdge(i, j));
				}
			}

			embeddings.emplace_back(std::move(overlap));
			return;
		}

		// Find a vertex in subgraph that can map to main graph
		auto v_sub = depth;
		for (auto v_main = 0; v_main < graph.num_vertices; ++v_main)
		{
			if (used_main[v_main]) continue;

			// Check that edge colors agree with previously mapped vertices
			bool ok = true;
			for (auto prev = 0; prev < depth; ++prev)
			{
				auto v_sub_prev = prev;
				auto v_main_prev = map_sub_to_main[v_sub_prev];
				if (v_main_prev == -1) continue;

				auto c_sub = subgraph.getEdge(v_sub, v_sub_prev);
				auto c_main = graph.getEdge(v_main, v_main_prev);
				if (c_sub != 0 && c_sub != c_main)
				{
					ok = false;
					break;
				}
			}

			if (!ok) continue;

			// Found valid mapping
			map_sub_to_main[v_sub] = v_main;
			used_main[v_main] = true;

			// Recurse then continue
			VF2_dfs(depth + 1);
			used_main[v_main] = false;
			map_sub_to_main[v_sub] = -1;
		}
	};

	VF2_dfs(0);
	return embeddings;
};

bool canEmbed(
	const EdgeColoredUndirectedGraph& subgraph,
	const EdgeColoredUndirectedGraph& graph) noexcept
{
	// Cannot embed subgraph into smaller graph
	if (subgraph.num_vertices > graph.num_vertices) return false;

	std::vector<int> map_sub_to_main(subgraph.num_vertices, -1);
	std::vector<bool> used_main(graph.num_vertices, false);
	std::function<bool(int)> VF2_dfs = [&](int depth)
	{
		// Full embedding found
		if (depth == subgraph.num_vertices)
		{
			return true;
		}

		// Find a vertex in subgraph that can map to main graph
		auto v_sub = depth;
		for (auto v_main = 0; v_main < graph.num_vertices; ++v_main)
		{
			if (used_main[v_main]) continue;

			// Check that edge colors agree with previously mapped vertices
			bool ok = true;
			for (auto prev = 0; prev < depth; ++prev)
			{
				auto v_sub_prev = prev;
				auto v_main_prev = map_sub_to_main[v_sub_prev];
				if (v_main_prev == -1) continue;

				auto c_sub = subgraph.getEdge(v_sub, v_sub_prev);
				auto c_main = graph.getEdge(v_main, v_main_prev);
				if (c_sub != 0 && c_sub != c_main)
				{
					ok = false;
					break;
				}
			}

			if (!ok) continue;

			// Found valid mapping
			map_sub_to_main[v_sub] = v_main;
			used_main[v_main] = true;

			// Recurse then continue
			if (VF2_dfs(depth + 1)) return true;

			used_main[v_main] = false;
			map_sub_to_main[v_sub] = -1;
		}

		return false;
	};

	return VF2_dfs(0);
};


void prop3() noexcept
{
	auto upsilon2 = loadBulk("graphs/62/upsilon2.adj");
	std::vector<EdgeColoredUndirectedGraph> ts = { make_T1(), make_T2() };

	std::vector<EdgeColoredUndirectedGraph> graphs;
	std::vector<int> num_verts_to_partials(33);
	std::vector<int> order_to_partials(17);

	int progress = 0;
	for (const auto& g : upsilon2)
	{
		// Exclude attaching sets of orders 1 + 16
		// Note that orders 2 + 15 could be excluded too
		// if (g.num_vertices < 19 || g.num_vertices > 32) continue;

		// Find the attaching set of u and v
		auto attach_u = g.num_vertices - 2;
		auto attach_v = g.num_vertices - 1;
		std::vector<size_t> attaching_set;
		for (auto v = 0; v < attach_u; ++v)
		{
			if (g.getEdge(attach_u, v) == 4 && g.getEdge(attach_v, v) == 4)
			{
				attaching_set.push_back(v);
			}
		}
		
		// Make sure each vertex in the attaching set is
		// embeddable into a good k16 for at least 2 colors
		bool is_embeddable = true;
		for (auto u : attaching_set)
		{
			int num_embeddable_neighborhoods = 0;
			for (Color c = 1; c <= 3; ++c)
			{
				// Get neighbood of u in color c
				std::vector<size_t> neighbors;
				for (auto v = 0; v < attach_u; ++v)
				{
					if (v == u) continue;
					if (g.getEdge(u, v) == c)
					{
						neighbors.push_back(v);
					}
				}

				EdgeColoredUndirectedGraph neighborhood(neighbors.size(), 3);
				for (auto i = 0; i < neighbors.size(); ++i)
				{
					for (auto j = i+1; j < neighbors.size(); ++j)
					{
						auto ec = g.getEdge(neighbors[i], neighbors[j]);
						neighborhood.setEdge(i, j, ec);
					}
				}

				// Check if neighborhood is embeddable into a good k16
				for (const auto& t : ts)
				{
					bool has_embedding = false;
					for (const auto& weak_n : neighborhood.getColorPermutations())
					{
						if (canEmbed(weak_n, t))
						{
							has_embedding = true;
							break;
						}
					}

					if (has_embedding)
					{
						++num_embeddable_neighborhoods;
						break;
					}
				}
			}

			std::printf("  u=%zu, embeddable=%d\n", u, num_embeddable_neighborhoods);
			if (num_embeddable_neighborhoods < 2)
			{
				is_embeddable = false;
				break;
			}
		}


		std::printf("Finished g%d\n", progress++);
		if (is_embeddable)
		{
			// Keep graph for upsilon3
			graphs.push_back(g);

			num_verts_to_partials[g.num_vertices]++;
			order_to_partials[attaching_set.size()]++;
		}
	}

	// Output results
	for (auto i = 0; i < num_verts_to_partials.size(); ++i)
	{
		std::printf(
			"%d Vertices - %d partial colorings\n",
			i,
			num_verts_to_partials[i]
		);
	}

	for (auto i = 1; i < order_to_partials.size(); ++i)
	{
		std::printf(
			"%d Order of attaching set - %d partial colorings\n",
			i,
			order_to_partials[i]
		);
	}

	std::printf("%zu remaining graphs\n", graphs.size());

	// Save Graphs
	writeGraphsToFile("graphs/62/upsilon3.adj", graphs);
}

int main(int argc, char** argv)
{
	// prop1();
	// prop2();
	prop3();
	

	// auto t = make_T1();
	// auto sub_sz = 4;
	// for (auto start = 0; start <= t.num_vertices-sub_sz; ++start)
	// {
	// 	EdgeColoredUndirectedGraph g(sub_sz, 3);
	// 	for (auto i = 0; i < sub_sz; ++i)
	// 	{
	// 		for (auto j = i+1; j < sub_sz; ++j)
	// 		{
	// 			auto c = t.getEdge(start+i, start+j);
	// 			if (c != 3) continue;
	// 			g.setEdge(i, j, c);
	// 		}
	// 	}
	//
	// 	auto res = embed(g, t);
	// 	std::printf("Start=%d: %zu embeddings\n", start, res.size());
	// }

	return 0;
}

