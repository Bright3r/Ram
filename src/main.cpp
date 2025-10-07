#include <filesystem>
#define MAXN (17*3)
#include "nauty2_9_1/nauty.h"

#include <cstdio>
#include <array>
#include <vector>
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>


// Setup colors
constexpr int Uncolored = 0;
constexpr int Red = 1;
constexpr int Blue = 2;
constexpr int Green = 3;
constexpr int Purple = 4;


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
			initThreads(e);
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
				if (c0 == 0) continue;

				for (int k = j+1; k < NumVertices; ++k)
				{
					int c1 = getEncodedEdge(i, k);
					if (c1 == 0) continue;

					int c2 = getEncodedEdge(j, k);
					if (c2 == 0) continue;

					if (c0 == c1 && c0 == c2 && c1 == c2) 
					{
						std::printf(
							"i=%d,j=%d,k=%d, c0=c1=c2 = %d\n", 
							i, j, k, c0
						);
						return false;
					}
				}
			}
		}

		return true;
	}

	std::vector<InternalGraph> getWeakIsomorphs() noexcept
	{
		std::vector<InternalGraph> res;

		return res;
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
	void initThreads(int e) noexcept
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

	int getEncodedEdge(int i, int j) noexcept
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
			if (i == j)
			{
				++j;
				continue;
			}

			int color = std::stoi(word);
			g.setEdge(i, j, color);
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

void printCanong(graph* cg, int n, int m) noexcept
{
	for (int i = 0; i < n*m; ++i)
	{
		std::printf("%lul", cg[i]);
	}
	std::printf("\n");
}

};	// end of namespace


int main(int argc, char** argv)
{
	// Make sure T1 and T2 are triangle-free
	auto t1 = Ram::make_T1();
	if (t1.isTriangleFree())
	{
		std::printf("No Triangles in T1.\n");
	}
	else std::printf("Triangle found in T1!\n");

	auto t2 = Ram::make_T2();
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

	if (Ram::isIsomorphic(canong_t1, canong_t2, n, m))
	{
		std::printf("T1 and T2 are isomorphic.\n");
	}
	else
	{
		std::printf("T1 and T2 are non-isomorphic.\n");
	}

	std::printf("\nCanonical Labeling of T1: \n");
	Ram::printCanong(canong_t1, n, m);

	std::printf("\nCanonical Labeling of T2: \n");
	Ram::printCanong(canong_t2, n, m);

	return 0;
}

