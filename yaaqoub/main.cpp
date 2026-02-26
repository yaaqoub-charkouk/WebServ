# include <iostream>

# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
// struct sockaddr_in {
//     short sin_family;      // Address family (AF_INET for IPv4)
//     unsigned short sin_port; // Port number (in network byte order)
//     struct in_addr sin_addr; // Internet address (IPv4 address)
//     char sin_zero[8];      // Padding to make the structure the same size as sockaddr
// };


int main(void)
{
    // 1 - create socket
    int server_socket;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        std::cerr << "Failed to create server socket" << std::endl;
        return (1);
    }

    // 2 - create ip struct
    struct sockaddr_in nic;
    memset(&nic, 0, sizeof(nic));

    nic.sin_family = AF_INET;
    nic.sin_port = htons(8080); // why htons ?
    inet_pton(nic.sin_family, "0.0.0.0", &nic.sin_addr);

    // 3 - bind the socket to (ip - port)
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bind(server_socket, reinterpret_cast <struct sockaddr*>(&nic), sizeof(nic));

    // 4 - start listen
    listen(server_socket, 10);

    // 5 - accept a client
    struct sockaddr_in  client;
    memset(&client, 0, sizeof(client));
    
    socklen_t   len = sizeof(client);
    int client_fd;
    client_fd = accept(server_socket, reinterpret_cast<struct sockaddr*>(&client), &len);

    // 6 - receive an http request 
    // recv();

    ssize_t bytes;
    char buffer[4096];
    while ((bytes = read(client_fd, buffer, sizeof(buffer))) > 0);
    std::cout << buffer << std::endl;

    // std::cout << "client : " << client.sin_addr << "sent a request" << std::endl;
   printf("Client IP: %s\n", inet_ntoa(client.sin_addr));
   printf("Client port: %d\n", ntohs(client.sin_port));

    std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "Hello";
    
   send(client_fd, response.c_str(), response.size(), 0);
   
   close(client_fd);
}
