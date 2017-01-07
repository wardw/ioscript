#pragma once

#include "qplot/qplot.h"

namespace qp {

#ifdef QPLOT_DEBUG
    struct Python  { static constexpr const char* cmd = "cat"; };
#else
    struct Python  { static constexpr const char* cmd = "python"; };
#endif

// See README.md and examples_process.cpp for details
struct PythonHeader
{
    void operator()(Subprocess<Python>& python) const
    {
        python.out()
            << "# This header has been added by Qplot. See qplot.h\n"
            << "import os\n"
            << "qp_data_in = list()\n"
            << "\n";

        for (int i=0; i<python.numChannels(); i++) {
            python.out()
                << "os.close(" << python.fd_w(i) << ")\n"
                << "qp_data_in.append(os.fdopen(" << python.fd_r(i) << ", 'r'))\n"
                << "\n";
        }
    }
};

template <typename S>
void addPrivateHeader(Qplot<Python,S>& python)
{
    python.addToHeader(PythonHeader{});
}

} // namespace
