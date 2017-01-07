#include "ioscript/ioscript.h"
#include "ioscript/python.h"

using namespace std;
using namespace iosc;

namespace {

// Using Ruby as an example
// asuming Ruby is installed and the command `ruby` is visible in your path
struct Ruby { static constexpr const char* cmd = "ruby"; };

void example1()
{
	// Create a Process object with a single r/w pipe
	// This pipe is referred to as 'channel 0' (by Process) but maps to arbitary system file descriptors
	Process<Ruby> ruby{0};

	ruby << "puts 'Hello, world!'" << endl;

	// Send some data via channel 0
	ruby.data_out(0)
		<< "22 67 14 42\n"
		<< "44 47 42 19\n"
		<< "02 11\n";

	// First, close the inherited but unused write end of the pipe on the Ruby side
	ruby
		<< "a = IO.new(" << ruby.fd_w(0) << ", \"w\")\n"
		<< "a.close\n";

	// Get data and process as one integer array per line
	ruby << R"(
io = IO.open()" << ruby.fd_r(0) << R"(, "r")
io.each_line {|line| puts "vals: #{line.split().map { |s| s.to_i }}" }
)" << endl;
}

void example2()
{
	Process<Python> python{0};

	python << "print \"Hello, world!\"" << endl;

	// Send some data via channel 0
	python.data_out(0)
		<< "22 67 14 42\n"
	    << "44 47 42 19\n"
	    << "02 11\n";

	// Close the write end
	python << "import os\n"
	       << "os.close(" << python.fd_w(0) << ")\n";

	// Create a file object to the read end of channel 0
	python << "iosc_in = os.fdopen(" << python.fd_r(0) << ", 'r')\n"
		   << R"(
for line in iosc_in:
	print "vals:", map(int, line.split())
)" << endl;
}

} // namespace

void example_process()
{
	example1();
	example2();
}
