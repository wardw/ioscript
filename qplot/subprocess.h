#pragma once

#include <iostream>
#include <streambuf>
#include <cstdio>
#include <cassert>
#include <unistd.h>  // for write
#include <vector>

namespace qp {

// Josuttis Chapter 15 p835
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
struct Cat     { static constexpr const char* cmd = "cat"; };

#ifdef QPLOT_DEBUG
struct Python  { static constexpr const char* cmd = "cat"; };
struct Gnuplot { static constexpr const char* cmd = "cat"; };
#else
struct Python  { static constexpr const char* cmd = "python"; };
struct Gnuplot { static constexpr const char* cmd = "gnuplot"; };
#endif


template <typename T>
class Subprocess
{
public:
    Subprocess(unsigned numChannels=1)
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
            channels_.push_back({filedes[0], filedes[1]});
        }

        // Wrap new fd_w in fd_ostream
        for (unsigned i=0; i<numChannels; i++)
        {
            fdout_.push_back(std::make_unique<fd_ostream>(channels_[i].fd_w));
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
        }

        // std::clog << "Pipe opened with read end " << filedes_[0]
        //           << " and write end " << filedes_[1] << std::endl;
    }
    ~Subprocess() {
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
        std::cout << "pclose returned: " << pclose(file_) << std::endl;
    }

    template <typename U>
    friend Subprocess& operator<<(Subprocess& process, U&& rhs) {
        *process.cfout_ << std::forward<U>(rhs);
        return process;
    }

    // Manipulators are also templates, so write non-template overloads (three types)
    using m1 = std::ostream&(*)(std::ostream&);
    using m2 = std::basic_ios<std::ostream::char_type,std::ostream::traits_type>&
                    (*)(std::basic_ios<std::ostream::char_type,std::ostream::traits_type>&);
    using m3 = std::ios_base&(*)(std::ios_base&);

    friend Subprocess& operator<<(Subprocess& process, m1 rhs) {
        *process.cfout_ << rhs;
        return process;
    }
    friend Subprocess& operator<<(Subprocess& process, m2 rhs) {
        *process.cfout_ << rhs;
        return process;
    }
    friend Subprocess& operator<<(Subprocess& process, m3 rhs) {
        *process.cfout_ << rhs;
        return process;
    }

    Subprocess(const Subprocess&) = delete;
    Subprocess& operator=(const Subprocess&) = delete;

    cf_ostream& out()  { return *cfout_; }
    fd_ostream& data_out(unsigned channel=0) {
        if (channel >= channels_.size())
            assert(false); // todo
        return *fdout_[channel];
    }

    unsigned numChannels() { return channels_.size(); }

    int fd_r(unsigned channel=0) {
        if (channel >= channels_.size())
            assert(false);
        return channels_[channel].fd_r;
    }
    int fd_w(unsigned channel=0) {
        if (channel >= channels_.size())
            assert(false);
        return channels_[channel].fd_w;
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

} // namespace qp
