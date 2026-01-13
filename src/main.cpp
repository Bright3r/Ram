#include "k62.h"
// #include "k61.h"

#include <map>
#include <vector>

using namespace Ram;

// b is a subset of a
bool isIsomorphicSubset(
	const std::vector<EdgeColoredUndirectedGraph>& gs_a,
	const std::vector<EdgeColoredUndirectedGraph>& gs_b) noexcept
{
	std::unordered_set<std::string> isomorphs;
	for (const auto& a : gs_a)
	{
		isomorphs.insert(canonize(a));
	}

	for (const auto& b : gs_b)
	{
		if (!isomorphs.contains(canonize(b)))
		{
			return false;
		}
	}

	return true;
}

bool isIsomorphicEqual(
	const std::vector<EdgeColoredUndirectedGraph>& gs_a,
	const std::vector<EdgeColoredUndirectedGraph>& gs_b) noexcept
{
	return isIsomorphicSubset(gs_a, gs_b) && isIsomorphicSubset(gs_b, gs_a);
}

void testEmbed() noexcept
{
	auto up4 = loadBulkAdj("graphs/jacks/upsilon4.adj");

	std::vector<EdgeColoredUndirectedGraph> gs;
	gs.emplace_back(make_tperms()[1].at(1));
	gs.emplace_back(make_tperms()[1].at(2));
	gs.emplace_back(make_tperms()[1].at(3));
	gs.emplace_back(make_tperms()[2].at(1));
	gs.emplace_back(make_tperms()[2].at(2));
	gs.emplace_back(make_tperms()[2].at(3));

	auto subg = getNeighborhood(gs[0], 0, 3);
	for (const auto& g : gs)
	{
		auto embeddings = embed(subg, g);
		std::printf("%zu embeddings\n", embeddings.size());
	}

	writeGraphsToFileMC("graphs/test/subgraph.mc", { subg });
	writeGraphsToFileMC("graphs/test/supergraphs.mc", gs);
}

void testNeighborhoods(const std::vector<EdgeColoredUndirectedGraph> gs) noexcept
{
	auto t_perms = make_tperms();
	int progress = 0;
	for (const auto& g : gs)
	{
		// Build attaching set
		auto attaching_set = getAttachingSet(g);

		bool is_good = true;
		for (auto x : attaching_set)
		{
			int num_embeddable_neighborhoods = 0;
			for (auto c = 1; c <= 3; ++c)
			{
				// Build neighborhood
				auto neighborhood = getNeighborhood(g, x, c);
				if (canEmbed(neighborhood, t_perms[1].at(c)) ||
					canEmbed(neighborhood, t_perms[2].at(c)))
				{
					++num_embeddable_neighborhoods;
				}
			}

			if (num_embeddable_neighborhoods < 2)
			{
				is_good = false;
				break;
			}
		}

		std::printf("g%d embeddable in >=2 colors: %b\n", ++progress, is_good);
	}
}


void compareNumVertices(
	const std::vector<EdgeColoredUndirectedGraph> a,
	const std::vector<EdgeColoredUndirectedGraph> b) noexcept
{
	std::map<int, int> gmap;
	for (auto& g : a)
	{
		gmap[g.num_vertices]++;
	}

	std::printf("A Graphs:\n");
	for (auto& [nv, cnt] : gmap)
	{
		std::printf("v%d: %d\n", nv, cnt);
	}

	std::map<int, int> jmap;
	for (auto& g : b)
	{
		jmap[g.num_vertices]++;
	}
	std::printf("B Graphs:\n");
	for (auto& [nv, cnt] : jmap)
	{
		std::printf("v%d: %d\n", nv, cnt);
	}
}


int main(int argc, char** argv)
{
	// upsilon61_1();
	// upsilon61_2();
	// upsilon61_3();
	// upsilon61_4();
	// upsilon61_5();

	// upsilon62_1();
	// upsilon62_2(loadBulkAdj("graphs/62/upsilon1.adj"));
	// upsilon62_3(loadBulkAdj("graphs/62/upsilon2.adj"));
	upsilon62_4(loadBulkAdj("graphs/62/upsilon3.adj"));
	// upsilon62_5(loadBulkAdj("graphs/62/upsilon4.adj"));

	// testEmbed();
	// testNeighborhoods(loadBulkAdj("graphs/62/upsilon3.adj"));
	// testNeighborhoods(loadBulkAdj("graphs/jacks/table7.adj"));
	// testPullback();
	

	// auto my_up = loadBulkAdj("graphs/62/upsilon3.adj");
	// std::printf("My Graphs: %d\n", my_up.size());
	//
	// auto jack_up = loadBulkAdj("graphs/jacks/upsilon3.adj");
	// std::printf("Jack Graphs: %d\n", jack_up.size());
	//
	// bool b = isIsomorphicEqual(my_up, jack_up);
	// std::printf("Result: %b\n", b);

	return 0;
}

