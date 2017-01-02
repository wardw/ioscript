#include "qplot/qplot.h"

using namespace std;
using namespace qp;

// Using Ruby as an example
// asuming Ruby is installed and the command `ruby` is visible in your path
struct Ruby { static constexpr const char* cmd = "ruby"; };

void example1()
{
	Subprocess<Ruby> ruby;

	ruby.out() << "puts 'Hello, world!'" << '\n';

	// Send some data via the data channel
	ruby.data_out()
		<< "22 67 14 42" << endl
		<< "44 47 42 19" << endl
		<< "02 11" << endl;

	// First, close the inherited but unused write end of the pipe on the Ruby side
	ruby.out()
		<< "a = IO.new(" << ruby.fd_w() << ", \"w\")" << '\n'
		<< "a.close" << '\n';

	// Get data and process as one integer array per line
	ruby.out() << R"(
io = IO.open()" << ruby.fd_r() << R"(, "r")
io.each_line {|line| puts "vals: #{line.split().map { |s| s.to_i }}" }
)" << '\n';
}

void example2()
{
	Subprocess<Python> python;

	python.out() << "print \"Hello, world!\"" << '\n';

	// Send some data via the data channel
	python.data_out() << "22 67 14 42" << endl
	               << "44 47 42 19" << endl
	               << "02 11" << endl;

	// Close the write end
	python.out() << "import os" << '\n'
	       << "os.close(" << python.fd_w() << ")" << '\n';

	// Create a file object to the read end
	python.out()
		<< "fo = os.fdopen(" << python.fd_r() << ", 'r')" << '\n'
		<< R"(
for line in fo:
	print "vals:", map(int, line.split())
)" << '\n';
}

void example_process()
{
	example1();
	example2();
}
