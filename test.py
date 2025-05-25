def calculate_traversal_order(ray_direction):
    """Calculate octree traversal order for a given ray direction."""
    traversal_order = []
    
    for z in range(2):
        for x in range(2):
            for y in range(2):
                # Determine actual bit values based on ray direction
                z_bit = (1 - z) if ray_direction[2] < 0.0 else z
                x_bit = (1 - x) if ray_direction[0] < 0.0 else x
                y_bit = (1 - y) if ray_direction[1] < 0.0 else y
                
                octant = (z_bit << 2) | (x_bit << 1) | y_bit
                traversal_order.append(octant)
    
    return traversal_order

def print_octant_bits(octant):
    """Helper function to print octant in binary form (z,x,y)"""
    z_bit = (octant >> 2) & 1
    x_bit = (octant >> 1) & 1
    y_bit = octant & 1
    return f"{octant}"

# Test with different ray directions
ray_directions = [
    (1.0, 0.0, 0.0),   # Positive X
    (1.0, 1.0, 0.0),   # Positive X and Y
    (1.0, 1.0, 1.0),   # Positive X, Y, and Z
    (1.0, 1.0, -1.0),
    (1.0, -1.0, 0.0),  # Positive X and Negative Y
    (1.0, -1.0, 1.0),
    (1.0, -1.0, -1.0),
    (-1.0, 0.0, 0.0),  # Negative X
    (0.0, 1.0, 0.0),   # Positive Y
    (0.0, 1.0, 1.0),
    (0.0, 1.0, -1.0),
    (0.0, -1.0, 0.0),  # Negative Y
    (0.0, -1.0, 1.0),
    (0.0, -1.0, -1.0),
    (0.0, 0.0, 1.0),   # Positive Z
    (0.0, 0.0, -1.0),  # Negative Z
    (1.0, 1.0, 1.0),   # All positive
    (-1.0, -1.0, -1.0) # All negative
]

# Print results for each direction
print("Octree Traversal Orders:")
print("=======================\n")

for direction in ray_directions:
    order = calculate_traversal_order(direction)
    print(f"Ray Direction {direction}:")
    print("Traversal order:", [print_octant_bits(o) for o in order])
    print("\n")

# Additional test case for ray direction (-1, 0, 1)
direction = (-1.0, 0.0, 1.0)
order = calculate_traversal_order(direction)
print(f"Ray Direction {direction}:")
print("Traversal order:", [print_octant_bits(o) for o in order])
