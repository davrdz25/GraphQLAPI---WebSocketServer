#pragma once

#include <string>
#include <map>

class JWT
{
public:
    struct Payload
    {
        std::string uuid;
        std::string apikey;
        uint64_t validUntil;
    };


    static void SetJWTSecret(const std::string &);
    static bool ValidateToken(const std::string &, Payload &);
    static std::string GenerateToken(const std::string&, /* const std::string&, */ const int &);

private:
    static inline std::string _secret{};
};
