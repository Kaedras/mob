#include "pch.h"
#include "string.h"
#include "../utility.h"

namespace mob {

    std::string replace_all(std::string s, const std::string& from,
                            const std::string& to)
    {
        std::size_t start = 0;

        for (;;) {
            const auto pos = s.find(from, start);
            if (pos == std::string::npos)
                break;

            s.replace(pos, from.size(), to);
            start = pos + to.size();
        }

        return s;
    }

    std::vector<std::string> split(const std::string& s, const std::string& seps)
    {
        std::vector<std::string> v;

        std::size_t start = 0;

        while (start < s.size()) {
            auto p = s.find_first_of(seps, start);
            if (p == std::string::npos)
                p = s.size();

            if (p - start > 0)
                v.push_back(s.substr(start, p - start));

            start = p + 1;
        }

        return v;
    }

    std::vector<std::string> split_quoted(const std::string& s, const std::string& seps)
    {
        std::vector<std::string> v;
        std::string token;

        // currently between double quotes
        bool q = false;

        for (std::size_t i = 0; i < s.size(); ++i) {
            if (seps.find(s[i]) != std::string::npos) {
                // this is a separator

                if (q) {
                    // in quotes, add to current token
                    token += s[i];
                }
                else if (!token.empty()) {
                    // not in quotes, push this token and reset
                    v.push_back(token);
                    token = "";
                }
            }
            else if (s[i] == '"') {
                // double quote

                if (q) {
                    // end of quoted token
                    q = false;

                    if (!token.empty()) {
                        // push this token and reset
                        v.push_back(token);
                        token = "";
                    }
                }
                else {
                    // start of quoted token
                    q = true;
                }
            }
            else {
                // not a separator, not a quote
                token += s[i];
            }
        }

        // last token
        if (!token.empty())
            v.push_back(token);

        return v;
    }

    template <class C>
    void trim_impl(std::basic_string<C>& s, std::basic_string_view<C> what)
    {
        while (!s.empty()) {
            if (what.find(s[0]) != std::string::npos)
                s.erase(0, 1);
            else if (what.find(s[s.size() - 1]) != std::string::npos)
                s.erase(s.size() - 1, 1);
            else
                break;
        }
    }

    template <class C>
    std::basic_string<C> trim_copy_impl(std::basic_string_view<C> s,
                                        std::basic_string_view<C> what)
    {
        std::basic_string<C> c(s);
        trim(c, what);
        return c;
    }

    void trim(std::string& s, std::string_view what)
    {
        trim_impl(s, what);
    }

    void trim(std::wstring& s, std::wstring_view what)
    {
        trim_impl(s, what);
    }

    std::string trim_copy(std::string_view s, std::string_view what)
    {
        return trim_copy_impl(s, what);
    }

    std::wstring trim_copy(std::wstring_view s, std::wstring_view what)
    {
        return trim_copy_impl(s, what);
    }

    std::string pad_right(std::string s, std::size_t n, char c)
    {
        if (s.size() < n)
            s.append(n - s.size(), c);

        return s;
    }

    std::string pad_left(std::string s, std::size_t n, char c)
    {
        if (s.size() < n)
            s.insert(s.begin(), n - s.size(), c);

        return s;
    }

    std::string table(const std::vector<std::pair<std::string, std::string>>& v,
                      std::size_t indent, std::size_t spacing)
    {
        std::size_t longest = 0;

        for (auto&& p : v)
            longest = std::max(longest, p.first.size());

        std::string s;

        for (auto&& p : v) {
            if (!s.empty())
                s += "\n";

            s += std::string(indent, ' ') + pad_right(p.first, longest) + " " +
                 std::string(spacing, ' ') + p.second;
        }

        return s;
    }

}  // namespace mob
