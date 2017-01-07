#pragma once

#include "ioscript/ioscript.h"

namespace iosc {

#ifdef QPLOT_DEBUG
    struct Python  { static constexpr const char* cmd = "cat"; };
#else
    struct Python  { static constexpr const char* cmd = "python"; };
#endif

// See README.md and examples_process.cpp for details
struct PythonHeader
{
    void operator()(Process<Python>& python) const
    {
        python.out()
            << "# This header has been added by Script. See ioscript.h\n"
            << "import os\n"
            << "iosc_in = list()\n"
            << "\n";

        for (int i=0; i<python.numChannels(); i++) {
            python.out()
                << "os.close(" << python.fd_w(i) << ")\n"
                << "iosc_in.append(os.fdopen(" << python.fd_r(i) << ", 'r'))\n"
                << "\n";
        }
    }
};

template <typename S>
void addPrivateHeader(Script<Python,S>& python)
{
    python.addToHeader(PythonHeader{});
}

} // namespace iosc
