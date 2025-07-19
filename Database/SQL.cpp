#include "SQL.hpp"

std::string SQL::ExtractError(const char *fn, SQLHANDLE handle, SQLSMALLINT type)
{
    if (!handle)
    {
        return std::string(" [ExtractError] Null Handle or not initialized.");
    }

    SQLINTEGER i = 0;
    SQLINTEGER native;
    SQLCHAR state[7];
    SQLCHAR text[256];
    SQLSMALLINT len;
    SQLRETURN ret;

    std::string errors;
    do
    {
        ret = SQLGetDiagRec(type, handle, ++i, state, &native, text, sizeof(text), &len);
        if (SQL_SUCCEEDED(ret))
        {
            errors += "SQL State: ";
            errors += reinterpret_cast<const char *>(state);
            errors += "\nMessage: ";
            errors += std::string(reinterpret_cast<const char *>(text), len);
        }
        else if (ret == SQL_INVALID_HANDLE)
        {
            errors += "[ExtractError] SQL_INVALID_HANDLE";
            break;
        }
    } while (ret == SQL_SUCCESS);

    return errors;
}

SQL::SQL() : henv(SQL_NULL_HENV), hdbc(SQL_NULL_HDBC), hstmt(SQL_NULL_HSTMT) {}

SQL::~SQL()
{
    Disconnect();
}

void SQL::ServerName(const std::string &_ServerName)
{
    this->_ServerName = _ServerName;
};

void SQL::DatabaseName(const std::string &_DatabaseName)
{
    this->_DatabaseName = _DatabaseName;
};

void SQL::UserName(const std::string &_UserName)
{
    this->_UserName = _UserName;
};

void SQL::Password(const std::string &_Password)
{
    this->_Password = _Password;
};

void SQL::TrustServerCertificate(const bool &_TrustServerCertificate)
{
    this->_TrustServerCertificate = _TrustServerCertificate;
};

bool SQL::Connect()
{
    try
    {
        SQLRETURN ret;

        std::string connString = "Driver={ODBC Driver 18 for SQL Server};Server=" + _ServerName + ";UID=" + _UserName + ";PWD=" + _Password + ";Database=" + _DatabaseName + ";TrustServerCertificate=yes;";

        ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
        if (!SQL_SUCCEEDED(ret))
            throw std::runtime_error(ExtractError("SQLAllocHandle ENV", henv, SQL_HANDLE_ENV));

        ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
        if (!SQL_SUCCEEDED(ret))
            throw std::runtime_error(ExtractError("SQLSetEnvAttr", henv, SQL_HANDLE_ENV));

        ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
        if (!SQL_SUCCEEDED(ret))
            throw std::runtime_error(ExtractError("SQLAllocHandle DBC", henv, SQL_HANDLE_ENV));

        ret = SQLDriverConnect(
            hdbc, nullptr,
            (SQLCHAR *)connString.c_str(),
            SQL_NTS, nullptr, 0, nullptr, SQL_DRIVER_NOPROMPT);

        if (!SQL_SUCCEEDED(ret))
            throw std::runtime_error(ExtractError("SQLConnect", hdbc, SQL_HANDLE_DBC));

        return true;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("[SQL::Connect] ") + e.what());
    }
}

void SQL::Disconnect()
{
    try
    {
        if (hdbc != SQL_NULL_HDBC)
        {
            SQLDisconnect(hdbc);
            SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
            hdbc = SQL_NULL_HDBC;
        }
        if (henv != SQL_NULL_HENV)
        {
            SQLFreeHandle(SQL_HANDLE_ENV, henv);
            henv = SQL_NULL_HENV;
        }
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("[SQL::Disconnect] ") + e.what() + "\n" +
                                 ExtractError("SQL::Disconnect", hdbc, SQL_HANDLE_DBC));
    }
}

void SQL::PrepareStatement(const std::string &query)
{
    try
    {
        if (hstmt != SQL_NULL_HSTMT)
        {
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            hstmt = SQL_NULL_HSTMT;
        }

        SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

        if (!SQL_SUCCEEDED(ret))
            throw std::runtime_error(ExtractError("SQLAllocHandle", hdbc, SQL_HANDLE_DBC));

        ret = SQLPrepare(hstmt, (SQLCHAR *)query.c_str(), SQL_NTS);

        std::cout << "Query: " << query << std::endl;

        if (!SQL_SUCCEEDED(ret))
            throw std::runtime_error(ExtractError("SQLPrepare", hstmt, SQL_HANDLE_STMT));
    }
    catch (const std::exception &ex)
    {
        throw std::runtime_error(std::string("[SQL::PrepareStatement] ") + ex.what());
    }
}

bool SQL::RunStatement(const std::string &query)
{
    try
    {
        SQLRETURN ret;

        if (hstmt != SQL_NULL_HSTMT)
        {
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            hstmt = SQL_NULL_HSTMT;
        }

        ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

        if (!SQL_SUCCEEDED(ret))
            throw std::runtime_error(ExtractError("SQLAllocHandle", hdbc, SQL_HANDLE_DBC));

        ret = SQLExecDirect(hstmt, (SQLCHAR *)query.c_str(), SQL_NTS);

        if (!SQL_SUCCEEDED(ret))
            throw std::runtime_error(ExtractError("SQL::RunStatement", hstmt, SQL_HANDLE_STMT));

        return true;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("[SQL::RunStatement] ") + e.what());
    }
}

bool SQL::BeginTransaction()
{
    try
    {
        SQLRETURN ret = SQLSetConnectAttr(hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0);

        if (!SQL_SUCCEEDED(ret))
            throw std::runtime_error(ExtractError("SQL::BeginTransaction", hdbc, SQL_HANDLE_DBC));

        return true;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("[SQL::BeginTransaction] ") + e.what());
    }
}

bool SQL::CommitTransaction()
{
    try
    {
        SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);

        if (!SQL_SUCCEEDED(ret))
            throw std::runtime_error(ExtractError("SQLAllocHandle", hdbc, SQL_HANDLE_DBC));

        ret = SQLSetConnectAttr(hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);

        if (!SQL_SUCCEEDED(ret))
            throw std::runtime_error(ExtractError("SQL::Commit", hdbc, SQL_HANDLE_DBC));

        return true;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("[SQL::Commit] ") + e.what());
    }
}

bool SQL::RollbackTransaction()
{
    try
    {
        SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_ROLLBACK);

        if (!SQL_SUCCEEDED(ret))
            throw std::runtime_error(ExtractError("SQL::Rollback", hdbc, SQL_HANDLE_DBC));

        ret = SQLSetConnectAttr(hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);

        if (!SQL_SUCCEEDED(ret))
            throw std::runtime_error(ExtractError("SQL::Rollback", hdbc, SQL_HANDLE_DBC));

        return true;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("[SQL::Rollback] ") + e.what());
    }
}

bool SQL::RunPrepared(const std::string &query, const std::vector<SQLParam> &params)
{
    try
    {
        SQLRETURN retcode;

        if (hstmt != SQL_NULL_HSTMT)
        {
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            hstmt = SQL_NULL_HSTMT;
        }

        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

        if (!SQL_SUCCEEDED(retcode))
            throw std::runtime_error(ExtractError("SQLPrepare", hdbc, SQL_HANDLE_STMT));

        retcode = SQLPrepare(hstmt, (SQLCHAR *)query.c_str(), SQL_NTS);

        if (!SQL_SUCCEEDED(retcode))
            throw std::runtime_error(ExtractError("SQLPrepare", hstmt, SQL_HANDLE_STMT));

        stringStorage.clear();
        stringStorage.reserve(params.size());

        intStorage.clear();
        intStorage.reserve(params.size());

        binaryStorage.clear();
        binaryStorage.reserve(params.size());

        indStorage.clear();
        indStorage.resize(params.size());

        SQLUSMALLINT paramIndex = 1;

        for (const auto &param : params)
        {
            std::cout << "[RunPrepared] Enlazando parámetro " << paramIndex << ": ";

            retcode = std::visit([&](auto &&value) -> SQLRETURN
                                 {
        using T = std::decay_t<decltype(value)>;

        SQLPOINTER dataPtr = nullptr;

        if constexpr (std::is_same_v<T, std::monostate>)
        {
            std::cout << "NULL" << std::endl;
            static char dummy = 0;
            dataPtr = &dummy;
            indStorage[paramIndex - 1] = SQL_NULL_DATA;

            return SQLBindParameter(
                hstmt,
                paramIndex,
                SQL_PARAM_INPUT,
                SQL_C_CHAR,
                SQL_VARCHAR,
                0, 0,
                dataPtr,
                0,
                &indStorage[paramIndex - 1]);
        }
        else if constexpr (std::is_same_v<T, int>)
        {
            intStorage.push_back(value);
            dataPtr = &intStorage.back();
            indStorage[paramIndex - 1] = sizeof(int);
            std::cout << "int = " << value << std::endl;

            return SQLBindParameter(
                hstmt,
                paramIndex,
                SQL_PARAM_INPUT,
                SQL_C_SLONG,
                SQL_INTEGER,
                0, 0,
                dataPtr,
                0,
                &indStorage[paramIndex - 1]);
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
   
            stringStorage.emplace_back(value);
            auto& storedStr = stringStorage.back();

            dataPtr = (SQLPOINTER)storedStr.c_str();
            indStorage[paramIndex - 1] = static_cast<SQLLEN>(storedStr.size());

            std::cout << "string (" << indStorage[paramIndex - 1] << " bytes) = '" << storedStr << "'" << std::endl;

            return SQLBindParameter(
                hstmt,
                paramIndex,
                SQL_PARAM_INPUT,
                SQL_C_CHAR,
                SQL_VARCHAR,
                indStorage[paramIndex - 1],
                0,
                dataPtr,
                0,
                &indStorage[paramIndex - 1]);
        }
        else if constexpr (std::is_same_v<T, std::vector<uint8_t>>)
        {
            binaryStorage.push_back(value);
            dataPtr = (SQLPOINTER)binaryStorage.back().data();
            indStorage[paramIndex - 1] = static_cast<SQLLEN>(binaryStorage.back().size());

            std::cout << "[RunPrepared] binary (" << indStorage[paramIndex - 1] << " bytes)" << std::endl;

            return SQLBindParameter(
                hstmt,
                paramIndex,
                SQL_PARAM_INPUT,
                SQL_C_BINARY,
                SQL_VARBINARY,
                indStorage[paramIndex - 1],
                0,
                dataPtr,
                indStorage[paramIndex - 1],
                &indStorage[paramIndex - 1]);
        }
        else
        {
            throw std::runtime_error("Unsupported SQL parameter type");
        } }, param);

            if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
            {
                std::string errMsg = ExtractError("SQLBindParameter", hstmt, SQL_HANDLE_STMT);
                errMsg += "[RunPrepared Error] Parameter bind failed" + std::to_string(paramIndex) + ": " + errMsg;

                throw std::runtime_error("Error binding parameter " + std::to_string(paramIndex));
            }

            paramIndex++;
        }

        retcode = SQLExecute(hstmt);

        if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
        //{
            //std::string errMsg = ExtractError("SQLExecute", hstmt, SQL_HANDLE_STMT);
            //std::cerr << "[RunPrepared Error] Falló SQLExecute: " << errMsg << std::endl;
            throw std::runtime_error(ExtractError("SQLExecute", hstmt, SQL_HANDLE_STMT));
        //}

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        hstmt = SQL_NULL_HSTMT;

        return true;
    }
    catch (const std::exception &ex)
    {
        if (hstmt != SQL_NULL_HSTMT)
        {
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            hstmt = SQL_NULL_HSTMT;
        }

        std::cerr << "[RunPrepared Exception] " << ex.what() << std::endl;
        throw;
    }
}

DataTable SQL::FetchResults(const std::string &query)
{
    DataTable dataTable;

    try
    {
        if (hstmt != SQL_NULL_HSTMT)
        {
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            hstmt = SQL_NULL_HSTMT;
        }

        SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        if (!SQL_SUCCEEDED(ret))
            throw std::runtime_error(ExtractError("SQLAllocHandle", hdbc, SQL_HANDLE_DBC));

        SQLRETURN retcode = SQLExecDirect(hstmt, (SQLCHAR *)query.c_str(), SQL_NTS);

        if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
            throw std::runtime_error(ExtractError("SQLExecDirect", hstmt, SQL_HANDLE_STMT));

        SQLSMALLINT columnCount;
        SQLNumResultCols(hstmt, &columnCount);

        std::vector<std::string> columnNames(columnCount);
        std::vector<SQLSMALLINT> nativeTypes(columnCount);

        SQLCHAR columnName[256];
        SQLSMALLINT columnNameLength, dataType, decimalDigits, nullable;
        SQLULEN columnSize;

        for (SQLUSMALLINT i = 1; i <= columnCount; ++i)
        {
            SQLDescribeCol(hstmt, i, columnName, sizeof(columnName), &columnNameLength,
                           &nativeTypes[i - 1], &columnSize, &decimalDigits, &nullable);

            columnNames[i - 1] = std::string(reinterpret_cast<char *>(columnName), columnNameLength);
        }

        std::vector<DataTable::Row> rows;

        while (SQLFetch(hstmt) == SQL_SUCCESS)
        {
            DataTable::Row row;
            row.SetColumns(columnNames);

            for (SQLUSMALLINT i = 1; i <= columnCount; ++i)
            {
                SQLLEN indicator = 0;
                SQLSMALLINT nativeType = nativeTypes[i - 1];

                if (nativeType == SQL_VARBINARY || nativeType == SQL_BINARY)
                {
                    std::vector<uint8_t> buffer(512);
                    SQLRETURN ret = SQLGetData(hstmt, i, SQL_C_BINARY, buffer.data(), (SQLLEN)buffer.size(), &indicator);

                    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
                    {
                        if (indicator == SQL_NULL_DATA)
                        {
                            row.Set(columnNames[i - 1], std::nullopt);
                        }
                        else
                        {
                            size_t size = (indicator > 0 && indicator < static_cast<SQLLEN>(buffer.size())) ? static_cast<size_t>(indicator) : buffer.size();
                            buffer.resize(size);
                            row.Set(columnNames[i - 1], buffer);
                        }
                    }
                    else
                    {
                        std::cerr << "[FetchResults] Error binario en columna " << i << std::endl;
                        row.Set(columnNames[i - 1], std::nullopt);
                    }
                }
                else if (nativeType == SQL_INTEGER || nativeType == SQL_SMALLINT || nativeType == SQL_TINYINT)
                {
                    int val = 0;
                    SQLRETURN ret = SQLGetData(hstmt, i, SQL_C_SLONG, &val, 0, &indicator);

                    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
                    {
                        if (indicator == SQL_NULL_DATA)
                            row.Set(columnNames[i - 1], std::nullopt);
                        else
                            row.Set(columnNames[i - 1], val);
                    }
                    else
                    {
                        std::cerr << "[FetchResults] Error int en columna " << i << std::endl;
                        row.Set(columnNames[i - 1], std::nullopt);
                    }
                }
                else
                {
                    char buffer[512];
                    SQLRETURN ret = SQLGetData(hstmt, i, SQL_C_CHAR, buffer, sizeof(buffer), &indicator);

                    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
                    {
                        if (indicator == SQL_NULL_DATA)
                            row.Set(columnNames[i - 1], std::nullopt);
                        else
                            row.Set(columnNames[i - 1], std::string(buffer));
                    }
                    else
                    {
                        std::cerr << "[FetchResults] Error string en columna " << i << std::endl;
                        row.Set(columnNames[i - 1], std::nullopt);
                    }
                }
            }

            rows.push_back(std::move(row));
        }

        dataTable.Fill(rows);

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        hstmt = SQL_NULL_HSTMT;

        return dataTable;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "[FetchResults] Excepción: " << ex.what() << std::endl;
        throw;
    }
}

DataTable SQL::FetchPrepared(const std::string &query, const std::vector<std::string> &params)
{
    DataTable dataTable;

    try
    {
        PrepareStatement(query);

        for (size_t i = 0; i < params.size(); ++i)
        {
            SQLRETURN retcode = SQLBindParameter(
                hstmt,
                static_cast<SQLUSMALLINT>(i + 1),
                SQL_PARAM_INPUT,
                SQL_C_CHAR,
                SQL_VARCHAR,
                0, 0,
                (SQLPOINTER)params[i].c_str(),
                0,
                nullptr);

            if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
                throw std::runtime_error("Error binding parameter " + std::to_string(i + 1));
        }

        SQLRETURN retcode = SQLExecute(hstmt);

        if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
            throw std::runtime_error(ExtractError("SQLExecute", hstmt, SQL_HANDLE_STMT));

        SQLSMALLINT columnCount;
        SQLNumResultCols(hstmt, &columnCount);

        std::vector<std::string> columnNames(columnCount);
        std::vector<SQLSMALLINT> nativeTypes(columnCount);

        SQLCHAR columnName[256];
        SQLSMALLINT columnNameLength, decimalDigits, nullable;
        SQLULEN columnSize;

        for (SQLUSMALLINT i = 1; i <= columnCount; ++i)
        {
            SQLDescribeCol(hstmt, i, columnName, sizeof(columnName), &columnNameLength,
                           &nativeTypes[i - 1], &columnSize, &decimalDigits, &nullable);
            columnNames[i - 1] = std::string(reinterpret_cast<char *>(columnName), columnNameLength);
        }

        std::vector<DataTable::Row> rows;

        while (SQLFetch(hstmt) == SQL_SUCCESS)
        {
            DataTable::Row row;
            row.SetColumns(columnNames);

            for (SQLUSMALLINT i = 1; i <= columnCount; ++i)
            {
                SQLLEN indicator = 0;

                SQLSMALLINT nativeType = nativeTypes[i - 1];

                if (nativeType == SQL_VARBINARY || nativeType == SQL_BINARY)
                {
                    // Leer datos binarios
                    std::vector<uint8_t> buffer(512);
                    SQLRETURN ret = SQLGetData(hstmt, i, SQL_C_BINARY, buffer.data(), (SQLLEN)buffer.size(), &indicator);

                    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
                    {
                        if (indicator == SQL_NULL_DATA)
                            row.Set(columnNames[i - 1], std::nullopt);
                        else
                        {
                            size_t size = (indicator > 0 && indicator < static_cast<SQLLEN>(buffer.size())) ? static_cast<size_t>(indicator) : buffer.size();
                            buffer.resize(size);
                            row.Set(columnNames[i - 1], buffer);
                        }
                    }
                    else
                    {
                        std::cerr << "[FetchPrepared] Error binario en columna " << i << std::endl;
                        row.Set(columnNames[i - 1], std::nullopt);
                    }
                }
                else if (nativeType == SQL_INTEGER || nativeType == SQL_SMALLINT || nativeType == SQL_TINYINT)
                {
                    int valInt = 0;
                    SQLRETURN ret = SQLGetData(hstmt, i, SQL_C_SLONG, &valInt, 0, &indicator);

                    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
                    {
                        if (indicator == SQL_NULL_DATA)
                            row.Set(columnNames[i - 1], std::nullopt);
                        else
                            row.Set(columnNames[i - 1], valInt);
                    }
                    else
                    {
                        std::cerr << "[FetchPrepared] Error int en columna " << i << std::endl;
                        row.Set(columnNames[i - 1], std::nullopt);
                    }
                }
                else
                {
                    char buffer[512];
                    SQLRETURN ret = SQLGetData(hstmt, i, SQL_C_CHAR, buffer, sizeof(buffer), &indicator);

                    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
                    {
                        if (indicator == SQL_NULL_DATA)
                            row.Set(columnNames[i - 1], std::nullopt);
                        else
                            row.Set(columnNames[i - 1], std::string(buffer));
                    }
                    else
                    {
                        std::cerr << "[FetchPrepared] Error string en columna " << i << std::endl;
                        row.Set(columnNames[i - 1], std::nullopt);
                    }
                }
            }

            rows.push_back(std::move(row));
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        hstmt = nullptr;

        dataTable.Fill(rows);
    }
    catch (const std::exception &ex)
    {
        std::cerr << "[FetchPrepared] Excepción: " << ex.what() << std::endl;
    }

    return dataTable;
}

DataTable SQL::FetchPrepared(const std::string &query, const std::string &param)
{
    return FetchPrepared(query, std::vector<std::string>{param});  
}
