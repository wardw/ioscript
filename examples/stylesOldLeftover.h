#pragma once

#include <iomanip>
#include <array>

struct ImageSize
{
	using supported_types = std::tuple<>;

	void plot(Qplot<Gnuplot>& gnuplot) const {
		gnuplot << "set image size " + std::to_string(x) + "," + std::to_string(y) + "\n";
	}
	int x = 800;
	int y = 640;
};

struct AxisExtents
{
	using supported_types = std::tuple<>;
	
	void plot(Qplot<Gnuplot>& gnuplot) const {
		gnuplot << "set axis extents\n";
	}
	int x[2] = {0,1};
	int y[2] = {0,1};
};

struct Point2 { float x; float y; };

struct ScatterChart
{
	using supported_types = std::tuple<std::vector<Point2>>;

	template<typename T>
	void plot(Qplot<Gnuplot>& gnuplot, const T& obj) const {
		gnuplot << "plot ScatterChart with " + objName(obj) + "\n";
	}
};


// Previously used a tuple<> rather than a map<size_t,any>
// However a tuple requires statically writing a tuple type with each variant's tuple index matching it's id


// Define a mapping from the type of plot to the styles that are available to draw it

struct Data2dPoints    { static constexpr size_t id = 1; using supported_styles = std::variant<>; };  // PointChart

// Define a mapping from the client's object types to the type of plot data it represents


template <> struct plot_traits<std::vector<Point2>>         { using type = Data2dPoints; };
