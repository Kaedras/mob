#include "../io.h"
#include "../pch.h"
#include "../string.h"

namespace mob {

    // global output mutex, avoids interleaving
    static std::mutex g_output_mutex;

    // streams
    extern u8stream u8cout(false);
    extern u8stream u8cerr(true);

    void set_std_streams() {}

    std::mutex& global_output_mutex()
    {
        return g_output_mutex;
    }

    yn ask_yes_no(const std::string& text, yn def)
    {
        u8cout << text << (text.empty() ? "" : " ")
               << (def == yn::yes ? "[Y/n]" : "[y/N]") << " ";

        // stdin is not utf8
        std::string line;
        std::getline(std::cin, line);

        // ctrl+c
        if (!std::cin)
            return yn::cancelled;

        if (line.empty())
            return def;
        else if (line == "y" || line == "Y")
            return yn::yes;
        else if (line == "n" || line == "N")
            return yn::no;
        else
            return yn::cancelled;
    }

    void u8stream::do_output(const std::string& s)
    {
        std::scoped_lock lock(g_output_mutex);

        if (err_) {
            std::cerr << s;
        }
        else {
            std::cout << s;
        }
    }

    void u8stream::write_ln(std::string_view utf8)
    {
        std::scoped_lock lock(g_output_mutex);

        if (err_) {
            std::cerr << utf8 << "\n";
        }
        else {
            std::cout << utf8 << "\n";
        }
    }

    console_color::console_color() : reset_(false), old_atts_(0) {}

    console_color::console_color(colors c) : reset_(false), old_atts_(0)
    {
        switch (c) {
        case colors::white:
            break;

        case colors::grey:
            reset_ = true;
            u8cout << "\033[38;2;150;150;150m";
            break;

        case colors::yellow:
            reset_ = true;
            u8cout << "\033[38;2;240;240;50m";
            break;

        case colors::red:
            reset_ = true;
            u8cout << "\033[38;2;240;50;50m";
            break;
        }
    }

    console_color::~console_color()
    {
        if (!reset_)
            return;

        u8cout << "\033[39m\033[49m";
    }

    font_restorer::font_restorer() : restore_(false) {}

    font_restorer::~font_restorer() {}

    void font_restorer::restore() {}

}  // namespace mob
