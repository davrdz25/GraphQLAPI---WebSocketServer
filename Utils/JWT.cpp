#include "JWT.hpp"
#include "Base64.hpp"
#include <sodium.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <boost/json.hpp>

bool JWT::ValidateToken(const std::string &token, JWT::Payload &outPayload)
{
    size_t pos1 = token.find('.');
    size_t pos2 = token.rfind('.');
    if (pos1 == std::string::npos || pos2 == std::string::npos || pos1 == pos2)
        return false;

    std::string header_b64 = token.substr(0, pos1);
    std::string payload_b64 = token.substr(pos1 + 1, pos2 - pos1 - 1);
    std::string signature_b64 = token.substr(pos2 + 1);

    std::string header_json = Base64::DecodeUrl(header_b64);
    std::string payload_json = Base64::DecodeUrl(payload_b64);
    std::string signature = Base64::DecodeUrl(signature_b64);

    std::string signing_input = header_b64 + "." + payload_b64;

    unsigned char mac[crypto_auth_BYTES];
    crypto_auth_hmacsha256_state state;
    crypto_auth_hmacsha256_init(&state, (const unsigned char *)_secret.data(), _secret.size());
    crypto_auth_hmacsha256_update(&state, (const unsigned char *)signing_input.data(), signing_input.size());
    crypto_auth_hmacsha256_final(&state, mac);

    if (signature.size() != crypto_auth_BYTES ||
        sodium_memcmp(signature.data(), mac, crypto_auth_BYTES) != 0)
    {
        std::cerr << "Firma JWT inválida\n";
        return false;
    }

    boost::json::value jv = boost::json::parse(payload_json);
    auto obj = jv.as_object();

    if (!obj.contains("uuid") || !obj.contains("apikey") || !obj.contains("validUntil"))
        return false;

    outPayload.uuid = obj["uuid"].as_string().c_str();
    outPayload.apikey = obj["apikey"].as_string().c_str();
    outPayload.validUntil = obj["validUntil"].as_int64();

    uint64_t now = std::time(nullptr);
    if (outPayload.validUntil < now)
    {
        std::cerr << "Token expirado\n";
        return false;
    }

    return true;
}

std::string JWT::GenerateToken(const std::string& uuid, /* const std::string& apikey,  */const int &expiresInSeconds)
{
    boost::json::object header;
    header["alg"] = "HS256";
    header["typ"] = "JWT";
    std::string header_json = boost::json::serialize(header);
    std::string header_b64 = Base64::EncodeUrl(header_json);

    boost::json::object payload;
    payload["uuid"] = uuid;
    //payload["apikey"] = apikey; Support API Key
    payload["validUntil"] = static_cast<int64_t>(std::time(nullptr) + expiresInSeconds);
    std::string payload_json = boost::json::serialize(payload);
    std::string payload_b64 = Base64::EncodeUrl(payload_json);

    std::string signing_input = header_b64 + "." + payload_b64;

    unsigned char mac[crypto_auth_BYTES];
    crypto_auth_hmacsha256_state state;
    crypto_auth_hmacsha256_init(&state, reinterpret_cast<const unsigned char*>(_secret.data()), _secret.size());
    crypto_auth_hmacsha256_update(&state, reinterpret_cast<const unsigned char*>(signing_input.data()), signing_input.size());
    crypto_auth_hmacsha256_final(&state, mac);

    std::string signature(reinterpret_cast<const char*>(mac), crypto_auth_BYTES);
    std::string signature_b64 = Base64::EncodeUrl(signature);

    return signing_input + "." + signature_b64;
};

void JWT::SetJWTSecret(const std::string &secret)
{
    JWT::_secret = secret;
};
