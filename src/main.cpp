#include "CanonicalAugmentation.h"
#include "EdgeColoredUndirectedGraph.h"

using namespace Ram;

EdgeColoredUndirectedGraph make_T1() noexcept
{
	return load_adj("graphs/T1.adj", 16, 3);
}

EdgeColoredUndirectedGraph make_T2() noexcept
{
	return load_adj("graphs/T2.adj", 16, 3);
}

int main(int argc, char** argv)
{
	verify();
	
	return 0;
}

