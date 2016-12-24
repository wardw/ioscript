#pragma once


struct ImageSize
{
	static constexpr int pass = 0;

	std::string plotString() const {
		return "set image size " + std::to_string(x) + "," + std::to_string(y) + "\n";
	}
	int x = 800;
	int y = 640;
};

struct AxisExtents
{
	static constexpr int pass = 0;
	
	std::string plotString() const {
		return "set axis extents\n";
	}
	int x[2] = {0,1};
	int y[2] = {0,1};
};

struct LineChart
{
	static constexpr int pass = 1;
	using supported_types = std::tuple<std::vector<int>,std::vector<float>>;

	template<typename T>
	std::string plotString(const T& obj) const {
		return "plot LineChart with " + objName(obj) + "\n";
	}
};

struct BarChart
{
	static constexpr int pass = 1;
	using supported_types = std::tuple<std::vector<int>,std::vector<float>>;

	template<typename T>
	std::string plotString(const T& obj) const {
		return "plot BarChart with " + objName(obj) + "\n";
	}
};

struct Point2 { float x; float y; };

struct ScatterChart
{
	static constexpr int pass = 1;
	using supported_types = std::tuple<std::vector<Point2>>;

	template<typename T>
	std::string plotString(const T& obj) const {
		return "plot ScatterChart with " + objName(obj) + "\n";
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
	std::string plotString(const T& obj) const { return "set style heat map\n"; }
	std::string plotStringPost() const { return "unset style heat map\n"; }
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

