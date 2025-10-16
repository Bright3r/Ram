#include "EdgeColoredUndirectedGraph.h"

#include <numeric>
#include <utility>
#include <fstream>
#include <sstream>


namespace Ram {

EdgeColoredUndirectedGraph::EdgeColoredUndirectedGraph(size_t num_vertices, Color max_color) noexcept
	: num_vertices(num_vertices)
	, max_color(max_color)
{
	num_layers = numLayersForMaxColor(max_color);

	graph = std::vector<std::vector<bool>>(
		numEncodedVertices(), 
		std::vector<bool>(numEncodedVertices(), false)
	);

	// Create a clique between vertical threads of color encoding vertices
	for (auto e = 0; e < num_vertices; ++e)
	{
		createEncodingThreads(e);
	}
}

size_t EdgeColoredUndirectedGraph::numEncodedVertices() const noexcept
{
	return num_vertices * num_layers;
}

size_t EdgeColoredUndirectedGraph::numWordsPerVertex() const noexcept
{
	return SETWORDSNEEDED(numEncodedVertices());
}

void EdgeColoredUndirectedGraph::addVertex() noexcept
{
	auto old_size = numEncodedVertices();
	++num_vertices;
	auto new_size = numEncodedVertices();

	for (auto& row : graph)
	{
		row.resize(new_size, false);
	}

	graph.insert(
		graph.end(),
		new_size - old_size,
		std::vector<bool>(new_size, false)
	);

	createEncodingThreads(num_vertices-1);
}

void EdgeColoredUndirectedGraph::setEdge(size_t i, size_t j, Color color) noexcept
{
	assert(i >= 0 && i < num_vertices && j >= 0 && j < num_vertices 
		&& "Invalid bounds on EdgeColoredGraph::setEdge()"
	);

	auto i_base = i * num_layers;
	auto j_base = j * num_layers;

	// Remove edges of other colors
	for (auto l = 0; l < num_layers; ++l)
	{
		auto i_encoded = i_base + l;
		auto j_encoded = j_base + l;

		graph[i_encoded][j_encoded] = false;
		graph[j_encoded][i_encoded] = false;
	}

	// Add edge of desired color
	if (color <= 0) return;

	// Encode color as binary tree representation of color's integer
	for (auto l = 0; l < num_layers; ++l)
	{
		auto i_encoded = i_base + l;
		auto j_encoded = j_base + l;

		bool bit_value = (color >> l) & 0x1;
		graph[i_encoded][j_encoded] = bit_value;
		graph[j_encoded][i_encoded] = bit_value;
	}
}

Color EdgeColoredUndirectedGraph::getEdge(size_t i, size_t j) const noexcept
{
	assert(i < num_vertices && j < num_vertices 
		&& "Invalid bounds on EdgeColoredGraph::getEdge()"
	);

	auto i_base = i * num_layers;
	auto j_base = j * num_layers;

	Color c = 0;
	for (auto l = 0; l < num_layers; ++l)
	{
		auto i_encoded = i_base + l;
		auto j_encoded = j_base + l;

		bool bit_value = graph[i_encoded][j_encoded];
		c |= (bit_value << l);
	}

	return c;
}

bool EdgeColoredUndirectedGraph::isTriangleFree() noexcept
{
	for (auto i = 0; i < num_vertices; ++i)
	{
		for (auto j = i+1; j < num_vertices; ++j)
		{
			Color c0 = getEdge(i, j);
			for (auto k = j+1; k < num_vertices; ++k)
			{
				Color c1 = getEdge(i, k);
				Color c2 = getEdge(j, k);
				if (c0 == 0 || c1 == 0 || c2 == 0) 
				{
					continue;
				}

				if (c0 == c1 && c0 == c2 && c1 == c2) 
				{
					return false;
				}
			}
		}
	}

	return true;
}

bool EdgeColoredUndirectedGraph::isPartial() noexcept
{
	for (auto i = 0; i < num_vertices; ++i)
	{
		for (auto j = i+1; j < num_vertices; ++j)
		{
			Color c0 = getEdge(i, j);
			for (auto k = j+1; k < num_vertices; ++k)
			{
				Color c1 = getEdge(i, k);
				Color c2 = getEdge(j, k);
				if (c0 == 0 || c1 == 0 || c2 == 0) 
				{
					return true;
				}
			}
		}
	}

	return false;
}

std::vector<EdgeColoredUndirectedGraph> 
EdgeColoredUndirectedGraph::getColorPermutations(int max_color) const noexcept
{
	if (max_color < 0) max_color = this->max_color;
	std::vector<Color> colors(max_color, 0);
	std::iota(colors.begin(), colors.end(), 1);

	std::vector<EdgeColoredUndirectedGraph> res;
	do
	{
		EdgeColoredUndirectedGraph g(num_vertices, this->max_color);
		for (auto i = 0; i < num_vertices; ++i)
		{
			for (auto j = i+1; j < num_vertices; ++j)
			{
				Color ec = getEdge(i, j);
				if (ec == 0) continue;

				// Use encoded color value as index into color permutation
				Color mapped_color = (ec > max_color) 
					? ec 
					: colors[ec-1];

				g.setEdge(i, j, mapped_color);
			}
		}
		res.push_back(g);
	} while (std::next_permutation(colors.begin(), colors.end()));

	return res;
}

std::string EdgeColoredUndirectedGraph::to_string() const noexcept
{
	std::stringstream ss;
	for (auto i = 0; i < num_vertices; ++i)
	{
		for (auto j = 0; j < num_vertices; ++j)
		{
			ss << static_cast<int>(getEdge(i, j)) << " ";
		}
		ss << "\n";
	}
	return ss.str();
}

EdgeColoredUndirectedGraph::NautyGraph EdgeColoredUndirectedGraph::nautify() const noexcept
{
	NautyGraph g(numEncodedVertices()*numWordsPerVertex());
	EMPTYGRAPH(g.data(), numEncodedVertices(), numWordsPerVertex());

	for (auto i = 0; i < numEncodedVertices(); ++i)
	{
		for (auto j = i+1; j < numEncodedVertices(); ++j)
		{
			if (graph[i][j]) ADDONEEDGE(g.data(), i, j, numWordsPerVertex());
		}
	}

	return g;
}

size_t EdgeColoredUndirectedGraph::numLayersForMaxColor(Color max_color) const noexcept
{
	// Get the number of bits used for the binary representation of max_color
	size_t num_layers = 0;
	while (max_color > 0)
	{
		++num_layers;
		max_color >>= 1;
	}
	return num_layers;
}

void EdgeColoredUndirectedGraph::createEncodingThreads(size_t v) noexcept
{
	auto v_base = v * num_layers;
	for (auto l0 = 0; l0 < num_layers; ++l0)
	{
		for (auto l1 = l0+1; l1 < num_layers; ++l1)
		{
			auto v0 = v_base + l0;
			auto v1 = v_base + l1;
			graph[v0][v1] = true;
			graph[v1][v0] = true;
		}
	}
}


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

};	// end of namespace

