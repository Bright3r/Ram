#pragma once

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <vector>
#include <cassert>
#include <string>

#include "nauty.h"

namespace Ram {

using Color = uint8_t;
using Vertex = uint64_t;

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

	ColoringGenerator(size_t num_edges, size_t num_colors)
		: num_edges(num_edges)
		, num_colors(num_colors)
		, coloring(num_edges, 1)
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


struct EdgeColoredUndirectedGraph 
{
	// Type to be used when interacting with nauty
	using NautyGraph = std::vector<setword>;

	std::vector<std::vector<bool>> graph;
	size_t num_vertices;
	size_t num_layers;
	Color max_color;


	EdgeColoredUndirectedGraph(size_t num_vertices, Color max_color) noexcept;

	size_t numEncodedVertices() const noexcept;

	size_t numWordsPerVertex() const noexcept;

	void addVertex() noexcept;

	void setEdge(Vertex i, Vertex j, Color color) noexcept;

	Color getEdge(Vertex i, Vertex j) const noexcept;

	bool hasEdge(Vertex i, Vertex j) const noexcept;

	std::string header_string() const noexcept;

	std::string to_string() const noexcept;

private:
	size_t numLayersForMaxColor(Color max_color) const noexcept;

	void createEncodingThreads(Vertex v) noexcept;
};

};	// end of namespace

