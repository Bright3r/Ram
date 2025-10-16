#pragma once

#include <vector>
#include <cassert>
#include <string>
#include <unordered_set>

#include "EdgeColoredUndirectedGraph.h"

void processRepresentative(
	const Ram::EdgeColoredUndirectedGraph& representative,
	const std::vector<std::vector<Ram::Color>>& colorings,
	std::vector<Ram::EdgeColoredUndirectedGraph>& new_graphs,
	std::unordered_set<std::string>& new_canons) noexcept;

void augment(int k_start = 3, int k_stop = 16, Ram::Color max_color = 3) noexcept;

void verify() noexcept;

