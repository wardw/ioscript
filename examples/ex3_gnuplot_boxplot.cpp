#include "gnuplot_styles_common.h"

#include <tuple>
#include <vector>
#include <fstream>
#include <iomanip>

#include "qplot/qplot.h"

using namespace std;

using BoxPlotRow = std::tuple<int,int,float>;

struct BoxPlot
{
	using supported_types = std::tuple<std::vector<BoxPlotRow>>;  // todo: something like Colums<3>

	template<typename T>
	void plot(Process<Gnuplot>& gnuplot, const T& obj) const
	{
		gnuplot <<
R"(
# set terminal canvas  rounded size 600,400 enhanced fsize 10 lw 1.6 fontscale 1 name "boxplot_1" jsdir "."
# set output 'boxplot.1.js'
set bar 1.000000 front
set border 2 front lt black linewidth 1.000 dashtype solid
set boxwidth 0.5 absolute
set style fill   solid 0.25 border lt -1
set style circle radius graph 0.02, first 0.00000, 0.00000
set style ellipse size graph 0.05, 0.03, first 0.00000 angle 0 units xy
unset key
set style textbox transparent margins  1.0,  1.0 border
unset logscale
set pointsize 0.5
set style data boxplot
set xtics border in scale 0,0 nomirror norotate  autojustify
set xtics  norangelimit
set xtics   ("A" 1.00000, "B" 2.00000)
set ytics border in scale 1,0.5 nomirror norotate  autojustify
unset paxis 1 tics
unset paxis 2 tics
unset paxis 3 tics
unset paxis 4 tics
unset paxis 5 tics
unset paxis 6 tics
unset paxis 7 tics
set yrange [ 0.00000 : 100.000 ] noreverse nowriteback
set paxis 1 range [ * : * ] noreverse nowriteback
set paxis 2 range [ * : * ] noreverse nowriteback
set paxis 3 range [ * : * ] noreverse nowriteback
set paxis 4 range [ * : * ] noreverse nowriteback
set paxis 5 range [ * : * ] noreverse nowriteback
set paxis 6 range [ * : * ] noreverse nowriteback
set paxis 7 range [ * : * ] noreverse nowriteback
set colorbox vertical origin screen 0.9, 0.2, 0 size screen 0.05, 0.6, 0 front  noinvert bdefault
x = 0.0
## Last datafile plotted: "silver.dat"
##plot '../silver.dat' using (1):2, '' using (2):(5*$3)
plot '-' using (1):2, '' using (2):(5*$3)
)";
		for (auto& row : obj) {
			gnuplot << std::get<0>(row) << " " << std::get<1>(row) << " " << std::get<2>(row) << " " << '\n';
		}
		gnuplot << "e\n";

		// Gnuplot requires the data is resent for subsequent plots/reads
		for (auto& row : obj) {
			gnuplot << std::get<0>(row) << " " << std::get<1>(row) << " " << std::get<2>(row) << " " << '\n';
		}
		gnuplot << "e\n";
	}
};

struct WidgetData  { static constexpr size_t id = 0; using supported_styles = std::variant<BoxPlot>; };

template <> struct plot_traits<std::vector<BoxPlotRow>> { using type = WidgetData; };


void example3()
{
	fstream fs("../examples/silver.dat", fstream::in);
	if (!fs.is_open()) {
		cout << "Error opening file" << endl;
		assert(false);
	}

	vector<BoxPlotRow> data;
	float f1, f2, f3;
	while (true)
	{
		if (fs.peek() == '#') {
			fs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			continue;
		}

		fs >> f1;
		fs >> f2;
		fs >> f3;

		if (fs.good())
			data.push_back({f1,f2,f3});
		else
			break;
	}

	Qplot<Gnuplot> qplot;
    qplot.plot(Filename("boxplot"), BoxPlot(), data);
}
