#pragma once
#include <string>
#include <vector>
#include <map>
#include "../Database/DataTable.hpp"
#include "SQLParams.hpp"

class ISQL {
public:
    virtual ~ISQL() = default;

    virtual bool Connect() = 0;
    virtual bool RunStatement(const std::string&) = 0;

    virtual bool RunPrepared(const std::string &, const std::vector<SQLParam> &) = 0;

    
    virtual DataTable FetchPrepared(const std::string&, const std::vector<std::string>&) = 0;
    virtual DataTable FetchPrepared(const std::string &, const std::string &) = 0;
    virtual DataTable FetchResults(const std::string&) = 0;
};
