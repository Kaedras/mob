#include "pch.h"
#include "tools.h"
#include "../core/process.h"

namespace mob {

    tool::tool(std::string name)
        : cx_(nullptr), name_(std::move(name)), interrupted_(false)
    {
    }

    tool::tool(tool&& t)
        : name_(std::move(t.name_)), interrupted_(t.interrupted_.load())
    {
    }

    tool& tool::operator=(tool&& t)
    {
        cx_          = t.cx_;
        name_        = std::move(t.name_);
        interrupted_ = t.interrupted_.load();
        return *this;
    }

    void tool::set_name(const std::string& s)
    {
        name_ = s;
    }

    const std::string& tool::name() const
    {
        return name_;
    }

    void tool::run(context& cx)
    {
        cx_ = &cx;

        // tell the context this tool is running, used for logs
        cx_->set_tool(this);
        guard g([&] {
            cx_->set_tool(nullptr);
        });

        do_run();
    }

    void tool::interrupt()
    {
        if (interrupted_)
            return;

        cx().debug(context::interruption, "interrupting {}", name_);
        interrupted_ = true;
        do_interrupt();
    }

    void tool::do_interrupt()
    {
        // no-op
    }

    bool tool::interrupted() const
    {
        return interrupted_;
    }

    const context& tool::cx() const
    {
        if (cx_)
            return *cx_;
        else
            return gcx();
    }

    fs::path qt::installation_path()
    {
        return conf().path().qt_install();
    }

    fs::path qt::bin_path()
    {
        return conf().path().get("qt_bin");
    }

    std::string qt::version()
    {
        return conf().version().get("qt");
    }

    std::string qt::vs_version()
    {
        return conf().version().get("qt_vs");
    }

    transifex::transifex(ops o)
        : basic_process_runner("transifex"), op_(o), stdout_(context::level::trace),
          min_(100), force_(false)
    {
    }

    fs::path transifex::binary()
    {
        return conf().tool().get("tx");
    }

    transifex& transifex::root(const fs::path& p)
    {
        root_ = p;
        return *this;
    }

    transifex& transifex::api_key(const std::string& key)
    {
        key_ = key;
        return *this;
    }

    transifex& transifex::url(const mob::url& u)
    {
        url_ = u;
        return *this;
    }

    transifex& transifex::minimum(int percent)
    {
        min_ = percent;
        return *this;
    }

    transifex& transifex::stdout_level(context::level lv)
    {
        stdout_ = lv;
        return *this;
    }

    transifex& transifex::force(bool b)
    {
        force_ = b;
        return *this;
    }

    void transifex::do_run()
    {
        switch (op_) {
        case init:
            do_init();
            break;

        case config:
            do_config();
            break;

        case pull:
            do_pull();
            break;

        default:
            cx().bail_out(context::generic, "tx unknown op {}", op_);
        }
    }

    void transifex::do_init()
    {
        op::create_directories(cx(), root_, op::unsafe);

        // exit code is 2 when the directory already contains a .tx

        execute_and_join(process()
                             .binary(binary())
                             .success_exit_codes({0, 2})
                             .flags(process::ignore_output_on_success)
                             .arg("init")
                             .cwd(root_));
    }

    void transifex::do_config()
    {
        if (url_.empty())
            cx().bail_out(context::generic, "missing transifex url");

        op::create_directories(cx(), root_, op::unsafe);

        execute_and_join(process()
                             .binary(binary())
                             .stdout_level(stdout_)
                             .arg("add")
                             .arg("remote")
                             .arg(url_)
                             .env(this_env::get().set("TX_TOKEN", key_))
                             .cwd(root_));
    }

    void transifex::do_pull()
    {
        op::create_directories(cx(), root_, op::unsafe);

        auto p = process()
                     .binary(binary())
                     .stdout_level(stdout_)
                     .arg("pull")
                     .arg("--all")
                     .arg("--minimum-perc", min_)
                     .env(this_env::get().set("TX_TOKEN", key_))
                     .cwd(root_);

        if (force_)
            p.arg("--force");

        execute_and_join(p);
    }

    lrelease::lrelease() : basic_process_runner("lrelease") {}

    fs::path lrelease::binary()
    {
        return conf().tool().get("lrelease");
    }

    lrelease& lrelease::project(const std::string& name)
    {
        project_ = name;
        return *this;
    }

    lrelease& lrelease::add_source(const fs::path& ts_file)
    {
        sources_.push_back(ts_file);
        return *this;
    }

    lrelease& lrelease::sources(const std::vector<fs::path>& v)
    {
        sources_ = v;
        return *this;
    }

    lrelease& lrelease::out(const fs::path& dir)
    {
        out_ = dir;
        return *this;
    }

    fs::path lrelease::qm_file() const
    {
        if (sources_.empty())
            cx().bail_out(context::generic, "lrelease: no sources");

        // source files are something like "fr.ts", get "fr" and use the project
        // name to make something like "modorganizer_fr.qm"
        const auto lang = trim_copy(path_to_utf8(sources_[0].stem()));

        if (lang.empty()) {
            cx().bail_out(context::generic, "lrelease: bad file name '{}'",
                          sources_[0]);
        }

        return project_ + "_" + lang + ".qm";
    }

    void lrelease::do_run()
    {
        // that's the output file
        const auto qm = qm_file();

        auto p = process().binary(binary()).arg("-silent").stderr_filter([](auto&& f) {
            if (f.line.find("dropping duplicate") != -1)
                f.lv = context::level::debug;
            else if (f.line.find("try -verbose") != -1)
                f.lv = context::level::debug;
        });

        // input .ts files
        for (auto&& s : sources_)
            p.arg(s);

        // output .qm file
        p.arg("-qm", (out_ / qm));

        execute_and_join(p);
    }

    void build_loop(const context& cx, std::function<bool(bool)> f)
    {
        // building sometimes fails with files being locked
        const int max_tries = 3;

        for (int tries = 0; tries < max_tries; ++tries) {
            // try a multiprocess build
            if (f(true)) {
                // building succeeded, done
                return;
            }

            cx.debug(context::generic,
                     "multiprocess build sometimes fails because of race "
                     "conditions; trying again");
        }

        cx.debug(context::generic,
                 "multiprocess build has failed more than {} times, "
                 "restarting one last time single process; that one should work",
                 max_tries);

        // do one last single process build
        f(false);
    }

}  // namespace mob
