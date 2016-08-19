#pragma once

#include "gnuplot.h"


class Qplot
{
public:
    enum { DRY_RUN, STD_OUT, GNUPLOT };
    static constexpr int target = STD_OUT;

	friend Qplot& operator<<(Qplot& qplot, const std::string& str);

	Qplot()
    {
        switch (target)
        {
            case DRY_RUN: gnuplot_ = std::make_unique<Gnuplot>("cat > /dev/null");  break;
            case STD_OUT: gnuplot_ = std::make_unique<Gnuplot>("cat");              break;
            case GNUPLOT: gnuplot_ = std::make_unique<Gnuplot>("gnuplot");          break;
            default : assert(false);
        }
    }

private:
    std::unique_ptr<Gnuplot> gnuplot_ = nullptr;
};

Qplot& operator<<(Qplot& qplot, const std::string& str)
{
	*qplot.gnuplot_ << str;
	return qplot;
}
