#pragma once

#include <cassert>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <vector>
#include <tuple>
#include <type_traits>

#include <unistd.h>  // for write

// Enable this macro definition to send all output to stdout in exactly the form as if sent to the subprocess
// #define QPLOT_DEBUG

// Enable this macro definition to use boost/variant instead of C++17 std::variant
// #define WITH_BOOST_VARIANT

constexpr unsigned NUM_CHANNELS = 3;

#ifdef WITH_BOOST_VARIANT
    #include <boost/variant.hpp>
#else
    #include <variant>
#endif


// Name iosc::variant as either boost or stl
namespace iosc {

#ifdef WITH_BOOST_VARIANT
    using boost::variant;
    using monostate = boost::blank;

    template <typename... Args>
    auto visit(Args&&... args) {
      return boost::apply_visitor(std::forward<Args>(args)...);
    }
#else
    using std::variant;
    using monostate = std::monostate;

    template <typename... Args>
    auto visit(Args&&... args) {
      return std::visit(std::forward<Args>(args)...);
    }
#endif


/// "Process.h" ///

// Adapted from Nicolai M. Josuttis, The C++ Standard Library - A Tutorial and Reference, 2nd Edition
class fdoutbuf : public std::streambuf
{
protected:
    int fd_;

public:
    fdoutbuf(int fd) : fd_(fd) {}

protected:
    virtual int_type overflow(int_type c)
    {
        if (c != EOF) {
            char z = c;
            if (write(fd_, &z, 1) != 1) {
                return EOF;
            }
        }
        return c;
    }

    virtual std::streamsize xsputn(const char* s, std::streamsize num) {
        return write(fd_, s, num);
    }
};

class fd_ostream : public std::ostream
{
protected:
    fdoutbuf buf_;

public:
    fd_ostream (int fd) : std::ostream(0), buf_(fd) {
        rdbuf(&buf_);
    }
};

class cf_outbuffer : public std::streambuf
{
public:
    cf_outbuffer(FILE* file) : file_(file) {}

protected:
    virtual int_type overflow(int_type c)
    {
        if (c != traits_type::eof()) {
            if (fwrite(&c, 1, 1, file_) != 1) {
                return traits_type::eof();
            }
        }
        return c;
    }

    virtual std::streamsize xsputn(const char* s, std::streamsize num) {
        return fwrite(s, 1, num, file_);
    }

private:
    FILE* file_;
};

struct cf_ostream : public std::ostream
{
    cf_ostream(FILE* file) : std::ostream(0), buffer_(file) {
        rdbuf(&buffer_);
    }

protected:
    cf_outbuffer buffer_;
};

struct Null    { static constexpr const char* cmd = "cat > /dev/null"; };
struct Cat     { static constexpr const char* cmd = "cat"; };

template <typename T>
class Process
{
public:
    Process(unsigned numChannels)
    {
        assert(numChannels < 1024); // todo

        // Open data pipes
        for (unsigned i=0; i<numChannels; i++)
        {
            int filedes[2];
            if (pipe(filedes) == -1) {
                std::cerr << "pipe() returned with error" << std::endl;
                assert(false);
            }
            channels_.push_back(Fd{filedes[0], filedes[1]});

            // Wrap new fd_w in fd_ostream
            fdout_.push_back(std::make_unique<fd_ostream>(channels_.back().fd_w));
        }

        // Fork process
        if (!(file_ = popen(T::cmd, "w"))) {
            std::cerr << "popen returned with error" << std::endl;
            assert(false);  // todo
        }

        cfout_ = std::make_unique<cf_ostream>(file_);

        // Close unused read ends on this process
        for (unsigned i=0; i<numChannels; i++)
        {
            if (close(channels_[i].fd_r) == -1)
                std::cerr << "error closing (read) file descriptor "
                          << channels_[i].fd_r << " on channel " << i << std::endl;

            // std::cerr << "Channel " << i << " opened with read end " << channels_[i].fd_r
            //           << " and write end " << channels_[i].fd_w << std::endl;
        }
    }
    ~Process() {
        // Close 'data' pipe
        for (unsigned i=0; i<channels_.size(); i++)
        {
            if (close(channels_[i].fd_w) == -1) {
                std::cerr << "error closing (write) file descriptor "
                          << channels_[i].fd_w << " on channel " << i << std::endl;
                assert(false);
            }
        }

        // Close process
        std::cerr << "pclose returned: " << pclose(file_) << std::endl;
    }

    template <typename U>
    friend Process& operator<<(Process& process, U&& rhs) {
        *process.cfout_ << std::forward<U>(rhs);
        return process;
    }

    // Manipulators are also templates, so write non-template overloads (three types)
    using m1 = std::ostream&(*)(std::ostream&);
    using m2 = std::basic_ios<std::ostream::char_type,std::ostream::traits_type>&
                    (*)(std::basic_ios<std::ostream::char_type,std::ostream::traits_type>&);
    using m3 = std::ios_base&(*)(std::ios_base&);

    friend Process& operator<<(Process& process, m1 rhs) {
        *process.cfout_ << rhs;
        return process;
    }
    friend Process& operator<<(Process& process, m2 rhs) {
        *process.cfout_ << rhs;
        return process;
    }
    friend Process& operator<<(Process& process, m3 rhs) {
        *process.cfout_ << rhs;
        return process;
    }

    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

    cf_ostream& out()  { return *cfout_; }
    fd_ostream& data_out(unsigned c) {
        if (c >= channels_.size())
            assert(false); // todo
        return *fdout_[c];
    }

    unsigned numChannels() { return channels_.size(); }

    int fd_r(unsigned c) {
        if (c >= channels_.size())
            assert(false);
        return channels_[c].fd_r;
    }
    int fd_w(unsigned c) {
        if (c >= channels_.size())
            assert(false);
        return channels_[c].fd_w;
    }

private:
    std::unique_ptr<cf_ostream> cfout_;
    std::vector<std::unique_ptr<fd_ostream>> fdout_;

    struct Fd {
        int fd_r;
        int fd_w;
    };

    std::vector<Fd> channels_;
    FILE* file_;
};


/// "util.h" ///

// Find whether a variant contains a given type
// Returns the index if the variant contains the type, or -1 otherwise
// 'Inspired' from http://stackoverflow.com/a/25958302/254035

template <typename T, size_t N, typename Tuple>
struct has_tuple_element;

// Element not found
template <typename T, size_t N>
struct has_tuple_element<T, N, std::tuple<>> : std::integral_constant<int,-1> {};

// Match: T matches tuple element T, at iteration N
template <typename T, size_t N, typename... Ts>
struct has_tuple_element<T, N, std::tuple<T, Ts...>> : std::integral_constant<int,N> {};

// No match: U is not T
template <typename T, size_t N, typename U, typename... Ts>
struct has_tuple_element<T, N, std::tuple<U, Ts...>> : has_tuple_element<T, N+1, std::tuple<Ts...>> {};

template <typename T, typename Tuple>
using tuple_element_index = typename has_tuple_element<T, 0, Tuple>::type;


// Same for variant
// Seems not possilbe to merge with the above by also matching `variant` as e.g. `typename Container` ?

template <typename T, size_t N, typename Variant>
struct has_variant_element;

template <typename T, size_t N>
struct has_variant_element<T, N, variant<>> : std::integral_constant<int,-1> {};   // TODO: breaks with monostate when type isn't in the variant

template <typename T, size_t N, typename... Ts>
struct has_variant_element<T, N, variant<T, Ts...>> : std::integral_constant<int,N> {};

template <typename T, size_t N, typename U, typename... Ts>
struct has_variant_element<T, N, variant<U, Ts...>> : has_variant_element<T, N+1, variant<Ts...>> {};

template <typename T, typename Variant>
using variant_element_index = typename has_variant_element<T, 0, Variant>::type;


// Test wether T is a variant of any args

template <typename T>
struct is_variant : std::false_type {};

template <typename T, typename... Ts>
struct is_variant<variant<T,Ts...>>  : std::true_type {};


template <typename SnippetVariant, typename Snippet, int key>
void updateTuple(SnippetVariant& snippetVariant, const Snippet& snippet, std::integral_constant<int,key>)
{
    snippetVariant = snippet;
}

template <typename SnippetVariant, typename Snippet>
void updateTuple(SnippetVariant& snippetVariant, const Snippet& snippet, std::integral_constant<int,-1>)
{
    // std::clog << "The snippets variant " << objName(snippetVariant)
    //           << " does not contain the snippet " << objName(snippet) << std::endl;
}

template<class Snippet, class Tuple, std::size_t N>
struct TupleUpdater {
    static void update(Tuple& scriptSnippets, const Snippet& snippet)
    {
        using SnippetVariant = std::tuple_element_t<N-1, Tuple>;

        static_assert(is_variant<SnippetVariant>::value, "");
        using Result = variant_element_index<Snippet, SnippetVariant>;

        auto& snippetVariant = std::get<N-1>(scriptSnippets);
        updateTuple(snippetVariant, snippet, Result{});

        TupleUpdater<Snippet, Tuple, N-1>::update(scriptSnippets, snippet);
    }
};

template<class Snippet, class Tuple>
struct TupleUpdater<Snippet, Tuple, 1> {
    static void update(Tuple& scriptSnippets, const Snippet& snippet)
    {
        using SnippetVariant = std::tuple_element_t<0, Tuple>;

        static_assert(is_variant<SnippetVariant>::value, "");
        using Result = variant_element_index<Snippet, SnippetVariant>;

        auto& snippetVariant = std::get<0>(scriptSnippets);
        updateTuple(snippetVariant, snippet, Result{});
    }
};

// Todo: can't we just have a base case for when N=0?
template<class Snippet, class Tuple>
struct TupleUpdater<Snippet, Tuple, 0> {
    static void update(Tuple& scriptSnippets, const Snippet& snippet)
    {
    }
};


// Determine wether a plot method of the right type exists for a class
// Adapted from http://stackoverflow.com/a/16824239/254035

template<typename, typename T>
struct has_member_function {
    static_assert(std::integral_constant<T,false>::value,
                  "Second template parameter needs to be of function type.");
};

// Specialization that does the checking

template<typename C, typename Ret, typename... Args>
struct has_member_function<C, Ret(Args...)> {
private:
    template<typename T>
    static constexpr auto check(T*) -> typename std::is_same<
            decltype(std::declval<T>()(std::declval<Args>()...)), Ret>::type;

    template<typename>
    static constexpr std::false_type check(...);

    using type = decltype(check<C>(0));

public:
    static constexpr bool value = type::value;
};


// + A 'canvas snippet' is any type which overloads `operator()` with the type `void(Script<P>&)`  ("first form")
// + An 'object snippet' works likewise, but for the type `void(Script<P>&, constT&)` -- but see note, below ("second form")

template <typename P, typename S>
class Script;

template <typename T, typename P, typename U = void>
struct is_script_snippet {
    static constexpr bool value = false;
};

template <typename T, typename P>
struct is_script_snippet<T, P, std::enable_if_t<has_member_function<T, void(Process<P>&)>::value>> {
    static constexpr bool value = true;
};

// This isn't great - there's probably a better way
// This tests for a second parameter accepting a `const T&` by indirectly checking it supports `void*`
template <typename T, typename P, typename U = void>
struct is_object_snippet {
    static constexpr bool value = false;
};

template <typename T, typename P>
struct is_object_snippet<T, P, std::enable_if_t<has_member_function<T, void(Process<P>&,const void*&)>::value>> {
    static constexpr bool value = true;
};


// With specializations in client code
template <typename T>
struct binds_to;


// Map a tuple of `types...` to a tuple of `binds_to<types...>::type`
// There's probably a cleaner way..
struct foo { using type = void; };

template <typename... Ts>
struct get_snippets;

template <typename... Xs>
struct get_snippets<std::tuple<Xs...>, std::tuple<>> : foo { using type = std::tuple<Xs...>; };

template <typename U, typename... Xs, typename... Ts>
struct get_snippets<std::tuple<Xs...>, std::tuple<U, Ts...>> : get_snippets<std::tuple<Xs...,typename binds_to<U>::type>, std::tuple<Ts...>> {};

template <typename Tuple>
using snippets_from_types = get_snippets<std::tuple<>, Tuple>;


// Check that the given type refers to a variant with at least one snippet
// todo: this check is supported on C++17 only
#ifndef WITH_BOOST_VARIANT
    template <typename T, typename U = void>
    struct has_related_snippet {
        static constexpr bool value = false;
    };

    template <typename T>
    struct has_related_snippet<T, std::enable_if_t<std::variant_size_v<typename binds_to<T>::type> != 0>> {
        static constexpr bool value = true;
    };

    template <typename... Ts>
    struct check_tuple;

    template <>
    struct check_tuple<std::tuple<>> : std::true_type{};

    template <typename T, typename... Ts>
    struct check_tuple<std::tuple<T, Ts...>> : check_tuple<std::tuple<Ts...>> {
        static_assert(has_related_snippet<T>::value, "A type T has been added to 'MyTypes' that does not refer to any snippet variant. "
                                                     "Add a specialization `binds_to<T>` or remove T from MyTypes. "
                                                     "(Related errors might say more about what type T is)");
    };
#endif


/// "ioscript.h" ///

// Primary template, with specializations for each process in separate headers
template <typename P, typename S>
void addPrivateHeader(Script<P,S>& python) {}


template <typename P, typename X>
class Script
{
    std::unique_ptr<Process<P>> subprocess_;
    std::ostringstream header_;

// todo: this check is supported on C++17 only
#ifndef WITH_BOOST_VARIANT
    using Check = typename check_tuple<X>::type;
#endif
    using S = typename snippets_from_types<X>::type;

    S snippets_;
    S snippetsHeader_;

public:
    template <typename... Ts>
    Script(Ts&&... args) :
        subprocess_(std::make_unique<Process<P>>(NUM_CHANNELS))
    {
        addPrivateHeader(*this);
        addToHeader(args...);
    }

    void processArgs() {}

	// Arg is a canvas-snippet (only)
    template <typename T, typename... Ts,
              std::enable_if_t< is_script_snippet<T,P>::value &&
                               !is_object_snippet<T,P>::value, int> = 0>
    void processArgs(const T& snippet, const Ts&... args)
    {
        snippet(*subprocess_);
		processArgs(args...);
    }

    // Arg is an object-snippet (only)
    template <typename T, typename... Ts,
              std::enable_if_t< is_object_snippet<T,P>::value &&
                               !is_script_snippet<T,P>::value, int> = 0>
    void processArgs(const T& snippet, const Ts&... args)
    {
        // Update snippets_ with this snippet for all variants in snippets_ that support it
        constexpr size_t NumSnippets = std::tuple_size<S>::value;
        TupleUpdater<T, S, NumSnippets>::update(snippets_, snippet);

        processArgs(args...);
    }

    // Arg is both an object-snippet and script-snippet
    template <typename T, typename... Ts,
              std::enable_if_t<is_object_snippet<T,P>::value &&
                               is_script_snippet<T,P>::value, int> = 0>
    void processArgs(const T& snippet, const Ts&... args)
    {
        snippet(*subprocess_);

        constexpr size_t NumSnippets = std::tuple_size<S>::value;
        TupleUpdater<T, S, NumSnippets>::update(snippets_, snippet);

        processArgs(args...);
    }

    // Arg is an object to plot
    template <typename T, typename... Ts,
              int key = tuple_element_index<typename binds_to<T>::type, S>::value,
              std::enable_if_t<!is_object_snippet<T,P>::value &&
                               !is_script_snippet<T,P>::value &&
                               key != -1, int> = 0>
    void processArgs(const T& obj, const Ts&... args)
    {
		// Get the snippet variant that's currently associated with the arg type T
        auto snippetVar = std::get<key>(snippets_);

		// Plot this object
        iosc::visit([this,&obj](auto&& snippet) {
            snippet(*subprocess_, obj);
		}, snippetVar);

		processArgs(args...);
    }

    template <typename T, typename... Ts,
              int key = tuple_element_index<typename binds_to<T>::type, S>::value,
              std::enable_if_t<!is_object_snippet<T,P>::value &&
                               !is_script_snippet<T,P>::value &&
                               key == -1, int> = 0>
    void processArgs(const T& obj, const Ts&... args)
    {
        // Disable this assert to silently ignore unrecognised objects passed to Script::run (not particularly recommended)
        static_assert(key != -1, "A type was passed to run() that is not recognised. (Did you forget to add this type to MyTypes?)");
        std::cerr << "Warning: An unrecognised type was passed to run() and was ignored" << std::endl;
    }

    template <typename... Ts>
	void run(const Ts&... args)
	{
        // Replay the header
        subprocess_->out() << header_.str();

        // Reload state for the chosen alternatives
        snippets_ = snippetsHeader_;

        // Recurse into arguments
		processArgs(args...);

        // Finally, close this process and reopen with a fresh instance
        // + This ends out code stream to the process, which e.g. for python allows the process to start execution
        // + Also important that each call to plot (aside from the intentional header) is stateless
        subprocess_.reset();  // destroy first
        subprocess_ = std::make_unique<Process<P>>(NUM_CHANNELS);
	}

    template <typename... Ts>
    void addToHeader(const Ts&... args)
    {
        // Everything to cfout goes to our local ostringstream
        auto cout_buffer = subprocess_->out().rdbuf();
        subprocess_->out().rdbuf(header_.rdbuf());

        // Also save the state of the chosen alternatives
        snippetsHeader_ = snippets_;

        // Capture in local header_ buffer
        processArgs(args...);

        // Swap back
        subprocess_->out().rdbuf(cout_buffer);
    }

    // cf_ostream& out()      { return subprocess_->out(); }
    // fd_ostream& data_out() { return subprocess_->data_out(); }

    // int fd_r() { return subprocess_->fd_r(); }
    // int fd_w() { return subprocess_->fd_w(); }
};

} // namespace iosc
