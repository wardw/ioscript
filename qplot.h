#pragma once

#include <tuple>
#include <type_traits>
#include <variant>
#include "gnuplot.h"
#include "util.h"

struct image_size_style : public std::string {};
struct axis_extents_style : public std::string {};

struct ImageSize
{
	static constexpr int pass = 0;
	using SupportedTypes = std::tuple<>;

	template<typename T>
	std::string plotString(const T& obj) const {
		return "set image size " + std::to_string(x) + "," + std::to_string(y) + "\n";
	}
	int x = 800;
	int y = 640;
};

struct AxisExtents
{
	static constexpr int pass = 0;
	using SupportedTypes = std::tuple<>; 
	
	template<typename T>
	std::string plotString(const T& obj) const {
		return "set axis extents\n";
	}
	int x[2] = {0,1};
	int y[2] = {0,1};
};

struct LineChart
{
	static constexpr int pass = 1;
	using SupportedTypes = std::tuple<std::vector<int>,std::vector<float>>;
	template<typename T>
	std::string plotString(const T& obj) const {
		return "plot LineChart with " + objName(obj) + "\n";
	}
};

struct BarChart
{
	static constexpr int pass = 1;
	using SupportedTypes = std::tuple<std::vector<int>,std::vector<float>>;
	template<typename T>
	std::string plotString(const T& obj) const {
		return "plot BarChart with " + objName(obj) + "\n";
	}
};

struct Point2 { float x; float y; };

struct ScatterChart
{
	using SupportedTypes = std::tuple<std::vector<Point2>>;
	template<typename T>
	std::string plotString(const T& obj) const {
		return "plot ScatterChart with " + objName(obj) + "\n";
	}
};

struct image_map_style : public std::string {};
struct vector_field_style : public std::string {};

struct Row
{
	int rows[10] = {};
};

struct HeatMap
{
	static constexpr int pass = 1;
	using PlotStyle = image_map_style;
	using SupportedTypes = std::tuple<>;

	template<typename T>
	std::string plotString(const T& obj) const { return "set style heat map\n"; }
	std::string plotStringPost() const { return "unset style heat map\n"; }
};

struct VectorIntRow
{
	VectorIntRow(const std::vector<int>& vec) : vec_(vec) {}
	Row operator()(int row) { return row < vec_.size() ? Row{row, vec_[row]} : Row{}; }
	const std::vector<int>& vec_;
};


template <typename T>
struct plot_defaults;

template <>
struct plot_defaults<std::vector<int>> {
	using PlotStyle = image_map_style;
	using RowType = VectorIntRow;
};

Row writeRow(std::vector<int>& vec, int row)
{
	return row < vec.size() ? Row{row, vec[row]} : Row{};
}

decltype(auto) rowData(const std::vector<int>& vec)
{

}

// template <>
// struct plot_defaults<std::vector<int>> {
// 	using PlotStyle = vector_field_style;
// };


template <typename T> struct is_canvas_property    { static constexpr bool value = false; };
template <> struct is_canvas_property<ImageSize>   { static constexpr bool value = true; };
template <> struct is_canvas_property<AxisExtents> { static constexpr bool value = true; };

template <typename T> struct is_plot_property     { static constexpr bool value = false; };
template <> struct is_plot_property<HeatMap> { static constexpr bool value = true; };

template <typename T> struct is_style     { static constexpr bool value = false; };
template <> struct is_style<HeatMap>      { static constexpr bool value = true; };
template <> struct is_style<ImageSize>    { static constexpr bool value = true; };
template <> struct is_style<AxisExtents>  { static constexpr bool value = true; };
template <> struct is_style<LineChart>    { static constexpr bool value = true; };
template <> struct is_style<BarChart>     { static constexpr bool value = true; };
template <> struct is_style<ScatterChart> { static constexpr bool value = true; };

using ChartTuple = std::tuple<std::variant<LineChart, BarChart>,
                              std::variant<HeatMap>
							 >;

template <typename T> struct chart_traits { using type = typename T::type; };
template <> struct chart_traits<std::vector<int>>    { static constexpr size_t index = 0; };
template <> struct chart_traits<std::vector<float>>  { static constexpr size_t index = 0; };
template <> struct chart_traits<std::vector<Point2>> { static constexpr size_t index = 1; };

// All vector<T>'s are index 0 (check)
//template <typename T> struct chart_traits<std::vector<T>> { static constexpr index = 0 };

using LineChartTypes = std::tuple<std::vector<int>, std::vector<float>>;
using BarChartTypes = std::tuple<std::vector<int>, std::vector<float>>;
using ScatterChartTypes = std::tuple<std::vector<Point2>>;

template<class T, class ObjTypes, std::size_t N>
struct TupleUpdater {
    static void update(ChartTuple& chartTuple, const T& style)
    {
		using Type = std::tuple_element_t<N-1, ObjTypes>;
		std::get<chart_traits<Type>::index>(chartTuple) = style;

        TupleUpdater<T, ObjTypes, N-1>::update(chartTuple, style);
    }
};
 
template<class T, class ObjTypes>
struct TupleUpdater<T, ObjTypes, 1> {
    static void update(ChartTuple& chartTuple, const T& style)
    {
		using Type = std::tuple_element_t<0, ObjTypes>;
		std::get<chart_traits<Type>::index>(chartTuple) = style;
    }
};

// Todo: can't we just have a base case for when N=0?
template<class T, class ObjTypes>
struct TupleUpdater<T, ObjTypes, 0> {
    static void update(ChartTuple& chartTuple, const T& style)
    {
    }
};

struct Foo {
	using PlotStyle = image_size_style;
};

std::tuple<image_size_style,
		   axis_extents_style
          >
          canvasStyleString;


std::tuple<vector_field_style,
           image_map_style
          >
          plotStyleString;


std::tuple<ImageSize,
           AxisExtents,
		   HeatMap
          >
          currentStyles;

template <typename Style, typename T>
void plotObject(Gnuplot& gnuplot, const Style& style, const T& obj)
{
	std::cout << style.plotString(obj).c_str();
}

struct Nothing {
	template <typename T>
	void operator()(const T& arg)
	{
	}	
};

class Qplot
{
public:
    enum class Target { DRY_RUN, STDOUT, GNUPLOT };
    static constexpr Target target = Target::STDOUT;

	friend Qplot& operator<<(Qplot& qplot, const std::string& str);

	Qplot()
    {
        switch (target)
        {
            case Target::DRY_RUN: gnuplot_ = std::make_unique<Gnuplot>("cat > /dev/null");  break;
            case Target::STDOUT:  gnuplot_ = std::make_unique<Gnuplot>("cat");              break;
            case Target::GNUPLOT: gnuplot_ = std::make_unique<Gnuplot>("gnuplot");          break;
            default : assert(false);
        }
    }

	ChartTuple chartTuple;

    void processArgs() {}
    void processCanvasArgs() {}
    void processPlotArgs() {}

	// Arg is a style object
    template <typename T, typename... Ts,
              std::enable_if_t<has_pass<T>::value, int> = 0>
    void processArgs(const T& style, const Ts&... args)
    {
		// constexpr size_t index = tuple_contains_type<T, decltype(currentStyles)>::value;
		// std::cout << "index " << index << " is " << arg.plotString() << std::endl;

		using ObjTypes = typename T::SupportedTypes;
		TupleUpdater<T, ObjTypes, std::tuple_size_v<ObjTypes>>::update(chartTuple, style);

		// if (index >= 0) {
		// 	std::get<index>(currentStyles) = arg;
		// } else {
		// 	std::cout << "Style type not registered with Qplot" << std::endl;
		// }

		processArgs(args...);
    }

	// Arg is a plot object
    template <typename T, typename... Ts,
              std::enable_if_t<!has_pass<T>::value, int> = 0>
    void processArgs(const T& obj, const Ts&... args)
    {
		// Get the style object that's currently associated with the arg type T
		auto style = std::get<chart_traits<T>::index>(chartTuple);

		std::visit([this,&obj](auto&& arg) {
			plotObject(*gnuplot_, arg, obj); 
		}, style);

		// std::visit(Nothing(), var);

    	// Actual objects to plot
    	// plotObject(*gnuplot_, style, obj);


		// constexpr size_t index = tuple_contains_type<typename T::PlotStyle, decltype(plotStyleString)>::value;
		// std::cout << "index " << index << " is " << arg.plotString() << std::endl;

		// if (index >= 0) {
			// std::get<index>(plotStyleString).assign(arg.plotString());
		// }

		processArgs(args...);
    }

    template <typename... Ts>
	void plot(const Ts&... args)
	{
		// processCanvasArgs(args...);
		// auto t = std::make_tuple(args...);

    	// writeStyleString(*gnuplot_, canvasStyleString);

    	// enable multiplot

		processArgs(args...);

		// disable multiplot
    	// writeStyleString(gnuplot_.get(), plotStyleString);
	}

private:
    std::unique_ptr<Gnuplot> gnuplot_ = nullptr;
};

// Direct pass-through to gnuplot
Qplot& operator<<(Qplot& qp, const std::string& str)
{
	*qp.gnuplot_ << str;
	return qp;
}

// Main convenience method for one-liner plotting
template <typename... Ts>
void qplot(const Ts&... args)
{
    static Qplot qp;
    qp.plot(args...);
}

// template <typename T>
// void plotObject(Gnuplot& gnuplot, const T& obj)
// {
// 	// write the style associated with this object

// 	// constexpr size_t index = tuple_contains_type<typename T::PlotStyle, decltype(plotStyleString)>::value;
// 	constexpr size_t index = tuple_contains_type<typename plot_defaults<T>::PlotStyle, decltype(plotStyleString)>::value;

// 	// writeStyleString(gnuplot, std::get<index>(plotStyleString));

// 	// write the row data
// 	// writeRowData(gnuplot, rowData(obj));
// }
