#include "pch.h"
#include "../../core/process.h"
#include "../tools.h"

namespace mob {

    vs::vs(ops o) : basic_process_runner("vs"), op_(o) {}

    fs::path vs::devenv_binary()
    {
        return conf().tool().get("devenv");
    }

    fs::path vs::installation_path()
    {
        return conf().path().vs();
    }

    fs::path vs::vswhere()
    {
        return conf().tool().get("vswhere");
    }

    fs::path vs::vcvars()
    {
        return conf().tool().get("vcvars");
    }

    std::string vs::version()
    {
        return conf().version().get("vs");
    }

    std::string vs::year()
    {
        return conf().version().get("vs_year");
    }

    std::string vs::toolset()
    {
        return conf().version().get("vs_toolset");
    }

    std::string vs::sdk()
    {
        return conf().version().get("sdk");
    }

    vs& vs::solution(const fs::path& sln)
    {
        sln_ = sln;
        return *this;
    }

    void vs::do_run()
    {
        switch (op_) {
        case upgrade: {
            do_upgrade();
            break;
        }

        default: {
            cx().bail_out(context::generic, "vs unknown op {}", op_);
        }
        }
    }

    void vs::do_upgrade()
    {
        // assume the project is already upgraded if UpgradeLog.htm exists, because
        // upgrading is slow even if it's not necessary
        if (fs::exists(sln_.parent_path() / "UpgradeLog.htm")) {
            cx().debug(context::generic, "project already upgraded");
            return;
        }

        execute_and_join(process()
                             .binary(devenv_binary())
                             .env(env::vs(arch::x64))
                             .arg("/upgrade")
                             .arg(sln_));
    }

    std::string vswhere::find_vs()
    {
        auto p = process()
                     .binary(vs::vswhere())
                     .arg("-products", "*")
                     .arg("-prerelease")
                     .arg("-version", vs::version())
                     .arg("-property", "installationPath")
                     .stdout_flags(process::keep_in_string)
                     .stderr_flags(process::inherit);

        p.run();
        p.join();

        if (p.exit_code() != 0)
            return {};

        return trim_copy(p.stdout_string());
    }

    iscc::iscc(fs::path iss) : basic_process_runner("iscc"), iss_(std::move(iss)) {}

    fs::path iscc::binary()
    {
        return conf().tool().get("iscc");
    }

    iscc& iscc::iss(const fs::path& p)
    {
        iss_ = p;
        return *this;
    }

    void iscc::do_run()
    {
        if (iss_.empty())
            cx().bail_out(context::generic, "iscc missing iss file");

        execute_and_join(process().binary(binary()).arg(iss_));
    }

}  // namespace mob
