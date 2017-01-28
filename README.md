## ioscript

ioscript is an interface for calling code written in higher level scripting languages directly from C++. The library motivates writing small function objects that embed useful scripting behaviour, either to be called independently or as small scripting 'snippets' to compose as a sequence.  Function objects may also be bound to C++ types and so provide a method to invoke script code directly on C++ objects.

The original use case was for quickly plotting C++ objects to debug scientific code using matplotlib and Gnuplot, however you might find if useful in any context where it's helpful to bind script code directly with C++ types for further processing.

```cpp
    // Example usage: Plot vec1 & vec2 using the python code wrapped in a LineChart,
    //  and vec3 using the python code wrapped in a BarChart, then Show to screen
    Script<Python,MyTypes> script;
    script.run(LineChart{}, vec1, vec2, BarChart{}, vec3, Show{});
```

The library is agnostic to the scripting language, it should work with any runtime that's installed on your system and accepts code via it's standard input. There is a set of examples for using the library with Python, Ruby and Gnuplot, but depending on the language it should be reasonably straightforward to set up with different runtimes.


### Use

The library is a single header plus optional definitions for each language's runtime.  For C++17/1z there are no further dependencies, otherwise there is a toggle to switch to boost for [Boost.Variant][link_boost_variant] (also header-only) (edit: but currently broken).

The interface is currently implemented in terms of POSIX pipes and so unfortunately there is no Windows support. Tested only on macOS, although I expect it to work fine on linux.


### Example

The following example plots the values of a `std::vector` with Python and the matplotlib plotting library.

```cpp
#include "ioscript/ioscript.h"
#include "ioscript/python.h"
#include <vector>

using namespace iosc;

struct LineChart
{
    template <typename T>
    void operator()(Process<Python>& python, const T& obj) const
    {
        python << R"(
import matplotlib.pyplot as plt
vals = map(int, iosc_in[0].readline().split())
plt.plot(vals, 'o-')
)";
        for (auto& elem : obj) {
            python.data_out(0) << elem << " ";
        }
        python.data_out(0) << std::endl;
    }
};

struct Show {
    void operator()(Process<Python>& python) const { python << "plt.show()\n"; }
};

// Associate std::vector with a LineChart
template <>
struct binds_to<std::vector<int>> { using type = variant<LineChart>; };

void main()
{
    std::vector<int> series1{0,1,1,2,3,5,8,13,21,34,55,89};
    std::vector<int> series2{0,1,3,6,10,15,21,28,36,45,55,66,78,91};

    using MyTypes = std::tuple<std::vector<int>>;
    Script<Python,MyTypes> script;

    script.run(LineChart{}, series1, series2, Show{});
}
```

The sections below outline the basic parts of the above example and for using ioscript in general.


### Wrapping script code in C++ function objects

Function objects like `LineChart` embed the scripting code to define the intended behaviour, for example the Python code used to describe how to plot a data type.  For functors that take an object as a second parameter, immediately following is a set of C++ statements to accept the object and describe how that data is sent to the Python subprocess.  In simple cases, since the two parts are kept together there's a direct continuity between how the data is sent and how the data is received.

In the above example, the call `python.iosc_out(0)` returns a `std::ostream` to send the data to the Python subprocess on 'channel 0'. In Python, the read end of channel 0 is represented by the file object `iosc_in[0]`.

If the embedded script code doesn't operate on any C++ object the second parameter to the overloaded `operator()` can be omitted. Since the library only requires that the function object is callable, you could write the code inline with a lambda:

``` cpp
script.run(LineChart{}, series1, series2, [](Process<Python>& py) {
    py << "plt.savefig('vals1.png')" << std::endl;
});
```

Alternatively keep it with a name:

```cpp
struct Title {
    void operator()(Process<Python>& python) const {
        python << "plt.title('" << title << "')" << std::endl;
    }
    const char* title = "Default title";
};
```

Function objects passed to ioscript must be default, copy and copy-assign constructable, but in broad terms the library imposes few requirements on the nature of the functor classes.


### Binding script code to C++ types

The following specifies the relationship that should we pass an object of type `std::vector` to `run()`, we want ioscript to call a `LineChart` function object on that `std::vector`.  To do this, first wrap `LineChart` in a `variant` (of which we only have one alternative) and then add a specialization for the type trait `binds_to<std::vector<int>>` to refer to this variant:

```cpp
// std::vector currently binds only to one alternative (LineChart)
template <>
struct binds_to<std::vector<int>> { using type = variant<LineChart>; };
```

Should you want to associate more data types to work with a `LineChart`, add additional `binds_to<>` statements.  Templates work as expected, too. The following makes a `std::array<T,N>` of any type `T` and size `N` bind for use with our `LineChart` 'snippet':

```cpp
template <typename  T, size_t N>
template<> struct binds_to<std::array<T,N>> { using type = variant<LineChart>; }
```

To call out one piece of housekeeping, you must register the types you expect to use in calls to `run()`.  That is, you must register at least one of every type such that all snippet types are visible to ioscript. This isn't ideal, but is the fundamental mechanism by which the static binding can work.  To do so, define a `std::tuple` that lists your types, and use this as the second parameter to instantiating a `Script<Process,MyTypes>`:

```cpp
using MyTypes = std::tuple<std::vector<int>
                           std::array<int,0>>;  // `0` is sufficient for all array<int,N>

// Later
Script<Python, MyTypes> script;
script.run(...);
```

If you forget to do so ioscript should provide a useful error message to that effect, rather than leave you at the fate of your compiler's template error messages<sup id="br1">[[1]](#fn1)</sup>.


### Calling script code

To execute a sequence of script snippets, instantiate a `Script<Python, MyTypes>` object and call its `run()` member function, passing objects of your snippet types. The call to `run()` takes any number of parameters, with arguments always parsed from left to right and processed in sequential order:

```cpp
std::vector<int> vec1, vec2;
std::array<int,10> arr1;
// Fill containers ...

Script<Python, MyStyles> script;
script.run(LineChart{}, Title{"Number sequences"}, vec1, vec2, arr1, Show{});
```

In the above example, the `run()` call will fork a new Python subprocess, keep a copy of our default-constructed `LineChart`, send our `Title` code to the Python interpreter and then call the `LineChart` copy with each `vector` and `array` argument in turn.

Notice each snippet object can have its own state. For example, you might modify `LineChart` to take a line colour parameter:

```cpp
script.run(LineChart{'r'}, vec1, LineChart{'b'}, vec2, Show{});
```

The basic approach adopted is

+ Any snippet that defines the one-parameter `operator()(Process<P>&)` will be called immediately in sequence.
+ Any snippet that defines the two parameter form `operator()(Process<P>&, const T&)` will instead be called once with each object of type `T` that binds to it.
+ A function object can contain either form of `operator()`, or both, with the above semantics applied to each form in turn.

### Adding additional variations

It's possible to declare a C++ object to be bound to more than one snippet alternative, with the choice of functor to be selected as part of each call to `run()`.  To do so, add alternative snippets as additional types to the relevant `variant<>`'s parameter list. For example, had we defined a `BarChart` and `PieChart` along similar lines to `LineChart`, then

```cpp
// A std::vector<int> is now available for use with three alternative snippet types
struct binds_to<std::vector<int>> { using type = variant<LineChart,BarChart,PieChart>; };
```

Where more than one alternative exists, the alternative chosen is the 'most recent' type passed that binds to that object (that is, as if reading from left to right). For example

```cpp
script.run(LineChart{}, vec1, vec2, BarChart{}, vec2, Show{});
```

will plot `vec1` and `vec2` as a `LineChart`, and `vec2` as a `BarChart`.

If no snippet is explicitly specified, a default-initialized snippet of the type of the first alternative is constructed.  This follows immediately from [`std::variant`][link_std_variant] or [`boost::variant`][link_boost_variant].

By default, objects passed that do not associate with any snippets will cause a deliberate compile error (under the assumption that there is never a purposeful reason to do so). You can change this behaviour in `ioscript.h` - the error will lead you directly to the line to change.


### The `Script` constructor

Each call to `run()` forks a new subprocess, sends the sequence of code snippets and terminates the process. No state is shared between run calls. Whilst this is noticeably slower, this significantly helps reason about each plot call independently.<sup id="br2">[[2]](#fn2)</sup>

If you do want to share state that's common to a number of successive runs, pass the snippets in the `Script` constructor:

```cpp
Script<Gnuplot,MyTypes> script(ImageSize{800,600});

script.run(vec1);                       // plots 800x600
script.run(ImageSize{640,480}, vec2);   // plots 640x480
script.run(vec3);                       // plots 800x600
```

All snippets passed in the constructor are stored in a common 'header' that's replayed at the beginning of each call to `run()`. The `run()` call remains independent, but each call is first prepended with any statements in the header.  The semantics for passing arguments in the constructor work analogously to the `run()` member function.

The `Script` constructor is useful for initializing script-wide variables, importing modules or defining default behaviour. Conceptually it may be thought of as analogous to a constructor for your script.

```cpp
struct Header
{
    void operator()(Process<Python>& python) const {
        python << "import matplotlib.pyplot as plt\n"
               << "figtitle = 'Default title'\n";
    }
};
```

In this sense additional snippets that mutate script-wide state might be thought of as interface methods to the script 'object' itself. From my experience, ioscript works best for relatively specific self-contained tasks, or those that perhaps share just a few script-wide variables (like plotting an object).  I suspect the library may be less suited at larger scales that require more structure.

If at some later time you want to add additional code to be available for all subsequent runs, you can use the method `addToHeader(args...)`. This likewise adopts the same semantics as `run()`.

```cpp
script.addToHeader(Colours{OCEAN});   // Use OCEAN colour palette from now on
script.run(vec2);
```


### Using ioscript for print-statement like debugging

Since a variant default-initializes its first alternative, the simplest usage reduces to

```cpp
// Call vec1 with the default bound function object
script.run(vec1);
```

Depending on your propensity for singletons, one approach may be to wrap a `Script` as a singleton for quick one-liner debugging.


## The `Process<Type>` class

The `Script` class uses the lower level `Process<Type>` to abstract handling of the subprocess itself. If you didn't want to use the `Script` interface, you could use `Process<Type>` independently and send data directly to that subprocess' standard input.

The `Process<Type>` class is parametrised on a type that has just one static member `cmd` - the command for the subprocess to execute.  For example, the following example forks a `ruby` subprocess and sends it a Ruby statement to be executed by the ruby interpreter:

```cpp
#include "ioscript.h"

struct Ruby { static constexpr const char* cmd = "ruby"; };

void main()
{
    Process<Ruby> ruby{1};  // Open 1 channel
    ruby << "puts 'Hello, world!'" << std::endl;
}
```

Since the subprocess' standard input is tied to sending the code to execute, to send data, the `Process<Type>` class opens additional pipes before forking the process.  In C++, the write end of these channels is available via the `data_out()` member function that returns a type inherited from `std::ostream`.

```cpp
// Send data via the first data channel
ruby.data_out(0) << "22 67 14 42" << endl;
```

To access the data from the subprocess, use the member function `fd_r()` to get the value of the open file descriptor that refers to subprocess' inherited the read end of the pipe.  Typically there will be some kind of I/O facility to map this file descriptor to a useful abstraction in the scripting language. In Ruby:

```cpp
// Get data as IO object
ruby << "io = IO.open(" << ruby.fd_r(0) << ", mode=\"r\")" << std::endl

// Process e.g. as a single integer array per line
ruby << R"(io.each_line {|line| puts "vals: #{line.split().map { |s| s.to_i }}" })"
     << std::endl;
```

It seems simplest to pass the `fd` value inline with the code string, rather than say trying to get a list of open file descriptors from the system via a system call on the subprocess' side.

One note: if you read past the end of the input stream Ruby will wait for further input as long as the `Process<Type>` object remains in scope.  However, Ruby will also wait on its own input - indefinitely - as Ruby's inherited write end of the pipe remains open.  In this case it's customary to first close the unused write end on the Ruby side:

```cpp
ruby << "a = IO.new(" << ruby.fd_w(0) << ", \"w\")" << std::endl
     << "a.close" << std::endl;
```

The approach in Python is similar:

```cpp
Process<Python> python;

// Send code
python << "print \"Hello, world!\"" << '\n';

// Send data
python.data_out(0) << "22 67 14 42" << endl;

// Close the write end
python << "import os" << '\n'
       << "os.close(" << python.fd_w(0) << ")" << '\n';

// Create a file object to the read end
python << R"(
iosc_in = os.fdopen()" << python.fd_r(0) << R"(, 'r')
for line in fo:
    print "vals:", map(int, line.split())
)";
```

See example_process.cpp for a working example.


## Excuses & limitations

This work is a tidying up of a previous version used for a project that's now finished. As yet - I've not yet had cause to use this more thoroughly, so this refactoring remains largely untested in real use.  It's also fair to concede that while usage remains fairly simple in practice, the use of templates and static binding can cause a number of gotchas for common errors. There is still a lot of scope to smooth the experience.  However, I wanted to get this down before moving on and if anyone finds all or parts of this useful they're welcome to hack it/raise an issue/get in touch.

_____

<b id="fn1">1</b>: There are a number of `static_assert`s that hopefully catch the most commonly expected issues, although at times where this isn't the case the (perhaps over-zealous) use of templates does unfortunately mean your typical compiler errors can be a little, lets say, verbose and indirect. [↩](#br1)

<b id="fn2">2</b>: Depending on the scripting language, there may be effective methods for cleanly resetting the interpreter's global state. In such cases there might be mileage in adding such behaviour to the script header and so modifying the `Script` class to sustain a single subprocess for the lifetime of the C++ program. [↩](#br2)

[link_boost_variant]: http://www.boost.org/doc/libs/1_63_0/doc/html/variant.html
[link_std_variant]: http://en.cppreference.com/w/cpp/utility/variant
