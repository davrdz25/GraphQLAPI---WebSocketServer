#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>
#include <boost/json.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <functional>

#include <graphqlservice/GraphQLService.h>
#include <graphqlservice/JSONResponse.h>

#include <graphqlservice/internal/Grammar.h>
#include <graphqlservice/internal/SyntaxTree.h>

#include "GraphQL/Resolvers/UserResolvers/UserQueryResolver.hpp"
#include "GraphQL/Resolvers/UserResolvers/UserMutationResolver.hpp"

#include "GraphQL/Resolvers/RootQueryResolver.hpp"
#include "GraphQL/Resolvers/RootMutationResolver.hpp"

#include "GraphQL/Generated/User/MutationObject.h"
#include "GraphQL/Generated/User/QueryObject.h"

#include "Repositories/User/UserRepository.hpp"
#include "Services/User/UserService.hpp"

#include "Utils/Base64.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace asio = boost::asio;
namespace net = boost::asio;
namespace json = boost::json;
namespace ssl = boost::asio::ssl;

using tcp = boost::asio::ip::tcp;

using namespace graphql;

boost::json::object astNodeToJson(const graphql::peg::ast_node *node)
{
    boost::json::object jsonNode;

    if (!node)
        return jsonNode;

    jsonNode["rule"] = node->type;

    if (node->has_content())
    {
        std::string content = std::string(node->string_view());
        if (!content.empty())
        {
            jsonNode["content"] = content;
        }
    }

    if (!node->children.empty())
    {
        boost::json::array children;
        for (const auto &child : node->children)
        {
            children.emplace_back(astNodeToJson(child.get()));
        }
        jsonNode["children"] = std::move(children);
    }

    return jsonNode;
}

void printNode(const graphql::peg::ast_node *node, int indent = 0)
{
    if (!node)
        return;

    std::string indentStr(indent * 2, ' ');
    std::cout << indentStr << "Rule: " << node->type << "\n";

    if (node->has_content())
    {
        const auto content = node->string_view();
        if (!content.empty())
        {
            std::cout << indentStr << "Content: " << content << "\n";
        }
    }

    for (const auto &child : node->children)
    {
        printNode(child.get(), indent + 1);
    }
}

http::response<http::string_body> handleGraphQLRequest(const std::string &body, std::shared_ptr<SQL> database)
{
    try
    {
        std::cout << "Request body:\n"
                  << body << "\n";

        boost::json::value jv = json::parse(body);
        boost::json::object obj = jv.as_object();

        std::string operationName;

        if (obj.contains("operationName") && !obj["operationName"].is_null())
        {
            operationName = obj["operationName"].as_string().c_str();
        }

        response::Value variables{response::Type::Map};

        if (obj.contains("variables") && !obj["variables"].is_null())
        {
            std::function<response::Value(const json::value &)> convertJsonToResponseValue;
            convertJsonToResponseValue = [&](const json::value &v) -> response::Value
            {
                switch (v.kind())
                {
                case json::kind::object:
                {
                    response::Value map{response::Type::Map};

                    for (auto &[key, val] : v.as_object())
                    {
                        map.emplace_back(std::string(key), convertJsonToResponseValue(val));
                    }
                    return map;
                }
                case json::kind::array:
                {
                    response::Value arr{response::Type::List};
                    for (auto &elem : v.as_array())
                    {
                        arr.emplace_back(convertJsonToResponseValue(elem));
                    }
                    return arr;
                }
                case json::kind::string:
                    return response::Value{std::string(v.as_string().c_str())};
                case json::kind::int64:
                    return response::Value{static_cast<response::IntType>(v.as_int64())};
                case json::kind::uint64:
                    return response::Value{static_cast<response::IntType>(v.as_uint64())};
                case json::kind::double_:
                    return response::Value{v.as_double()};
                case json::kind::bool_:
                    return response::Value{v.as_bool()};
                case json::kind::null:
                default:
                    return response::Value{};
                }
            };

            variables = convertJsonToResponseValue(obj["variables"]);
        }

        std::shared_ptr<graphql::service::Request> request;

        std::shared_ptr<UserRepository> userRepository = std::make_shared<UserRepository>(database);
        std::shared_ptr<UserService> userService = std::make_shared<UserService>(userRepository);

        std::shared_ptr<UserQueryResolver> userQ = std::make_shared<UserQueryResolver>(userService);
        std::shared_ptr<UserMutationResolver> userM = std::make_shared<UserMutationResolver>(userService);

        std::shared_ptr<graphql::resolvers::RootQueryResolver> rootQ = std::make_shared<graphql::resolvers::RootQueryResolver>(userQ);
        std::shared_ptr<graphql::resolvers::RootMutationResolver> rootM = std::make_shared<graphql::resolvers::RootMutationResolver>(userM);

        auto qObj = std::shared_ptr<graphql::user::object::Query>(new graphql::user::object::Query(rootQ));
        auto mObj = std::shared_ptr<graphql::user::object::Mutation>(new graphql::user::object::Mutation(rootM));

        request = std::make_shared<graphql::user::Operations>(
            qObj,
            mObj);

        std::string queryStr = obj["query"].as_string().c_str();

        std::cerr << "Ejecutando Query:\n"
                  << queryStr << "\n";

        graphql::peg::ast ast = graphql::peg::parseString(queryStr);

        if (!ast.root)
        {
            throw std::runtime_error("Error al parsear la consulta GraphQL.");
        }

        std::cout << "AST parseado correctamente\n";

        boost::json::object jsonAst = astNodeToJson(ast.root.get());
        std::cout << "AST (JSON):\n"
                  << boost::json::serialize(jsonAst) << "\n";

        service::RequestResolveParams params{
            ast,
            std::string_view{operationName},
            std::move(variables),
            graphql::service::await_async{}};

        std::map<std::string, graphql::response::Value> clientParams;

        auto futureResult = request->resolve(params);
        auto result = futureResult.get();

        std::string responseBody = graphql::response::toJSON(std::move(result));

        http::response<http::string_body> res{http::status::ok, 11};
        res.set(http::field::content_type, "application/json");
        res.body() = responseBody;
        res.prepare_payload();

        return res;
    }
    catch (graphql::service::schema_exception& e)
    {
        http::response<http::string_body> res{http::status::ok, 11};
        res.set(http::field::content_type, "application/json");

        std::string errorJson = graphql::response::toJSON(e.getErrors());
        res.body() = errorJson;
        res.prepare_payload();
        return res;
    }

    /* catch (const std::exception &e)
    {
        http::response<http::string_body> res{http::status::bad_request, 11};
        res.set(http::field::content_type, "application/json");

        std::string errorJson = R"({"errors": [{"message": ")" + std::string(e.what()) + R"("}]})";
        res.body() = errorJson;
        res.prepare_payload();
        return res;
    } */
}

void start_http_server(net::io_context &ioc, unsigned short port, std::shared_ptr<SQL> database)
{
    auto acceptor = std::make_shared<tcp::acceptor>(ioc, tcp::endpoint{tcp::v4(), port});

    auto do_accept = std::make_shared<std::function<void()>>();

    *do_accept = [acceptor, &ioc, database, do_accept]()
    {
        auto socket = std::make_shared<tcp::socket>(ioc);
        acceptor->async_accept(*socket, [socket, database, &ioc, do_accept](boost::system::error_code ec)
                               {
            if (!ec)
            {
                net::post(ioc, [socket, database]() {
                    try
                    {
                        beast::flat_buffer buffer;
                        http::request<http::string_body> req;
                        http::read(*socket, buffer, req);

                        auto remote_ip = socket->remote_endpoint().address().to_string();
                        std::cout << "\nNueva conexión desde: " << remote_ip << "\n";
                        std::cout << "Método: " << req.method_string() << "\n";
                        std::cout << "Ruta: " << req.target() << "\n";
                        std::cout << "Headers:" << std::endl;

                        for (const auto& field : req)
                        {
                            std::cout << "   " << field.name_string() << ": " << field.value() << "\n";
                        }
                        std::cout << "Body:\n" << req.body() << "\n";

                        if (req[http::field::content_type] != "application/json")
                        {
                            http::response<http::string_body> res{http::status::unsupported_media_type, req.version()};
                            res.set(http::field::content_type, "text/plain");
                            res.body() = "Unsupported Content-Type. Only application/json is accepted.";
                            res.prepare_payload();
                            http::write(*socket, res);
                            socket->shutdown(tcp::socket::shutdown_send);
                            return;
                        }

                        auto res = handleGraphQLRequest(req.body(), database);
                        http::write(*socket, res);
                        socket->shutdown(tcp::socket::shutdown_send);
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << "[HTTP Error] " << e.what() << "\n";
                    }
                });
            }
            (*do_accept)(); });
    };

    (*do_accept)();
}

void start_websocket_server(net::io_context &ioc, unsigned short port)
{
    auto acceptor = std::make_shared<tcp::acceptor>(ioc, tcp::endpoint{tcp::v4(), port});

    auto do_accept = std::make_shared<std::function<void()>>();

    *do_accept = [acceptor, &ioc, do_accept]()
    {
        auto socket = std::make_shared<tcp::socket>(ioc);
        acceptor->async_accept(*socket, [socket, &ioc, do_accept](boost::system::error_code ec)
                               {
            if (!ec)
            {
                net::post(ioc, [socket]() {
                    try
                    {
                        std::cout << "Web socket server iniciado" << std::endl;
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << "[WebSocket Session Error] " << e.what() << '\n';
                    }
                });
            }

            (*do_accept)(); });
    };

    (*do_accept)();
}

int main()
{
    Base64::SetSecret("_.:Development:._");
    JWT::SetJWTSecret("_.::JWT_Development:._");

    net::io_context ioc;

    auto database = std::make_shared<SQL>();

    database->ServerName("192.168.1.253");
    database->UserName("sa");
    database->Password("Development..");
    database->DatabaseName("POS");
    database->TrustServerCertificate(true);

    if (!database->Connect())
    {
        std::cerr << "[ERROR] No se pudo conectar a la base de datos.\n";
        return 1;
    }

    start_http_server(ioc, 8081, database);
    start_websocket_server(ioc, 8082);

    std::vector<std::thread> threads;
    const int thread_count = std::thread::hardware_concurrency();

    for (int i = 0; i < thread_count; ++i)
    {
        threads.emplace_back([&ioc]()
                             { ioc.run(); });
    }

    for (auto &t : threads)
    {
        t.join();
    }

    return 0;
}
