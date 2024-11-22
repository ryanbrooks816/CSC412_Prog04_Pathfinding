import matplotlib.pyplot as plt
import networkx as nx
import numpy as np

"""
Reads the nodes from a file and returns a list of tuples.
Example format:
0 2 4
1 18 11
2 18 4

Args:
    filename: str, the name of the file to read from.

Returns:
    list of tuples: [(node, x, y), ...]
"""
def read_nodes(filename):
    nodes = []
    with open(filename, 'r') as file:
        for line in file:
            parts = line.split(" ")
            node = int(parts[0])
            x = int(parts[1])
            y = int(parts[2])
            nodes.append((node, x, y))
    return nodes

"""
Reads the edges from a file and returns a list of tuples.
Example format:
0 1 2 3
5 6 8
7 6 5 8

Args:
    filename: str, the name of the file to read from.

Returns:
    list of tuples: [(node1, node2), ...]
"""
def read_edges(filename):
    edges = []
    with open(filename, 'r') as file:
        for line in file:
            parts = list(map(int, line.strip().split()))
            node = parts[0]
            connections = parts[1:]
            for conn in connections:
                edges.append((node, conn))
    return edges

"""
Reads the grid from a file and returns a numpy array.
Example format:
3 4
1.0 2.0 3.0 4.0
5.0 6.0 7.0 8.0
9.0 10.0 11.0 12.0

Args:
    filename: str, the name of the file to read from.

Returns:
    numpy array: 2D numpy array of the grid.
    int: number of rows in the grid.
    int: number of columns in the grid.
"""
def read_grid(filename):
    with open(filename, 'r') as file:
        columns, rows = map(int, file.readline().strip().split())
        grid = []
        for _ in range(rows):
            row = list(map(float, file.readline().strip().split()))
            grid.append(row)
    return np.array(grid), rows, columns

"""
Using the networkx library, visualizes the graph on top of the grid by overlaying the graph nodes and edges on the grid.
The grid cells are represented by rectangles with the cell weight displayed in the center of each cell.
The graph nodes are represented by circles with the node number displayed at the bottom-right of each node.
The graph edges are represented by lines connecting the nodes.

Args:
    nodes: list of tuples, [(node, x, y), ...]
    edges: list of tuples, [(node1, node2), ...]
    grid: numpy array, 2D numpy array of the grid.
    rows: int, number of rows in the grid.
    columns: int, number of columns in the grid.
"""
def visualize_graph_on_grid(nodes, edges, grid, rows, columns):
    G = nx.Graph()
    
    # Add nodes with positions
    for node, x, y in nodes:
        G.add_node(node, pos=(y, x))
        
    # Add edges
    G.add_edges_from(edges)
    
    # Get node positions for visualization
    pos = nx.get_node_attributes(G, 'pos')
    
    # Set up the plot
    _, ax = plt.subplots(figsize=(columns, rows))
    ax.set_xlim(-0.5, columns - 0.5)
    ax.set_ylim(-0.5, rows - 0.5)
    ax.invert_yaxis()  # Invert y-axis for intuitive grid layout
    
    # Draw grid cells and add weight text
    for y in range(rows):
        for x in range(columns):
            ax.text(x, y, f"{int(grid[y, x])}", ha='center', va='center', color='black')
            rect = plt.Rectangle((x - 0.5, y - 0.5), 1, 1, fill=False, color='gray', lw=1)
            ax.add_patch(rect)
        
    # Add axis numbers to know the indices of the grid cells
    for x in range(columns):
        ax.text(x, -0.5, f"{x}", ha='center', va='center', color='black')
    
    for y in range(rows):
        ax.text(-0.5, y, f"{y}", ha='center', va='center', color='black')
    
    # Overlay the graph nodes and edges
    nx.draw(G, pos, ax=ax, with_labels=False, node_size=500, node_color='skyblue', font_size=10, font_weight='bold', edge_color='black')
    
    # Adjust labels to appear offset from the nodes
    label_pos = {node: (coords[0] + 0.15, coords[1] - 0.15) for node, coords in pos.items()}
    nx.draw_networkx_labels(G, label_pos, labels={node: node for node in G.nodes}, font_size=10, font_weight='bold')
    
    plt.title("Graph Overlay on Weighted Grid")
    plt.show()

# File paths
nodes_file = "nodes.txt"
edges_file = "edges.txt"
grid_file = r"../../DataSet2/grid_14x24/grid.txt"

# Load data
nodes = read_nodes(nodes_file)
edges = read_edges(edges_file)
grid, rows, columns = read_grid(grid_file)

# Visualize graph on top of the grid
visualize_graph_on_grid(nodes, edges, grid, rows, columns)
