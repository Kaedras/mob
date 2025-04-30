#include "../pch.h"
#include "../process.h"
#include "../../net.h"
#include "../conf.h"
#include "../context.h"
#include "../op.h"
#include "../pipe.h"


namespace mob {
    // handle to dev/null
    //
    int get_bit_bucket()
    {
        return open("/dev/null", O_WRONLY);
    }

    process& process::binary(const fs::path& p)
    {
        if (p.extension() == ".exe") {
            exec_.bin = p.parent_path() / p.stem();
        } else {
            exec_.bin = p;
        }
        return *this;
    }

    void process::do_run(const std::string& what)
    {
        delete_external_log_file();

        STARTUPINFOW si = {};
        handle_ptr stdout_handle = redirect_stdout(si);
        handle_ptr stderr_handle = redirect_stderr(si);
        handle_ptr stdin_handle  = redirect_stdin(si);

        io_.out.buffer = encoded_buffer(io_.out.encoding);
        io_.err.buffer = encoded_buffer(io_.err.encoding);

        create("", what,  exec_.cwd.native(), si);
    }

    void process::create_job()
    {
    }

    handle_ptr process::redirect_stdout(STARTUPINFOW& si)
    {
        handle_ptr h;

        switch (io_.out.flags) {
        case forward_to_log:
        case keep_in_string: {
            impl_.stdout_pipe = std::make_unique<async_pipe_stdout>((context)*cx_);
            h             = impl_.stdout_pipe->create();
            si.stdOut = h.get();
            break;
        }

        case bit_bucket: {
            si.stdOut = get_bit_bucket();
            break;
        }

        case inherit: {
            si.stdOut = fileno(stdout);
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
            impl_.stderr_pipe = std::make_unique<async_pipe_stdout>(*cx_);
            h            = impl_.stderr_pipe->create();
            si.stdErr = h.get();
            break;
        }

        case bit_bucket: {
            si.stdErr = get_bit_bucket();
            break;
        }

        case inherit: {
            si.stdErr = fileno(stderr);
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

        si.stdIn = h.get();

        return h;
    }

    void process::create(std::string, std::string args, std::string cwd,
                         STARTUPINFOW si)
    {
        cx_->trace(context::cmd, "creating process");

        if (!cwd.empty()) {
            // the path might be relative, especially when it comes from the command
            // line, in which case it would fail the safety check
            op::create_directories(*cx_, fs::absolute(cwd));
        }

        // create argument list for execvp
        std::vector<std::string> argument_strings;
        std::istringstream iss(args);
        std::string arg;
        // Use std::quoted to handle quoted strings
        while (iss >> std::quoted(arg)) {
            // ignore empty strings
            if (arg.empty()) {
                continue;
            }
            // remove leading and trailing quotation marks
            if (arg.front() == '\"' || arg.front() == '\'') {
                arg.erase(0, 1);
            }
            if (arg.back() == '\"' || arg.back()== '\'') {
                arg.pop_back();
            }
            argument_strings.push_back(arg);
        }

        // convert strings to c-strings
        std::vector<const char*> arguments;
        arguments.reserve(argument_strings.size());
        for (const auto& argument : argument_strings) {
            arguments.push_back(argument.c_str());
        }

        // array must be terminated by a null pointer
        arguments.push_back(nullptr);

        pid_t pid = fork();

        // error
        if (pid == -1) {
            const int e = errno;
            cx_->bail_out(context::cmd, "failed to start '{}', {}", args,
                          strerror(e));
        }

        // child
        if (pid == 0) {
            if (!cwd.empty()) {
                chdir(cwd.c_str());
            }

            if (dup2(si.stdIn, 0) == -1) {
                cx_->error(context::cmd, "failed to redirect stdIn");
            }
            if (dup2(si.stdOut, 1) == -1) {
                cx_->error(context::cmd, "failed to redirect stdOut");
            }
            if (dup2(si.stdErr, 2) == -1) {
                cx_->error(context::cmd, "failed to redirect stdErr");
            }

            execvp(arguments[0], const_cast<char* const*>(arguments.data()));

            // exec only returns on error
            const int e = errno;
            cx_->error(context::cmd, "exec failed trying to run {}, {}", arguments[0],
                       strerror(e));
            return;
        }

        // parent
        cx_->trace(context::cmd, "pid {}", pid);

        // pid fd
        impl_.handle.reset(pidfd_open(pid, 0));
    }

    nativeString process::make_cmd_args(const std::string& what) const
    {
        return {};
    }

    void process::join()
    {
        if (!impl_.handle)
            return;

        // remembers if the process was already interrupted
        bool interrupted = false;

        // close the handle quickly after termination
        guard g([&] {
            impl_.handle = -1;
        });

        cx_->trace(context::cmd, "joining");

        for (;;) {
            pollfd pfd{impl_.handle.get(), POLLHUP | POLLIN, 0};
            int result = poll(&pfd, 1, wait_timeout);

            if (result > 0) {
                on_completed();
                break;
            }
            else if (result == 0) {
                on_timeout(interrupted);
            }
            else {
                const int e = errno;
                cx_->bail_out(context::cmd, "failed to wait on process",
                              strerror(e));
            }
        }

        if (interrupted)
            cx_->trace(context::cmd, "process interrupted and finished");
    }

    void process::terminate()
    {
        gcx().trace(context::cmd, "terminating process");

        int result = pidfd_send_signal(impl_.handle.get(), SIGTERM, nullptr, 0);

        if (result == -1) {
            const int e = errno;
            gcx().warning(context::cmd, "failed to terminate process, {}", strerror(e));
        }
    }


}