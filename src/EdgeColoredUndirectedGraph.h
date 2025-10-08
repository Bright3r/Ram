#pragma once

#include <cstdio>
#include <array>
#include <vector>
#include <cassert>
#include <algorithm>
#include <numeric>

#define MAXN (17*3)
#include "nauty.h"

namespace Ram {

// Calculates the number of bits needed to represent 0-x in binary
// Ex: x=4 requires 3 bits (range 0-4 --> binary range of 000-100)
consteval int bits_needed_for_max_value(int x)
{
	int i { 0 };
	while (x > 0)
	{
		++i;
		x >>= 1;
	}

	return i;
}

template <size_t NumVertices, size_t MaxColor>
struct EdgeColoredUndirectedGraph 
{
	using InternalGraph = EdgeColoredUndirectedGraph<NumVertices, MaxColor>;

	// Corresponds to number of bits needed to represent the number of colors in binary
	// Details found in edge-colored encoding mechanism of Nauty manual
	static constexpr int EncodingSize = bits_needed_for_max_value(MaxColor);

	// Corresponds to n in nauty
	static constexpr int GraphSize = NumVertices * EncodingSize;

	// Corresponds to m in nauty
	static constexpr int VertexWords = SETWORDSNEEDED(GraphSize);

	// Type to be used when interacting with nauty
	using NautyGraph = std::array<setword, GraphSize*VertexWords>;



	std::array<std::array<bool, GraphSize>, GraphSize> m_graph { false };

	EdgeColoredUndirectedGraph() noexcept
	{
		// Create a path between vertical threads of color encoding vertices
		for (int e = 0; e < NumVertices; ++e)
		{
			createEncodingThreads(e);
		}
	}

	void setEdge(int i, int j, int color) noexcept
	{
		assert(i >= 0 && i < NumVertices && j >= 0 && j < NumVertices 
			&& "Invalid bounds on EdgeColoredGraph::setEdge()"
		);

		setEncodedEdge(i, j, color);
	}

	int getEdge(int i, int j) noexcept
	{
		assert(i >= 0 && i < NumVertices && j >= 0 && j < NumVertices 
			&& "Invalid bounds on EdgeColoredGraph::getEdge()"
		);

		return getEncodedEdge(i, j);
	}

	bool isTriangleFree() noexcept
	{
		for (int i = 0; i < NumVertices; ++i)
		{
			for (int j = i+1; j < NumVertices; ++j)
			{
				int c0 = getEncodedEdge(i, j);

				for (int k = j+1; k < NumVertices; ++k)
				{
					int c1 = getEncodedEdge(i, k);
					int c2 = getEncodedEdge(j, k);
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
		for (int i = 0; i < NumVertices; ++i)
		{
			for (int j = i+1; j < NumVertices; ++j)
			{
				int c0 = getEncodedEdge(i, j);

				for (int k = j+1; k < NumVertices; ++k)
				{
					int c1 = getEncodedEdge(i, k);
					int c2 = getEncodedEdge(j, k);
					if (c0 == 0 || c1 == 0 || c2 == 0) 
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	std::vector<InternalGraph> getWeakIsomorphs(int max_color = MaxColor) const noexcept
	{
		std::vector<int> colors(max_color, 0);
		std::iota(colors.begin(), colors.end(), 1);

		std::vector<InternalGraph> res;
		do
		{
			InternalGraph g;
			for (int i = 0; i < NumVertices; ++i)
			{
				for (int j = i+1; j < NumVertices; ++j)
				{
					int ec = getEncodedEdge(i, j);
					if (ec == 0) continue;

					int mapped_color = (ec > max_color) 
						? ec 
						: colors[ec-1];

					g.setEncodedEdge(i, j, mapped_color);
				}
			}
			res.push_back(g);
		} while (std::next_permutation(colors.begin(), colors.end()));

		return res;
	}

	void print() const noexcept
	{
		for (int i = 0; i < NumVertices; ++i)
		{
			for (int j = 0; j < NumVertices; ++j)
			{
				std::printf("%d ", getEncodedEdge(i, j));
			}
			std::printf("\n");
		}
	}

	NautyGraph nautify() const noexcept
	{
		NautyGraph g {};
		EMPTYGRAPH(g.data(), VertexWords, GraphSize);

		for (int i = 0; i < GraphSize; ++i)
		{
			for (int j = i+1; j < GraphSize; ++j)
			{
				if (m_graph[i][j]) ADDONEEDGE(g.data(), i, j, VertexWords);
			}
		}

		return g;
	}

private:
	void createEncodingThreads(int e) noexcept
	{
		int e_base = e * EncodingSize;
		for (int c1 = 0; c1 < EncodingSize; ++c1)
		{
			int e1 = e_base + c1;
			for (int c2 = 0; c2 < EncodingSize; ++c2)
			{
				if (c1 == c2) continue;

				int e2 = e_base + c2;
				m_graph[e1][e2] = true;
			}
		}
	}

	void setEncodedEdge(int i, int j, int color) noexcept
	{
		int i_base = i * EncodingSize;
		int j_base = j * EncodingSize;
		for (int color_offset = 0; color_offset < EncodingSize; ++color_offset)
		{
			int i_encoded = i_base + color_offset;
			int j_encoded = j_base + color_offset;

			int color_bit = (color >> color_offset) & 1;
			m_graph[i_encoded][j_encoded] = color_bit;
			m_graph[j_encoded][i_encoded] = color_bit;
		}
	}

	int getEncodedEdge(int i, int j) const noexcept
	{
		int color = 0;

		int i_base = i * EncodingSize;
		int j_base = j * EncodingSize;
		for (int color_offset = EncodingSize-1; color_offset >= 0; --color_offset)
		{
			int i_encoded = i_base + color_offset;
			int j_encoded = j_base + color_offset;

			int color_bit = m_graph[i_encoded][j_encoded] << color_offset;
			color |= color_bit;
		}

		return color;
	}
};

};	// end of namespace

