#include <iostream>
#include <vector>

#include "qplot/qplot.h"

#include "styles.h"
#include "examples.h"

using namespace std;

#include <iostream>

void gnuplotTesting()
{
	std::vector<int> ints = {1,2,3,4,5,6,7};

	Qplot<Gnuplot> qplot;
	// qplot.plot(ints, AxisExtents{{0,1}, {0,2}}, ImageSize{400,300}, ints);
    // qplot.plot(ints, BarChart(), ints, ints, LineChart(), ints);

}

void mplTesting()
{
	std::vector<int> ints = {1,2,3,4,5,6,7};

	Qplot<Mpl> qplot;
	// qplot.plot(ints, AxisExtents{{0,1}, {0,2}}, ImageSize{400,300}, ints);
    // qplot.plot(ints, ints, ints, LineChart(), ints);
    qplot.plot(PythonInit(), ints, ints);
}

int main()
{
	// gnuplotTesting();
	// mplTesting();

    std::cout << "Testing has_plot: " << has_plot<LineChart, void(Process<Mpl>&,const vector<int>&)>::value << std::endl;
    cout << "has_pass: " << has_pass<LineChart>::value << endl;

	examples();

	return 0;
}
