import matplotlib.pyplot as plt

class TreeNode:
    def __init__(self, value):
        self.value = value
        self.children = []
        self.color = 'lightblue'  # Default color

    def add_child(self, child_node):
        self.children.append(child_node)

def build_tree(lines):
    root_value = None
    root = None

    for line in lines:
        numbers = list(map(int, line.split()))
        if not root_value:
            root_value = numbers[0]
            root = TreeNode(root_value)

        current_node = root
        for number in numbers[1:]:
            # Check if the child already exists
            child_node = next((child for child in current_node.children if child.value == number), None)
            if not child_node:
                child_node = TreeNode(number)
                current_node.add_child(child_node)
            current_node = child_node
            
    return root

def overwrite_colors_from_valid_paths(node, valid_node):
    """
    Recursively overwrite colors in the original tree node if there's a matching path in the valid paths tree.
    """
    # Set the color to green if the node exists in the valid paths tree
    node.color = 'green'
    
    # Traverse child nodes if they exist in both trees
    for child in node.children:
        valid_child = next((vchild for vchild in valid_node.children if vchild.value == child.value), None)
        if valid_child:
            overwrite_colors_from_valid_paths(child, valid_child)

def plot_tree(node, pos=None, ax=None, level=0, width=2.0):
    if pos is None:
        pos = {node: (0, 0)}

    if ax is None:
        fig, ax = plt.subplots(figsize=(10, 6))
        ax.axis('off')

    x = pos[node][0]
    y = pos[node][1]

    # Draw node with its assigned color
    ax.text(x, y, str(node.value), fontsize=12, ha='center', va='center',
            bbox=dict(boxstyle='round,pad=0.3', edgecolor='black', facecolor=node.color))

    # Set up the positions for children
    num_children = len(node.children)
    if num_children > 0:
        dx = width / num_children
        for i, child in enumerate(node.children):
            child_x = x - width / 2 + dx * (i + 0.5)
            child_y = y - 1  # Move child down
            pos[child] = (child_x, child_y)
            # Draw lines to children
            ax.plot([x, child_x], [y - 0.2, child_y + 0.2], color='black')
            # Recursively plot children
            plot_tree(child, pos, ax, level + 1, dx)

    return ax

def main():
    # Read the original tree file
    original_tree_path = 'tree.txt'  # Change this to your original file path
    with open(original_tree_path, 'r') as file:
        original_lines = file.readlines()
    
    # Build the original tree
    tree_root = build_tree(original_lines)

    # Read the valid paths file and build the valid paths tree
    valid_paths_file_path = 'tree_valid.txt'  # Change this to your valid paths file path
    with open(valid_paths_file_path, 'r') as file:
        valid_lines = file.readlines()
    
    # Build a separate tree from valid paths
    valid_root = build_tree(valid_lines)

    # Overwrite colors in the original tree using the valid paths tree
    overwrite_colors_from_valid_paths(tree_root, valid_root)

    # Plot the tree
    plot_tree(tree_root)
    plt.title('Tree Visualization with Valid Paths Highlighted')
    plt.show()

if __name__ == "__main__":
    main()
