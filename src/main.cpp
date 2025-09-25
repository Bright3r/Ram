#define MAXN 64
#include "nauty2_9_1/nauty.h"

#include <cstdio>
#include <array>
#include <cassert>

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
class EdgeColoredGraph 
{
	static constexpr int EncodingSize = log2_ceil(NumColors);
	static constexpr int NumEncodingVertices = EncodingSize * NumVertices;
	static constexpr int GraphSize = NumEncodingVertices;

public:
	std::array<std::array<int, GraphSize>, GraphSize> graph {};

	EdgeColoredGraph() noexcept
	{
		// Create a clique between vertical threads of color encoding vertices
		for (int e = 0; e < NumVertices; ++e)
		{
			setTheadClique(e);
		}
	}

	void setEdge(int i, int j, int color) noexcept
	{
		assert(i < NumVertices && j < NumVertices 
			&& "Invalid bounds on EdgeColoredGraph::addEdge()"
		);

		setEncodedEdge(i, j, color);
	}

private:
	void setTheadClique(int e) noexcept
	{
		int e_base = e * EncodingSize;
		for (int c1 = 0; c1 < EncodingSize; ++c1)
		{
			int e1 = e_base + c1;
			for (int c2 = 0; c2 < EncodingSize; ++c2)
			{
				if (c1 == c2) continue;

				int e2 = e_base + c2;
				graph[e1][e2] = 1;
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
			graph[i_encoded][j_encoded] = color_bit;
			graph[j_encoded][i_encoded] = color_bit;
			color >>= 1;
		}
	}
};

};	// end of namespace

int main(int argc, char** argv)
{
	int n = 62;
	int m = SETWORDSNEEDED(n);
	nauty_check(WORDSIZE, m, n, NAUTYVERSIONID);

	std::printf("N=%d, M=%d", n, m);

	Ram::EdgeColoredGraph<62, 4> g;

	return 0;
}

