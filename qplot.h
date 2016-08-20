#pragma once

#include <tuple>
#include <type_traits>

#include "gnuplot.h"


struct image_size_canvas_property{};

struct ImageSize
{
	using CanvasType = image_size_canvas_property;

	std::string plotString() {
		return "set terminal pngcairo size " + std::to_string(x) + "," + std::to_string(y) + "\n";
	}
	int x = 800;
	int y = 640;
};

struct grid_object_style : std::string {};

struct HeatMapStyle
{
	using ObjectStyle = grid_object_style;
};


// http://stackoverflow.com/a/25958302/254035
template <typename T, typename Tuple>
struct has_type;

template <typename T>
struct has_type<T, std::tuple<>> : std::false_type {};

template <typename T, typename... Ts>
struct has_type<T, std::tuple<T, Ts...>> : std::true_type {};

template <typename T, typename U, typename... Ts>
struct has_type<T, std::tuple<U, Ts...>> : has_type<T, std::tuple<Ts...>> {};

template <typename T, typename Tuple>
using tuple_contains_type = typename has_type<T, Tuple>::type;

std::tuple<image_size_canvas_property,
           grid_object_style
          > styleString{};

class Qplot
{
public:
    enum { DRY_RUN, STDOUT, GNUPLOT };
    static constexpr int target = STDOUT;

	friend Qplot& operator<<(Qplot& qplot, const std::string& str);

	Qplot()
    {
        switch (target)
        {
            case DRY_RUN: gnuplot_ = std::make_unique<Gnuplot>("cat > /dev/null");  break;
            case STDOUT:  gnuplot_ = std::make_unique<Gnuplot>("cat");              break;
            case GNUPLOT: gnuplot_ = std::make_unique<Gnuplot>("gnuplot");          break;
            default : assert(false);
        }
    }

    template <typename T, typename... Ts>
    void processArgs(const T& arg, const Ts&... args)
    {

    }

    template <typename... Ts>
	void plot(const Ts&... args)
	{
		auto t = std::make_tuple(args...);

		std::cout << std::boolalpha;
		std::cout << tuple_contains_type<ImageSize::CanvasType, decltype(styleString)>::value << std::endl;

		// processArgs(args...);


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

