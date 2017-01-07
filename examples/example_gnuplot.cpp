#include <tuple>
#include <array>
#include <cmath>

#include "ioscript/ioscript.h"
#include "ioscript/gnuplot.h"

using namespace std;
using namespace iosc;

namespace {

constexpr int SIZE_M = 20;
constexpr int SIZE_N = 20;

template <size_t M, size_t N>
using Array2d = std::array<std::array<int, M>, N>;

using MyArray = Array2d<SIZE_M,SIZE_N>;

// By default Gnuplot takes data on stdin, expected after a `plot '-'` statement
void sendData(Process<Gnuplot>& gnuplot, const MyArray& arr)
{
    for (int i=0; i<arr.size(); i++)
    {
        for (int j=0; j<arr[0].size(); j++)
        {
            gnuplot << i << " " << j << " " << arr[i][j] << '\n';
        }
    }
    gnuplot << "e" << endl;  // 'e' is Gnuplot's terminating character
}

struct HeatMap
{
	template<typename T>
	void operator()(Process<Gnuplot>& gnuplot, const T& obj) const
	{
    	gnuplot << "plot '-' using 1:2:3 with image" << endl;
        sendData(gnuplot, obj);
	}
};

struct NumberGrid
{
	template<typename T>
	void operator()(Process<Gnuplot>& gnuplot, const T& obj) const
	{
    	gnuplot
            <<  "plot '-' using 1:2:3 with image"
            <<     ", '-' using 1:2:3 with labels font \"PTMono,8\"" << endl;

        // Gnuplot requires the data is resent for additional plots
        sendData(gnuplot, obj);
    	sendData(gnuplot, obj);
	}
};

struct ContourPlot
{
    template<typename T>
    void operator()(Process<Gnuplot>& gnuplot, const T& obj) const
    {
        gnuplot
            << "set dgrid3d " << SIZE_M << ", " << SIZE_N << "\n"
            << "set contour surface\n"
            << "splot '-' using 1:2:3 with lines linetype 2 linewidth 1" << endl;

        sendData(gnuplot, obj);

        gnuplot << "unset contour\n"
                << "unset dgrid3d" << endl;
    }
};

struct Header
{
    void operator()(Process<Gnuplot>& gnuplot) const {
        gnuplot
            << "set terminal png size 640, 480\n"
            << "set output 'output.png'" << endl;
    }
};

struct Filename
{
    void operator()(Process<Gnuplot>& gnuplot) const {
        gnuplot << "set output '" << filename << ".png'" << endl;
    }
    std::string filename = "output.png";
};

struct ImageSize
{
    void operator()(Process<Gnuplot>& gnuplot) const {
        gnuplot << "set terminal png size " << size_x << ", " << size_y << endl;
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

    void operator()(Process<Gnuplot>& gnuplot) const {
        switch (palette)
        {
            case OCEAN:    gnuplot << "set palette rgbformulae 23,28,3 " << endl;  break;
            case RAINBOW:  gnuplot << "set palette rgbformulae 33,13,10" << endl;  break;
            case HOT:      gnuplot << "set palette rgbformulae 21,22,23" << endl;  break;
        }
    }

    Palette palette;
};

} // namespace


using Scalar2D = iosc::variant<HeatMap, NumberGrid, ContourPlot>;

template <>                   struct binds_to<MyArray>      { using type = Scalar2D; };

// Alternatively, associate Array2d's of all sizes with Scalar2D
template <size_t M, size_t N> struct binds_to<Array2d<M,N>> { using type = Scalar2D; };

using MyTypes = std::tuple<MyArray>;

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

	Script<Gnuplot,MyTypes> script(Header{}, HeatMap{}, Colours{Colours::RAINBOW});
    script.run(Filename{"Grid1"}, array);
    script.run(Filename{"Grid2"}, ContourPlot{}, array);
    script.run(Filename{"Grid3"}, ImageSize{800,600}, array);     // Same as Grid1, but larger

    script.addToHeader(Colours{Colours::OCEAN});
    script.run(array);
}
