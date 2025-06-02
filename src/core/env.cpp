#include "pch.h"
#include "env.h"
#include "../tools/tools.h"
#include "../utility.h"
#include "conf.h"
#include "context.h"
#include "nativeString.h"
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

    nativeString* env::find(nativeStringView name)
    {
        return const_cast<std::string*>(std::as_const(*this).find(name));
    }

    const nativeString* env::find(nativeStringView name) const
    {
        if (!data_)
            return {};

        for (auto& var : data_->vars) {
            if (_wcsicmp(var.first.c_str(), name.data()) == 0)
                return &var.second;
        }

        return {};
    }

    void this_env::prepend_to_path(const fs::path& p)
    {
        gcx().trace(context::generic, "prepending to PATH: {}", p);
        set("PATH", path_to_utf8(p) + pathSeparator, env::prepend);
    }

    void this_env::append_to_path(const fs::path& p)
    {
        gcx().trace(context::generic, "appending to PATH: {}", p);
        set("PATH", pathSeparator + path_to_utf8(p), env::append);
    }

}  // namespace mob
