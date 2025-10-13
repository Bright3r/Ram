#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include <algorithm>
#include <numeric>

#define MAXN (17*4)
#include "nauty.h"

namespace Ram {

struct EdgeColoredUndirectedGraph 
{
	// Type to be used when interacting with nauty
	using NautyGraph = std::vector<setword>;


	std::vector<std::vector<bool>> graph;
	int num_vertices;
	int max_color;
	int num_layers;

	EdgeColoredUndirectedGraph(int num_vertices, int max_color) noexcept
		: num_vertices(num_vertices)
		, max_color(max_color)
	{
		num_layers = numLayersForMaxColor(max_color);

		graph = std::vector<std::vector<bool>>(
			numEncodedVertices(), 
			std::vector<bool>(numEncodedVertices(), false)
		);

		// Create a clique between vertical threads of color encoding vertices
		for (int e = 0; e < num_vertices; ++e)
		{
			createEncodingThreads(e);
		}
	}

	int numEncodedVertices() const noexcept
	{
		return num_vertices * num_layers;
	}

	int numWordsPerVertex() const noexcept
	{
		return SETWORDSNEEDED(numEncodedVertices());
	}

	void addVertex() noexcept
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

	void setEdge(int i, int j, int color) noexcept
	{
		assert(i >= 0 && i < num_vertices && j >= 0 && j < num_vertices 
			&& "Invalid bounds on EdgeColoredGraph::setEdge()"
		);

		int i_base = i * num_layers;
		int j_base = j * num_layers;

		// Remove edges of other colors
		for (int l = 0; l < num_layers; ++l)
		{
			int i_encoded = i_base + l;
			int j_encoded = j_base + l;

			graph[i_encoded][j_encoded] = false;
			graph[j_encoded][i_encoded] = false;
		}

		// Add edge of desired color
		if (color <= 0) return;

		// Encode color as binary tree representation of color's integer
		for (int l = 0; l < num_layers; ++l)
		{
			int i_encoded = i_base + l;
			int j_encoded = j_base + l;

			bool bit_value = (color >> l) & 0x1;
			graph[i_encoded][j_encoded] = bit_value;
			graph[j_encoded][i_encoded] = bit_value;
		}
	}

	int getEdge(int i, int j) const noexcept
	{
		assert(i >= 0 && i < num_vertices && j >= 0 && j < num_vertices 
			&& "Invalid bounds on EdgeColoredGraph::getEdge()"
		);

		int i_base = i * num_layers;
		int j_base = j * num_layers;

		int c = 0;
		for (int l = 0; l < num_layers; ++l)
		{
			int i_encoded = i_base + l;
			int j_encoded = j_base + l;

			bool bit_value = graph[i_encoded][j_encoded];
			c |= (bit_value << l);
		}

		return c;
	}

	bool isTriangleFree() noexcept
	{
		for (int i = 0; i < num_vertices; ++i)
		{
			for (int j = i+1; j < num_vertices; ++j)
			{
				int c0 = getEdge(i, j);
				for (int k = j+1; k < num_vertices; ++k)
				{
					int c1 = getEdge(i, k);
					int c2 = getEdge(j, k);
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

	bool isPartial() noexcept
	{
		for (int i = 0; i < num_vertices; ++i)
		{
			for (int j = i+1; j < num_vertices; ++j)
			{
				int c0 = getEdge(i, j);
				for (int k = j+1; k < num_vertices; ++k)
				{
					int c1 = getEdge(i, k);
					int c2 = getEdge(j, k);
					if (c0 == 0 || c1 == 0 || c2 == 0) 
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	std::vector<EdgeColoredUndirectedGraph> getColorPermutations(int max_color = -1) const noexcept
	{
		if (max_color < 0) max_color = this->max_color;
		std::vector<int> colors(max_color, 0);
		std::iota(colors.begin(), colors.end(), 1);

		std::vector<EdgeColoredUndirectedGraph> res;
		do
		{
			EdgeColoredUndirectedGraph g(num_vertices, this->max_color);
			for (int i = 0; i < num_vertices; ++i)
			{
				for (int j = i+1; j < num_vertices; ++j)
				{
					int ec = getEdge(i, j);
					if (ec == 0) continue;

					// Use encoded color value as index into color permutation
					int mapped_color = (ec > max_color) 
						? ec 
						: colors[ec-1];

					g.setEdge(i, j, mapped_color);
				}
			}
			res.push_back(g);
		} while (std::next_permutation(colors.begin(), colors.end()));

		return res;
	}

	std::string to_string() const noexcept
	{
		std::stringstream ss;
		for (int i = 0; i < num_vertices; ++i)
		{
			for (int j = 0; j < num_vertices; ++j)
			{
				ss << getEdge(i, j) << " ";
			}
			ss << "\n";
		}
		return ss.str();
	}

	NautyGraph nautify() const noexcept
	{
		NautyGraph g(numEncodedVertices()*numWordsPerVertex());
		EMPTYGRAPH(g.data(), numEncodedVertices(), numWordsPerVertex());

		for (int i = 0; i < numEncodedVertices(); ++i)
		{
			for (int j = i+1; j < numEncodedVertices(); ++j)
			{
				if (graph[i][j]) ADDONEEDGE(g.data(), i, j, numWordsPerVertex());
			}
		}

		return g;
	}

private:
	int numLayersForMaxColor(int max_color) const noexcept
	{
		// Get the number of bits used for the binary representation of max_color
		int num_layers = 0;
		while (max_color > 0)
		{
			++num_layers;
			max_color >>= 1;
		}
		return num_layers;
	}

	void createEncodingThreads(int v) noexcept
	{
		int v_base = v * num_layers;
		for (int l0 = 0; l0 < num_layers; ++l0)
		{
			for (int l1 = l0+1; l1 < num_layers; ++l1)
			{
				int v0 = v_base + l0;
				int v1 = v_base + l1;
				graph[v0][v1] = true;
				graph[v1][v0] = true;
			}
		}
	}
};

};	// end of namespace

