#include <chrono>
#include <cstdio>
#include <array>
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


template <size_t NumVertices, size_t MaxColor>
EdgeColoredUndirectedGraph<NumVertices, MaxColor> load_adj(std::filesystem::path filename) noexcept
{
	std::ifstream file(filename);
	assert(file.is_open() && "Ram::load_adj() Failed: file not found.");

	// Parse File
	std::string line;
	std::string word;

	// Read File line by line to construct graph
	int i = 0;
	EdgeColoredUndirectedGraph<NumVertices, MaxColor> g;
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

EdgeColoredUndirectedGraph<16, 3> make_T1() noexcept
{
	return load_adj<16, 3>("graphs/T1.adj");
}

EdgeColoredUndirectedGraph<16, 3> make_T2() noexcept
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


void checkT1T2() noexcept
{
	// Make sure T1 and T2 are triangle-free
	auto t1 = make_T1();
	if (t1.isTriangleFree())
	{
		std::printf("No Triangles in T1.\n");
	}
	else std::printf("Triangle found in T1!\n");

	auto t2 = make_T2();
	if (t2.isTriangleFree())
	{
		std::printf("No Triangles in T2.\n");
	}
	else std::printf("Triangle found in T2!\n");


	// Make sure T1 and T2 are non-isomorphic
	constexpr int n = t1.GraphSize;
	constexpr int m = t1.VertexWords;

	// Nauty return data
	int lab[n], ptn[n], orbits[n];
	statsblk stats;
	for (int i = 0; i < n; ++i)
	{
		lab[i] = i;
		ptn[i] = (i+1 < n) ? 1 : 0;
	}

	// Setup options
	DEFAULTOPTIONS_GRAPH(options);
	options.getcanon = true;

	auto nauty_t1 = t1.nautify();
	graph canong_t1[n*m];
	densenauty(nauty_t1.data(), lab, ptn, orbits, &options, &stats, m, n, canong_t1);

	auto nauty_t2 = t2.nautify();
	graph canong_t2[n*m];
	densenauty(nauty_t2.data(), lab, ptn, orbits, &options, &stats, m, n, canong_t2);

	if (isIsomorphic(canong_t1, canong_t2, n, m))
	{
		std::printf("T1 and T2 are isomorphic.\n");
	}
	else
	{
		std::printf("T1 and T2 are non-isomorphic.\n");
	}

	std::printf("\nCanonical Labeling of T1: \n");
	printCanong(canong_t1, n, m);

	std::printf("\nCanonical Labeling of T2: \n");
	printCanong(canong_t2, n, m);


	// Get all weak isomorphs of { T1, T2 }
	auto weak_isomorphs = t1.getWeakIsomorphs();
	auto t2_weak_isomorphs = t2.getWeakIsomorphs();
	weak_isomorphs.insert(
		weak_isomorphs.end(), t2_weak_isomorphs.begin(), t2_weak_isomorphs.end()
	);

	// Get all canonical labelings of weak isomorphs of { T1, T2 }
	std::unordered_set<std::string> weak_isomorph_canons;
	for (const auto& g : weak_isomorphs)
	{
		auto nauty_g = g.nautify();
		graph canong_g[n*m];
		densenauty(nauty_g.data(), lab, ptn, orbits, &options, &stats, m, n, canong_g);

		weak_isomorph_canons.insert(getCanonString(canong_g, n, m));
	}

	std::printf(
		"\nNumber of weak isomorphs: %d\n", 
		static_cast<int>(weak_isomorph_canons.size())
	);

	std::printf("T1:\n");
	t1.print();

	std::printf("\nT2:\n");
	t2.print();
}

void prop1() noexcept
{
	// Good k16
	auto t1 = make_T1();

	// Copy good k16 to partial k17
	Ram::EdgeColoredUndirectedGraph<17, 4> baseg;
	for (int i = 0; i < 16; ++i)
	{
		for (int j = i+1; j < 16; ++j)
		{
			int c = t1.getEdge(i, j);
			assert(c != 0 && "T1 must be complete");
			baseg.setEdge(i, j, c);
		}
	}

	std::vector<std::vector<int>> perms;
	for (int s = 0; s < 16; ++s)
	{
		perms.push_back({ s });
	}

	std::unordered_set<std::string> canons;
	for (auto& perm : perms)
	{
		Ram::EdgeColoredUndirectedGraph<17, 4> marked_g = baseg;

		// Mark subset with color4
		for (int v : perm)
		{
			marked_g.setEdge(16, v, 4);
		}

		for (auto& g : marked_g.getWeakIsomorphs(3))
		{
			int n = g.GraphSize;
			int m = g.VertexWords;

			// Nauty return data
			int lab[n], ptn[n], orbits[n];
			statsblk stats;
			for (int i = 0; i < n; ++i)
			{
				lab[i] = i;
				ptn[i] = (i+1 < n) ? 1 : 0;
			}

			// Setup options
			DEFAULTOPTIONS_GRAPH(options);
			options.getcanon = true;

			auto nauty_g = g.nautify();
			graph canong[n*m];
			densenauty(nauty_g.data(), lab, ptn, orbits, &options, &stats, m, n, canong);

			canons.insert(getCanonString(canong, n, m));
		}
	}

	std::printf(
		"Non-isomorphic marked subsets of order: %d\n",
		static_cast<int>(canons.size())
	);
}

template <size_t num_vertices, size_t max_color>
void backtrack(
	std::unordered_set<std::string>& canons,
	std::unordered_set<std::string>& seen,
	Ram::EdgeColoredUndirectedGraph<num_vertices, max_color>& g,
	int i = 0, int j = 0) noexcept
{
	// Full coloring with no triangles
	if (!g.isPartial())
	{
		std::string canon_str = "";
		for (auto& weak_g : g.getWeakIsomorphs())
		{
			// Nauty return data
			statsblk stats;
			int n = weak_g.GraphSize;
			int m = weak_g.VertexWords;
			int lab[n], ptn[n], orbits[n];
			for (int i = 0; i < n; ++i)
			{
				lab[i] = i;
				ptn[i] = (i+1 < n) ? 1 : 0;
			}

			// Setup options
			DEFAULTOPTIONS_GRAPH(options);
			options.getcanon = true;

			// Canonicalize
			auto nauty_g = weak_g.nautify();
			graph canong[n*m];
			densenauty(nauty_g.data(), lab, ptn, orbits, &options, &stats, m, n, canong);

			std::string weak_canon_str = getCanonString(canong, n, m);
			if (canon_str.empty() || weak_canon_str < canon_str)
			{
				canon_str = weak_canon_str;
			}
		}

		canons.insert(canon_str);
	}


	if (i >= num_vertices - 1) return;

	int next_i = i;
	int next_j = j+1;
	if (next_j >= num_vertices)
	{
		++next_i;
		next_j = next_i + 1;
	}

	for (int c = 1; c <= max_color; ++c)
	{
		g.setEdge(i, j, c);

		// Only backtrack on this edge color if a triangle is not created
		bool created_tri = false;
		for (int k = 0; k < num_vertices; ++k)
		{
			if (k == i || k == j) continue;
			if (g.getEdge(i, k) == c && g.getEdge(j, k) == c)
			{
				created_tri = true;
				break;
			}
		}
		if (!created_tri) 
		{
			backtrack(canons, seen, g, next_i, next_j);
		}

		g.setEdge(i, j, 0);
	}
}

template <size_t num_vertices>
void verify() noexcept
{
	std::unordered_set<std::string> canons;
	std::unordered_set<std::string> seen;
	Ram::EdgeColoredUndirectedGraph<num_vertices, 3> g;
	backtrack(canons, seen, g);

	std::printf(
		"Num Isomorphs for k%d: %d\n",
		static_cast<int>(num_vertices),
		static_cast<int>(canons.size())
	);
}

template <size_t num_vertices>
void run() noexcept
{
	auto start = std::chrono::high_resolution_clock::now();

	verify<num_vertices>();

	auto end = std::chrono::high_resolution_clock::now();
	double time_ms = 
		std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	int ms = time_ms;

	int sec = ms / 1000;
	ms %= 1000;

	int min = sec / 60;
	sec %= 60;

	std::printf(
		"K%d computed in %dm %ds %dms\n",
		static_cast<int>(num_vertices),
		min,
		sec,
		ms
	);
}

int main(int argc, char** argv)
{
	run<3>();
	run<4>();
	run<5>();
	run<6>();
	run<7>();
	run<8>();
	run<9>();
	
	return 0;
}

