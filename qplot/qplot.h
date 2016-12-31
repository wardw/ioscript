#pragma once

#include <ostream>
#include <sstream>
#include <unordered_map>
#include <tuple>
#include <type_traits>
#include <variant>
#include <any>

#include "subprocess.h"
#include "util.h"

using PlotStyles = std::unordered_map<size_t, std::any>;
// using PlotStyles = std::array<std::any, MAX_STYLES>;

// Specializations in client code
template <typename T> struct associated_styles;

template <typename T>
struct Styles;

template <typename P, typename S = void>
class Qplot
{
    std::unique_ptr<Subprocess<P>> process_;
    std::ostringstream header_;

    PlotStyles plotStyles_;
    Styles<S> styles_;

public:
    template <typename... Ts>
    Qplot(Ts&&... args) :
        process_(std::make_unique<Subprocess<P>>())
    {
        addToHeader(args...);
    }

    void processArgs() {}

	// Arg is an object-style
    template <typename T, typename... Ts,
              // typename = typename Styles<S>::types ,
              std::enable_if_t<is_object_style<T>::value, int> = 0>
    void processArgs(const T& style, const Ts&... args)
    {
		// Nothing to plot now, but update our plotStyles_ with this style where variants that support it
		using ObjTypes = typename T::supported_types;
        constexpr size_t NumObjects = std::tuple_size_v<ObjTypes>;
		MapUpdater<T, PlotStyles, ObjTypes, NumObjects>::update(plotStyles_, style);
	
        printTuple(std::cout, typename Styles<S>::types{});

		processArgs(args...);
    }

	// Arg is a canvas-style
    template <typename T, typename... Ts,
              std::enable_if_t<is_canvas_style<T,P>::value, int> = 0>
    void processArgs(const T& style, const Ts&... args)
    {
        // plotStyle(*this, style);
        style(*this);

		processArgs(args...);
    }

    // Arg is an object to plot
    template <typename T, typename... Ts,
              std::enable_if_t<!is_object_style<T>::value &&
                               !is_canvas_style<T,P>::value, int> = 0>
    void processArgs(const T& obj, const Ts&... args)
    {
		// Get the style variant that's currently associated with the arg type T
        size_t key = associated_styles<T>::type::id;
        using StyleVariant = typename associated_styles<T>::type::supported_styles;
		auto styleVar = std::any_cast<StyleVariant>(plotStyles_[key]);

		// Plot this object
		std::visit([this,&obj](auto&& style) {
			// plotObject(*this, style, obj);
            style(*this, obj);
		}, styleVar);

		processArgs(args...);
    }

    template <typename... Ts>
	void plot(const Ts&... args)
	{
        // Subprocess header
        *this << header_.str();

        // Recuse into arguments
		processArgs(args...);

        // Finally, close this process and reopen with a fresh instance
        // + This ends out code stream to the process, which e.g. for python alows the process to start executin
        // + Also important that each call to plot (aside from the intentional header) is stateless
        process_.reset();  // destory first
        process_ = std::make_unique<Subprocess<P>>();
	}

    template <typename... Ts>
    void addToHeader(const Ts&... args)
    {
        // Everything to cfout goes to our local ostringstream
        auto cout_buffer = process_->cfout().rdbuf();
        process_->cfout().rdbuf(header_.rdbuf());

        // Capture in local header_ buffer
        processArgs(args...);

        // Swap back
        process_->cfout().rdbuf(cout_buffer);
    }

    // cf_ostream& cfout() { return *cfout_; }
    fd_ostream& fdout() { return process_->fdout(); }

    int fd_r() { return process_->fd_r(); }
    int fd_w() { return process_->fd_w(); }

    template <typename U>
    friend Qplot& operator<<(Qplot& qplot, const U& str)
    {
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


/*
// These non-members provide another level of indirection to enable completely overloading how an object+style is plotted
// Currently disabled as unused, but left here as a future possibility

template <typename P>
class Qplot;

template <typename P, typename Style, typename T,
          std::enable_if_t<!is_object_style<Style,P>::value, int> = 0>
void plotObject(Qplot<P>& process, const Style& style, const T& obj)
{
    // Do nothing where no plot member exists for this Subprocess<P>
}

// Note that this will be instantiated for all style variants associated with the obj type (regardless of the actual obj type)
template <typename P, typename Style, typename T,
          std::enable_if_t<is_object_style<Style,P>::value, int> = 0>
void plotObject(Qplot<P>& process, const Style& style, const T& obj)
{
    style(process, obj);
}


template <typename P, typename Style,
          std::enable_if_t<!is_canvas_style<Style,P>::value, int> = 0>
void plotStyle(Qplot<P>& process, const Style& style)
{
    // Do nothing where no plot member exists for this Subprocess<P>
}

template <typename P, typename Style,
          std::enable_if_t<is_canvas_style<Style,P>::value, int> = 0>
void plotStyle(Qplot<P>& process, const Style& style)
{
    style(process);
}
*/
