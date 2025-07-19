#pragma once

#include "../Generated/User/AuthPayloadObject.h"
#include "../../Models/AuthPayloadModel.hpp"

class AuthPayloadResolver {
public:
    explicit AuthPayloadResolver(const AuthPayloadModel& model)
        : m_model(model) {}

   
    graphql::service::AwaitableScalar<std::string> getToken(graphql::service::FieldParams&&) const {
        co_return m_model.Token;
    };

    graphql::service::AwaitableObject<std::shared_ptr<graphql::user::object::UserPublic>> getUser(graphql::service::FieldParams&&) const {
        auto userResolver = std::make_shared<UserPublicResolver>(m_model.User); 
        co_return std::make_shared<graphql::user::object::UserPublic>(userResolver);
    }

    void beginSelectionSet(const graphql::service::SelectionSetParams&) const {}
    void endSelectionSet(const graphql::service::SelectionSetParams&) const {}

private:
    AuthPayloadModel m_model;
};
