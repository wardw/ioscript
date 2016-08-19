#pragma once

#include <string>
#include <iostream>
#include <streambuf>
#include <cstdio>
#include <cassert>

// Adapted from Nicolai M. Josuttis, The C++ Standard Library - A Tutorial and Reference, 2nd Edition
class cfoutbuf : public std::streambuf
{
public:
    cfoutbuf(FILE* file) : file_(file) {}
    ~cfoutbuf() { pclose(file_); }

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

class cfostream : public std::ostream
{
protected:
    cfoutbuf buf_;

public:
    cfostream(FILE* file) : std::ostream(0), buf_(file) {
        rdbuf(&buf_);
    }

};

class Gnuplot
{
public:
    Gnuplot(const char* gnuplot_cmd);
    ~Gnuplot();

    template <typename RowIterator>
    void writeRows(RowIterator rowIter, RowIterator last);

    friend Gnuplot& operator<<(Gnuplot& plt, const std::string& text);

    Gnuplot(const Gnuplot&) = delete;
    Gnuplot& operator=(const Gnuplot&) = delete;

private:
    cfostream* cfout_;
    FILE* file_;
};

template <typename RowIterator>
void Gnuplot::writeRows(RowIterator iter, RowIterator last)
{
    while (iter != last) {
        typename RowIterator::RowType row = *iter;
        for (int i=0; i<RowIterator::cols; ++i) {
            *cfout_ << row[i] << " ";
        }
        *cfout_ << "\n";
        ++iter;
    }

    // while (iter ) {
    //  sendEntry(*iter.get<0>());
    //  sendEntry(*iter.get<1>());
    //  sendEntry(*iter.get<2>());
    //  *stream << "\n";
    //  ++iter;
    // }

    *cfout_ << "e" << std::endl; // gnuplot's "end of array" token
}

inline Gnuplot::Gnuplot(const char* gnuplot_cmd)
{
	if (!(file_ = popen(gnuplot_cmd, "w"))) {
		std::cout << "popen returned with error";
		assert(false);  // todo
	}

	cfout_ = new cfostream(file_);
}

inline Gnuplot::~Gnuplot()
{
    delete cfout_;  // closes file_
}

inline Gnuplot& operator<<(Gnuplot& plt, const std::string& text)
{
    *plt.cfout_ << text.c_str();
    return plt;
}
