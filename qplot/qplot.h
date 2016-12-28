#pragma once

#include <tuple>
#include <type_traits>
#include <variant>
#include "process.h"
#include "util.h"

// todo
#include "examples/styles.h"

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

using NoStyles = std::tuple<>;
// using ChartStyles = NoStyles;
using ChartStyles = MyStyles;

template <typename P>
class Qplot
{
    std::unique_ptr<Process<P>> process_ = nullptr;

public:

	Qplot() : process_(std::make_unique<Process<P>>()) {}
	ChartStyles chartStyles_;

    void processArgs() {}

	// Arg is an object style
    template <typename T, typename... Ts,
			  int pass = T::pass,
              std::enable_if_t<has_supported_types<T>::value, int> = 0>
    void processArgs(const T& style, const Ts&... args)
    {
		// Nothing to plot now, but update our current chartStyles_ with this style, for all variants that support it
		using ObjTypes = typename T::supported_types;
		TupleUpdater<T, ChartStyles, ObjTypes, std::tuple_size_v<ObjTypes>>::update(chartStyles_, style);
	
		processArgs(args...);
    }

	// Arg is a canvas style
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
			// int I = chart_traits<T>::index>
    void processArgs(const T& obj, const Ts&... args)
    {
		// Get the style variant that's currently associated with the arg type T
		auto styleVariants = std::get<chart_traits<T>::index>(chartStyles_);

		// Plot this object
		std::visit([this,&obj](auto&& arg) {
			plotObject(*process_, arg, obj);
		}, styleVariants);

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
