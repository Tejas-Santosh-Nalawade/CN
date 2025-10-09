import socket

def dns_lookup():
    print("1. Resolve Hostname → IP")
    print("2. Resolve IP → Hostname")
    choice = input("Enter choice (1/2): ")

    try:
        if choice == "1":
            hostname = input("Enter hostname (e.g. www.google.com): ")
            ip = socket.gethostbyname(hostname)
            print(f"IP address of {hostname} is {ip}")

        elif choice == "2":
            ip = input("Enter IP address (e.g. 8.8.8.8): ")
            hostname = socket.gethostbyaddr(ip)
            print(f"Hostname for {ip} is {hostname[0]}")

        else:
            print("Invalid choice.")

    except socket.gaierror as e:
        print("Error:", e)

if __name__ == "__main__":
    dns_lookup()
