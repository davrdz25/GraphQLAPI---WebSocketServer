#pragma once

#include <variant>
#include <string>
#include <vector>
#include <optional>

using SQLParam = std::variant<
    std::monostate,          // NULL
    int,                    // Enteros
    std::string,            // Texto
    std::vector<uint8_t>    // Datos binarios
>;

template <typename T>
inline SQLParam MakeSQLParam(const std::optional<T>& opt)
{
    if (opt.has_value())
        return SQLParam{*opt};
    else
        return SQLParam{std::monostate{}};
}

inline SQLParam MakeSQLParam(int val)
{
    return SQLParam{val};
}

inline SQLParam MakeSQLParam(const std::string& val)
{
    return SQLParam{val};
}

inline SQLParam MakeSQLParam(const std::vector<uint8_t>& val)
{
    return SQLParam{val};
}