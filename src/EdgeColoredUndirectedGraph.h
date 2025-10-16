#pragma once

#include <chrono>
#include <cstdio>
#include <ctime>
#include <vector>
#include <cassert>
#include <string>
#include <filesystem>

#include "nauty.h"

namespace Ram {

using Color = uint8_t;

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

	void setEdge(size_t i, size_t j, Color color) noexcept;

	Color getEdge(size_t i, size_t j) const noexcept;

	bool isTriangleFree() noexcept;

	bool isPartial() noexcept;

	std::vector<EdgeColoredUndirectedGraph> 
	getColorPermutations(int max_color = -1) const noexcept;

	std::string to_string() const noexcept;

	NautyGraph nautify() const noexcept;

private:
	size_t numLayersForMaxColor(Color max_color) const noexcept;

	void createEncodingThreads(size_t v) noexcept;
};


EdgeColoredUndirectedGraph load_adj(
	std::filesystem::path filename,
	size_t num_vertices,
	Color max_color) noexcept;

std::vector<EdgeColoredUndirectedGraph> load_bulk(
	std::filesystem::path filename,
	size_t num_vertices,
	Color max_color) noexcept;

};	// end of namespace

