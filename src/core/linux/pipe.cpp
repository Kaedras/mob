#include "../pch.h"
#include "../context.h"
#include "../process.h"
#include "pipe_linux.h"

namespace mob {

    async_pipe_stdout::async_pipe_stdout(const context& cx)
        : cx_(cx), buffer_(std::make_unique<char[]>(buffer_size)), closed_(true)
    {
        std::memset(buffer_.get(), 0, buffer_size);
    }

    bool async_pipe_stdout::closed() const
    {
        return closed_;
    }

    handle_ptr async_pipe_stdout::create()
    {
        int pipeFd[2];

        if (pipe(pipeFd) == -1) {
            const int e = errno;
            cx_.bail_out(context::cmd, "CreatePipe failed, {}", strerror(e));
        }

        pipe_.reset(pipeFd[0]);
        closed_ = false;

        return handle_ptr(pipeFd[1]);
    }

    std::string_view async_pipe_stdout::read(bool finish)
    {
        std::string_view s;

        if (closed_) {
            // no-op
            return s;
        }

        s = try_read();

        if (finish && s.empty()) {
            // a future call to read() will be a no-op and closed() will return true
            closed_ = true;
        }

        // the bytes that were read, if any
        return s;
    }

    std::string_view async_pipe_stdout::try_read()
    {
        size_t bytes_read = 0;

        // read bytes from the pipe
        bytes_read = ::read(pipe_.get(), buffer_.get(), buffer_size);

        if (bytes_read) {
            return {buffer_.get(), bytes_read};
        }

        const int e = errno;
        if (e == EPIPE) {
            // broken pipe means the process is finished
            closed_ = true;
        } else {
            // some other hard error
            cx_.bail_out(context::cmd, "async_pipe_stdout read failed, {}",
                         strerror(e));
        }

        // nothing available
        return {};
    }

    async_pipe_stdin::async_pipe_stdin(const context& cx) : cx_(cx) {}

    handle_ptr async_pipe_stdin::create()
    {
        int pipeFd[2];

        if (pipe(pipeFd) == -1) {
            const int e = errno;
            cx_.bail_out(context::cmd, "CreatePipe failed, {}", strerror(e));
        }

        // keep the end that's written to
        pipe_.reset(pipeFd[1]);

        // give to other end to the new process
        return handle_ptr(pipeFd[0]);
    }

    std::size_t async_pipe_stdin::write(std::string_view s)
    {
        // bytes to write
        const size_t n = s.length();

        // bytes actually written
        size_t written = ::write(pipe_, s.data(), n);

        if (written != n) {
            // hard error
            const auto e = errno;

            cx_.bail_out(context::cmd, "WriteFile failed in async_pipe_stdin, {}",
                         strerror(e));
        }

        return written;
    }

    void async_pipe_stdin::close()
    {
        pipe_ = {};
    }

}  // namespace mob
