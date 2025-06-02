#include "pch.h"
#include "../../tools/tools.h"
#include "../../utility.h"
#include "../conf.h"
#include "../context.h"
#include "../env.h"
#include "../op.h"
#include "../process.h"

namespace mob {

    env env::vs(arch a)
    {
        return {};
    }

    env& env::set(std::string_view k, std::string_view v, flags f)
    {
        copy_for_write();
        set_impl(nativeString(k), nativeString(v), f);
        return *this;
    }

    std::string env::get(std::string_view k) const
    {
        if (!data_)
            return {};

        auto current = find(k);
        if (!current)
            return {};

        return *current;
    }

    void env::create_sys() const
    {
        // Execle() expects an array of pointers to strings.
        // By convention, these strings have the form "name=value".
        // The last pointer in this array must have the value NULL

        data_->clearEnviron();

        data_->environ =
            static_cast<char**>(malloc((data_->vars.size() + 1) * sizeof(char*)));

        int i = 0;
        for (auto&& v : data_->vars) {
            std::string tmp     = v.first + "=" + v.second;
            data_->environ[i++] = strdup(tmp.c_str());
        }

        data_->environ[i] = nullptr;
    }

    std::string* env::find(std::string_view name)
    {
        return const_cast<std::string*>(std::as_const(*this).find(name));
    }

    const std::string* env::find(std::string_view name) const
    {
        if (!data_)
            return {};

        for (auto itor = data_->vars.begin(); itor != data_->vars.end(); ++itor) {
            if (strcasecmp(itor->first.c_str(), name.data()) == 0)
                return &itor->second;
        }

        return {};
    }

    // mob's environment variables are only retrieved once and are kept in sync
    // after that; this must also be thread-safe
    static std::mutex g_sys_env_mutex;
    static env g_sys_env;
    static bool g_sys_env_inited;

    env this_env::get()
    {
        std::scoped_lock lock(g_sys_env_mutex);

        if (g_sys_env_inited) {
            // already done
            return g_sys_env;
        }

        // first time, get the variables from the system

        for (int i = 0; environ[i] != nullptr; i++) {
            char* name = environ[i];
            // equal sign
            const char* equal = std::strchr(name, '=');

            // key
            std::string key(name, static_cast<std::size_t>(equal - name));

            // value
            const char* value_start = equal + 1;
            std::string value(value_start);

            if (!key.empty())
                g_sys_env.set(key, value);
        }

        g_sys_env_inited = true;

        return g_sys_env;
    }

    void this_env::set(const std::string& k, const std::string& v, env::flags f)
    {
        std::string newV = v;
        switch (f) {
        case env::replace: {
            int result = setenv(k.c_str(), v.c_str(), 1);
            if (result != 0) {
                const int e = errno;
                gcx().bail_out(context::generic, "setenv failed: {}", strerror(e));
            }
            break;
        }

        case env::append: {
            const std::string current = get_impl(k).value_or("");
            newV                      = current + newV;
            int result                = setenv(k.c_str(), newV.c_str(), 1);
            if (result != 0) {
                const int e = errno;
                gcx().bail_out(context::generic, "setenv failed: {}", strerror(e));
            }
            break;
        }

        case env::prepend: {
            const std::string current = get_impl(k).value_or("");
            newV                      = newV + current;
            int result                = setenv(k.c_str(), newV.c_str(), 1);
            if (result != 0) {
                const int e = errno;
                gcx().bail_out(context::generic, "setenv failed: {}", strerror(e));
            }
            break;
        }
        }

        // keep in sync
        {
            std::scoped_lock lock(g_sys_env_mutex);
            if (g_sys_env_inited)
                g_sys_env.set(k, newV);
        }
    }

    void this_env::prepend_to_path(const fs::path& p)
    {
        gcx().trace(context::generic, "prepending to PATH: {}", p);
        set("PATH", p.string() + ":", env::prepend);
    }

    void this_env::append_to_path(const fs::path& p)
    {
        gcx().trace(context::generic, "appending to PATH: {}", p);
        set("PATH", ":" + p.string(), env::append);
    }

    std::string this_env::get(const std::string& name)
    {
        auto v = get_impl(name);
        if (!v) {
            gcx().bail_out(context::generic, "environment variable {} doesn't exist",
                           name);
        }

        return *v;
    }

    std::optional<std::string> this_env::get_opt(const std::string& name)
    {
        auto v = get_impl(name);
        if (v)
            return *v;
        else
            return {};
    }

    std::optional<nativeString> this_env::get_impl(const std::string& k)
    {
        auto result = getenv(k.c_str());
        return result == nullptr ? std::nullopt : std::optional<nativeString>(result);
    }

}  // namespace mob
