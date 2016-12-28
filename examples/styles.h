#pragma once

#include <iomanip>

#include "examples.h"

struct ImageSize
{
	static constexpr int pass = 0;

	void plot(Process<Gnuplot>& gnuplot) const {
		gnuplot << "set image size " + std::to_string(x) + "," + std::to_string(y) + "\n";
	}
	int x = 800;
	int y = 640;
};

struct AxisExtents
{
	static constexpr int pass = 0;
	
	void plot(Process<Gnuplot>& gnuplot) const {
		gnuplot << "set axis extents\n";
	}
	int x[2] = {0,1};
	int y[2] = {0,1};
};

template <typename T>
void sendData(Process<Gnuplot>& gnuplot, const std::vector<T>& obj)
{
    for (int i=0; i<obj.size(); ++i) {
        gnuplot << std::to_string(i) << " " << std::to_string(obj[i]) << "\n";
    }
}

struct Filename
{
	Filename(const std::string& filename) : filename(filename) {}
	static constexpr int pass = 1;

	void plot(Process<Gnuplot>& gnuplot) const
	{
        gnuplot << "set output '" << filename << ".png'\n"
                << "set terminal pngcairo size 500, 500\n";
    }
	std::string filename;
};

struct BarChart
{
	static constexpr int pass = 1;
	using supported_types = std::tuple<std::vector<int>,std::vector<float>>;

	template<typename T>
	void plot(Process<Gnuplot>& gnuplot, const T& obj) const {
		gnuplot << "plot BarChart with " + objName(obj) + "\n";
	}

	template<typename T>
	void plot(Process<Mpl>& gnuplot, const T& obj) const {
        gnuplot << "plot '-' using 1:2\n";
        // sendData(gnuplot, obj);
        gnuplot << "e\n";
	}
};

struct Candlestick
{
	static constexpr int pass = 1;
	using supported_types = std::tuple<std::vector<CandlestickRow>>;  // todo: something like Colums<3>

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
			gnuplot << std::setprecision(3) << std::fixed << std::get<0>(row) << " " << std::get<1>(row) << " " << std::get<2>(row) << " " << '\n';
		}
		gnuplot << "e\n";

		// Gnuplot requires the data is resent for subsequent plots/reads
		for (auto& row : obj) {
			gnuplot << std::setprecision(3) << std::fixed << std::get<0>(row) << " " << std::get<1>(row) << " " << std::get<2>(row) << " " << '\n';
		}
		gnuplot << "e\n";
	}
};

struct PythonInit
{
	static constexpr int pass = 1;

	void plot(Process<Mpl>& mpl) const {
        mpl <<
R"(
import os
os.close(4)
fo = os.fdopen(3, 'r')
)"
		<< "\n";
	}
};

struct LineChart
{
	static constexpr int pass = 1;
	using supported_types = std::tuple<std::vector<int>,std::vector<float>>;

	template<typename T>
	void plot(Process<Gnuplot>& gnuplot, const T& obj) const {
        gnuplot << "plot '-' using 1:2\n";
        // sendData(gnuplot, obj);
        gnuplot << "e\n";
	}

	template<typename T>
	void plot(Process<Mpl>& mpl, const T& obj) const
	{
        mpl <<
R"(
for line in fo:
	if line == 'EOT\n':
		break
	print "(Python) " + line,
print "(Python) Done"
)"
		<< "\n";

		for (auto elem : obj)
		{
			mpl.fdout() << elem << '\n';
		}
		mpl.fdout() << "EOT" << '\n';
	}
};

struct Point2 { float x; float y; };

struct ScatterChart
{
	static constexpr int pass = 1;
	using supported_types = std::tuple<std::vector<Point2>>;

	template<typename T>
	void plot(Process<Gnuplot>& gnuplot, const T& obj) const {
		gnuplot << "plot ScatterChart with " + objName(obj) + "\n";
	}
};

struct Row
{
	int rows[10] = {};
};

struct HeatMap
{
	static constexpr int pass = 1;
	using supported_types = std::tuple<>;

	template<typename T>
	void plot(Process<Gnuplot>& gnuplot, const T& obj) const { gnuplot << "set style heat map\n"; }
	// void plotPost() const { gnuplot << "unset style heat map\n"; }
};


// Previously used a tuple<> rather than a map<size_t,any>
// However a tuple requires statically writing a tuple type with each variant's tuple index matching it's id

struct Data1d          { static constexpr size_t id = 0; using supported_styles = std::variant<LineChart, BarChart>; };
struct Data2dPoints    { static constexpr size_t id = 1; using supported_styles = std::variant<>; };  // PointChart
struct WidgetData      { static constexpr size_t id = 2; using supported_styles = std::variant<Candlestick>; };
struct ScalarFieldData { static constexpr size_t id = 3; using supported_styles = std::variant<HeatMap>; };

template <typename T> struct plot_traits;
template <> struct plot_traits<std::vector<CandlestickRow>> { using type = WidgetData; };
template <> struct plot_traits<std::vector<int>>            { using type = Data1d; };
template <> struct plot_traits<std::vector<float>>          { using type = Data1d; };
template <> struct plot_traits<std::vector<Point2>>         { using type = Data2dPoints; };
