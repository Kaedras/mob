#include "pch.h"
#include "env.h"
#include "../tools/tools.h"
#include "../utility.h"
#include "conf.h"
#include "context.h"
#include "op.h"
#include "process.h"

#ifdef __unix__
#define PATH "PATH"
static constexpr auto pathSeparator = ":";
#else
#define PATH L"PATH"
static constexpr auto pathSeparator = L";";
#endif


namespace mob {

    // retrieves the Visual Studio environment variables for the given architecture;
    // this is pretty expensive, so it's called on demand and only once, and is
    // stored as a static variable in vs_x86() and vs_x64() below
    //
    env get_vcvars_env(arch a)
    {
        // translate arch to the string needed by vcvars
        std::string arch_s;

        switch (a) {
        case arch::x86:
            arch_s = "x86";
            break;

        case arch::x64:
            arch_s = "amd64";
            break;

        case arch::dont_care:
        default:
            gcx().bail_out(context::generic, "get_vcvars_env: bad arch");
        }

        gcx().trace(context::generic, "looking for vcvars for {}", arch_s);

        // the only way to get these variables is to
        //   1) run vcvars in a cmd instance,
        //   2) call `set`, which outputs all the variables to stdout, and
        //   3) parse it
        //
        // the process class doesn't really have a good way of dealing with this
        // and it's not worth adding all this crap to it just for vcvars, so most
        // of this is done manually

        // stdout will be redirected to this
        const fs::path tmp = make_temp_file();

        // runs `"vcvarsall.bat" amd64 && set > temp_file`
        const std::string cmd = "\"" + path_to_utf8(vs::vcvars()) + "\" " + arch_s +
                                " && set > \"" + path_to_utf8(tmp) + "\"";

        // cmd_unicode() is necessary so `set` outputs in utf16 instead of codepage
        process::raw(gcx(), cmd).cmd_unicode(true).run();

        gcx().trace(context::generic, "reading from {}", tmp);

        // reads the file, converting utf16 to utf8
        std::stringstream ss(op::read_text_file(gcx(), encodings::utf16, tmp));
        op::delete_file(gcx(), tmp);

        // `ss` contains all the variables in utf8

        env e;

        gcx().trace(context::generic, "parsing variables");

        for (;;) {
            std::string line;
            std::getline(ss, line);
            if (!ss)
                break;

            const auto sep = line.find('=');

            if (sep == std::string::npos)
                continue;

            std::string name  = line.substr(0, sep);
            std::string value = line.substr(sep + 1);

            gcx().trace(context::generic, "{} = {}", name, value);
            e.set(std::move(name), std::move(value));
        }

        return e;
    }

    env env::vs_x86()
    {
        static env e = get_vcvars_env(arch::x86);
        return e;
    }

    env env::vs_x64()
    {
        static env e = get_vcvars_env(arch::x64);
        return e;
    }

    env env::vs(arch a)
    {
        switch (a) {
        case arch::x86:
            return vs_x86();

        case arch::x64:
            return vs_x64();

        case arch::dont_care:
            return {};

        default:
            gcx().bail_out(context::generic, "bad arch for env");
        }
    }

    env::env() : own_(false)
    {
        // empty env, does not own
    }

    env::env(const env& e) : data_(e.data_), own_(false)
    {
        // copy data, does not own
    }

    env::env(env&& e) : data_(std::move(e.data_)), own_(e.own_)
    {
        // move data, owns if `e` did
    }

    env& env::operator=(const env& e)
    {
        // copy data, does not own
        data_ = e.data_;
        own_  = false;
        return *this;
    }

    env& env::operator=(env&& e)
    {
        // copy data, owns if `e` did
        data_ = std::move(e.data_);
        own_  = e.own_;
        return *this;
    }

    env& env::append_path(const fs::path& p)
    {
        append_path(std::vector<fs::path>{p});
        return *this;
    }

    env& env::prepend_path(const fs::path& p)
    {
        prepend_path(std::vector<fs::path>{p});
        return *this;
    }

    env& env::prepend_path(const std::vector<fs::path>& v)
    {
        change_path(v, prepend);
        return *this;
    }

    env& env::append_path(const std::vector<fs::path>& v)
    {
        change_path(v, append);
        return *this;
    }

    env& env::change_path(const std::vector<fs::path>& v, flags f)
    {
        copy_for_write();

        nativeString path;

        switch (f) {
        case replace: {
            // convert to utf16 strings, join with ;
            const auto strings = mob::map(v, [&](auto&& p) {
                return p.native();
            });

            path = join(strings, pathSeparator);

            break;
        }

        case append: {
            auto current = find(PATH);
            if (current)
                path = *current;

            // append all paths as utf16 strings to the current value, if any
            for (auto&& p : v) {
                if (!path.empty())
                    path += pathSeparator;

                path += p.native();
            }

            break;
        }

        case prepend: {
            auto current = find(PATH);
            if (current)
                path = *current;

            // prepend all paths as utf16 strings to the current value, if any
            for (auto&& p : v) {
                if (!path.empty())
                    path = pathSeparator + path;

                path = p.native() + path;
            }

            break;
        }
        }

        set(PATH, path, replace);

        return *this;
    }

    void env::set_impl(nativeString k, nativeString v, flags f)
    {
        auto current = find(k);

        if (!current) {
            data_->vars.emplace(std::move(k), std::move(v));
            return;
        }

        switch (f) {
        case replace:
            *current = std::move(v);
            break;

        case append:
            *current += v;
            break;

        case prepend:
            *current = v + *current;
            break;
        }
    }


    env::map env::get_map() const
    {
        if (!data_)
            return {};

        std::scoped_lock lock(data_->m);
        return data_->vars;
    }

    void* env::get_unicode_pointers() const
    {
        if (!data_ || data_->vars.empty())
            return nullptr;

        // create string if it doesn't exist
        {
            std::scoped_lock lock(data_->m);
            if (data_->sys.empty())
                create_sys();
        }

        return (void*)data_->sys.c_str();
    }

    void env::copy_for_write()
    {
        if (own_) {
            // this is called every time something is about to change; if this
            // instance already owns the data, the sys strings must still be cleared
            // out so they're recreated if get_unicode_pointers() is every called
            if (data_)
                data_->sys.clear();

            return;
        }

        if (data_) {
            // remember the shared data
            auto shared = data_;

            // create a new owned instance
            data_.reset(new data);

            // copying
            std::scoped_lock lock(shared->m);
            data_->vars = shared->vars;
        }
        else {
            // creating own, empty data
            data_.reset(new data);
        }

        // this instance owns the data
        own_ = true;
    }

}