#pragma once

#include "../Generated/User/UserPublicObject.h"
#include "../../Models/UserPublicModel.hpp"

class UserPublicResolver {
public:
    explicit UserPublicResolver(const UserPublicModel& model)
        : m_model(model) {}

    graphql::service::AwaitableScalar<int> getEntry(graphql::service::FieldParams&&) const {
        co_return m_model.Entry;
    }

    graphql::service::AwaitableScalar<std::string> getCode(graphql::service::FieldParams&&) const {
        co_return m_model.Code;
    }

    graphql::service::AwaitableScalar<std::string> getFirstName(graphql::service::FieldParams&&) const {
        co_return m_model.FirstName;
    }

    graphql::service::AwaitableScalar<std::optional<std::string>> getMiddleName(graphql::service::FieldParams&&) const {
        co_return m_model.MiddleName;
    }

    graphql::service::AwaitableScalar<std::string> getLastName(graphql::service::FieldParams&&) const {
        co_return m_model.LastName;
    }

    graphql::service::AwaitableScalar<std::optional<std::string>> getSecondLastName(graphql::service::FieldParams&&) const {
        co_return m_model.SecondLastName;
    }

    graphql::service::AwaitableScalar<std::string> getEmail(graphql::service::FieldParams&&) const {
        co_return m_model.Email;
    }

    graphql::service::AwaitableScalar<std::string> getPhone(graphql::service::FieldParams&&) const {
        co_return m_model.Phone;
    }

    void beginSelectionSet(const graphql::service::SelectionSetParams&) const {}
    void endSelectionSet(const graphql::service::SelectionSetParams&) const {}

private:
    UserPublicModel m_model;
};
