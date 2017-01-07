#include "qplot/qplot.h"
#include "qplot/python.h"
#include <vector>

using namespace qp;

namespace {

struct LineChart {
    template <typename T>
    void operator()(Subprocess<Python>& python, const T& obj) const
    {
        python << R"(
import matplotlib.pyplot as plt
vals = map(int, qp_data_in[0].readline().split())
plt.plot(vals, 'o-')
)";
        for (auto& elem : obj) {
            python.data_out(0) << elem << " ";
        }
        python.data_out(0) << std::endl;
    }
};

struct Show {
    void operator()(Subprocess<Python>& python) const { python << "plt.show()\n"; }
};

} // namespace

using MyTypes = std::tuple<std::vector<int>>;

template <>
struct has_styles<std::vector<int>> { using type = variant<LineChart>; };

int example_readme()
{
    std::vector<int> series1{0,1,1,2,3,5,8,13,21,34,55,89};
    std::vector<int> series2{0,1,3,6,10,15,21,28,36,45,55,66,78,91};

    Qplot<Python,MyTypes> qplot;
    qplot.plot(LineChart{}, series1, series2, Show{});

    return 0;
}
