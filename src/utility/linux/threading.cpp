#include "../threading.h"
#include "../../utility.h"
#include "../pch.h"
#include <execinfo.h>


namespace mob {

    constexpr std::size_t max_name_length      = 1000;
    constexpr std::size_t max_frames               = 100;
    constexpr std::size_t exception_message_length = 5000;

    static void* frame_addresses[max_frames];
    static char **strings;
    // static char undecorated_name[max_name_length + 1] = {};
    // static unsigned char sym_buffer[sizeof(SYMBOL_INFOW) + max_name_length];
    // static SYMBOL_INFOW* sym = (SYMBOL_INFOW*)sym_buffer;
    static char exception_message[exception_message_length + 1] = {};

    // static LPTOP_LEVEL_EXCEPTION_FILTER g_previous_handler = nullptr;

    void dump_stacktrace(const char* what)
    {
        // don't use 8ucout, don't lock the global out mutex, this can be called
        // while the mutex is locked
        std::cerr << what << "\n\nmob has crashed\n"
                   << "*****************************\n\n"
                   << what << "\n\n";

        const int frame_count = backtrace(frame_addresses, max_frames);
        strings = backtrace_symbols(frame_addresses, frame_count);

        for (std::size_t i = 0; i < frame_count; ++i) {
            std::cerr << strings[i] << "\n";
        }

        free(strings);

        if (IsDebuggerPresent())
            raise(SIGTRAP);
        else
            exit(1);
    }

    void terminate_handler() noexcept
    {
        try {
            std::rethrow_exception(std::current_exception());
        }
        catch (std::exception& e) {
        }
        catch (...) {
        }

        dump_stacktrace("unhandled exception");
    }

    void unhandled_exception_handler(int sig) noexcept
    {
        dump_stacktrace(sigdescr_np(sig));
    }

    void set_thread_exception_handlers()
    {
        signal(SIGTERM, unhandled_exception_handler);
        std::set_terminate(mob::terminate_handler);
    }

    std::size_t make_thread_count(std::optional<std::size_t> count)
    {
        static const auto def = std::thread::hardware_concurrency();
        return std::max<std::size_t>(1, count.value_or(def));
    }

    thread_pool::thread_pool(std::optional<std::size_t> count)
        : count_(make_thread_count(count))
    {
        for (std::size_t i = 0; i < count_; ++i)
            threads_.emplace_back(std::make_unique<thread_info>());
    }

    thread_pool::~thread_pool()
    {
        join();
    }

    void thread_pool::join()
    {
        for (auto&& t : threads_) {
            if (t->thread.joinable())
                t->thread.join();
        }
    }

    void thread_pool::add(fun thread_fun)
    {
        for (;;) {
            if (try_add(thread_fun))
                break;

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    bool thread_pool::try_add(fun thread_fun)
    {
        for (auto& t : threads_) {
            if (t->running)
                continue;

            // found one

            if (t->thread.joinable())
                t->thread.join();

            t->running    = true;
            t->thread_fun = thread_fun;

            t->thread = start_thread([&] {
                t->thread_fun();
                t->running = false;
            });

            return true;
        }

        return false;
    }

}  // namespace mob