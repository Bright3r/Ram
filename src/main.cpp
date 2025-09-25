#define MAXN (17*3)
#include "nauty2_9_1/nauty.h"

#include <cstdio>
#include <array>
#include <cassert>
#include <memory>

using NautyGraph = graph;

namespace Ram {

consteval int log2_ceil(int x)
{
	int i { 0 };
	while (x > 0)
	{
		++i;
		x >>= 1;
	}
	return i;
}

template <size_t NumVertices, size_t NumColors>
struct EdgeColoredGraph 
{
	static constexpr int EncodingSize = log2_ceil(NumColors);

	// Corresponds to n in nauty
	static constexpr int GraphSize = NumVertices * EncodingSize;
	// Corresponds to m in nauty
	static constexpr int VertexWords = SETWORDSNEEDED(GraphSize);

	std::array<std::array<bool, GraphSize>, GraphSize> m_graph { false };

	EdgeColoredGraph() noexcept
	{
		// Create a path between vertical threads of color encoding vertices
		for (int e = 0; e < NumVertices; ++e)
		{
			initThreads(e);
		}
	}

	void setEdge(int i, int j, int color) noexcept
	{
		assert(i < NumVertices && j < NumVertices 
			&& "Invalid bounds on EdgeColoredGraph::addEdge()"
		);

		setEncodedEdge(i, j, color);
	}

	std::unique_ptr<NautyGraph> nautify() const noexcept
	{
		auto g = std::make_unique<NautyGraph>(GraphSize*VertexWords);
		EMPTYGRAPH(g.get(), VertexWords, GraphSize);

		for (int i = 0; i < GraphSize; ++i)
		{
			for (int j = i+1; j < GraphSize; ++j)
			{
				if (m_graph[i][j]) 
					ADDONEEDGE(g.get(), i, j, VertexWords);
			}
		}
		return g;
	}

private:
	void initThreads(int e) noexcept
	{
		int e_base = e * EncodingSize;
		for (int c1 = 0; c1 < EncodingSize-1; ++c1)
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

			int color_bit = color & 0x1;
			m_graph[i_encoded][j_encoded] = color_bit;
			m_graph[j_encoded][i_encoded] = color_bit;
			color >>= 1;
		}
	}
};

};	// end of namespace


// Setup colors
constexpr int Uncolored = 0;
constexpr int Red = 1;
constexpr int Blue = 2;
constexpr int Green = 3;
constexpr int Purple = 4;

int main(int argc, char** argv)
{
	// Test graph
	constexpr int NumVertices = 2;
	constexpr int NumColors = 4;
	using MyGraph = Ram::EdgeColoredGraph<NumVertices, NumColors>;
	MyGraph g;
	g.setEdge(0, 1, Red);

	static constexpr int n = MyGraph::GraphSize;
	static constexpr int m = MyGraph::VertexWords;
	nauty_check(WORDSIZE, m, n, NAUTYVERSIONID);
	std::printf("N=%d, M=%d\n", n, m);

	// Setup dense nauty
	auto nauty_g = g.nautify();
	int lab[n], ptn[n], orbits[n];
	DEFAULTOPTIONS_GRAPH(options);
	statsblk stats;

	for (int i = 0; i < n; ++i)
	{
		lab[i] = i;
		ptn[i] = (i+1 < n) ? 1 : 0;
	}

	densenauty(nauty_g.get(), lab, ptn, orbits, &options, &stats, m, n, NULL);
	std::printf("Automorphism group size: %lu\n", (unsigned long) stats.grpsize1);

	return 0;
}

