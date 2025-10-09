import ipaddress
import math

def print_header(text):
    """Prints a formatted header to the console."""
    print("\n" + "=" * 40)
    print(f" {text} ")
    print("=" * 40)

def calculate_subnetting(base_cidr, num_subnets):
    """
    Calculates and displays the details for a specified number of subnets
    created from a base IP network.
    """
    try:
        # 1. Parse the initial network from the CIDR string
        base_network = ipaddress.ip_network(base_cidr, strict=False)
        print_header("Initial Network Details")
        print(f"Base Network:      {base_network.network_address}")
        print(f"Original Prefix:   /{base_network.prefixlen}")
        print(f"Original Subnet Mask: {base_network.netmask}")

        # 2. Determine the number of bits to "borrow" to create the subnets
        # We need to find the smallest power of 2 that is >= num_subnets.
        # For N subnets, we need 'n' bits where 2^n >= N.
        bits_to_borrow = math.ceil(math.log2(num_subnets))

        # 3. Calculate the new prefix length for the subnets
        new_prefix = base_network.prefixlen + bits_to_borrow

        if new_prefix > 32:
            print("\nError: Not enough address space to create the desired number of subnets.")
            return

        print(f"\nTo create {num_subnets} subnets, we need to borrow {bits_to_borrow} bits.")
        print(f"New Prefix Length for subnets will be: /{new_prefix}")

        # 4. Generate the new subnets
        # The ipaddress module can automatically generate the subnets for us.
        subnets = list(base_network.subnets(new_prefix=new_prefix))
        
        # Ensure we have enough generated subnets (in case of rounding)
        if len(subnets) < num_subnets:
            subnets = list(base_network.subnets(prefixlen_diff=bits_to_borrow))

        print_header("Calculated Subnet Details")

        # 5. Display the details for each calculated subnet
        for i, subnet in enumerate(subnets):
            if i >= num_subnets:
                break # Stop after displaying the requested number of subnets

            print(f"\n--- Subnet #{i + 1} ---")
            print(f"Network ID:        {subnet.network_address}/{subnet.prefixlen}")
            print(f"Subnet Mask:       {subnet.netmask}")
            
            # Usable hosts are all addresses except network and broadcast
            usable_hosts = list(subnet.hosts())
            if len(usable_hosts) > 0:
                first_usable = usable_hosts[0]
                last_usable = usable_hosts[-1]
            else:
                # For very small subnets like /31 or /32
                first_usable = "N/A"
                last_usable = "N/A"

            print(f"Usable Host Range: {first_usable} - {last_usable}")
            print(f"Broadcast Address: {subnet.broadcast_address}")
            # Total usable hosts is 2^(32-prefix) - 2
            print(f"Usable Hosts:      {len(usable_hosts)}")

    except ValueError as e:
        print(f"\nError: Invalid input. Please check your CIDR notation. ({e})")
    except Exception as e:
        print(f"\nAn unexpected error occurred: {e}")

if __name__ == "__main__":
    print_header("IP Subnetting Calculator")
    
    # Get user input
    base_cidr_input = input("Enter the base network in CIDR notation (e.g., 192.168.1.0/24): ")
    try:
        num_subnets_input = int(input("Enter the number of subnets to create: "))
        if num_subnets_input <= 0:
            print("Error: Number of subnets must be a positive integer.")
        else:
            calculate_subnetting(base_cidr_input, num_subnets_input)
    except ValueError:
        print("Error: Please enter a valid integer for the number of subnets.")
