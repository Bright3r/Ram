#include "GraphUtils.h"
#include "EdgeColoredUndirectedGraph.h"

#include <fstream>
#include <chrono>

using namespace Ram;

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

std::string canonize(const Ram::EdgeColoredUndirectedGraph& g) noexcept
{
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

void writeGraphsToFile(
	const std::filesystem::path& path,
	const std::vector<Ram::EdgeColoredUndirectedGraph>& graphs)
{
	auto start_time = std::chrono::high_resolution_clock::now();

	std::ofstream out(path);
	for (const auto& g : graphs)
	{
		out << g.header_string() << "\n";
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

std::vector<EdgeColoredUndirectedGraph> loadBulk(std::filesystem::path filename) noexcept
{
	std::ifstream file(filename);
	assert(file.is_open() && "Ram::load_bulk() Failed: file not found.");

	std::vector<EdgeColoredUndirectedGraph> res;

	// Parse File
	std::string line;
	std::string word;

	while (std::getline(file, line))
	{
		// Parse Header
		size_t num_vertices;
		Color max_color;

		int temp;
		std::stringstream ss(line);

		ss >> temp;
		num_vertices = static_cast<size_t>(temp);

		ss >> temp;
		max_color = static_cast<Color>(temp);

		// Read File line by line to construct graph
		size_t i = 0;
		EdgeColoredUndirectedGraph g(num_vertices, max_color);
		while (std::getline(file, line)) 
		{
			// Read line for this vertex's edge colors
			ss = std::stringstream(line);
			size_t j = 0;
			bool is_end = true;
			while (ss >> word) 
			{
				is_end = false;
				Color color = static_cast<Color>(std::stoi(word));

				g.setEdge(i, j, color);
				++j;
			}
			++i;

			// Blank line marks new graph
			if (is_end)
			{
				res.emplace_back(std::move(g));
				break;
			}
		}
	}

	return res;
}

