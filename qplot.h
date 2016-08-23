#pragma once

#include <tuple>
#include <type_traits>

#include "gnuplot.h"
#include "util.h"

struct image_size_style : public std::string {};
struct axis_extents_style : public std::string {};

struct ImageSize
{
	using CanvasType = image_size_style;

	std::string plotString() const {
		return "set terminal pngcairo size " + std::to_string(x) + "," + std::to_string(y) + "\n";
	}
	int x = 800;
	int y = 640;
};

struct AxisExtents
{
	using CanvasType = axis_extents_style;

	std::string plotString() const {
		return "axis extents\n";
	}
	int x[2] = {0,1};
	int y[2] = {0,1};
};

struct image_map_style : public std::string {};
struct vector_field_style : public std::string {};

struct Row
{
	int rows[10] = {};
};

struct HeatMapStyle
{
	using PlotStyle = image_map_style;

	std::string plotString() const { return "heat map\n"; }
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


template <typename T> struct is_canvas_property { static constexpr bool value = false; };
template <> struct is_canvas_property<ImageSize>   { static constexpr bool value = true; };
template <> struct is_canvas_property<AxisExtents> { static constexpr bool value = true; };

template <typename T> struct is_plot_property { static constexpr bool value = false; };
template <> struct is_plot_property<HeatMapStyle> { static constexpr bool value = true; };

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

template <typename T>
void plotObject(Gnuplot& gnuplot, const T& obj)
{
	// write the style associated with this object
	// constexpr size_t index = tuple_contains_type<typename T::PlotStyle, decltype(plotStyleString)>::value;
	constexpr size_t index = tuple_contains_type<typename plot_defaults<T>::PlotStyle, decltype(plotStyleString)>::value;

	writeStyleString(gnuplot, std::get<index>(plotStyleString));

	// write the row data
	// writeRowData(gnuplot, rowData(obj));
}

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



    void processCanvasArgs() {}
    void processPlotArgs() {}

    template <typename T, typename... Ts,
              std::enable_if_t<is_canvas_property<T>::value, int> = 0>
    void processCanvasArgs(const T& arg, const Ts&... args)
    {
		constexpr size_t index = tuple_contains_type<typename T::CanvasType, decltype(canvasStyleString)>::value;
		// std::cout << "index " << index << " is " << arg.plotString() << std::endl;

		if (index >= 0) {
			std::get<index>(canvasStyleString).assign(arg.plotString());
		}

		processCanvasArgs(args...);
    }

    template <typename T, typename... Ts,
              std::enable_if_t<!is_canvas_property<T>::value, int> = 0>
    void processCanvasArgs(const T& arg, const Ts&... args)
    {
    	// Do nothing: remove this argument and continue
		processCanvasArgs(args...);
    }

    template <typename T, typename... Ts,
              std::enable_if_t<is_canvas_property<T>::value, int> = 0>
    void processPlotArgs(const T& arg, const Ts&... args)
    {
    	// Do nothing: remove this argument and continue
    	processPlotArgs(args...);
    }

    template <typename T, typename... Ts,
              std::enable_if_t<is_plot_property<T>::value, int> = 0>
    void processPlotArgs(const T& arg, const Ts&... args)
    {
		constexpr size_t index = tuple_contains_type<typename T::PlotStyle, decltype(plotStyleString)>::value;
		// std::cout << "index " << index << " is " << arg.plotString() << std::endl;

		if (index >= 0) {
			std::get<index>(plotStyleString).assign(arg.plotString());
		}

		processPlotArgs(args...);
    }

    template <typename T, typename... Ts,
              std::enable_if_t<!is_plot_property<T>::value &&
                               !is_canvas_property<T>::value, int> = 0>
    void processPlotArgs(const T& arg, const Ts&... args)
    {
    	// Actual objects to plot
    	plotObject(*gnuplot_, arg);


		// constexpr size_t index = tuple_contains_type<typename T::PlotStyle, decltype(plotStyleString)>::value;
		// std::cout << "index " << index << " is " << arg.plotString() << std::endl;

		// if (index >= 0) {
			// std::get<index>(plotStyleString).assign(arg.plotString());
		// }

		processPlotArgs(args...);
    }

    template <typename... Ts>
	void plot(const Ts&... args)
	{
		processCanvasArgs(args...);
		// auto t = std::make_tuple(args...);

    	writeStyleString(*gnuplot_, canvasStyleString);

    	// enable multiplot

		processPlotArgs(args...);

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

