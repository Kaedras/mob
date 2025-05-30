#include "pch.h"
#include "../../net.h"
#include "../conf.h"
#include "../context.h"
#include "../op.h"
#include "../pipe.h"
#include "../process.h"

namespace mob {
    // handle to dev/null
    //
    HANDLE get_bit_bucket()
    {
        SECURITY_ATTRIBUTES sa{.nLength = sizeof(sa), .bInheritHandle = TRUE};
        return ::CreateFileW(L"NUL", GENERIC_WRITE, 0, &sa, OPEN_EXISTING, 0, 0);
    }

    process& process::binary(const fs::path& p)
    {
        exec_.bin = p;
        return *this;
    }

    void process::do_run(const std::string& what)
    {
        delete_external_log_file();
        create_job();

        io_.out.buffer = encoded_buffer(io_.out.encoding);
        io_.err.buffer = encoded_buffer(io_.err.encoding);

        STARTUPINFOW si = {};
        si.cb           = sizeof(si);
        si.dwFlags      = STARTF_USESTDHANDLES;

        // these handles are given to STARTUPINFOW and must stay alive until the
        // process is created in create(), they can be closed after that
        handle_ptr stdout_handle = redirect_stdout(si);
        handle_ptr stderr_handle = redirect_stderr(si);
        handle_ptr stdin_handle  = redirect_stdin(si);

        const std::wstring cmd  = utf8_to_utf16(this_env::get("COMSPEC"));
        const std::wstring args = make_cmd_args(what);
        const std::wstring cwd  = exec_.cwd.native();

        create(cmd, args, cwd, si);
    }

    void process::create_job()
    {
        SetLastError(0);
        HANDLE job   = CreateJobObjectW(nullptr, nullptr);
        const auto e = GetLastError();

        if (job == 0) {
            cx_->warning(context::cmd, "failed to create job, {}", error_message(e));
        }
        else {
            MOB_ASSERT(e != ERROR_ALREADY_EXISTS);
            impl_.job.reset(job);
        }
    }

    handle_ptr process::redirect_stdout(STARTUPINFOW& si)
    {
        handle_ptr h;

        switch (io_.out.flags) {
        case forward_to_log:
        case keep_in_string: {
            impl_.stdout_pipe.reset(new async_pipe_stdout(*cx_));
            h             = impl_.stdout_pipe->create();
            si.hStdOutput = h.get();
            break;
        }

        case bit_bucket: {
            si.hStdOutput = get_bit_bucket();
            break;
        }

        case inherit: {
            si.hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
            break;
        }
        }

        return h;
    }

    handle_ptr process::redirect_stderr(STARTUPINFOW& si)
    {
        handle_ptr h;

        switch (io_.err.flags) {
        case forward_to_log:
        case keep_in_string: {
            impl_.stderr_pipe.reset(new async_pipe_stdout(*cx_));
            h            = impl_.stderr_pipe->create();
            si.hStdError = h.get();
            break;
        }

        case bit_bucket: {
            si.hStdError = get_bit_bucket();
            break;
        }

        case inherit: {
            si.hStdError = ::GetStdHandle(STD_ERROR_HANDLE);
            break;
        }
        }

        return h;
    }

    handle_ptr process::redirect_stdin(STARTUPINFOW& si)
    {
        handle_ptr h;

        if (io_.in) {
            impl_.stdin_pipe.reset(new async_pipe_stdin(*cx_));
            h = impl_.stdin_pipe->create();
        }
        else {
            h.reset(get_bit_bucket());
        }

        si.hStdInput = h.get();

        return h;
    }

    void process::create(std::wstring cmd, std::wstring args, std::filesystem::path cwd,
                         STARTUPINFOW si)
    {
        cx_->trace(context::cmd, "creating process");

        if (!cwd.empty()) {
            // the path might be relative, especially when it comes from the command
            // line, in which case it would fail the safety check
            op::create_directories(*cx_, fs::absolute(cwd));
        }

        // cwd
        const wchar_t* cwd_p = (cwd.empty() ? nullptr : cwd.c_str());

        // flags
        const DWORD flags =
            // will forward sigint to child processes
            CREATE_NEW_PROCESS_GROUP |

            // the pointer given for environment variables is a utf16 string, not
            // codepage
            CREATE_UNICODE_ENVIRONMENT;

        // creating process
        PROCESS_INFORMATION pi = {};
        const auto r =
            ::CreateProcessW(cmd.c_str(), args.data(), nullptr, nullptr,
                             TRUE,  // inherit handles
                             flags, exec_.env.get_unicode_pointers(), cwd_p, &si, &pi);

        if (!r) {
            const auto e = GetLastError();
            cx_->bail_out(context::cmd, "failed to start '{}', {}", args,
                          error_message(e));
        }

        if (impl_.job) {
            if (!::AssignProcessToJobObject(impl_.job.get(), pi.hProcess)) {
                // this shouldn't fail, but the only consequence is that ctrl-c
                // won't be able to kill everything, so make it a warning
                const auto e = GetLastError();
                cx_->warning(context::cmd, "can't assign process to job, {}",
                             error_message(e));
            }
        }

        cx_->trace(context::cmd, "pid {}", pi.dwProcessId);

        // not needed
        ::CloseHandle(pi.hThread);

        // process handle
        impl_.handle.reset(pi.hProcess);
    }

    nativeString process::make_cmd_args(const std::string& what) const
    {
        std::wstring s;

        // /U forces cmd builtins to output utf16, such as `set` or `env`, used by
        // vcvars to get the environment variables
        if (io_.unicode)
            s += L"/U ";

        // /C runs the command and exits
        s += L"/C ";

        s += L"\"";

        // run chcp first if necessary
        if (io_.chcp != -1)
            s += L"chcp " + std::to_wstring(io_.chcp) + L" && ";

        // process command line
        s += utf8_to_utf16(what);

        s += L"\"";

        return s;
    }

    void process::join()
    {
        if (!impl_.handle)
            return;

        // remembers if the process was already interrupted
        bool interrupted = false;

        // close the handle quickly after termination
        guard g([&] {
            impl_.handle = {};
        });

        cx_->trace(context::cmd, "joining");

        for (;;) {
            // returns if the process is done or after the timeout
            const auto r = WaitForSingleObject(impl_.handle.get(), wait_timeout);

            if (r == WAIT_OBJECT_0) {
                on_completed();
                break;
            }
            else if (r == WAIT_TIMEOUT) {
                on_timeout(interrupted);
            }
            else {
                const auto e = GetLastError();
                cx_->bail_out(context::cmd, "failed to wait on process",
                              error_message(e));
            }
        }

        if (interrupted)
            cx_->trace(context::cmd, "process interrupted and finished");
    }

    void process::terminate()
    {
        UINT exit_code = 0xff;

        if (impl_.job) {
            // kill all the child processes in the job

            JOBOBJECT_BASIC_ACCOUNTING_INFORMATION info = {};

            const auto r = ::QueryInformationJobObject(
                impl_.job.get(), JobObjectBasicAccountingInformation, &info,
                sizeof(info), nullptr);

            if (r) {
                gcx().trace(context::cmd,
                            "terminating job, {} processes ({} spawned total)",
                            info.ActiveProcesses, info.TotalProcesses);
            }
            else {
                gcx().trace(context::cmd, "terminating job");
            }

            if (::TerminateJobObject(impl_.job.get(), exit_code)) {
                // done
                return;
            }

            const auto e = GetLastError();
            gcx().warning(context::cmd, "failed to terminate job, {}",
                          error_message(e));
        }

        // either job creation failed or job termination failed, last ditch attempt
        ::TerminateProcess(impl_.handle.get(), exit_code);
    }

}  // namespace mob
