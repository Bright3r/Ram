#include <functional>
#include <numeric>
#include <string>
#include <unordered_set>

#define MAXN (62*4)
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



using Embedding = std::vector<int>;

std::vector<Embedding> embed(
	const EdgeColoredUndirectedGraph& subgraph,
	const EdgeColoredUndirectedGraph& graph) noexcept
{
	std::vector<Embedding> embeddings;

	// Cannot embed subgraph into smaller graph
	if (subgraph.num_vertices > graph.num_vertices) return embeddings;

	Embedding map_sub_to_main(subgraph.num_vertices, -1);
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

			embeddings.emplace_back(map_sub_to_main);
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

	Embedding map_sub_to_main(subgraph.num_vertices, -1);
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


// Get neighbood of v in color c
EdgeColoredUndirectedGraph getNeighborhood(
	const EdgeColoredUndirectedGraph& g,
	Vertex v,
	Color c) noexcept
{
	std::vector<Vertex> neighbors;
	for (auto u = 0; u < g.num_vertices; ++u)
	{
		if (u == v) continue;
		if (g.getEdge(u, v) == c)
		{
			neighbors.push_back(u);
		}
	}

	EdgeColoredUndirectedGraph neighborhood(neighbors.size(), g.max_color);
	for (auto i = 0; i < neighbors.size(); ++i)
	{
		for (auto j = i+1; j < neighbors.size(); ++j)
		{
			auto ec = g.getEdge(neighbors[i], neighbors[j]);
			neighborhood.setEdge(i, j, ec);
		}
	}

	return neighborhood;
}


// Get neighbood of v in color c
EdgeColoredUndirectedGraph getNeighborhood(
	const EdgeColoredUndirectedGraph& g,
	std::vector<Vertex>& neighbors,
	Vertex v,
	Color c) noexcept
{
	for (auto u = 0; u < g.num_vertices; ++u)
	{
		if (u == v) continue;
		if (g.getEdge(u, v) == c)
		{
			neighbors.push_back(u);
		}
	}

	EdgeColoredUndirectedGraph neighborhood(neighbors.size(), g.max_color);
	for (auto i = 0; i < neighbors.size(); ++i)
	{
		for (auto j = i+1; j < neighbors.size(); ++j)
		{
			auto ec = g.getEdge(neighbors[i], neighbors[j]);
			neighborhood.setEdge(i, j, ec);
		}
	}

	return neighborhood;
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


void prop2_1() noexcept
{
	auto upsilon1 = loadBulk("graphs/62/upsilon1.adj");
	std::vector<EdgeColoredUndirectedGraph> ts = { make_T1(), make_T2() };

	std::vector<std::unordered_set<std::string>> canons(17);
	std::vector<EdgeColoredUndirectedGraph> graphs;
	int progress = 0;
	for (const auto& g : upsilon1)
	{
		// Get marked vertices of the graph
		std::vector<Vertex> marked;
		for (auto v = 0; v < 16; ++v)
		{
			if (g.getEdge(v, 16) == 4)
			{
				marked.push_back(v);
			}
		}

		// Construct base of overlapping graph
		EdgeColoredUndirectedGraph overlap_base(32 - marked.size(), 4);
		for (auto i = 0; i < 16; ++i)
		{
			for (auto j = i+1; j < 16; ++j)
			{
				overlap_base.setEdge(i, j, g.getEdge(i, j));
			}
		}

		// Construct subgraph of marked vertices
		EdgeColoredUndirectedGraph gm(marked.size(), 4);
		for (auto i = 0; i < marked.size(); ++i)
		{
			for (auto j = i+1; j < marked.size(); ++j)
			{
				gm.setEdge(i, j, g.getEdge(marked[i], marked[j]));
			}
		}

		// Find embeddings of marked subgraphs into T
		for (const auto& t : ts)
		{
			// Construct overlaps from embedding
			auto embeddings = embed(gm, t);
			for (const auto& emb : embeddings)
			{
				auto overlap = overlap_base;

				// Map vertices of t into new overlap graph
				std::vector<int> map_t_to_overlap(t.num_vertices, -1);

				// Start with marked vertices (overlapping in g and t)
				for (auto vm_idx = 0; vm_idx < marked.size(); ++vm_idx)
				{
					Vertex vm_in_g = marked[vm_idx];
					Vertex vm_in_t = emb[vm_idx];
					map_t_to_overlap[vm_in_t] = vm_in_g;
				}

				// Assign non-marked vertices of t
				auto next_v = 16;
				for (auto vt = 0; vt < t.num_vertices; ++vt)
				{
					if (map_t_to_overlap[vt] == -1)
					{
						map_t_to_overlap[vt] = next_v;
						++next_v;
					}
				}

				// Fill in edge colors of overlap graph
				for (auto i = 0; i < t.num_vertices; ++i)
				{
					for (auto j = i+1; j < t.num_vertices; ++j)
					{
						auto ec = t.getEdge(i, j);
						overlap.setEdge(map_t_to_overlap[i], map_t_to_overlap[j], ec);
					}
				}
				
				// Add u and v of attaching set
				overlap.addVertex();
				overlap.addVertex();
				Vertex u = overlap.num_vertices - 2;
				Vertex v = overlap.num_vertices - 1;
				for (auto i = 0; i < marked.size(); ++i)
				{
					overlap.setEdge(marked[i], u, 4);
					overlap.setEdge(marked[i], v, 4);
				}

				// Canonize graph
				auto canon = canonize(overlap);
				if (!canons[marked.size()].contains(canon))
				{
					canons[marked.size()].insert(canon);
					graphs.emplace_back(std::move(overlap));
				}
			}
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


void prop2_2() noexcept
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
		std::vector<Vertex> attaching_set;
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
				// // Get neighbood of u in color c
				auto neighborhood = getNeighborhood(g, u, c);

				// Check if neighborhood is embeddable into a good k16
				for (const auto& t : ts)
				{
					bool has_embedding = false;
					for (const auto& weak_n : getColorPermutations(neighborhood))
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

			// Reject coloring
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


std::unordered_map<int, std::unordered_map<int, EdgeColoredUndirectedGraph>> make_tperms() noexcept
{
	// Get all T_i(c), where T_i is one of the 2 good 3-colorings of K16
	// and c E { 1, 2, 3 } is a color replaced by color 4
	std::unordered_map<int, std::unordered_map<int, EdgeColoredUndirectedGraph>> t_perms;
	auto t1 = make_T1();
	auto t2 = make_T2();
	for (auto c = 1; c <= 3; ++c)
	{
		for (auto t_idx = 1; t_idx <= 2; ++t_idx)
		{
			EdgeColoredUndirectedGraph tperm = t1;
			if (t_idx == 2) tperm = t2;

			for (auto i = 0; i < tperm.num_vertices; ++i)
			{
				for (auto j = i+1; j < tperm.num_vertices; ++j)
				{
					if (tperm.getEdge(i, j) == c)
					{
						tperm.setEdge(i, j, 4);
					}
				}
			}

			t_perms[t_idx].emplace(c, std::move(tperm));
		}
	}

	return t_perms;
}


void prop3() noexcept
{
	auto t_perms = make_tperms();
	auto upsilon3 = loadBulk("graphs/62/upsilon3.adj");
	std::vector<EdgeColoredUndirectedGraph> pullbacks;
	for (const auto& g : upsilon3)
	{
		// Build attaching set
		auto attach_u = g.num_vertices - 2;
		auto attach_v = g.num_vertices - 1;
		std::vector<Vertex> attaching_set;
		for (auto v = 0; v < attach_u; ++v)
		{
			if (g.getEdge(attach_u, v) == 4 && g.getEdge(attach_v, v) == 4)
			{
				attaching_set.push_back(v);
			}
		}

		bool is_good = true;
		for (auto x : attaching_set)
		{
			int num_embeddable_neighborhoods = 0;
			for (auto c = 1; c <= 3; ++c)
			{
				// Build neighborhood
				auto neighborhood = getNeighborhood(g, x, c);
				if (canEmbed(neighborhood, t_perms[1].at(c)) ||
					canEmbed(neighborhood, t_perms[2].at(c)))
				{
					++num_embeddable_neighborhoods;
				}
			}

			if (num_embeddable_neighborhoods < 2)
			{
				is_good = false;
				break;
			}
		}

		if (is_good)
		{
			pullbacks.push_back(g);
		}
	}

	std::printf("%zu pullbacks found\n", pullbacks.size());


	// Pull back graphs
	std::vector<EdgeColoredUndirectedGraph> graphs;
	std::unordered_set<std::string> canons;
	std::vector<int> attaching_orders(17, 0);
	int progress = 0;
	for (const auto& g : pullbacks)
	{
		// Get first vertex of attaching set
		auto attach_u = g.num_vertices - 2;
		auto attach_v = g.num_vertices - 1;
		std::vector<Vertex> attaching_set;
		for (auto v = 0; v < attach_u; ++v)
		{
			if (g.getEdge(attach_u, v) == 4 && g.getEdge(attach_v, v) == 4)
			{
				attaching_set.push_back(v);
			}
		}
		Vertex v_extend = attaching_set[0];

		
		// Get embeddings of neighborhoods into T1(c) and T2(c)
		for (auto c = 1; c <= 3; ++c)
		{
			// Build neighborhood
			std::vector<Vertex> neighbors;
			auto neighborhood = getNeighborhood(g, neighbors, v_extend, c);

			// Embed
			std::vector<EdgeColoredUndirectedGraph> ts = { t_perms[1].at(c), t_perms[2].at(c) };
			for (const auto& t : ts)
			{
				auto embeddings = embed(neighborhood, t);
				for (const auto& emb : embeddings)
				{
					// Pull back embedding
					auto partial = g;
					for (auto i = 0; i < neighbors.size(); ++i)
					{
						auto u = neighbors[i];
						for (auto j = i+1; j < neighbors.size(); ++j)
						{
							auto v = neighbors[j];
							if (!partial.hasEdge(u, v))
							{
								auto ec = t.getEdge(emb[i], emb[j]);
								partial.setEdge(u, v, ec);
							}
						}
					}

					// Check if triangle-free
					if (!isTriangleFree(partial)) continue;

					// Check if non-isomorhpic
					auto canon = canonize(partial);
					if (!canons.contains(canon))
					{
						canons.insert(canon);
						graphs.emplace_back(std::move(partial));
						attaching_orders[attaching_set.size()]++;
					}
				}
			}
		}

		std::printf("Finished g%d\n", ++progress);
	}


	for (auto i = 0; i < attaching_orders.size(); ++i)
	{
		std::printf("Attaching Set Order %d: %d\n", i, attaching_orders[i]);
	}
	std::printf("Found %zu partial colorings extended by one vertex\n", canons.size());

	// Save to file
	writeGraphsToFile("graphs/62/upsilon4.adj", graphs);
}


void prop4() noexcept
{
	auto t_perms = make_tperms();
	auto upsilon4 = loadBulk("graphs/62/upsilon4.adj");

	// Cull colorings that are not embeddable in two colors
	std::vector<EdgeColoredUndirectedGraph> embeddable;
	for (const auto& g : upsilon4)
	{
		// Find the attaching set of u and v
		auto attach_u = g.num_vertices - 2;
		auto attach_v = g.num_vertices - 1;
		std::vector<Vertex> attaching_set;
		for (auto v = 0; v < attach_u; ++v)
		{
			if (g.getEdge(attach_u, v) == 4 && g.getEdge(attach_v, v) == 4)
			{
				attaching_set.push_back(v);
			}
		}

		// Check each vertex in attaching set for embeddability
		bool is_good = true;
		for (auto x : attaching_set)
		{
			auto num_embeddable_colors = 0;
			for (auto c = 1; c <= 3; ++c)
			{
				std::vector<EdgeColoredUndirectedGraph> ts = { t_perms[1].at(c), t_perms[2].at(c) };
				auto neighborhood = getNeighborhood(g, x, c);
				for (const auto& t : ts)
				{
					if (canEmbed(neighborhood, t))
					{
						++num_embeddable_colors;
						break;
					}
				}
			}

			if (num_embeddable_colors < 2)
			{
				is_good = false;
				break;
			}
		}

		if (is_good)
		{
			embeddable.push_back(g);
		}
	}

	std::printf("%zu graphs are embeddable in two colors\n", embeddable.size());


}

int main(int argc, char** argv)
{
	// prop1();
	// prop2_1();
	// prop2_2();
	// prop3();
	prop4();
	
	return 0;
}

