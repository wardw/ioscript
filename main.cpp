#include <iostream>
#include <vector>

#include "qplot.h"

using namespace std;

#include <iostream>

int main()
{
    using T = std::tuple<int, double, char>;
   
    // std::cout << std::boolalpha;

    // std::cout << tuple_contains_type<int, T>::value << std::endl;
    // std::cout << tuple_contains_type<double, T>::value << std::endl;
    // std::cout << tuple_contains_type<char, T>::value << std::endl;

    // std::cout << tuple_contains_type<float, T>::value << std::endl;
    // std::cout << tuple_contains_type<short, T>::value << std::endl;
    // std::cout << tuple_contains_type<long, T>::value << std::endl;

    // std::cout << std::is_same<tuple_contains_type<int, T>, std::true_type>::value << std::endl;

    // std::cout << tuple_contains_type<int, T>::value << std::endl;
    // std::cout << tuple_contains_type<double, T>::value << std::endl;
    // std::cout << tuple_contains_type<char, T>::value << std::endl;
    // std::cout << tuple_contains_type<long, T>::value << std::endl;


    cout << std::get<0>(canvasStyleString);

	std::vector<int> ints = {1,2,3,4,5,6,7};

	Qplot qplot;
	qplot.plot(AxisExtents{{0,1}, {0,2}}, ImageSize{400,300});
	// qplot.plot(ints, AxisExtents{{0,1}, {0,2}}, ImageSize{400,300}, HeatMap{});

	qplot << "\nsometime later...\n\n";
    
    qplot.plot(ints, BarChart(), ints, ints, LineChart(), ints);

	qplot << "reset\n";

    cout << "has_pass: " << has_pass<LineChart>::value << endl;
}
