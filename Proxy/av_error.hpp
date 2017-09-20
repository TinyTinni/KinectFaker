#pragma once


/*
    Implements a system error domain for AVERROR codes (ffmpeg error codes)

    classes: 
        - av_error_code // extends  'std::error_code'
        - av_error // exception, extends 'std::system_error' with 'av_error_category')
    functions:
        - av_error_category // returns the 'av_error_code' category
*/

#include <exception>
#include <system_error>

#if __cplusplus
extern "C"
#endif
{
// for convience but not needed in header file
#include <libavutil/error.h>
}

/// returns the category for AVERRORs
extern const std::error_category &av_error_category();

class av_error_code :public std::error_code
{
public:
    av_error_code(int c) noexcept
        : std::error_code(c, av_error_category()) {}
    av_error_code() noexcept
        :std::error_code(0, av_error_category()) {}
    inline operator bool() const noexcept { return std::error_code::value() >= 0; }

    av_error_code& operator=(const av_error_code& c) = default;
    ~av_error_code() = default;
};

class av_error : public std::system_error
{
public:
    // constructors of system_error aren't noexcept
    av_error(int ec, const std::string& what_arg)
        :std::system_error(av_error_code(ec), what_arg) {}
    av_error(int ec, const char* what_arg)
        :std::system_error(av_error_code(ec), what_arg) {}
    av_error(int ec)
        :std::system_error(av_error_code(ec)) {}
    av_error(av_error_code ec, const std::string& what_arg)
        :std::system_error(ec, what_arg) {}
    av_error(av_error_code ec, const char* what_arg)
        :std::system_error(ec, what_arg) {}
    av_error(av_error_code ec)
        :std::system_error(ec) {}
};