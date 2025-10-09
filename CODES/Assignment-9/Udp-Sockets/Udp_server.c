#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9090
#define BUFFER_SIZE 4096

void receiveFile(int sock, struct sockaddr_in *client, socklen_t client_len, char *out_filename) {
    FILE *fp = fopen(out_filename, "wb");
    if (!fp) {
        perror("Error opening file");
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    int bytes_received;

    while (1) {
        bytes_received = recvfrom(sock, buffer, BUFFER_SIZE, 0,
                                  (struct sockaddr *)client, &client_len);

        if (bytes_received < 0) {
            perror("Receive failed");
            exit(1);
        }

        // Check for END marker
        if (strcmp(buffer, "END") == 0) {
            break;
        }

        fwrite(buffer, sizeof(char), bytes_received, fp);
    }

    fclose(fp);
    printf("File [%s] received successfully.\n", out_filename);
} 

int main() {
    int sock;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    printf("UDP Server is listening on port %d\n", PORT);

    // Receive 4 files (script, text, audio, video)
    receiveFile(sock, &client, client_len, "received_script.sh");
    receiveFile(sock, &client, client_len, "received_text.txt");
    receiveFile(sock, &client, client_len, "received_audio.mp3");
    receiveFile(sock, &client, client_len, "received_video.mp4");

    close(sock);
    return 0;
}
