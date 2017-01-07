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
	Process<Ruby> ruby;

	ruby << "puts 'Hello, world!'" << endl;

	// Send some data via the data channel
	ruby.data_out()
		<< "22 67 14 42\n"
		<< "44 47 42 19\n"
		<< "02 11\n";

	// First, close the inherited but unused write end of the pipe on the Ruby side
	ruby
		<< "a = IO.new(" << ruby.fd_w() << ", \"w\")\n"
		<< "a.close\n";

	// Get data and process as one integer array per line
	ruby << R"(
io = IO.open()" << ruby.fd_r() << R"(, "r")
io.each_line {|line| puts "vals: #{line.split().map { |s| s.to_i }}" }
)" << endl;
}

void example2()
{
	Process<Python> python;

	python << "print \"Hello, world!\"" << endl;

	// Send some data via the data channel
	python.data_out()
		<< "22 67 14 42\n"
	    << "44 47 42 19\n"
	    << "02 11\n";

	// Close the write end
	python << "import os\n"
	       << "os.close(" << python.fd_w() << ")\n";

	// Create a file object to the read end
	python << "iosc_in = os.fdopen(" << python.fd_r() << ", 'r')\n"
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
