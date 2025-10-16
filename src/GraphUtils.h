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

// IO
void writeGraphsToFile(
	const std::filesystem::path& path,
	const std::vector<Ram::EdgeColoredUndirectedGraph>& graphs);

