#include "examples/gnuplot_styles_common.h"

#include <vector>
#include <tuple>
#include <fstream>

#include "qplot/qplot.h"

using namespace std;

using Array2d = std::array<std::array<int, 10>, 10>;

struct HeatMap
{
	using supported_types = std::tuple<Array2d>;

	template<typename T>
	void plot(Qplot<Gnuplot>& gnuplot, const T& obj) const
	{
    	gnuplot <<  "plot '-' using 1:2:3 with image\n";

    	for (int i=0; i<obj.size(); i++)
    	{
    		for (int j=0; j<obj[0].size(); j++)
    		{
    			gnuplot << i << " " << j << " " << obj[i][j] << '\n';
    		}
    	}
    	gnuplot << "e\n";
	}
};

struct NumberGrid
{
	using supported_types = std::tuple<Array2d>;

	template<typename T>
	void plot(Qplot<Gnuplot>& gnuplot, const T& obj) const
	{
    	gnuplot <<  "plot '-' using 1:2:($3 == 0 ? \"\" : sprintf(\"%4.2f\",$3) ) with labels font \"PTMono,8\"\n";
    	// gnuplot <<  ", '-' using 1:2:($3 == 0 ? \"\" : sprintf(\"%4.2f\",$3) ) with labels font \"PTMono,8\"\n";

    	for (int i=0; i<obj.size(); i++)
    	{
    		for (int j=0; j<obj[0].size(); j++)
    		{
    			gnuplot << i << " " << j << " " << obj[i][j] << '\n';
    		}
    	}
    	gnuplot << "e\n";
	}
};

struct Scalar2d { static constexpr size_t id = 0; using supported_styles = std::variant<HeatMap, NumberGrid>; };

template <> struct plot_traits<Array2d> { using type = Scalar2d; };

void example2()
{
	Array2d array;
	for (int i=0; i<array.size(); i++)
    {
    	for (int j=0; j<array[0].size(); j++)
  		{
			array[i][j] = i - j;
    	}
    }

	Qplot<Gnuplot> qplot(Colours{Colours::RAINBOW});
    qplot.plot(Filename("grid1"), Colours{Colours::HOT}, HeatMap(), array);
    qplot.plot(Filename("grid2"), HeatMap(), array);  // this will be rainbow

    qplot.addToHeader(Colours{Colours::OCEAN});
    qplot.plot(Filename("grid3"), HeatMap(), array);
    qplot.plot(Filename("grid4"), HeatMap(), array);

    // qplot.plot(Filename("grid2"), HeatMap(), array, NumberGrid(), array);
}
