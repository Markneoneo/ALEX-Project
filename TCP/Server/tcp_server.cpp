#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h> // For inet_addr
#include <csignal>     // For signal handling

#define PI_IP "192.168.88.162"

// Signal handler for graceful termination
void signalHandler( int signum ) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    // Cleanup and close up stuff here
    
    // Terminate program
    exit(signum);  
}

int main() {
    // Register signal SIGINT and signal handler  
    signal(SIGINT, signalHandler);

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    const int PORT = 12345; // Port number where the server will listen

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(PI_IP);
    address.sin_port = htons(PORT);
    
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on " << PI_IP << ":" << PORT << std::endl;
    
    while(true) {
        char buffer[1024] = {0};
        
        std::cout << "Waiting for a connection..." << std::endl;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            continue; // Continue to the next iteration of the loop
        }
        
        std::cout << "Connection accepted" << std::endl;
        
        // This inner loop reads data sent by a connected client
        while(true) {
            ssize_t bytes_read = read(new_socket, buffer, 1024);
            if (bytes_read <= 0) {
                // If read returns 0, the client has closed the connection
                // If read returns -1, an error occurred
                break;
            }
            std::cout << "Message from client: " << buffer << std::endl;
            memset(buffer, 0, sizeof(buffer)); // Clear the buffer
        }
        
        close(new_socket);
        std::cout << "Connection closed" << std::endl;
    }
    
    // The program should never reach here in normal operation
    close(server_fd);
    
    return 0;
}
