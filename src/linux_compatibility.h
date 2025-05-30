#pragma once

#include <cerrno>
#include <csignal>
#include <fstream>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>

extern "C" {
#include <sys/pidfd.h>
}

#define CTRL_BREAK_EVENT SIGINT

using HANDLE                              = int;
static constexpr int INVALID_HANDLE_VALUE = -1;

// Detect if the application is running inside a debugger.
// Source: https://stackoverflow.com/a/69842462
inline bool IsDebuggerPresent()
{
    std::ifstream sf("/proc/self/status");
    std::string s;
    while (sf >> s) {
        if (s == "TracerPid:") {
            int pid;
            sf >> pid;
            return pid != 0;
        }
        std::getline(sf, s);
    }

    return false;
}

inline void DebugBreak()
{
    raise(SIGTRAP);
}

class FdCloser {
public:
    FdCloser() : m_fd(-1) {}
    FdCloser(int fd) : m_fd(fd) {}

    ~FdCloser()
    {
        if (m_fd != -1) {
            close(m_fd);
        }
    }

    FdCloser& operator=(int fd)
    {
        if (m_fd != -1) {
            close(m_fd);
        }
        m_fd = fd;

        return *this;
    }

    operator bool() const noexcept { return m_fd != -1; }

    int get() const { return m_fd; }
    void reset(int value = -1)
    {
        if (m_fd != -1) {
            close(m_fd);
        }
        m_fd = value;
    }

    int release()
    {
        int tmp = m_fd;
        m_fd    = -1;
        return tmp;
    }

    bool isValid() const { return m_fd != -1; }

private:
    int m_fd;
};

inline void CloseHandle(int h)
{
    close(h);
}
inline int GetLastError()
{
    return errno;
}

inline int GetProcessId(int pidFd)
{
    return pidfd_getpid(pidFd);
}

inline bool GenerateConsoleCtrlEvent(int, int pidFd)
{
    int result = pidfd_send_signal(pidFd, SIGINT, nullptr, 0);
    return result == 0;
}

inline bool GetExitCodeProcess(int pidFd, int* exitCode)
{
    siginfo_t info{};
    int result = waitid(P_PIDFD, pidFd, &info, WEXITED | WSTOPPED);
    if (result == -1) {
        return false;
    }

    *exitCode = info.si_status;
    return true;
}
