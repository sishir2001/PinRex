import pandas as pd
import argparse
import os

# Initializes a Node object with the given height. 
# Sets the 10 children to None and children_count to 0.
class Node:
    def __init__(self, height):
        self.height = height
        self.children = [None] * 10
        self.children_count = 0

# Sorts and groups pincodes from a CSV file by state.
# Reads the CSV file into a DataFrame. 
# Groups the pincodes by state into lists.
# Sorts the lists of pincodes for each state.
# Writes the sorted grouped pincodes to a new CSV file.
# Returns a DataFrame with the sorted grouped pincodes.
def sort_and_group_pincodes(input_file) : 
    # Read the CSV file
    df = pd.read_csv(input_file)
    # Group the pin codes by state
    grouped = df.groupby('State')['Pin'].apply(list)
    sorted_grouped = grouped.apply(sorted)
    # Print the sorted pin codes grouped by state
    # print(sorted_grouped)
    sorted_grouped.to_csv('temp.csv', index=True)
    return sorted_grouped


"""Constructs a trie tree recursively for the given pincode.
Traverses each digit in the pincode to find or create the corresponding child node
Returns the root node of the trie tree after inserting the pincode
Args:
    root (Node): Root node of the tree.
    pincode (str): Pin code.
Returns:
    Node: Root node of the constructed tree.
"""
def construct_tree_according_pincode(root,pincode):
    current_node = root
    for digit in pincode:
        digitInt = int(digit)
        if current_node.children[digitInt] == None:
            # TODO : create a child and assign
            child_node = Node(current_node.height+1)
            current_node.children[digitInt] = child_node
            current_node.children_count += 1
        current_node = current_node.children[digitInt]
    return root

"""Constructs a trie tree recursively for the given list of pincodes.

Creates a root node, then iterates through the pincode list, constructing the trie
by recursively calling construct_tree_according_pincode() on each pincode.

Args:
    list (list): The list of pincodes to construct the trie from.

Returns:
    Node: The root node of the constructed trie tree.
"""
def construct_tree_according_pincodes_list(list):
    root = Node(0)
    for pincode in list:
        root = construct_tree_according_pincode(root,pincode)
    return root

# Truncates a list of regexes to a single regex based on height.
# If height is 5, combines all regexes in a list.
# Otherwise combines regexes with | separator.
# Returns the truncated regex string.
def truncate_regexes_list(regexes,height):
    if regexes == None or len(regexes) == 0:
        return ""
    if(len(regexes) == 1):
        return regexes[0]
    if height == 5:
        result = "["
        for regex in regexes:
            result += regex
        result += "]"
        return result
    result = "("
    for i, regex in enumerate(regexes):
        if i == len(regexes) - 1:
            result += f"{regex})"
        else:
            result += f"{regex}|"
    return result

"""Generates a regex representing all pincodes under the given trie node.

Recursively traverses the trie, generating regexes for each child node. 
Combines the child regexes using the height to determine truncation.

Args:
    root: The root trie node to generate the regex for.

Returns:
    A regex string representing all pincodes under the given trie node.
"""
def regex_from_pincodes_tree(root):
    if root == None or root.children_count == 0:
        return ""
    child_regexes = [];
    # Traverse the children array and generate the regex
    for i in range(0,10):
        child = root.children[i];
        if child != None:
            child_regex = regex_from_pincodes_tree(child)
            if root.height == 0:
                child_regexes.append(f"^{i}{child_regex}")
            else:
                child_regexes.append(f"{i}{child_regex}")
    # Convert the list of regexes to a regex
    result = truncate_regexes_list(child_regexes,root.height)
    return result

"""Constructs a map of state to regex representing pincodes for that state.

Takes a map of state to list of pincodes. For each state, constructs a trie 
from the pincode list and generates a regex representing all pincodes in the 
list. Returns a map of state to generated regex.

Args:
    state_pincodes_map: Map of state to list of pincodes for that state.

Returns: 
    Map of state to regex representing pincodes for that state.
"""
def contruct_state_pincode_regex_map(state_pincodes_map):
    state_pincode_regex_map = {}
    for state, pincodes in state_pincodes_map.items():
        # pincodes tree from the list
        root = construct_tree_according_pincodes_list(pincodes)
        # regex from the pincodes tree
        regex = regex_from_pincodes_tree(root)
        state_pincode_regex_map[state] = regex
    return state_pincode_regex_map

"""Converts the sorted and grouped DataFrame to a dictionary mapping state to list of pincodes."""
def convert_df_to_map():
    # Read the CSV into a DataFrame
    df = pd.read_csv('temp.csv')
    # TODO : delete the temp.csv file 
    os.remove('temp.csv')
    # Convert the DataFrame to a dictionary
    state_pin_map = df.set_index('State')['Pin'].apply(lambda x: x.strip('[]').split(', ')).to_dict()
    # Display the state and its associated pin codes
    return state_pin_map

"""Writes a dictionary to a CSV file.

Args:
    data (dict): Dictionary containing data to write. Keys are column headers.
    filename (str): Path to output CSV file.
"""
def dict_to_csv(data, filename):
    with open(filename, "w") as f:
        f.write("State,Regex\n")
        for key, value in data.items():
            f.write(f"{key},{value}\n")
    
# ... Rest of the code ...
def main():
    """Main function to execute the pin code processing pipeline."""
    parser = argparse.ArgumentParser(description="Create a regex of pincodes from a list of pincodes")
    parser.add_argument("-i", "--input", required=True, help="Input CSV file")
    parser.add_argument("-o", "--output", required=True, help="Output CSV file")
    args = parser.parse_args()
    input_file = args.input
    output_file = args.output
    sort_and_group_pincodes(input_file)
    state_pincodes_map = convert_df_to_map()
    state_pincode_regex_map = contruct_state_pincode_regex_map(state_pincodes_map)
    dict_to_csv(state_pincode_regex_map, output_file)

main()