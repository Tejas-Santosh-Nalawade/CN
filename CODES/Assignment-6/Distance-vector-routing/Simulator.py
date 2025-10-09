import heapq
import sys

# A representation of infinity, used for nodes not yet reached.
INFINITY = float('inf')

class RoutingSimulator:
    def __init__(self, graph):
        """
        Initializes the simulator with a network graph.
        The graph should be a dictionary where keys are node names,
        and values are another dictionary of neighbors with their costs.
        Example: {'A': {'B': 1, 'C': 4}, 'B': {'A': 1, 'C': 2, 'D': 5}}
        """
        self.graph = graph
        self.nodes = list(graph.keys())

    def link_state_dijkstra(self, start_node, end_node):
        """
        Implements the Link-State routing protocol using Dijkstra's algorithm
        to find the shortest path from a start node to all other nodes.
        """
        if start_node not in self.nodes or end_node not in self.nodes:
            return None, None

        # Priority queue to store (cost, node, path_list)
        pq = [(0, start_node, [])]
        
        # Dictionary to store the minimum cost to reach each node found so far
        min_costs = {node: INFINITY for node in self.nodes}
        min_costs[start_node] = 0
        
        # A set to keep track of visited nodes to avoid cycles and redundant work
        visited = set()

        while pq:
            # Get the node with the smallest cost from the priority queue
            cost, current_node, path = heapq.heappop(pq)

            if current_node in visited:
                continue

            # Add the current node to the path
            path = path + [current_node]
            visited.add(current_node)

            # If we've reached the destination, we're done
            if current_node == end_node:
                return path, cost

            # Explore neighbors of the current node
            for neighbor, weight in self.graph.get(current_node, {}).items():
                if neighbor not in visited:
                    new_cost = cost + weight
                    # If we found a cheaper path to the neighbor, update it
                    if new_cost < min_costs[neighbor]:
                        min_costs[neighbor] = new_cost
                        heapq.heappush(pq, (new_cost, neighbor, path))
        
        return None, INFINITY # Path not found

    def distance_vector_bellman_ford(self, start_node, end_node):
        """
        Implements the Distance-Vector routing protocol using the Bellman-Ford algorithm.
        This simulates how a router builds its table based on neighbors' information.
        """
        if start_node not in self.nodes or end_node not in self.nodes:
            return None, None
            
        # Step 1: Initialize distances from start_node to all other nodes as INFINITY
        distances = {node: INFINITY for node in self.nodes}
        predecessors = {node: None for node in self.nodes}
        distances[start_node] = 0

        # Step 2: Relax edges repeatedly
        # This loop simulates the sharing of distance vectors between neighbors.
        for _ in range(len(self.nodes) - 1):
            for u in self.nodes:
                for v, weight in self.graph.get(u, {}).items():
                    if distances[u] != INFINITY and distances[u] + weight < distances[v]:
                        distances[v] = distances[u] + weight
                        predecessors[v] = u
        
        # Step 3: Check for negative-weight cycles (not typical in routing but part of Bellman-Ford)
        for u in self.nodes:
            for v, weight in self.graph.get(u, {}).items():
                if distances[u] != INFINITY and distances[u] + weight < distances[v]:
                    print("Graph contains a negative-weight cycle")
                    return None, None

        # Step 4: Reconstruct the path from start to end node
        path = []
        current = end_node
        while current is not None:
            path.insert(0, current)
            current = predecessors[current]
        
        if path[0] == start_node:
            return path, distances[end_node]
        else:
            return None, INFINITY

def print_header(text):
    print("\n" + "=" * 50)
    print(f" {text} ")
    print("=" * 50)

def main():
    """Main function to run the simulator."""
    # Define the network topology (graph)
    # This represents the routers and the cost of the links between them.
    network_graph = {
        'A': {'B': 2, 'D': 8},
        'B': {'A': 2, 'C': 4, 'D': 5},
        'C': {'B': 4, 'D': 1, 'E': 3},
        'D': {'A': 8, 'B': 5, 'C': 1, 'E': 2},
        'E': {'C': 3, 'D': 2}
    }
    
    simulator = RoutingSimulator(network_graph)
    
    print_header("Routing Protocol Simulator")
    print("Available nodes:", ", ".join(simulator.nodes))
    
    while True:
        try:
            start = input("Enter the source node: ").upper()
            end = input("Enter the destination node: ").upper()
            
            if start not in simulator.nodes or end not in simulator.nodes:
                print("Error: One or both nodes are not in the network. Please try again.")
                continue

            protocol_choice = input("Choose protocol (1 for Link-State, 2 for Distance-Vector): ")

            if protocol_choice == '1':
                print_header("Link-State (Dijkstra's Algorithm) Result")
                path, cost = simulator.link_state_dijkstra(start, end)
            elif protocol_choice == '2':
                print_header("Distance-Vector (Bellman-Ford Algorithm) Result")
                path, cost = simulator.distance_vector_bellman_ford(start, end)
            else:
                print("Invalid choice. Please enter 1 or 2.")
                continue

            if path:
                print(f"Shortest Path: {' -> '.join(path)}")
                print(f"Total Cost: {cost}")
            else:
                print(f"No path found from {start} to {end}.")

        except KeyboardInterrupt:
            print("\nSimulator exiting.")
            break
        except Exception as e:
            print(f"An error occurred: {e}")

        if input("\nRun again? (y/n): ").lower() != 'y':
            break

if __name__ == "__main__":
    main()
