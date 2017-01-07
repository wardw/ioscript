#pragma once

#include "ioscript.h"

namespace iosc {

#ifdef QPLOT_DEBUG
	struct Gnuplot { static constexpr const char* cmd = "cat"; };
#else
	struct Gnuplot { static constexpr const char* cmd = "gnuplot"; };
#endif

// For Gnuplot there's nothing to do. Gnuplot accepts data inline with code on it's standard input
template <typename S>
void addPrivateHeader(Script<Gnuplot,S>& python)
{
}

} // namespace iosc
