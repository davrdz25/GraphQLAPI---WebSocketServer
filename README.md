This repository contains a modular C++ backend architecture for interacting with Microsoft SQL Server (MSSQL). It uses a clean separation of concerns with Models, Repositories, Services, and Interfaces, and integrates a GraphQL API as the primary interface for data access. The project is written in C++20 and uses CMake for building.

📘 Overview

This project provides the backend foundation for C++ applications that interact with MSSQL databases via a GraphQL interface. It leverages patterns like Repository and Service to ensure modularity, maintainability, and scalability.

🔧 Requirements

C++20 compatible compiler
CMake (version ≥ 3.20 recommended)
ODBC Driver for SQL Server (e.g., ODBC Driver 17 for SQL Server)
Boost (recommended modules: boost::asio, boost::beast, boost::json)
libsodium (for encryption and secure data handling)
cppgraphqlgen (GraphQL code generation and runtime support is required)
⚙️ Building
Go to GraphQL/Schemas and run to generate GraphQl files
schemagen \       
  --schema User.graphql \
  --prefix User \
  --namespace user \
  --header-dir ../Generated/User \
  --source-dir ../Generated/User

Clone the repository:
git clone https://github.com/davrdz25/SQL.git
Build the project:
cd SQL  
mkdir build && cd build  
cmake ..  
make
This will generate the application binary inside the build/ directory.

🚀 Usage

Run the compiled application:
./main

Configure your MSSQL connection in the appropriate source file (e.g., Database/Connection.cpp) using a valid connection string.

GraphQL is the main interface of this system. Queries and mutations should be sent via HTTP POST requests to the configured GraphQL endpoint. The GraphQL schema and resolvers are defined in the GraphQL/ directory.

📚 Contributions

Contributions are welcome! You can help by:

Expanding GraphQL schema support
Adding new entity models and resolvers
Improving MSSQL integration and query optimization
Enhancing security and error handling
Feel free to open an issue or submit a pull request.

📄 License

Please add a license (e.g., MIT, Apache 2.0) in a LICENSE file if needed.

📬 Contact

For suggestions or issues, open an issue on GitHub or contact the repository owner directly
