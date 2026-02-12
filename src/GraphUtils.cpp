#include "GraphUtils.h"
#include "EdgeColoredUndirectedGraph.h"
#include "Utils.h"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <chrono>
#include <functional>
#include <numeric>

namespace Ram
{

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


std::string canonize(const EdgeColoredUndirectedGraph& g) noexcept
{
	std::string canon_str = "";
	for (auto& weak_g : getColorPermutations(g))
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
		auto nauty_g = nautify(weak_g);
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


EdgeColoredUndirectedGraph::NautyGraph nautify(const EdgeColoredUndirectedGraph& g) noexcept
{
	EdgeColoredUndirectedGraph::NautyGraph ng(g.numEncodedVertices()*g.numWordsPerVertex());
	EMPTYGRAPH(ng.data(), g.numEncodedVertices(), g.numWordsPerVertex());

	for (auto i = 0; i < g.numEncodedVertices(); ++i)
	{
		for (auto j = i+1; j < g.numEncodedVertices(); ++j)
		{
			if (g.graph[i][j]) ADDONEEDGE(ng.data(), i, j, g.numWordsPerVertex());
		}
	}

	return ng;
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


bool isTriangleFree(const EdgeColoredUndirectedGraph& g) noexcept
{
	for (auto i = 0; i < g.num_vertices; ++i)
	{
		for (auto j = i+1; j < g.num_vertices; ++j)
		{
			Color c0 = g.getEdge(i, j);
			for (auto k = j+1; k < g.num_vertices; ++k)
			{
				Color c1 = g.getEdge(i, k);
				Color c2 = g.getEdge(j, k);
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


bool isPartial(const EdgeColoredUndirectedGraph& g) noexcept
{
	for (auto i = 0; i < g.num_vertices; ++i)
	{
		for (auto j = i+1; j < g.num_vertices; ++j)
		{
			Color c0 = g.getEdge(i, j);
			for (auto k = j+1; k < g.num_vertices; ++k)
			{
				Color c1 = g.getEdge(i, k);
				Color c2 = g.getEdge(j, k);
				if (c0 == 0 || c1 == 0 || c2 == 0) 
				{
					return true;
				}
			}
		}
	}

	return false;
}


std::vector<EdgeColoredUndirectedGraph> getColorPermutations(
	const EdgeColoredUndirectedGraph& g,
	int max_color) noexcept
{
	if (max_color < 0) max_color = g.max_color;
	std::vector<Color> colors(max_color, 0);
	std::iota(colors.begin(), colors.end(), 1);

	std::vector<EdgeColoredUndirectedGraph> res;
	do
	{
		EdgeColoredUndirectedGraph gc(g.num_vertices, max_color);
		for (auto i = 0; i < g.num_vertices; ++i)
		{
			for (auto j = i+1; j < g.num_vertices; ++j)
			{
				Color ec = g.getEdge(i, j);
				if (ec == 0) continue;

				// Use encoded color value as index into color permutation
				Color mapped_color = (ec > max_color) 
					? ec 
					: colors[ec-1];

				gc.setEdge(i, j, mapped_color);
			}
		}
		res.push_back(gc);
	} while (std::next_permutation(colors.begin(), colors.end()));

	return res;
}


//
// Embeddability
//
std::vector<Embedding> embed(
	const EdgeColoredUndirectedGraph& subgraph,
	const EdgeColoredUndirectedGraph& graph) noexcept
{
	std::vector<Embedding> embeddings;

	// Cannot embed subgraph into smaller graph
	if (subgraph.num_vertices > graph.num_vertices) return embeddings;

	Embedding map_sub_to_main(subgraph.num_vertices, -1);
	std::vector<bool> used_main(graph.num_vertices, false);
	std::function<void(int)> VF2_dfs = [&](int depth)
	{
		// Full embedding found
		if (depth == subgraph.num_vertices)
		{
			// Construct overlapping subgraph
			EdgeColoredUndirectedGraph overlap(graph.num_vertices, subgraph.max_color);

			// Copy vertices of subgraph into overlap
			for (auto i = 0; i < subgraph.num_vertices; ++i)
			{
				// Find corresponding vertex in main graph
				auto vi_main = map_sub_to_main[i];

				// Copy the edges from vertices in subgraph
				// as edges from vertices of main graph in overlap
				for (auto j = i+1; j < subgraph.num_vertices; ++j)
				{
					auto vj_main = map_sub_to_main[j];
					overlap.setEdge(vi_main, vj_main, subgraph.getEdge(i, j));
				}
			}

			embeddings.emplace_back(map_sub_to_main);
			return;
		}

		// Find a vertex in subgraph that can map to main graph
		auto v_sub = depth;
		for (auto v_main = 0; v_main < graph.num_vertices; ++v_main)
		{
			if (used_main[v_main]) continue;

			// Check that edge colors agree with previously mapped vertices
			bool ok = true;
			for (auto prev = 0; prev < depth; ++prev)
			{
				auto v_sub_prev = prev;
				auto v_main_prev = map_sub_to_main[v_sub_prev];
				if (v_main_prev == -1) continue;

				auto c_sub = subgraph.getEdge(v_sub, v_sub_prev);
				auto c_main = graph.getEdge(v_main, v_main_prev);
				if (c_sub != 0 && c_sub != c_main)
				{
					ok = false;
					break;
				}
			}

			if (!ok) continue;

			// Found valid mapping
			map_sub_to_main[v_sub] = v_main;
			used_main[v_main] = true;

			// Recurse then continue
			VF2_dfs(depth + 1);
			used_main[v_main] = false;
			map_sub_to_main[v_sub] = -1;
		}
	};

	VF2_dfs(0);
	return embeddings;
};


bool canEmbed(
	const EdgeColoredUndirectedGraph& subgraph,
	const EdgeColoredUndirectedGraph& graph) noexcept
{
	// Cannot embed subgraph into smaller graph
	if (subgraph.num_vertices > graph.num_vertices) return false;

	Embedding map_sub_to_main(subgraph.num_vertices, -1);
	std::vector<bool> used_main(graph.num_vertices, false);
	std::function<bool(int)> VF2_dfs = [&](int depth)
	{
		// Full embedding found
		if (depth == subgraph.num_vertices)
		{
			return true;
		}

		// Find a vertex in subgraph that can map to main graph
		auto v_sub = depth;
		for (auto v_main = 0; v_main < graph.num_vertices; ++v_main)
		{
			if (used_main[v_main]) continue;

			// Check that edge colors agree with previously mapped vertices
			bool ok = true;
			for (auto prev = 0; prev < depth; ++prev)
			{
				auto v_sub_prev = prev;
				auto v_main_prev = map_sub_to_main[v_sub_prev];
				if (v_main_prev == -1) continue;

				auto c_sub = subgraph.getEdge(v_sub, v_sub_prev);
				auto c_main = graph.getEdge(v_main, v_main_prev);
				if (c_sub != 0 && c_sub != c_main)
				{
					ok = false;
					break;
				}
			}

			if (!ok) continue;

			// Found valid mapping
			map_sub_to_main[v_sub] = v_main;
			used_main[v_main] = true;

			// Recurse then continue
			if (VF2_dfs(depth + 1)) return true;

			used_main[v_main] = false;
			map_sub_to_main[v_sub] = -1;
		}

		return false;
	};

	return VF2_dfs(0);
};

bool canEmbed(
	const EdgeColoredUndirectedGraph& subgraph,
	const std::vector<EdgeColoredUndirectedGraph>& graphs
) noexcept
{
	for (const auto& g : graphs)
	{
		if (canEmbed(subgraph, g))
		{
			return true;
		}
	}

	return false;
}


EdgeColoredUndirectedGraph getNeighborhood(
	const EdgeColoredUndirectedGraph& g,
	Vertex v,
	Color c) noexcept
{
	std::vector<Vertex> neighbors;
	for (auto u = 0; u < g.num_vertices; ++u)
	{
		if (u == v) continue;
		if (g.getEdge(u, v) == c)
		{
			neighbors.push_back(u);
		}
	}

	EdgeColoredUndirectedGraph neighborhood(neighbors.size(), g.max_color);
	for (auto i = 0; i < neighbors.size(); ++i)
	{
		for (auto j = i+1; j < neighbors.size(); ++j)
		{
			auto ec = g.getEdge(neighbors[i], neighbors[j]);
			neighborhood.setEdge(i, j, ec);
		}
	}

	return neighborhood;
}


EdgeColoredUndirectedGraph getNeighborhood(
	const EdgeColoredUndirectedGraph& g,
	std::vector<Vertex>& neighbors,
	Vertex v,
	Color c) noexcept
{
	for (auto u = 0; u < g.num_vertices; ++u)
	{
		if (u == v) continue;
		if (g.getEdge(u, v) == c)
		{
			neighbors.push_back(u);
		}
	}

	EdgeColoredUndirectedGraph neighborhood(neighbors.size(), g.max_color);
	for (auto i = 0; i < neighbors.size(); ++i)
	{
		for (auto j = i+1; j < neighbors.size(); ++j)
		{
			auto ec = g.getEdge(neighbors[i], neighbors[j]);
			neighborhood.setEdge(i, j, ec);
		}
	}

	return neighborhood;
}

//
// CNF
//
CNF getCNF(const EdgeColoredUndirectedGraph& g, bool add_colors) noexcept
{
	// Map edges to a variable
	// Indexed by (Vertex1, Vertex2, Color)
	std::vector<std::vector<std::vector<int>>> edge_to_var(
		g.num_vertices,
		std::vector<std::vector<int>>(g.num_vertices,
			std::vector<int>(g.max_color + 1, 0)
		)
	);

	int var = 1;
	for (auto i = 0; i < g.num_vertices; ++i)
	{
		for (auto j = i+1; j < g.num_vertices; ++j)
		{
			for (auto c = 1; c <= g.max_color; ++c)
			{
				edge_to_var[i][j][c] = var;
				edge_to_var[j][i][c] = var;
				++var;
			}
		}
	}


	// Each edge of one and only one color
	CNF cnf;
	for (auto i = 0; i < g.num_vertices; ++i)
	{
		for (auto j = i+1; j < g.num_vertices; ++j)
		{
			// Write color clause for current color
			for (auto c_curr = 1; c_curr <= g.max_color; ++c_curr)
			{
				// Edge must be at least one color
				std::stringstream clause;
				for (auto c = 1; c <= g.max_color; ++c)
				{
					clause << edge_to_var[i][j][c] << " ";
				}
				clause << "0";
				cnf.emplace_back(clause.str());

				// Edge must be at most one color
				for (auto c1 = 1; c1 <= g.max_color; ++c1)
				{
					for (auto c2 = c1+1; c2 <= g.max_color; ++c2)
					{
						clause = std::stringstream();
						clause << "-" << edge_to_var[i][j][c1] << " ";
						clause << "-" << edge_to_var[i][j][c2] << " ";
						clause << "0";
						cnf.emplace_back(clause.str());
					}
				}
			}
		}
	}
	

	// No monochromatic triangle
	for (auto i = 0; i < g.num_vertices; ++i)
	{
		for (auto j = i+1; j < g.num_vertices; ++j)
		{
			for (auto k = j+1; k < g.num_vertices; ++k)
			{
				for (auto c = 1; c <= g.max_color; ++c)
				{
					auto ec1 = edge_to_var[i][j][c];
					auto ec2 = edge_to_var[i][k][c];
					auto ec3 = edge_to_var[j][k][c];
					
					std::stringstream clause;
					clause << "-" << ec1 << " ";
					clause << "-" << ec2 << " ";
					clause << "-" << ec3 << " ";
					clause << "0";
					cnf.emplace_back(clause.str());
				}
			}
		}
	}


	// Add graph's coloring
	if (add_colors)
	{
		for (auto i = 0; i < g.num_vertices; ++i)
		{
			for (auto j = i+1; j < g.num_vertices; ++j)
			{
				if (!g.hasEdge(i, j)) continue;

				auto ec = g.getEdge(i, j);
				auto var = edge_to_var[i][j][ec];

				std::stringstream clause;
				clause << var << " 0";
				cnf.emplace_back(clause.str());
			}
		}
	}
	

	// Write header to start
	std::stringstream header;
	auto max_var = var - 1;
	auto num_clauses = cnf.size();
	header << "p cnf " << max_var << " " << num_clauses;
	cnf.insert(cnf.begin(), header.str());

	return cnf;
}


std::unique_ptr<CaDiCaL::Solver> getCNFSolver(
	const EdgeColoredUndirectedGraph& g,
	std::vector<std::vector<std::vector<int>>>& edge_to_var,
	bool add_colors) noexcept
{
	// Map edges to a variable
	// Indexed by (Vertex1, Vertex2, Color)
	edge_to_var = std::vector<std::vector<std::vector<int>>>(
		g.num_vertices,
		std::vector<std::vector<int>>(g.num_vertices,
			std::vector<int>(g.max_color + 1, 0)
		)
	);

	int var = 1;
	for (auto i = 0; i < g.num_vertices; ++i)
	{
		for (auto j = i+1; j < g.num_vertices; ++j)
		{
			for (auto c = 1; c <= g.max_color; ++c)
			{
				edge_to_var[i][j][c] = var;
				edge_to_var[j][i][c] = var;
				++var;
			}
		}
	}


	// Each edge of one and only one color
	auto solver = std::make_unique<CaDiCaL::Solver>();
	for (auto i = 0; i < g.num_vertices; ++i)
	{
		for (auto j = i+1; j < g.num_vertices; ++j)
		{
			// Write color clause for current color
			for (auto c_curr = 1; c_curr <= g.max_color; ++c_curr)
			{
				// Edge must be at least one color
				std::stringstream clause;
				for (auto c = 1; c <= g.max_color; ++c)
				{
					solver->add(edge_to_var[i][j][c]);
				}
				solver->add(0);

				// Edge must be at most one color
				for (auto c1 = 1; c1 <= g.max_color; ++c1)
				{
					for (auto c2 = c1+1; c2 <= g.max_color; ++c2)
					{
						solver->add(-edge_to_var[i][j][c1]);
						solver->add(-edge_to_var[i][j][c2]);
						solver->add(0);
					}
				}
			}
		}
	}
	

	// No monochromatic triangle
	for (auto i = 0; i < g.num_vertices; ++i)
	{
		for (auto j = i+1; j < g.num_vertices; ++j)
		{
			for (auto k = j+1; k < g.num_vertices; ++k)
			{
				for (auto c = 1; c <= g.max_color; ++c)
				{
					auto ec1 = edge_to_var[i][j][c];
					auto ec2 = edge_to_var[i][k][c];
					auto ec3 = edge_to_var[j][k][c];
					
					solver->add(-ec1);
					solver->add(-ec2);
					solver->add(-ec3);
					solver->add(0);
				}
			}
		}
	}


	// Add graph's coloring
	if (add_colors)
	{
		for (auto i = 0; i < g.num_vertices; ++i)
		{
			for (auto j = i+1; j < g.num_vertices; ++j)
			{
				if (!g.hasEdge(i, j)) continue;

				auto ec = g.getEdge(i, j);
				auto var = edge_to_var[i][j][ec];

				solver->assume(var);
			}
		}
	}
	
	return solver;
}


//
// IO
//
void writeCNFToFile(std::filesystem::path file_path, const CNF& cnf)
{
	std::ofstream of(file_path);
	for (const auto& str : cnf)
	{
		of << str << "\n";
	}
}



constexpr uint8_t EXTEND_GSIZE = 126;
constexpr uint64_t MAX_VERTS_SINGLE_BYTE = 62;
constexpr uint64_t MAX_VERTS_FOUR_BYTE = 258047;
constexpr uint64_t MAX_VERTS_EIGHT_BYTE = 68719476735;

constexpr uint8_t MCBIAS = 63;

uint64_t readMCGraphSize(const std::string& mc) noexcept
{
	auto byte0 = mc[0] - MCBIAS;
	auto byte1 = mc[1] - MCBIAS;

	auto start_graph_size = 0;
	auto end_graph_size = 0;
	if (byte0 == EXTEND_GSIZE && byte1 == EXTEND_GSIZE)
	{
		start_graph_size = 2;
		end_graph_size = 8;
	}
	else if (byte0 == EXTEND_GSIZE)
	{
		start_graph_size = 1;
		end_graph_size = 4;
	}

	uint64_t graph_size { 0 };
	auto shift_pos = 0;
	for (auto i = end_graph_size; i <= start_graph_size; ++i)
	{
		// Get 6-bit byte value
		uint64_t byte = (mc[i] - 63);
		
		// Shift byte into position of binary representation
		byte <<= shift_pos;
		shift_pos += 6;

		// OR byte into graph size
		graph_size |= byte;
	}

	return graph_size;
}

int readMCColors(const std::string& mc, uint64_t graph_size) noexcept
{
	auto byte_idx = 1;
	if (graph_size > MAX_VERTS_SINGLE_BYTE) byte_idx = 5;
	if (graph_size > MAX_VERTS_FOUR_BYTE) byte_idx = 9;

	int num_colors = mc[byte_idx] - MCBIAS;
	return num_colors;
}

EdgeColoredUndirectedGraph readMC(const std::string& mc) noexcept
{
	auto graph_size = readMCGraphSize(mc);
	auto max_color = readMCColors(mc, graph_size);
	auto color_bits = numBitsInBinary(max_color);
	EdgeColoredUndirectedGraph g(graph_size, max_color);

	// Get starting byte of adjacency matrix
	auto byte_idx = 2;
	if (graph_size > MAX_VERTS_SINGLE_BYTE) byte_idx = 6;
	if (graph_size > MAX_VERTS_FOUR_BYTE) byte_idx = 10;

	// Read rest of bytes into a vector of bits
	std::vector<bool> bits;
	for (auto i = byte_idx; i < mc.size(); ++i)
	{
		auto byte = mc[i] - MCBIAS;
		for (auto j = 5; j >= 0; --j)
		{
			auto bit = (byte >> j) & 0x1;
			bits.push_back(bit);
		}
	}

	// Read bits as a list of colors
	std::vector<Color> colors;
	for (auto i = 0; i < bits.size();)
	{
		Color color { 0 };
		for (auto b = 0; b < color_bits; ++b)
		{
			color <<= 1;
			color |= bits[i];
			++i;
		}

		colors.push_back(color);
	}

	// Read edges of graph from colors list
	auto c_idx = 0;
	for (auto j = 1; j < g.num_vertices; ++j)
	{
		for (auto i = 0; i < j; ++i)
		{
			g.setEdge(i, j, colors[c_idx]);
			++c_idx;
		}
	}

	return g;
}

std::vector<EdgeColoredUndirectedGraph> loadBulkMC(std::filesystem::path file_path)
{
	std::vector<EdgeColoredUndirectedGraph> graphs;

	std::ifstream file(file_path);
	std::string line;
	while (std::getline(file, line))
	{
		graphs.emplace_back(readMC(line));
	}
	
	return graphs;
}

std::string getGraphSizeMC(const EdgeColoredUndirectedGraph& g) noexcept
{
	std::string size_str {};

	uint64_t gsize = g.num_vertices;
	if (g.num_vertices <= MAX_VERTS_SINGLE_BYTE)
	{
		char byte = gsize + MCBIAS;
		size_str += byte;
	}
	else if (g.num_vertices <= MAX_VERTS_FOUR_BYTE)
	{
		size_str += EXTEND_GSIZE;
		for (auto i = 2; i >= 0; --i)
		{
			// Get 6-bit group
			auto bits = (gsize >> 6*i) & 0b111111;
			uint8_t byte = bits + MCBIAS;
			size_str += byte;
		}
	}
	else if (g.num_vertices <= MAX_VERTS_EIGHT_BYTE)
	{
		size_str += EXTEND_GSIZE;
		size_str += EXTEND_GSIZE;
		for (auto i = 6; i >= 0; --i)
		{
			// Get 6-bit group
			auto bits = (gsize >> 6*i) & 0b111111;
			uint8_t byte = bits + MCBIAS;
			size_str += byte;
		}
	}

	return size_str;
}

std::string getGraphNumColorsMC(const EdgeColoredUndirectedGraph& g) noexcept
{
	uint8_t byte = g.max_color + MCBIAS;

	std::string color;
	color += byte;
	return color;
}

std::string getGraphEdgeColorsMC(const EdgeColoredUndirectedGraph& g) noexcept
{
	auto color_bits = numBitsInBinary(g.max_color);

	// Get bit string of edge colors
	std::vector<bool> bits;
	for (auto j = 1; j < g.num_vertices; ++j)
	{
		for (auto i = 0; i < j; ++i)
		{
			auto color = g.getEdge(i, j);
			for (auto b = color_bits; b-- > 0;)
			{
				auto bit = (color >> b) & 0x1;
				bits.push_back(bit);
			}
		}
	}

	// Pad bit string to be divisible by 6
	while (bits.size() % 6 != 0) bits.push_back(0);

	int bits_size = std::ceil(color_bits * g.num_vertices * (g.num_vertices-1) / (float) 12) * 6;
	if (bits.size() != bits_size) std::printf("ERROR CONVERTING GRAPH TO MC\n");


	// Convert bit string to bytes
	std::string colors;
	for (auto i = 0; i < bits.size();)
	{
		uint8_t byte { 0 };
		for (auto b = 0; b < 6; ++b)
		{
			byte <<= 1;
			byte |= bits[i];
			++i;
		}
		byte += MCBIAS;

		colors += byte;
	}

	return colors;
}

std::string getGraphMC(const EdgeColoredUndirectedGraph& g) noexcept
{
	std::string mc;
	mc += getGraphSizeMC(g);
	mc += getGraphNumColorsMC(g);
	mc += getGraphEdgeColorsMC(g);

	return mc;
}

void writeGraphsToFileMC(
	const std::filesystem::path& path,
	const std::vector<EdgeColoredUndirectedGraph>& graphs)
{
	std::ofstream out(path);
	for (const auto& g : graphs)
	{
		out << getGraphMC(g) << "\n";
	}
	out.flush();

	std::printf(
		"Wrote to %s\n\n",
		path.c_str()
	);
}

std::vector<EdgeColoredUndirectedGraph> loadBulkAdj(std::filesystem::path file_path)
{
	std::ifstream file(file_path);
	assert(file.is_open() && "load_bulk() Failed: file not found.");

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

void writeGraphsToFileAdj(
	const std::filesystem::path& path,
	const std::vector<EdgeColoredUndirectedGraph>& graphs)
{
	std::ofstream out(path);
	for (const auto& g : graphs)
	{
		out << g.header_string() << "\n";
		out << g.to_string() << "\n";
	}
	out.flush();

	std::printf(
		"Wrote to %s\n\n",
		path.c_str()
	);
}

};	// end of namespace

