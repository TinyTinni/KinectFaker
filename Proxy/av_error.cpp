#include "av_error.hpp"
#include <string>

class av_error_category_impl : public std::error_category
{
public:
    // Return a short descriptive name for the category
    virtual const char *name() const noexcept override final { return "AVERROR"; }
    virtual std::string message(int c) const override final
    {
        std::string r(AV_ERROR_MAX_STRING_SIZE, '\0');
        av_make_error_string(&r[0], r.size(), c);
        r.erase(std::begin(r) + r.find_first_of('\0') + 1, std::end(r));
        return r; //RVO
    }
};

const std::error_category &av_error_category()
{
    static av_error_category_impl c;
    return c;
}
