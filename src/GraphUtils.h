#pragma once

#include <cstdio>
#include <ctime>
#include <vector>
#include <cassert>
#include <string>
#include <filesystem>

#include "cadical.hpp"

#include "EdgeColoredUndirectedGraph.h"

namespace Ram
{

// Isomorphism
bool isIsomorphic(graph* cg1, graph* cg2, int n, int m) noexcept;

std::string getCanonString(graph* cg, int n, int m) noexcept;

std::string canonize(const EdgeColoredUndirectedGraph& g) noexcept;

EdgeColoredUndirectedGraph::NautyGraph 
nautify(const EdgeColoredUndirectedGraph& g) noexcept;


// Coloring
std::vector<std::vector<Color>> generateAllColorings(size_t e, size_t k);

bool isTriangleFree(const EdgeColoredUndirectedGraph& g) noexcept;

bool isPartial(const EdgeColoredUndirectedGraph& g) noexcept;

std::vector<EdgeColoredUndirectedGraph> 
getColorPermutations(const EdgeColoredUndirectedGraph& g, int max_color = -1) noexcept;


// Embeddability
using Embedding = std::vector<int>;

std::vector<Embedding> embed(
	const EdgeColoredUndirectedGraph& subgraph,
	const EdgeColoredUndirectedGraph& graph) noexcept;

bool canEmbed(
	const EdgeColoredUndirectedGraph& subgraph,
	const EdgeColoredUndirectedGraph& graph) noexcept;

EdgeColoredUndirectedGraph getNeighborhood(
	const EdgeColoredUndirectedGraph& g,
	Vertex v,
	Color c) noexcept;

EdgeColoredUndirectedGraph getNeighborhood(
	const EdgeColoredUndirectedGraph& g,
	std::vector<Vertex>& neighbors,
	Vertex v,
	Color c) noexcept;


// CNF
using CNF = std::vector<std::string>;
CNF getCNF(const EdgeColoredUndirectedGraph& g, bool add_colors = false) noexcept;

std::unique_ptr<CaDiCaL::Solver> getCNFSolver(
	const EdgeColoredUndirectedGraph& g,
	std::vector<std::vector<std::vector<int>>>& edge_to_var,
	bool add_colors = false) noexcept;


// IO
void writeGraphsToFile(
	const std::filesystem::path& path,
	const std::vector<EdgeColoredUndirectedGraph>& graphs);
	
void writeCNFToFile(std::filesystem::path file_path, const CNF& cnf);

std::vector<EdgeColoredUndirectedGraph> loadBulk(std::filesystem::path file_path);

std::vector<EdgeColoredUndirectedGraph> loadBulkMC(std::filesystem::path file_path);

};	// end of namespace

