#pragma once
#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include <stdexcept>
#include <vector>
#include <map>
#include <string>
#include "../Interfaces/ISQL.hpp"
#include "DataTable.hpp"
#include "SQLParams.hpp"

class SQL : public ISQL{
    private:
        SQLHENV henv;
        SQLHDBC hdbc;
        SQLHSTMT hstmt;

        std::string _ServerName;
        std::string _DatabaseName;
        std::string _UserName;
        std::string _Password;
        bool _TrustServerCertificate;

        void PrepareStatement(const std::string&);
        std::string ExtractError(const char *, SQLHANDLE, SQLSMALLINT);
        void RunQuery(const std::string&);

        std::vector<std::string> stringStorage;
        std::vector<std::vector<uint8_t>> binaryStorage;
        std::vector<int> intStorage;
        std::vector<SQLLEN> indStorage;  

    public:
        SQL();
        ~SQL();
        void ServerName(const std::string&);
        void DatabaseName(const std::string&);
        void UserName(const std::string&);
        void Password(const std::string&);
        void TrustServerCertificate(const bool&);
        bool Connect() override;
        bool RunStatement(const std::string&) override;
        
        bool RunPrepared(const std::string &, const std::vector<SQLParam> &) override;

        DataTable FetchPrepared(const std::string&, const std::vector<std::string>&) override;
        DataTable FetchPrepared(const std::string &, const std::string &) override;
        DataTable FetchResults(const std::string&) override;

        bool BeginTransaction();
        bool CommitTransaction();
        bool RollbackTransaction();

        void Disconnect();
    };
    