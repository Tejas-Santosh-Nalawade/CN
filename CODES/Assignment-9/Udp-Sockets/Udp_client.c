#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9090
#define BUFFER_SIZE 4096

void sendFile(int sock, struct sockaddr_in *server, char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Error opening file");
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0) {
        if (sendto(sock, buffer, bytes_read, 0, (struct sockaddr *)server, sizeof(*server)) < 0) {
            perror("Send failed");
            exit(1);
        }
    }

    // Send END marker
    strcpy(buffer, "END");
    sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)server, sizeof(*server));

    fclose(fp);
    printf("File [%s] sent successfully.\n", filename);
}

int main() {
    int sock;
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");  // Change to server IP if on another machine

    // Send 4 files: script, text, audio, video
    sendFile(sock, &server, "script.sh");
    sendFile(sock, &server, "text.txt");
    sendFile(sock, &server, "audio.mp3");
    sendFile(sock, &server, "video.mp4");

    close(sock);
    return 0;
}
