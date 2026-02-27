# Configuration Parser - Webserv Project

A professional, modular C++98 configuration file parser for the Webserv project (42 curriculum).

## Architecture

The parser follows **SOLID principles** with clear **separation of concerns**:

```
config/
├── ConfigException.hpp    # Exception hierarchy
├── Token.hpp             # Token types and structure
├── LocationConfig.hpp/cpp # Location configuration data
├── ServerConfig.hpp/cpp   # Server configuration data
├── Lexer.hpp/cpp         # Tokenization (lexical analysis)
├── Parser.hpp/cpp        # Syntax analysis (recursive descent)
└── Validator.hpp/cpp     # Semantic validation
```

### Design Pattern: Three-Stage Pipeline

1. **Lexer (Tokenization)**
   - Reads configuration file
   - Removes comments (`#`)
   - Converts text into tokens
   - Output: `vector<Token>`

2. **Parser (Syntax Analysis)**
   - Recursive descent parsing
   - Validates syntax structure
   - Builds configuration objects
   - Output: `vector<ServerConfig>`

3. **Validator (Semantic Analysis)**
   - Validates configuration values
   - Checks business rules
   - Ensures consistency
   - Output: Validated configuration or exception

## Compilation

```bash
make        # Compile the project
make clean  # Remove object files
make fclean # Remove all generated files
make re     # Recompile from scratch
make test   # Compile and run test
```

**Compilation flags:** `-Wall -Wextra -Werror -std=c++98`

## Usage

```cpp
#include "config/Lexer.hpp"
#include "config/Parser.hpp"
#include "config/Validator.hpp"

int main(int ac, char **av)
{
    if (ac != 2)
        return 1;

    try {
        Lexer lexer(av[1]);
        Parser parser(lexer.getTokens());
        parser.parse();

        Validator validator(parser.getServers());
        validator.validate();

        // Use parser.getServers() to access configuration
        std::cout << "Configuration valid!" << std::endl;
    }
    catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
```

## Configuration Syntax

NGINX-like configuration syntax:

```nginx
server {
    listen 8080;
    host 127.0.0.1;
    server_name localhost;

    root /var/www/html;
    index index.html;

    client_max_body_size 10M;

    error_page 404 /404.html;

    location /upload {
        methods GET POST;
        upload_store /var/www/uploads;
        autoindex on;
    }

    location /cgi {
        methods GET POST;
        cgi_extension .php;
        root /var/www/cgi;
    }

    location /redirect {
        return 301 http://example.com;
    }
}
```

### Supported Directives

#### Server Block
- `listen <port>` - Port number (1-65535)
- `host <address>` - Host address
- `server_name <name>` - Server name
- `root <path>` - Root directory
- `index <file>` - Default index file
- `client_max_body_size <size>` - Max body size (supports K, M, G suffixes)
- `error_page <code> <path>` - Custom error pages
- `location <path> { }` - Location block

#### Location Block
- `methods <method...>` - Allowed HTTP methods (GET, POST, DELETE)
- `root <path>` - Location root directory
- `index <file>` - Location index file
- `autoindex on|off` - Directory listing
- `upload_store <path>` - Upload directory (requires POST)
- `cgi_extension <ext>` - CGI file extension (must start with `.`)
- `return <code> <url>` - HTTP redirect (300-399)

### Body Size Format

Supports multiple formats:
- `1000` - Bytes
- `10K` - Kilobytes
- `5M` - Megabytes
- `1G` - Gigabytes

## Validation Rules

### Server Validation
- Port must be between 1 and 65535
- Root directory cannot be empty
- Host cannot be empty
- No duplicate ports across servers
- `client_max_body_size` must be > 0

### Location Validation
- Only GET, POST, DELETE methods allowed
- Redirect code must be 300-399
- CGI extension must start with `.`
- `upload_store` requires POST method
- Location path cannot be empty

## Error Handling

All errors throw exceptions with descriptive messages:

```cpp
try {
    // Parser code
}
catch (LexerException& e) {
    // Tokenization errors
}
catch (ParserException& e) {
    // Syntax errors
}
catch (ValidatorException& e) {
    // Validation errors
}
catch (ConfigException& e) {
    // Generic configuration errors
}
```

## Features

✅ **C++98 compliant** - No C++11 features, no Boost  
✅ **No memory leaks** - Proper resource management  
✅ **No external libraries** - Uses only STL  
✅ **Clean architecture** - Modular, testable, extensible  
✅ **Exception-based errors** - Clear error messages with line/column  
✅ **Multiple server blocks** - Support for multiple servers  
✅ **Multiple locations** - Support for location blocks  
✅ **Comment support** - Lines starting with `#`  
✅ **Defensive programming** - Input validation at every stage  

## Example Output

```
Parsing configuration file: test.conf
==========================================

✓ Lexical analysis complete
✓ Parsing complete
✓ Validation complete

Found 2 server block(s)

=== Server Configuration ===
Port: 8080
Host: 127.0.0.1
Server Name: localhost
Root: /var/www/html
Index: index.html
Client Max Body Size: 10485760 bytes

Error Pages:
  404 -> /404.html

Locations:
  Location: /upload
    Methods: GET, POST
    Upload Store: /var/www/uploads
    Autoindex: on
  Location: /cgi
    Methods: GET, POST
    Root: /var/www/cgi
    CGI Extension: .php
  Location: /redirect
    Redirect: 301 -> http://example.com

✓ Configuration is valid!
```

## Testing

Run the included test configuration:

```bash
make test
# or
./config_parser test.conf
```

## Code Quality

- ✅ Compiles with `-Wall -Wextra -Werror`
- ✅ No global variables
- ✅ No static singletons
- ✅ Const-correctness
- ✅ Include guards
- ✅ Clean formatting
- ✅ Comprehensive error messages

## Integration with Webserv

Access parsed configuration:

```cpp
Parser parser(lexer.getTokens());
parser.parse();

const std::vector<ServerConfig>& servers = parser.getServers();

for (size_t i = 0; i < servers.size(); ++i) {
    const ServerConfig& server = servers[i];
    
    int port = server.getPort();
    std::string host = server.getHost();
    std::string root = server.getRoot();
    
    const std::vector<LocationConfig>& locations = server.getLocations();
    // ... use configuration to set up server
}
```

## License

This project is part of the 42 curriculum.

---

**Author:** Configuration Parser System  
**Standard:** C++98  
**Project:** Webserv (42)
