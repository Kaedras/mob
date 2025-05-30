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
            if (data_->environ == nullptr)
                create_sys();
        }

        return data_->environ;
    }

    void env::copy_for_write()
    {
        if (own_) {
            // this is called every time something is about to change; if this
            // instance already owns the data, the sys strings must still be cleared
            // out so they're recreated if get_unicode_pointers() is every called
            data_->clearEnviron();

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

}  // namespace mob
