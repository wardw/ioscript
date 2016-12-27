#pragma once


struct ImageSize
{
	static constexpr int pass = 0;

	void plot(Process<Gnuplot>& gnuplot) const {
		gnuplot << "set image size " + std::to_string(x) + "," + std::to_string(y) + "\n";
	}
	int x = 800;
	int y = 640;
};

struct AxisExtents
{
	static constexpr int pass = 0;
	
	void plot(Process<Gnuplot>& gnuplot) const {
		gnuplot << "set axis extents\n";
	}
	int x[2] = {0,1};
	int y[2] = {0,1};
};

template <typename T>
void sendData(Process<Gnuplot>& gnuplot, const std::vector<T>& obj)
{
    for (int i=0; i<obj.size(); ++i) {
        gnuplot << std::to_string(i) << " " << std::to_string(obj[i]) << "\n";
    }
}

struct LineChart
{
	static constexpr int pass = 1;
	using supported_types = std::tuple<std::vector<int>,std::vector<float>>;

	template<typename T>
	void plot(Process<Gnuplot>& gnuplot, const T& obj) const {
        gnuplot << "plot '-' using 1:2\n";
        // sendData(gnuplot, obj);
        gnuplot << "e\n";
	}

	template<typename T>
	void plot(Process<Mpl>& mpl, const T& obj) const {
        // mpl << "print \"hello python\"\n";
        mpl <<
R"(
import os
os.close(4)
fo = os.fdopen(3, 'r')
for line in fo:
    print "(Python) " + line,
print "(Python) Done"
)"
		<< "\n";

		mpl.fdout() << "1\n2\n3\n4\n5\n";
	}
};

struct BarChart
{
	static constexpr int pass = 1;
	using supported_types = std::tuple<std::vector<int>,std::vector<float>>;

	template<typename T>
	void plot(Process<Gnuplot>& gnuplot, const T& obj) const {
		gnuplot << "plot BarChart with " + objName(obj) + "\n";
	}

	template<typename T>
	void plot(Process<Mpl>& gnuplot, const T& obj) const {
        gnuplot << "plot '-' using 1:2\n";
        // sendData(gnuplot, obj);
        gnuplot << "e\n";
	}
};

struct Point2 { float x; float y; };

struct ScatterChart
{
	static constexpr int pass = 1;
	using supported_types = std::tuple<std::vector<Point2>>;

	template<typename T>
	void plot(Process<Gnuplot>& gnuplot, const T& obj) const {
		gnuplot << "plot ScatterChart with " + objName(obj) + "\n";
	}
};

struct Row
{
	int rows[10] = {};
};

struct HeatMap
{
	static constexpr int pass = 1;
	using supported_types = std::tuple<>;

	template<typename T>
	void plot(Process<Gnuplot>& gnuplot, const T& obj) const { gnuplot << "set style heat map\n"; }
	// void plotPost() const { gnuplot << "unset style heat map\n"; }
};

struct VectorIntRow
{
	VectorIntRow(const std::vector<int>& vec) : vec_(vec) {}
	Row operator()(int row) { return row < vec_.size() ? Row{row, vec_[row]} : Row{}; }
	const std::vector<int>& vec_;
};


using MyStyles = std::tuple<std::variant<LineChart, BarChart>,
                            std::variant<HeatMap>
							>;

template <typename T> struct chart_traits;
template <> struct chart_traits<std::vector<int>>    { static constexpr size_t index = 0; };
template <> struct chart_traits<std::vector<float>>  { static constexpr size_t index = 0; };
template <> struct chart_traits<std::vector<Point2>> { static constexpr size_t index = 1; };

// All vector<T>'s are index 0 (check)
//template <typename T> struct chart_traits<std::vector<T>> { static constexpr index = 0 };

