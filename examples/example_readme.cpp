#include "qplot/qplot.h"
#include <vector>

using namespace qp;

struct LineChart
{
    template <typename T>
    void operator()(Subprocess<Python>& python, const T& obj)
    {
        python << R"(
import matplotlib.pyplot as plt

vals = map(int, qp_data_in.readline().split())
plt.plot(vals, 'o-')
)" << std::endl;

        for (auto& elem : obj) {
            python.data_out() << elem << " ";
        }
        python.data_out() << std::endl;
    }
};

template <> struct has_styles<std::vector<int>> { using type = variant<LineChart>; };

using MyTypes = std::tuple<std::vector<int>>;

void example_readme()
{
    std::vector<int> series1{0,1,1,2,3,5,8,13,21,34,55,89};
    std::vector<int> series2{0,1,3,6,10,15,21,28,36,45,55,66,78,91};

    auto show = [](Subprocess<Python>& py) {
        py << "plt.show()" << std::endl;
    };

    qp::Qplot<Python,MyTypes> qp;
    qp.plot(LineChart{}, series1, series2, show);
}
