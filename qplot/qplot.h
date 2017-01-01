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

// With specializations in client code
template <typename T>
struct has_styles;

template <typename P, typename S>
class Qplot
{
    std::unique_ptr<Subprocess<P>> subprocess_;
    std::ostringstream header_;

    S styles_;
    S stylesHeader_;

public:
    template <typename... Ts>
    Qplot(Ts&&... args) :
        subprocess_(std::make_unique<Subprocess<P>>())
    {
        addToHeader(args...);
    }

    void processArgs() {}

	// Arg is a canvas-style only
    template <typename T, typename... Ts,
              std::enable_if_t< is_canvas_style<T,P>::value &&
                               !is_object_style<T,P>::value, int> = 0>
    void processArgs(const T& style, const Ts&... args)
    {
        style(*subprocess_);
		processArgs(args...);
    }

    // Arg is an object-style only
    template <typename T, typename... Ts,
              std::enable_if_t< is_object_style<T,P>::value &&
                               !is_canvas_style<T,P>::value, int> = 0>
    void processArgs(const T& style, const Ts&... args)
    {
        // Update styles_ with this style for all variants in style_ that support it
        constexpr size_t NumStyles = std::tuple_size_v<S>;
        TupleUpdater<T, S, NumStyles>::update(styles_, style);

        processArgs(args...);
    }

    // Arg is both an object-style and canvas_style
    template <typename T, typename... Ts,
              std::enable_if_t<is_object_style<T,P>::value &&
                               is_canvas_style<T,P>::value, int> = 0>
    void processArgs(const T& style, const Ts&... args)
    {
        style(*subprocess_);

        constexpr size_t NumStyles = std::tuple_size_v<S>;
        TupleUpdater<T, S, NumStyles>::update(styles_, style);

        processArgs(args...);
    }

    // Arg is an object to plot
    template <typename T, typename... Ts,
              std::enable_if_t<!is_object_style<T,P>::value &&
                               !is_canvas_style<T,P>::value, int> = 0>
    void processArgs(const T& obj, const Ts&... args)
    {
		// Get the style variant that's currently associated with the arg type T
        constexpr size_t key = tuple_contains_type<typename has_styles<T>::type, S>::value;
        auto styleVar = std::get<key>(styles_);

		// Plot this object
		std::visit([this,&obj](auto&& style) {
            style(*subprocess_, obj);
		}, styleVar);

		processArgs(args...);
    }

    template <typename... Ts>
	void plot(const Ts&... args)
	{
        // Replay the header
        *this << header_.str();

        // Reload state for the chosen alternatives
        styles_ = stylesHeader_;

        // Recurse into arguments
		processArgs(args...);

        // Finally, close this process and reopen with a fresh instance
        // + This ends out code stream to the process, which e.g. for python alows the process to start executin
        // + Also important that each call to plot (aside from the intentional header) is stateless
        subprocess_.reset();  // destory first
        subprocess_ = std::make_unique<Subprocess<P>>();
	}

    template <typename... Ts>
    void addToHeader(const Ts&... args)
    {
        // Everything to cfout goes to our local ostringstream
        auto cout_buffer = subprocess_->cfout().rdbuf();
        subprocess_->cfout().rdbuf(header_.rdbuf());

        // Also save the state of the chosen alternatives
        stylesHeader_ = styles_;

        // Capture in local header_ buffer
        processArgs(args...);

        // Swap back
        subprocess_->cfout().rdbuf(cout_buffer);
    }

    // cf_ostream& cfout() { return *cfout_; }
    fd_ostream& fdout() { return subprocess_->fdout(); }

    int fd_r() { return subprocess_->fd_r(); }
    int fd_w() { return subprocess_->fd_w(); }

    template <typename U>
    friend Qplot& operator<<(Qplot& qplot, const U& str)
    {
        *qplot.subprocess_ << str;
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
