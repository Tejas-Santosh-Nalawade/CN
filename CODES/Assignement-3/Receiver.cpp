// receiver.cpp
// Usage: ./receiver <listen_port>
// Example: ./receiver 5005

#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;

// CRC16 (same as sender)
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

vector<int> unpack_bits_from_bytes(const vector<uint8_t>& bytes, int bitcount) {
    vector<int> bits(bitcount);
    for (int i = 0; i < bitcount; ++i) bits[i] = (bytes[i/8] >> (i%8)) & 1;
    return bits;
}

// decode hamming: returns pair(corrected_byte_value, status_string)
pair<int,string> decode_hamming_bits(const vector<int>& encoded_bits) {
    int n = encoded_bits.size();
    // compute r (num parity bits)
    int r = 0;
    while ((1<<r) <= n) ++r;
    // compute syndrome
    int syndrome = 0;
    for (int i = 0; i < r; ++i) {
        int ppos = 1 << i;
        int parity = 0;
        for (int j = 1; j <= n; ++j) {
            if (j & ppos) parity ^= encoded_bits[j-1];
        }
        if (parity) syndrome |= ppos;
    }
    vector<int> corrected = encoded_bits;
    if (syndrome != 0) {
        if (syndrome <= n) {
            // flip bit
            corrected[syndrome-1] ^= 1;
        } else {
            // impossible, but mark
        }
    }
    // extract data bits from non-power positions:
    vector<int> data_bits;
    for (int i = 1; i <= n; ++i) {
        if ((i & (i-1)) != 0) data_bits.push_back(corrected[i-1]);
    }
    // form byte value (LSB-first in data_bits)
    int val = 0;
    for (int i = 0; i < (int)data_bits.size(); ++i) {
        val |= (data_bits[i] & 1) << i;
    }
    string status;
    if (syndrome == 0) status = "OK (no error)";
    else {
        // Check if double-bit error: re-calc parity overall: if parity checks fail, detect
        // For Hamming single-bit correction we treat syndrome!=0 as single-bit error corrected
        status = "Single-bit error detected and corrected at position " + to_string(syndrome);
    }
    return {val, status};
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <listen_port>\n";
        return 1;
    }
    int port = atoi(argv[1]);
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }
    cout << "Listening on UDP port " << port << "...\n";

    while (true) {
        uint8_t buf[65536];
        sockaddr_in src{};
        socklen_t slen = sizeof(src);
        ssize_t len = recvfrom(sock, buf, sizeof(buf), 0, (sockaddr*)&src, &slen);
        if (len <= 0) continue;
        vector<uint8_t> pkt(buf, buf + len);
        if (pkt.size() < 4) continue;
        char mode = (char)pkt[0];
        int ascii_bits = pkt[1];
        int payload_len = (pkt[2] << 8) | pkt[3];
        if ((int)pkt.size() < 4 + payload_len) {
            cout << "Truncated packet\n";
            continue;
        }
        vector<uint8_t> payload(pkt.begin() + 4, pkt.begin() + 4 + payload_len);
        cout << "Received packet from " << inet_ntoa(src.sin_addr) << ":" << ntohs(src.sin_port)
             << " mode=" << mode << " ascii_bits=" << ascii_bits
             << " payload_len=" << payload_len << "\n";

        string reconstructed;
        size_t pos = 0;
        if (mode == 'H') {
            while (pos < payload.size()) {
                int enc_bitcount = payload[pos++];
                int bytes_needed = (enc_bitcount + 7) / 8;
                if (pos + bytes_needed > payload.size()) break;
                vector<uint8_t> enc_bytes(payload.begin()+pos, payload.begin()+pos+bytes_needed);
                pos += bytes_needed;
                auto bits = unpack_bits_from_bytes(enc_bytes, enc_bitcount);
                auto pr = decode_hamming_bits(bits);
                int chval = pr.first & 0xFF;
                reconstructed.push_back((char)chval);
                cout << "Decoded char '" << (isprint(chval)? (char)chval : '?')
                     << "' (" << chval << ") -> " << pr.second << "\n";
            }
        } else if (mode == 'C') {
            while (pos + 3 <= payload.size()) {
                uint8_t ch = payload[pos++];
                uint16_t rx_crc = (payload[pos++] << 8);
                rx_crc |= payload[pos++];
                vector<uint8_t> v{ch};
                uint16_t calc = crc16_ccitt(v);
                if (calc == rx_crc) {
                    reconstructed.push_back((char)ch);
                    cout << "CRC OK for '" << (isprint(ch)?(char)ch:'?') << "' (" << (int)ch << ")\n";
                } else {
                    cout << "CRC FAIL for byte " << (int)ch << " received crc=0x" << hex << rx_crc
                         << " calc=0x" << calc << dec << "\n";
                    reconstructed.push_back('?'); // placeholder
                }
            }
        } else {
            cout << "Unknown mode\n";
        }
        cout << "Reconstructed message: \"" << reconstructed << "\"\n\n";
    }

    close(sock);
    return 0;
}
