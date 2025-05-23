#include "../pch.h"
#include "../env.h"
#include "../../tools/tools.h"
#include "../../utility.h"
#include "../conf.h"
#include "../context.h"
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
        // CreateProcess() wants a string where every key=value is separated by a
        // null and also terminated by a null, so there are two null characters at
        // the end

        data_->sys.clear();

        for (auto&& v : data_->vars) {
            data_->sys += v.first + "=" + v.second;
            data_->sys.append(1, '\0');
        }

        data_->sys.append(1, '\0');
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

            // the strings contain all sorts of weird stuff, like variables to
            // keep track of the current directory, those start with an equal sign,
            // so just ignore them
            if (!key.empty())
                g_sys_env.set(key, value);
        }

        g_sys_env_inited = true;

        return g_sys_env;
    }

    void this_env::set(const std::string& k, const std::string& v, env::flags f)
    {
        const std::string wk = k;
        std::string wv       = v;

        switch (f) {
        case env::replace: {
            setenv(wk.c_str(), wv.c_str(), 1);
            break;
        }

        case env::append: {
            const std::string current = get_impl(k).value_or("");
            wv                        = current + wv;
            setenv(wk.c_str(), wv.c_str(), 1);
            break;
        }

        case env::prepend: {
            const std::string current = get_impl(k).value_or("");
            wv                        = wv + current;
            setenv(wk.c_str(), wv.c_str(), 1);
            break;
        }
        }

        // keep in sync
        {
            std::scoped_lock lock(g_sys_env_mutex);
            if (g_sys_env_inited)
                g_sys_env.set(k, v);
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
