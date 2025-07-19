#pragma once
#include "UserPublicModel.hpp"

struct AuthPayloadModel{
    std::string Token;
    UserPublicModel User;
};