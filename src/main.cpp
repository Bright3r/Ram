#include <chrono>
#include <cstdio>
#include <ctime>
#include <utility>
#include <vector>
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <unordered_set>

#include "EdgeColoredUndirectedGraph.h"

using namespace Ram;

// Colors
constexpr Color Uncolored = 0;
constexpr Color Red = 1;
constexpr Color Blue = 2;
constexpr Color Green = 3;
constexpr Color Purple = 4;

// Type Aliases
namespace Timing 
{
	using seconds = std::chrono::duration<double>;
	using millis = std::chrono::duration<double, std::milli>;
};

struct ColoringGenerator
{
	size_t num_edges;
	size_t num_colors;
	bool is_done = false;

	std::vector<Color> coloring;

	ColoringGenerator(size_t e, size_t k)
		: num_edges(e)
		, num_colors(k)
		, coloring(e, 1)
	{ }

	bool next(std::vector<Color>& out)
	{
		if (is_done) return false;
		out = coloring;

		int pos = num_edges-1;
		while (pos >= 0)
		{
			++coloring[pos];
			if (coloring[pos] <= num_colors) break;
			coloring[pos] = 1;
			--pos;
		}

		if (pos < 0) is_done = true;
		return true;
	}
};



EdgeColoredUndirectedGraph load_adj(
	std::filesystem::path filename,
	size_t num_vertices,
	Color max_color) noexcept
{
	std::ifstream file(filename);
	assert(file.is_open() && "Ram::load_adj() Failed: file not found.");

	// Parse File
	std::string line;
	std::string word;

	// Read File line by line to construct graph
	size_t i = 0;
	EdgeColoredUndirectedGraph g(num_vertices, max_color);
	while (std::getline(file, line)) {
		// Read line for this vertex's edge colors
		std::stringstream ss(line);
		size_t j = 0;
		while (ss >> word) {
			if (j > i)
			{
				Color color = static_cast<Color>(std::stoi(word));
				g.setEdge(i, j, color);
			}
			++j;
		}
		++i;
	}

	return g;
}

std::vector<EdgeColoredUndirectedGraph> load_bulk(
	std::filesystem::path filename,
	size_t num_vertices,
	Color max_color) noexcept
{
	std::ifstream file(filename);
	assert(file.is_open() && "Ram::load_bulk() Failed: file not found.");

	std::vector<EdgeColoredUndirectedGraph> res;

	// Parse File
	std::string line;
	std::string word;

	// Read File line by line to construct graph
	size_t i = 0;
	EdgeColoredUndirectedGraph g(num_vertices, max_color);
	while (std::getline(file, line)) {
		// Read line for this vertex's edge colors
		std::stringstream ss(line);
		size_t j = 0;
		bool is_end = true;
		while (ss >> word) {
			is_end = false;
			Color color = static_cast<Color>(std::stoi(word));

			g.setEdge(i, j, color);
			++j;
		}
		++i;

		// Blank line marks new graph
		if (is_end)
		{
			res.push_back(std::move(g));
			g = EdgeColoredUndirectedGraph(num_vertices, max_color);
			i = 0;
		}
	}

	return res;
}

EdgeColoredUndirectedGraph make_T1() noexcept
{
	return load_adj("graphs/T1.adj", 16, 3);
}

EdgeColoredUndirectedGraph make_T2() noexcept
{
	return load_adj("graphs/T2.adj", 16, 3);
}

bool isIsomorphic(graph* cg1, graph* cg2, int n, int m) noexcept
{
	for (auto i = 0; i < n*m; ++i)
	{
		if (cg1[i] != cg2[i]) return false;
	}
	return true;
}

std::string getCanonString(graph* cg, int n, int m) noexcept
{
	std::string canon;
	for (auto i = 0; i < n*m; ++i)
	{
		canon += std::to_string(cg[i]);
	}
	return canon;
}

void printCanong(graph* cg, int n, int m) noexcept
{
	auto str = getCanonString(cg, n, m);
	std::printf("%s\n", str.c_str());
}


void writeGraphsToFile(
	const std::filesystem::path& path,
	const std::vector<Ram::EdgeColoredUndirectedGraph>& graphs)
{
	auto start_time = std::chrono::high_resolution_clock::now();

	std::ofstream out(path);
	for (const auto& g : graphs)
	{
		out << g.to_string() << "\n";
	}
	out.flush();

	auto end_time = std::chrono::high_resolution_clock::now();
	Timing::seconds time = end_time - start_time;
	std::printf(
		"Wrote to %s in %.2f seconds\n",
		path.c_str(),
		time.count()
	);
	std::printf("\n");
}

std::vector<std::vector<Color>> generateAllColorings(size_t e, size_t k)
{
	auto start_time = std::chrono::high_resolution_clock::now();

	std::vector<std::vector<Color>> res;
	std::vector<Color> coloring(e, 1);
	while (true)
	{
		res.push_back(coloring);

		int pos = e-1;
		while (pos >= 0)
		{
			coloring[pos]++;
			if (coloring[pos] <= k) break;
			
			coloring[pos] = 1;
			--pos;
		}

		if (pos < 0) break;
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	Timing::seconds time = end_time - start_time;
	std::printf(
		"Generated colorings for %d edges, %d colors in %.2f seconds.\n",
		static_cast<int>(e),
		static_cast<int>(k),
		time.count()
	);

	return res;
}

std::string canonize(const Ram::EdgeColoredUndirectedGraph& g) noexcept
{
	// Canonicalize graph
	std::string canon_str = "";
	for (auto& weak_g : g.getColorPermutations())
	{
		// Nauty return data
		statsblk stats;
		int n = weak_g.numEncodedVertices();
		int m = weak_g.numWordsPerVertex();
		int lab[n], ptn[n], orbits[n];
		for (auto i = 0; i < n; ++i)
		{
			lab[i] = i;
			ptn[i] = (i+1 < n) ? 1 : 0;
		}

		// Setup options
		DEFAULTOPTIONS_GRAPH(options);
		options.getcanon = true;

		// Dense Nauty
		auto nauty_g = weak_g.nautify();
		graph canong[n*m];
		densenauty(nauty_g.data(), lab, ptn, orbits, &options, &stats, m, n, canong);

		// Only keep lexicographically smallest canonization
		std::string weak_canon_str = getCanonString(canong, n, m);
		if (canon_str.empty() || weak_canon_str < canon_str)
		{
			canon_str = weak_canon_str;
		}
	}

	return canon_str;
}

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
		new_graphs.reserve(2700000);
		new_canons.reserve(2700000);
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

int main(int argc, char** argv)
{
	augment(6);
	
	return 0;
}

