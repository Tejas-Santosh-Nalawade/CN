#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Function for sending and receiving "Hello"
void sayHello(int client_sock) {
    char buffer[BUFFER_SIZE] = {0};
    char hello[] = "Hello from Server!";

    // Receiving hello message from client
    recv(client_sock, buffer, BUFFER_SIZE, 0);
    printf("Client: %s\n", buffer);

    // Sending hello message to client
    send(client_sock, hello, strlen(hello), 0);
    printf("Hello message sent to client\n");
}

// Function for file transfer
void fileTransfer(int client_sock) {
    char buffer[BUFFER_SIZE] = {0};
    char *filename = "server_received_file.txt";
    FILE *file = fopen(filename, "w");

    if (!file) {
        perror("Error opening file");
        return;
    }

    int bytes_received;
    while ((bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, sizeof(char), bytes_received, file);
        if (bytes_received < BUFFER_SIZE) break;
    }

    fclose(file);
    printf("File received successfully\n");
}

// Function for calculator
void calculator(int client_sock) {
    char expression[BUFFER_SIZE] = {0};
    char result[BUFFER_SIZE] = {0};

    recv(client_sock, expression, BUFFER_SIZE, 0);
    printf("Expression received: %s\n", expression);

    int num1, num2;
    char op;
    int calc_result = 0;

    if (sscanf(expression, "%d%c%d", &num1, &op, &num2) != 3) {
        snprintf(result, BUFFER_SIZE, "Error: invalid expression");
        send(client_sock, result, strlen(result), 0);
        return;
    }

    switch (op) {
        case '+': calc_result = num1 + num2; break;
        case '-': calc_result = num1 - num2; break;
        case '*': calc_result = num1 * num2; break;
        case '/': calc_result = (num2 != 0) ? num1 / num2 : 0; break;
        default: snprintf(result, BUFFER_SIZE, "Error: invalid operation");
                 send(client_sock, result, strlen(result), 0);
                 return;
    }

    snprintf(result, BUFFER_SIZE, "%d", calc_result);
    send(client_sock, result, strlen(result), 0);
    printf("Calculation result sent: %s\n", result);
}

int main() {
    int server_fd, client_sock;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Creating socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Binding
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listening
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        // Accept incoming client
        if ((client_sock = accept(server_fd, (struct sockaddr *)&address,
                                  (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected!\n");

        int option;
        recv(client_sock, &option, sizeof(option), 0);

        switch (option) {
            case 1: sayHello(client_sock); break;
            case 2: fileTransfer(client_sock); break;
            case 3: calculator(client_sock); break;
            default: printf("Invalid option selected.\n"); break;
        }

        close(client_sock);  // Close client after handling
        printf("Client disconnected.\n\n");
    }

    close(server_fd);
    return 0;
}
