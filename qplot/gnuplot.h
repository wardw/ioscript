#pragma once

#include "qplot/qplot.h"

namespace qp {

#ifdef QPLOT_DEBUG
	struct Gnuplot { static constexpr const char* cmd = "cat"; };
#else
	struct Gnuplot { static constexpr const char* cmd = "gnuplot"; };
#endif

// For Gnuplot there's nothing to do. Gnuplot accepts data inline with code on it's standard input
template <typename S>
void addPrivateHeader(Qplot<Gnuplot,S>& python)
{
}

} // namespace qp
