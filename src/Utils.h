#pragma once

#include <cstdint>
#include <vector>

namespace Ram
{

std::vector<std::vector<int>> permute(int n, int k) noexcept;

std::vector<std::vector<int>> choose(int n, int k) noexcept;

uint64_t numBitsInBinary(uint64_t num) noexcept;

};	// end of namespace

