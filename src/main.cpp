// #include "k62.h"
#include "k60.h"

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

// void testEmbed() noexcept
// {
// 	auto up4 = loadBulkAdj("graphs/jacks/upsilon4.adj");
//
// 	std::vector<EdgeColoredUndirectedGraph> gs;
// 	gs.emplace_back(make_tperms()[1].at(1));
// 	gs.emplace_back(make_tperms()[1].at(2));
// 	gs.emplace_back(make_tperms()[1].at(3));
// 	gs.emplace_back(make_tperms()[2].at(1));
// 	gs.emplace_back(make_tperms()[2].at(2));
// 	gs.emplace_back(make_tperms()[2].at(3));
//
// 	auto subg = getNeighborhood(gs[0], 0, 3);
// 	for (const auto& g : gs)
// 	{
// 		auto embeddings = embed(subg, g);
// 		std::printf("%zu embeddings\n", embeddings.size());
// 	}
// 	writeGraphsToFileMC("graphs/test/subgraph.mc", { subg });
// 	writeGraphsToFileMC("graphs/test/supergraphs.mc", gs);
// }
//
// void testNeighborhoods(const std::vector<EdgeColoredUndirectedGraph> gs) noexcept
// {
// 	auto t_perms = make_tperms();
// 	int progress = 0;
// 	for (const auto& g : gs)
// 	{
// 		// Build attaching set
// 		auto attaching_set = getAttachingSet(g);
//
// 		bool is_good = true;
// 		for (auto x : attaching_set)
// 		{
// 			int num_embeddable_neighborhoods = 0;
// 			for (auto c = 1; c <= 3; ++c)
// 			{
// 				// Build neighborhood
// 				auto neighborhood = getNeighborhood(g, x, c);
// 				if (canEmbed(neighborhood, t_perms[1].at(c)) ||
// 					canEmbed(neighborhood, t_perms[2].at(c)))
// 				{
// 					++num_embeddable_neighborhoods;
// 				}
// 			}
//
// 			if (num_embeddable_neighborhoods < 2)
// 			{
// 				is_good = false;
// 				break;
// 			}
// 		}
//
// 		std::printf("g%d embeddable in >=2 colors: %b\n", ++progress, is_good);
// 	}
// }


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


void trySolve() noexcept
{
	// auto up5 = loadBulkMC("graphs/stan/prop5.4_at5.mc");
	// std::printf("%d graphs loaded\n", up5.size());

	// std::printf("SATISFIABLE=%d\n", CaDiCaL::SATISFIABLE);
	// std::printf("UNSATISFIABLE=%d\n", CaDiCaL::UNSATISFIABLE);
	// for (auto& g : up5)
	// {
	// 	EdgeColoredUndirectedGraph base(62, 4);
	// 	for (auto i = 0; i < g.num_vertices; ++i)
	// 	{
	// 		for (auto j = i+1; j < g.num_vertices; ++j)
	// 		{
	// 			base.setEdge(i, j, g.getEdge(i, j));
	// 		}
	// 	}
	//
	// 	auto num_verts = 4;
	// 	auto cperms = generateAllColorings(num_verts, 4);
	// 	std::printf("%d cperms\n", cperms.size());
	// 	for (auto& cperm : cperms)
	// 	{
	// 		auto fin = base;
	// 		for (auto v = 0; v < num_verts; ++v)
	// 		{
	// 			fin.setEdge(v, 61, cperm[v]);
	// 		}
	//
	// 		std::vector<std::vector<std::vector<int>>> edge_to_var;
	// 		auto solver = getCNFSolver(fin, edge_to_var, true);
	// 		auto res = solver->solve();
	//
	// 		std::printf("Result: %d\n", res);
	// 	}
	// }
}


void countGoodNeighborhoods(
	const std::vector<EdgeColoredUndirectedGraph>& gs,
	int num_colors
) noexcept
{
	std::map<int, int> map;
	for (auto& g : gs)
	{
		auto aset = getAttachingSet(g);

		auto good = 0;
		for (auto c = 1; c <= num_colors; ++c)
		{
			auto n = getNeighborhood(g, aset[0], c);
			bool is_good = true;
			for (auto i = 0; i < n.num_vertices; ++i)
			{
				for (auto j = i+1; j < n.num_vertices; ++j)
				{
					auto ec = n.getEdge(i, j);
					if (ec == 0) 
					{
						is_good = false;
					}
				}
			}

			if (is_good) ++good;
		}

		map[good]++;
		// std::printf("Good: %d\n", good);
	}

	for (auto [good_neighborhoods, cnt] : map)
	{
		std::printf("%d good neighborhoods: %d graphs\n", good_neighborhoods, cnt);
	}
}


int main(int argc, char **argv)
{
	// auto gs = loadBulkAdj("graphs/60/upsilon3.adj");
	// std::printf("%d graphs\n", gs.size());
	// upsilon60_1();
	// upsilon60_2(loadBulkAdj("graphs/60/upsilon1.adj"));
	// upsilon60_3(loadBulkAdj("graphs/60/upsilon2.adj"));
	upsilon60_4(loadBulkAdj("graphs/60/upsilon3.adj"));
	// upsilon60_5(loadBulkAdj("graphs/60/upsilon4.adj"));
	// upsilon60_6(loadBulkAdj("graphs/60/upsilon5.adj"));

	// upsilon62_1();
	// upsilon62_2(loadBulkAdj("graphs/62/upsilon1.adj"));
	// upsilon62_3(loadBulkAdj("graphs/62/upsilon2.adj"));
	// upsilon62_4(loadBulkAdj("graphs/62/upsilon3.adj"));
	// upsilon62_5(loadBulkAdj("graphs/62/upsilon4.adj"));

	// testEmbed();
	// testNeighborhoods(loadBulkAdj("graphs/62/upsilon3.adj"));
	// testNeighborhoods(loadBulkAdj("graphs/jacks/table7.adj"));
	// testPullback();
	

	// auto my_up = loadBulkAdj("graphs/62/upsilon5.adj");
	// std::printf("My Graphs: %d\n", my_up.size());
	//
	// auto stan_up = loadBulkAdj("graphs/stan/upsilon5.adj");
	// std::printf("Stan Graphs: %d\n", stan_up.size());
	//
	// bool b = isIsomorphicEqual(my_up, stan_up);
	// std::printf("Result: %b\n", b);
	//
	// countGoodNeighborhoods(my_up, 4);




	// auto t_perms = make_tperms();
	// for (auto i = 1; i <= 2; ++i)
	// {
	// 	for (auto c = 1; c <= 3; ++c)
	// 	{
	// 		auto& g = t_perms[i].at(c);
	// 		std::printf("%s\n", g.to_string().c_str());
	// 	}
	// }
	//
	// auto t = make_T1();
	// std::printf("%s\n", t.to_string().c_str());

	return 0;
}

