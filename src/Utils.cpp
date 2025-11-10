#include "Utils.h"

#include <algorithm>
#include <numeric>

namespace Ram
{

std::vector<std::vector<int>> permute(int n, int k) noexcept
{
	std::vector<std::vector<int>> res;
	std::vector<int> items(n);
	std::iota(items.begin(), items.end(), 0);

	std::vector<int> combo;
	std::vector<bool> select(n, false);
	std::fill(select.begin(), select.begin() + k, true);

	do
	{
		combo.clear();
		for (auto i = 0; i < n; ++i)
		{
			if (select[i]) combo.emplace_back(items[i]);
		}

		// generate all permutations of current combo
		std::sort(combo.begin(), combo.end());
		do
		{
			res.emplace_back(combo);
		} while (std::next_permutation(combo.begin(), combo.end()));
	} while (std::prev_permutation(select.begin(), select.end()));

	return res;
}

std::vector<std::vector<int>> choose(int n, int k) noexcept
{
	std::vector<std::vector<int>> res;
	std::vector<int> items(n);
	std::iota(items.begin(), items.end(), 0);

	std::vector<int> combo;
	std::vector<bool> select(n, false);
	std::fill(select.begin(), select.begin() + k, true);

	do
	{
		combo.clear();
		for (auto i = 0; i < n; ++i)
		{
			if (select[i]) combo.emplace_back(items[i]);
		}
		res.emplace_back(combo);
	} while (std::prev_permutation(select.begin(), select.end()));

	return res;
}

uint64_t numBitsInBinary(uint64_t num) noexcept
{
	size_t num_bits = 0;
	while (num > 0)
	{
		++num_bits;
		num >>= 1;
	}
	return num_bits;
}

};	// end of namespace
