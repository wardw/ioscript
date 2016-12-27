#pragma once

#include <string>
#include <iostream>
#include <streambuf>
#include <cstdio>
#include <cassert>

// Adapted from Nicolai M. Josuttis, The C++ Standard Library - A Tutorial and Reference, 2nd Edition
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
struct Gnuplot { static constexpr const char* cmd = "cat"; };  // temporary while testing
struct Mpl     { static constexpr const char* cmd = "python"; };

template <typename T>
class Process
{
public:
    Process() {
        if (!(file_ = popen(T::cmd, "w"))) {
            std::cout << "popen returned with error";
            assert(false);  // todo
        }

	    fout_ = std::make_unique<cf_ostream>(file_);
    }
    ~Process() { pclose(file_); }

    friend Process& operator<<(Process& proces, const std::string& text) {
        *proces.fout_ << text.c_str();
        return proces;
    }

    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

private:
    std::unique_ptr<cf_ostream> fout_;
    FILE* file_;
};
