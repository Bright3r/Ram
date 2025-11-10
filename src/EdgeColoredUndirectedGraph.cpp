#include "EdgeColoredUndirectedGraph.h"
#include "Utils.h"

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


void EdgeColoredUndirectedGraph::setEdge(Vertex i, Vertex j, Color color) noexcept
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


Color EdgeColoredUndirectedGraph::getEdge(Vertex i, Vertex j) const noexcept
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


bool EdgeColoredUndirectedGraph::hasEdge(Vertex i, Vertex j) const noexcept
{
	return getEdge(i, j) != 0;
}


std::string EdgeColoredUndirectedGraph::header_string() const noexcept
{
	std::stringstream ss;
	ss << static_cast<int>(num_vertices) 
		<< " " 
		<< static_cast<int>(max_color);

	return ss.str();
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


size_t EdgeColoredUndirectedGraph::numLayersForMaxColor(Color max_color) const noexcept
{
	// Get the number of bits used for the binary representation of max_color
	return numBitsInBinary(max_color);
}


void EdgeColoredUndirectedGraph::createEncodingThreads(Vertex v) noexcept
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

};	// end of namespace

