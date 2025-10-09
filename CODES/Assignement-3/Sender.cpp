// sender.cpp
// Usage: ./sender <peer_ip> <peer_port> <mode> <7|8> 
// mode: H for Hamming, C for CRC
// Example: ./sender 192.168.1.10 5005 H 7
// Example: ./sender 127.0.0.1 5005 C 8

#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

// ---------------- CRC-16-CCITT ----------------
uint16_t crc16_ccitt(const vector<uint8_t>& data) {
    uint16_t crc = 0xFFFF;
    for (uint8_t b : data) {
        crc ^= (uint16_t)b << 8;
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
            else crc <<= 1;
        }
    }
    return crc & 0xFFFF;
}

// ---------------- Hamming encoder for 7 or 8 bits ----------------
// Place parity bits at positions that are powers of two (1-based).
// For m data bits and r parity bits where 2^r >= m + r + 1

vector<int> int_to_bits(uint8_t val, int bits) {
    vector<int> out(bits);
    for (int i = 0; i < bits; ++i) out[i] = (val >> i) & 1;
    return out;
}

uint8_t bits_to_byte(const vector<int>& bits, int start) {
    uint8_t b = 0;
    for (int i = 0; i < 8 && start + i < (int)bits.size(); ++i)
        b |= (bits[start + i] & 1) << i;
    return b;
}

vector<int> encode_hamming_bits(const vector<int>& data_bits) {
    int m = data_bits.size();
    int r = 1;
    while ((1 << r) < (m + r + 1)) ++r;
    int n = m + r;
    vector<int> out(n+1, 0); // 1-based indexing for simplicity
    // fill data bits into non-power positions
    int data_idx = 0;
    for (int i = 1; i <= n; ++i) {
        if ((i & (i-1)) != 0) { // not power of two
            out[i] = data_bits[data_idx++];
        }
    }
    // compute parity bits
    for (int i = 0; i < r; ++i) {
        int ppos = 1 << i;
        int parity = 0;
        for (int j = 1; j <= n; ++j) {
            if (j & ppos) parity ^= out[j];
        }
        out[ppos] = parity;
    }
    // return 0-based vector
    vector<int> res(n);
    for (int i = 0; i < n; ++i) res[i] = out[i+1];
    return res;
}

// Pack a vector<int> bits into bytes (LSB-first in each byte)
vector<uint8_t> pack_bits_to_bytes(const vector<int>& bits) {
    int n = bits.size();
    int bytes = (n + 7) / 8;
    vector<uint8_t> out(bytes, 0);
    for (int i = 0; i < n; ++i) {
        if (bits[i]) out[i/8] |= (1 << (i%8));
    }
    return out;
}

int main(int argc, char** argv) {
    if (argc < 5) {
        cerr << "Usage: " << argv[0] << " <peer_ip> <peer_port> <mode H|C> <7|8>\n";
        return 1;
    }
    string peer_ip = argv[1];
    int peer_port = atoi(argv[2]);
    char mode = argv[3][0];
    int ascii_bits = atoi(argv[4]);
    if (!(mode=='H' || mode=='C')) { cerr<<"mode H or C\n"; return 1; }
    if (!(ascii_bits==7 || ascii_bits==8)) { cerr<<"7 or 8 bits\n"; return 1; }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    sockaddr_in peer{};
    peer.sin_family = AF_INET;
    peer.sin_port = htons(peer_port);
    inet_pton(AF_INET, peer_ip.c_str(), &peer.sin_addr);

    cout << "Enter a line to send (empty line to quit):\n";
    string line;
    while (true) {
        getline(cin, line);
        if (line.size() == 0) break;

        // Build packet: header [mode 'H'/'C', ascii_bits (1 byte), payload length (2 bytes)] then payload bytes
        vector<uint8_t> payload;

        for (unsigned char ch : line) {
            uint8_t ch8 = (uint8_t)ch;
            vector<int> data_bits;
            for (int i = 0; i < ascii_bits; ++i) data_bits.push_back((ch8 >> i) & 1);
            if (mode == 'H') {
                auto encoded_bits = encode_hamming_bits(data_bits); // e.g., 11 or 12 bits
                auto packed = pack_bits_to_bytes(encoded_bits);
                // Prepend a single byte indicating how many encoded bits (for decoding)
                payload.push_back((uint8_t)encoded_bits.size());
                payload.insert(payload.end(), packed.begin(), packed.end());
            } else { // CRC
                // send original byte, then append CRC per-character (or we can append one CRC for whole message)
                // Here we append per-character CRC for clarity in packet trace.
                vector<uint8_t> v{ch8};
                uint16_t crc = crc16_ccitt(v);
                payload.push_back(ch8);
                payload.push_back((crc >> 8) & 0xFF);
                payload.push_back(crc & 0xFF);
            }
        }

        // Packet header
        vector<uint8_t> pkt;
        pkt.push_back((uint8_t)mode);
        pkt.push_back((uint8_t)ascii_bits);
        uint16_t payload_len = payload.size();
        pkt.push_back((payload_len >> 8) & 0xFF);
        pkt.push_back(payload_len & 0xFF);
        pkt.insert(pkt.end(), payload.begin(), payload.end());

        // send
        ssize_t sent = sendto(sock, pkt.data(), pkt.size(), 0, (sockaddr*)&peer, sizeof(peer));
        if (sent < 0) perror("sendto");
        else cout << "Sent " << sent << " bytes\n";
    }

    close(sock);
    cout << "Sender exiting.\n";
    return 0;
}
