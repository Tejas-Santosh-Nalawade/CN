#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void sayHello(int sock) {
    char buffer[BUFFER_SIZE] = {0};
    char hello[] = "Hello from Client!";

    send(sock, hello, strlen(hello), 0);
    printf("Hello message sent\n");

    recv(sock, buffer, BUFFER_SIZE, 0);
    printf("Server: %s\n", buffer);
}

void fileTransfer(int sock) {
    char buffer[BUFFER_SIZE] = {0};
    char *filename = "client_file.txt";
    FILE *file = fopen(filename, "r");

    if (!file) {
        perror("Error opening file");
        return;
    }

    int bytes_read;
    while ((bytes_read = fread(buffer, sizeof(char), BUFFER_SIZE, file)) > 0) {
        send(sock, buffer, bytes_read, 0);
    }

    fclose(file);
    printf("File sent successfully\n");
}

void calculator(int sock) {
    char expression[BUFFER_SIZE] = {0};
    char result[BUFFER_SIZE] = {0};

    printf("Enter a mathematical expression (e.g., 5+3): ");
    fgets(expression, BUFFER_SIZE, stdin);
    expression[strcspn(expression, "\n")] = 0;

    send(sock, expression, strlen(expression), 0);

    recv(sock, result, BUFFER_SIZE, 0);
    printf("Result from server: %s\n", result);
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/Address not supported\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return -1;
    }

    int option;
    printf("Choose an option:\n1. Say Hello\n2. File Transfer\n3. Calculator\nEnter: ");
    scanf("%d", &option);
    getchar();

    send(sock, &option, sizeof(option), 0);

    switch (option) {
        case 1: sayHello(sock); break;
        case 2: fileTransfer(sock); break;
        case 3: calculator(sock); break;
        default: printf("Invalid option selected.\n"); break;
    }

    close(sock);
    return 0;
}
