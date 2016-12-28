#pragma once

#include <unordered_map>
#include <tuple>
#include <type_traits>
#include <variant>
#include <any>
#include "process.h"
#include "util.h"

// todo
// #include "examples/styles.h"

// These non-members are called for every object to plot, along with the it's currently associated style
// Overload to completely govern how this type is plotted

// Plotting styles associated to objects

template <typename P, typename Style, typename T>
void plotObject(Process<P>& process, const Style& style, const T& obj)
{
    // style.plot(process, obj);
}

// Note that this will be instantiated for all style variants associated with the obj type (regardless of the actual obj type)
template <typename Style, typename T,
          std::enable_if_t<has_plot<Style, void(Process<Gnuplot>&,const T&)>::value, int> = 0>
void plotObject(Process<Gnuplot>& gnuplot, const Style& style, const T& obj)
{
    style.plot(gnuplot, obj);
}

template <typename Style, typename T,
          std::enable_if_t<has_plot<Style, void(Process<Mpl>&,const T&)>::value, int> = 0>
void plotObject(Process<Mpl>& mpl, const Style& style, const T& obj)
{
    style.plot(mpl, obj);
}

// Plotting styles with no associated object

template <typename P, typename Style>
void plotStyle(Process<P>& process, const Style& style)
{
    // style.plot(process);
}

template <typename Style,
          std::enable_if_t<has_plot<Style, void(Process<Gnuplot>&)>::value, int> = 0>
void plotStyle(Process<Gnuplot>& gnuplot, const Style& style)
{
    style.plot(gnuplot);
}

template <typename Style,
          std::enable_if_t<has_plot<Style, void(Process<Mpl>&)>::value, int> = 0>
void plotStyle(Process<Mpl>& mpl, const Style& style)
{
    style.plot(mpl);
}

using PlotStyles = std::unordered_map<size_t, std::any>;
// using PlotStyles = std::array<std::any, MAX_STYLES>;

// Specializations in client code
template <typename T> struct plot_traits;

template <typename P>
class Qplot
{
    std::unique_ptr<Process<P>> process_ = nullptr;

public:

	Qplot() : process_(std::make_unique<Process<P>>()) {}

    PlotStyles plotStyles_;

    void processArgs() {}

	// Arg is style (for an object)
    template <typename T, typename... Ts,
			  int pass = T::pass,
              std::enable_if_t<has_supported_types<T>::value, int> = 0>
    void processArgs(const T& style, const Ts&... args)
    {
		// Nothing to plot now, but update our plotStyles_ with this style where variants that support it
		using ObjTypes = typename T::supported_types;
        constexpr size_t NumObjects = std::tuple_size_v<ObjTypes>;
		MapUpdater<T, PlotStyles, ObjTypes, NumObjects>::update(plotStyles_, style);
	
		processArgs(args...);
    }

	// Arg is a style (for the canvas)
    template <typename T, typename... Ts,
			  int pass = T::pass,
              std::enable_if_t<!has_supported_types<T>::value, int> = 0>
    void processArgs(const T& style, const Ts&... args)
    {
        plotStyle(*process_, style);

		processArgs(args...);
    }

	// Arg is an object to plot
    template <typename T, typename... Ts,
              std::enable_if_t<!has_pass<T>::value, int> = 0>
    void processArgs(const T& obj, const Ts&... args)
    {
		// Get the style variant that's currently associated with the arg type T
        size_t key = plot_traits<T>::type::id;
        using StyleVariant = typename plot_traits<T>::type::supported_styles;
		auto styleVar = std::any_cast<StyleVariant>(plotStyles_[key]);

		// Plot this object
		std::visit([this,&obj](auto&& arg) {
			plotObject(*process_, arg, obj);
		}, styleVar);

		processArgs(args...);
    }

    template <typename... Ts>
	void plot(const Ts&... args)
	{
		processArgs(args...);
	}

    friend Qplot& operator<<(Qplot& qplot, const std::string& str) {
        *qplot.process_ << str;
        return qplot;
    }

};

// Main convenience method for one-liner plotting
// template <typename... Ts>
// void qplot(const Ts&... args)
// {
//     static Qplot qp;
//     qp.plot(args...);
// }
