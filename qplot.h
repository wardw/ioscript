#pragma once

#include <tuple>
#include <type_traits>
#include <variant>
#include "gnuplot.h"
#include "util.h"

#include "styles.h"

template <typename T>
struct plot_defaults;

template <>
struct plot_defaults<std::vector<int>> {
	// using PlotStyle = image_map_style;
	// using RowType = VectorIntRow;
};

Row writeRow(std::vector<int>& vec, int row)
{
	return row < vec.size() ? Row{row, vec[row]} : Row{};
}

decltype(auto) rowData(const std::vector<int>& vec)
{

}

template <typename Style, typename T>
void plotObject(Gnuplot& gnuplot, const Style& style, const T& obj)
{
	gnuplot << style.plotString(obj);
}


using NoStyles = std::tuple<>;

// using ChartStyles = NoStyles;
using ChartStyles = MyStyles;


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

	ChartStyles chartStyles;

    void processArgs() {}

	// Arg is an object style
    template <typename T, typename... Ts,
			  int P = T::pass,
              std::enable_if_t<has_supported_types<T>::value, int> = 0>
    void processArgs(const T& style, const Ts&... args)
    {
		// Nothing to plot now, but update our current chartStyles with this style, for all variants that support it
		using ObjTypes = typename T::supported_types;
		TupleUpdater<T, ChartStyles, ObjTypes, std::tuple_size_v<ObjTypes>>::update(chartStyles, style);
	
		processArgs(args...);
    }

	// Arg is a canvas style
    template <typename T, typename... Ts,
			  int P = T::pass,
              std::enable_if_t<!has_supported_types<T>::value, int> = 0>
    void processArgs(const T& style, const Ts&... args)
    {
		*gnuplot_ << style.plotString();

		processArgs(args...);
    }

	// Arg is an object to plot
    template <typename T, typename... Ts,
              std::enable_if_t<!has_pass<T>::value, int> = 0>
			// int I = chart_traits<T>::index>
    void processArgs(const T& obj, const Ts&... args)
    {
		// Get the style object that's currently associated with the arg type T
		auto style = std::get<chart_traits<T>::index>(chartStyles);

		// Plot this object
		std::visit([this,&obj](auto&& arg) {
			plotObject(*gnuplot_, arg, obj); 
		}, style);

		processArgs(args...);
    }

    template <typename... Ts>
	void plot(const Ts&... args)
	{
		processArgs(args...);
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
