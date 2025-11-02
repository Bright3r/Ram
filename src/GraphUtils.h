#pragma once

#include <cstdio>
#include <ctime>
#include <vector>
#include <cassert>
#include <string>
#include <filesystem>

#include "EdgeColoredUndirectedGraph.h"

// Isomorphism
bool isIsomorphic(graph* cg1, graph* cg2, int n, int m) noexcept;

std::string getCanonString(graph* cg, int n, int m) noexcept;

std::string canonize(const Ram::EdgeColoredUndirectedGraph& g) noexcept;


// Colorings
std::vector<std::vector<Ram::Color>> generateAllColorings(size_t e, size_t k);

bool isTriangleFree(const Ram::EdgeColoredUndirectedGraph& g) noexcept;

bool isPartial(const Ram::EdgeColoredUndirectedGraph& g) noexcept;

std::vector<Ram::EdgeColoredUndirectedGraph> 
getColorPermutations(const Ram::EdgeColoredUndirectedGraph& g, int max_color = -1) noexcept;

Ram::EdgeColoredUndirectedGraph::NautyGraph 
nautify(const Ram::EdgeColoredUndirectedGraph& g) noexcept;


// IO
void writeGraphsToFile(
	const std::filesystem::path& path,
	const std::vector<Ram::EdgeColoredUndirectedGraph>& graphs);
	
std::vector<Ram::EdgeColoredUndirectedGraph> loadBulk(std::filesystem::path filename) noexcept;


