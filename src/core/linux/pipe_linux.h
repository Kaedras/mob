#ifndef PIPE_LINUX_H
#define PIPE_LINUX_H

#include "../../utility.h"
#include <cstddef>
#include <memory>

namespace mob {
    class context;

    // a pipe connected to a process's stdout or stderr, it is read from
    //
    class async_pipe_stdout {
    public:
        async_pipe_stdout(const context& cx);
        ~async_pipe_stdout();
        int create();
        std::string_view read(bool finish);
        bool closed() const;

    private:
        // calling context, used for logging
        const context& cx_;

        // the maximum number of bytes that can be put in the pipe
        static constexpr std::size_t buffer_size = 50 * 1024;

        // end of the pipe that is read from
        int pipe_;

        // internal buffer of `buffer_size` bytes, the data from the pipe is put
        // in there
        std::unique_ptr<char[]> buffer_;

        // whether the last read attempt had `finished` true and nothing was
        // available in the pipe; in this case, the pipe is considered closed
        bool closed_;

        // called when pending_ is false; tries to read from the pipe, which may
        // start an async operation, in which case pending_ is set to true and an
        // empty string is returned
        //
        // if the read operation was completed synchronously, returns the bytes
        // that were read
        //
        std::string_view try_read();
    };

    // a pipe connected to a process's stdin, it is written to; this pipe is
    // synchronous and does not keep a copy of the given buffer, see write()
    //
    class async_pipe_stdin {
    public:
        async_pipe_stdin(const context& cx);

        int create();

        // tries to send all of `s` down the pipe, returns the number of bytes
        // actually written
        //
        std::size_t write(std::string_view s);

        // closes the pipe, should be called as soon as everything has been written
        //
        void close();

    private:
        // calling context, used for logging
        const context& cx_;

        // end of the pipe that is written to
        int pipe_;
    };
}  // namespace mob

#endif  // PIPE_LINUX_H
