#include <tuple>
#include <array>

#include "qplot/qplot.h"

using namespace std;
using namespace qp;

constexpr int SIZE_M = 20;
constexpr int SIZE_N = 20;

template <size_t M, size_t N>
using Array2d = std::array<std::array<int, M>, N>;

using MyArray = Array2d<SIZE_M,SIZE_N>;

void sendData(Subprocess<Gnuplot>& gnuplot, const MyArray& arr)
{
    for (int i=0; i<arr.size(); i++)
    {
        for (int j=0; j<arr[0].size(); j++)
        {
            gnuplot.out() << i << " " << j << " " << arr[i][j] << '\n';
        }
    }
    gnuplot.out() << "e\n";  // 'e' is Gnuplot's terminating character
}

struct HeatMap
{
	template<typename T>
	void operator()(Subprocess<Gnuplot>& gnuplot, const T& obj) const
	{
    	gnuplot.out() <<  "plot '-' using 1:2:3 with image\n";
        sendData(gnuplot, obj);
	}
};

struct NumberGrid
{
	template<typename T>
	void operator()(Subprocess<Gnuplot>& gnuplot, const T& obj) const
	{
    	gnuplot.out()
            <<  "plot '-' using 1:2:3 with image"
            <<     ", '-' using 1:2:3 with labels font \"PTMono,8\""
            << '\n';

        // Gnuplot requires the data is resent for additional plots
        sendData(gnuplot, obj);
    	sendData(gnuplot, obj);
	}
};

struct ContourPlot
{
    template<typename T>
    void operator()(Subprocess<Gnuplot>& gnuplot, const T& obj) const
    {
        gnuplot.out()
            << "set dgrid3d " << SIZE_M << ", " << SIZE_N << "\n"
            << "set contour surface\n"
            << "splot '-' using 1:2:3 with lines linetype 2 linewidth 1\n";

        sendData(gnuplot, obj);

        gnuplot.out() << "unset contour\n"
                << "unset dgrid3d\n";
    }
};

struct Header
{
    void operator()(Subprocess<Gnuplot>& gnuplot) const {
        gnuplot.out()
            << "set terminal png size 640, 480\n"
            << "set output 'output.png'\n";
    }
};

struct Filename
{
    void operator()(Subprocess<Gnuplot>& gnuplot) const {
        gnuplot.out() << "set output '" << filename << ".png'\n";
    }
    std::string filename = "output.png";
};

struct ImageSize
{
    void operator()(Subprocess<Gnuplot>& gnuplot) const {
        gnuplot.out() << "set terminal png size " << size_x << ", " << size_y << "\n";
    }
    unsigned size_x = 640;
    unsigned size_y = 480;
};

struct Colours
{
    enum Palette {
        OCEAN,
        RAINBOW,
        HOT
    };

    Colours(Palette palette) : palette(palette) {}

    void operator()(Subprocess<Gnuplot>& gnuplot) const {
        switch (palette)
        {
            case OCEAN:    gnuplot.out() << "set palette rgbformulae 23,28,3 \n";  break;
            case RAINBOW:  gnuplot.out() << "set palette rgbformulae 33,13,10\n";  break;
            case HOT:      gnuplot.out() << "set palette rgbformulae 21,22,23\n";  break;
        }
    }

    Palette palette;
};


using Scalar2D = qp::variant<HeatMap, NumberGrid, ContourPlot>;

template <> struct has_styles<MyArray> { using type = Scalar2D; };

// Alternatively, associate 2d arrays of all sizes with our Scalar2D variant
template <size_t M, size_t N> struct has_styles<Array2d<M,N>> { using type = Scalar2D; };

using MyStyles = std::tuple<Scalar2D>;
using Qp = Qplot<Gnuplot,MyStyles>;

void example_gnuplot()
{
	MyArray array;
	for (int i=0; i<array.size(); i++)
    {
    	for (int j=0; j<array[0].size(); j++)
  		{
			array[i][j] = pow(i-SIZE_M/2.f,2) - pow(j-SIZE_N/2.f,2);
    	}
    }

	Qp qplot(Header{}, HeatMap{}, Colours{Colours::RAINBOW});
    qplot.plot(Filename{"Grid1"}, array);
    qplot.plot(Filename{"Grid2"}, ContourPlot{}, array);
    qplot.plot(Filename{"Grid3"}, ImageSize{800,600}, array);     // Same as Grid1, but larger

    qplot.addToHeader(Colours{Colours::OCEAN});
    qplot.plot(array);
}
