#include <iostream>
#include <vector>

#include "qplot.h"

using namespace std;

#include <iostream>

int main()
{
	std::vector<int> ints = {1,2,3,4,5,6,7};

	Qplot qplot;
	qplot.plot(ints, AxisExtents{{0,1}, {0,2}}, ImageSize{400,300}, ints);

	qplot << "\nsometime later...\n\n";
    
    qplot.plot(ints, BarChart(), ints, ints, LineChart(), ints);

	qplot << "reset\n";

    cout << "has_pass: " << has_pass<LineChart>::value << endl;
}
