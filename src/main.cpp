#include <chrono>
#include <cstdio>
#include <ctime>
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
constexpr int Uncolored = 0;
constexpr int Red = 1;
constexpr int Blue = 2;
constexpr int Green = 3;
constexpr int Purple = 4;

// Type Aliases
namespace Timing 
{
	using seconds = std::chrono::duration<double>;
	using millis = std::chrono::duration<double, std::milli>;
};

struct ColoringGenerator
{
	int num_edges;
	int num_colors;
	bool is_done = false;

	std::vector<int> coloring;

	ColoringGenerator(int e, int k)
		: num_edges(e)
		, num_colors(k)
		, coloring(e, 1)
	{ }

	bool next(std::vector<int>& out)
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



template <size_t NumVertices, size_t MaxColor>
EdgeColoredUndirectedGraph load_adj(std::filesystem::path filename) noexcept
{
	std::ifstream file(filename);
	assert(file.is_open() && "Ram::load_adj() Failed: file not found.");

	// Parse File
	std::string line;
	std::string word;

	// Read File line by line to construct graph
	int i = 0;
	EdgeColoredUndirectedGraph g(NumVertices, MaxColor);
	while (std::getline(file, line)) {
		// Read line for this vertex's edge colors
		std::stringstream ss(line);
		int j = 0;
		while (ss >> word) {
			if (j > i)
			{
				int color = std::stoi(word);
				g.setEdge(i, j, color);
			}
			++j;
		}
		++i;
	}

	return g;
}

EdgeColoredUndirectedGraph make_T1() noexcept
{
	return load_adj<16, 3>("graphs/T1.adj");
}

EdgeColoredUndirectedGraph make_T2() noexcept
{
	return load_adj<16, 3>("graphs/T2.adj");
}

bool isIsomorphic(graph* cg1, graph* cg2, int n, int m) noexcept
{
	for (int i = 0; i < n*m; ++i)
	{
		if (cg1[i] != cg2[i]) return false;
	}
	return true;
}

std::string getCanonString(graph* cg, int n, int m) noexcept
{
	std::string canon;
	for (int i = 0; i < n*m; ++i)
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

std::vector<std::vector<int>> generateAllColorings(int e, int k)
{
	auto start_time = std::chrono::high_resolution_clock::now();

	std::vector<std::vector<int>> res;
	std::vector<int> coloring(e, 1);
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
		e,
		k,
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
		for (int i = 0; i < n; ++i)
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
	int num_new_edges, 
	int v,
	std::vector<Ram::EdgeColoredUndirectedGraph>& new_graphs,
	std::unordered_set<std::string>& new_canons) noexcept
{
	// Add vertex to rep
	auto rep_plus_one = representative;
	rep_plus_one.addVertex();

	// Go through all new edge colors for new vertex
	ColoringGenerator gen(num_new_edges, 3);
	std::vector<int> coloring;
	while (gen.next(coloring))
	{
		// Apply edge coloring
		auto g = rep_plus_one;
		int new_vertex = v-1;
		for (int i = 0; i < new_vertex; ++i)
		{
			g.setEdge(new_vertex, i, coloring[i]);
		}

		// Check if triangle was added
		bool has_tri = false;
		for (int i = 0; i < new_vertex; ++i)
		{
			for (int j = i+1; j < new_vertex; ++j)
			{
				auto e0 = g.getEdge(new_vertex, i);
				auto e1 = g.getEdge(new_vertex, j);
				auto e2 = g.getEdge(i, j);
				if ((e0 == e1) && (e0 == e2) && (e1 == e2)) 
				{
					has_tri = true;
					break;
				}
			}

			if (has_tri) break;
		}
		if (has_tri) continue;

		// Track distinct colorings
		auto canon_str = canonize(g);
		if (!new_canons.contains(canon_str))
		{
			new_canons.insert(canon_str);
			new_graphs.push_back(g);
		}
	}
}

void augment() noexcept
{
	// Get all k2's
	std::vector<Ram::EdgeColoredUndirectedGraph> graphs;
	Ram::EdgeColoredUndirectedGraph base(2, 3);
	for (int c = 1; c <= 3; ++c)
	{
		auto g = base;
		g.setEdge(0, 1, c);
		graphs.push_back(g);
	}

	// Iterate through k3-k16
	for (int v = 3; v <= 16; ++v)
	{
		auto start_time = std::chrono::high_resolution_clock::now();


		// Go through all previous canonical representatives
		int num_new_edges = v - 1;
		auto colorings = generateAllColorings(num_new_edges, 3);
		std::vector<Ram::EdgeColoredUndirectedGraph> new_graphs;
		std::unordered_set<std::string> new_canons;
		for (const auto& representative : graphs)
		{
			processRepresentative(
				representative,
				num_new_edges,
				v,
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
	augment();
	
	return 0;
}

