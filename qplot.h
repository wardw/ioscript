#pragma once

#include <tuple>
#include <type_traits>
#include <variant>
#include "process.h"
#include "util.h"

#include "styles.h"


// This is called for every object to plot, along with the it's currently associated style
// Overload to completely govern how this type is plotted
template <typename P, typename Style, typename T>
void plotObject(Process<P>& gnuplot, const Style& style, const T& obj)
{
	style.plot(gnuplot, obj);
}

using NoStyles = std::tuple<>;
// using ChartStyles = NoStyles;
using ChartStyles = MyStyles;

class Qplot
{
    std::unique_ptr<Process<Gnuplot>> process_ = nullptr;

public:
    enum class Target { DRY_RUN, STDOUT, GNUPLOT };
    static constexpr Target target = Target::STDOUT;

	friend Qplot& operator<<(Qplot& qplot, const std::string& str);

	Qplot() : process_(std::make_unique<Process<Gnuplot>>()) {}
	ChartStyles chartStyles_;

    void processArgs() {}

	// Arg is an object style
    template <typename T, typename... Ts,
			  int P = T::pass,
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
			  int P = T::pass,
              std::enable_if_t<!has_supported_types<T>::value, int> = 0>
    void processArgs(const T& style, const Ts&... args)
    {
		style.plot(*process_);

		processArgs(args...);
    }

	// Arg is an object to plot
    template <typename T, typename... Ts,
              std::enable_if_t<!has_pass<T>::value, int> = 0>
			// int I = chart_traits<T>::index>
    void processArgs(const T& obj, const Ts&... args)
    {
		// Get the style object that's currently associated with the arg type T
		auto style = std::get<chart_traits<T>::index>(chartStyles_);

		// Plot this object
		std::visit([this,&obj](auto&& arg) {
			plotObject(*process_, arg, obj);
		}, style);

		processArgs(args...);
    }

    template <typename... Ts>
	void plot(const Ts&... args)
	{
		processArgs(args...);
	}
};

// Direct pass-through to gnuplot
Qplot& operator<<(Qplot& qp, const std::string& str)
{
	*qp.process_ << str;
	return qp;
}

// Main convenience method for one-liner plotting
template <typename... Ts>
void qplot(const Ts&... args)
{
    static Qplot qp;
    qp.plot(args...);
}
