#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <variant>

class DataTable
{
public:
    class Row
    {
    public:
        using VariantType = std::variant<int, std::string, std::vector<uint8_t>>;
        using Value = std::optional<VariantType>;

        class ValueProxy
        {
        public:
            ValueProxy(Value *valuePtr) : valuePtr_(valuePtr) {}

            template <typename T>
            operator std::optional<T>() const
            {
                if (!valuePtr_ || !valuePtr_->has_value())
                    return std::nullopt;

                if (auto val = std::get_if<T>(&valuePtr_->value()))
                    return *val;

                throw std::runtime_error("Error parsing value to type");
            }
            
            template <typename T>
            operator T() const
            {
                if (!valuePtr_ || !valuePtr_->has_value())
                    throw std::runtime_error("Value is null");

                if (auto val = std::get_if<T>(&valuePtr_->value()))
                    return *val;

                throw std::runtime_error("Error parsing value to type");
            }

            bool IsNull() const
            {
                return !valuePtr_ || !valuePtr_->has_value();
            }

        private:
            Value *valuePtr_;
        };

        void Set(const std::string &column, const Value &value);
        void SetColumns(const std::vector<std::string> &columns);

        ValueProxy operator[](const std::string &column);
        const Value &operator[](const std::string &column) const;

    private:
        std::unordered_map<std::string, Value> values;
    };

    void SetColumns(const std::vector<std::string> &cols);
    void Fill(const std::vector<Row> &newData);
    Row &operator[](int index);
    const Row &operator[](int index) const;
    int RowsCount() const;

private:
    std::vector<std::string> columns;
    std::vector<Row> data;
};
