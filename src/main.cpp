#include "k62.h"
// #include "k61.h"
#include "CanonicalAugmentation.h"

int main(int argc, char** argv)
{
	augment();

	// upsilon62_1();
	// upsilon62_2();
	// upsilon62_3();
	// upsilon62_4();
	// upsilon62_5();

	// upsilon61_1();
	// upsilon61_2();
	// upsilon61_3();
	// upsilon61_4();
	// upsilon61_5();

	// for (auto i = 1; i <= 5; ++i)
	// {
	// 	// Read .adjs
	// 	std::stringstream path;
	// 	path << "graphs/62/upsilon" << i << ".adj";
	//
	// 	auto graphs = loadBulkAdj(path.str());
	//
	// 	// Write as .mcs
	// 	std::stringstream mc_path;
	// 	mc_path << "graphs/62/upsilon" << i << ".mc";
	//
	// 	writeGraphsToFileMC(mc_path.str(), graphs);
	// }
	//
	// auto graphs = loadBulkAdj("graphs/62/upsilon3.adj");
	//
	// std::vector<EdgeColoredUndirectedGraph> mcs;
	// for (const auto& g : graphs)
	// {
	// 	// Strip u and v from graph
	// 	EdgeColoredUndirectedGraph new_g(g.num_vertices - 2, 3);
	// 	for (auto i = 0; i < new_g.num_vertices; ++i)
	// 	{
	// 		for (auto j = i+1; j < new_g.num_vertices; ++j)
	// 		{
	// 			new_g.setEdge(i, j, g.getEdge(i, j));
	// 		}
	// 	}
	//
	// 	mcs.emplace_back(std::move(new_g));
	// }
	//
	// writeGraphsToFileMC("graphs/62/upsilon3_no_uvs.mc", mcs);

	return 0;
}

