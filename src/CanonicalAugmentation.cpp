#include <chrono>
#include <cstdio>
#include <ctime>
#include <vector>
#include <cassert>
#include <sstream>
#include <string>
#include <filesystem>
#include <unordered_set>

#include "EdgeColoredUndirectedGraph.h"
#include "GraphUtils.h"

using namespace Ram;

void processRepresentative(
	const Ram::EdgeColoredUndirectedGraph& representative,
	const std::vector<std::vector<Color>>& colorings,
	std::vector<Ram::EdgeColoredUndirectedGraph>& new_graphs,
	std::unordered_set<std::string>& new_canons) noexcept
{
	// Add vertex to rep
	auto rep_plus_one = representative;
	rep_plus_one.addVertex();

	// Go through all new edge colors for new vertex
	for (size_t c = 0; c < colorings.size(); ++c)
	{
		const auto& curr_coloring = colorings[c];

		// Apply edge coloring
		auto g = rep_plus_one;
		size_t new_vertex = rep_plus_one.num_vertices - 1;
		for (auto i = 0; i < new_vertex; ++i)
		{
			g.setEdge(new_vertex, i, curr_coloring[i]);
		}

		// Check if triangle was added
		bool has_tri = false;
		size_t tri_maker_idx = 0;
		for (auto i = 0; i < new_vertex; ++i)
		{
			for (auto j = i+1; j < new_vertex; ++j)
			{
				auto e0 = g.getEdge(new_vertex, i);
				auto e1 = g.getEdge(new_vertex, j);
				auto e2 = g.getEdge(i, j);
				if ((e0 == e1) && (e0 == e2) && (e1 == e2)) 
				{
					has_tri = true;
					tri_maker_idx = j;
					break;
				}
			}

			if (has_tri) break;
		}

		// Skip colorings with triangles
		if (has_tri)
		{
			// Find next coloring that breaks current triangle
			size_t new_c = c;
			Color tri_color = curr_coloring[tri_maker_idx];
			while (new_c < colorings.size() && 
				colorings[new_c][tri_maker_idx] == tri_color)
			{
				++new_c;
			}

			c = new_c-1;
			continue;
		}

		// Track distinct colorings
		auto canon_str = canonize(g);
		if (!new_canons.contains(canon_str))
		{
			new_canons.insert(canon_str);
			new_graphs.push_back(g);
		}
	}
}

void augment(int k_start = 3, int k_stop = 16, Color max_color = 3) noexcept
{
	std::vector<Ram::EdgeColoredUndirectedGraph> graphs;
	if (k_start == 3)
	{
		// Get all k2's
		Ram::EdgeColoredUndirectedGraph base(2, max_color);
		for (auto c = 1; c <= max_color; ++c)
		{
			auto g = base;
			g.setEdge(0, 1, static_cast<Color>(c));
			graphs.push_back(g);
		}
	}
	else
	{
		std::stringstream start_file;
		start_file << "graphs/k" << k_start-1 << ".adj";
		graphs = load_bulk(start_file.str(), k_start-1, max_color);
	}


	// Iterate through k3-k16
	for (auto v = k_start; v <= k_stop; ++v)
	{
		auto start_time = std::chrono::high_resolution_clock::now();


		// Go through all previous canonical representatives
		auto num_new_edges = v-1;
		auto colorings = generateAllColorings(num_new_edges, max_color);

		std::vector<Ram::EdgeColoredUndirectedGraph> new_graphs;
		std::unordered_set<std::string> new_canons;
		for (const auto& representative : graphs)
		{
			processRepresentative(
				representative,
				colorings,
				new_graphs,
				new_canons
			);
		}


		auto end_time = std::chrono::high_resolution_clock::now();
		Timing::seconds time = end_time - start_time;

		std::printf(
			"Found %d distinct colorings for k%d in %.2f seconds.\n",
			static_cast<int>(new_graphs.size()),
			v,
			time.count()
		);

		graphs = new_graphs;

		std::stringstream file_path; 
		file_path << "graphs/k" << v << ".adj";
		writeGraphsToFile(file_path.str(), graphs);
	}
}

void verify() noexcept
{
	for (auto k = 3; k <= 16; ++k)
	{
		std::stringstream start_file;
		start_file << "graphs/k" << k << ".adj";
		auto graphs = load_bulk(start_file.str(), k, 3);

		std::unordered_set<std::string> canons;
		for (const auto& g : graphs)
		{
			canons.insert(canonize(g));
		}

		std::printf(
			"%d distinct colorings of k%d.\n",
			static_cast<int>(canons.size()),
			static_cast<int>(k)
		);
	}
}

