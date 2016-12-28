#include <string>

#include "qplot/qplot.h"

struct Filename
{
	Filename(const std::string& filename) : filename(filename) {}
	static constexpr int pass = 1;

	void plot(Process<Gnuplot>& gnuplot) const
	{
        gnuplot << "set output '" << filename << ".png'\n"
                << "set terminal pngcairo size 500, 500\n";
    }
	std::string filename;
};

struct Colours
{
	enum Palette {
		OCEAN,
		RAINBOW,
		HOT
	};

	Colours(Palette palette) : palette_(palette) {}
	static constexpr int pass = 0;

	void plot(Process<Gnuplot>& gnuplot) const {
		switch (palette_)
		{
			case OCEAN:    gnuplot << "set palette rgbformulae 23,28,3 \n";  break;
			case RAINBOW:  gnuplot << "set palette rgbformulae 33,13,10\n";  break;
			case HOT:      gnuplot << "set palette rgbformulae 21,22,23\n";  break;
		}

	}
	Palette palette_;
};
