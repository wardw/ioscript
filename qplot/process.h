#pragma once

#include <string>
#include <iostream>
#include <streambuf>
#include <cstdio>
#include <cassert>
#include <unistd.h>  // for write

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
struct Gnuplot { static constexpr const char* cmd = "gnuplot"; };
// struct Gnuplot { static constexpr const char* cmd = "cat"; };
struct Python     { static constexpr const char* cmd = "python"; };
// struct Python     { static constexpr const char* cmd = "cat"; };

template <typename T>
class Process
{
public:
    Process() {
        // Open a new pipe
        if (pipe(filedes_) == -1) {
            std::cout << "pipe() returned with error" << std::endl;
        }

        // Fork process
        if (!(file_ = popen(T::cmd, "w"))) {
            std::cout << "popen returned with error" << std::endl;
            assert(false);  // todo
        }

        // Close unused read end on this process
        if (close(filedes_[0]) == -1)
            std::cout << "error closing filedes[0]" << std::endl;

        cfout_ = std::make_unique<cf_ostream>(file_);
        fdout_ = std::make_unique<fd_ostream>(filedes_[1]);

        std::cout << "Pipe opened with read end " << filedes_[0] << " and write end " << filedes_[1] << std::endl;
    }
    ~Process() {
        // Close 'data' pipe
        if (close(filedes_[1]) == -1)
            std::cout << "close(filedes_[1]) returned with error" << std::endl;

        // Close process
        pclose(file_);
    }

    template <typename U>
    friend Process& operator<<(Process& process, const U& obj) {
        *process.cfout_ << obj;
        return process;
    }

    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

    cf_ostream& cfout() { return *cfout_; }
    fd_ostream& fdout() { return *fdout_; }

    int fd_r() { return filedes_[0]; }
    int fd_w() { return filedes_[1]; }

private:
    std::unique_ptr<cf_ostream> cfout_;
    std::unique_ptr<fd_ostream> fdout_;

    int filedes_[2];
    FILE* file_;
};
