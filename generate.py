import random

def generate_actors(n, x_range, y_range, views):
    actors = []
    for _ in range(n):
        actor = {
            "name": "actor{}".format(_),
            "x": random.randint(*x_range),
            "y": random.randint(*y_range),
            "block": bool(random.getrandbits(1)),
            "view": random.choice(views)
        }

        # Add the additional properties with 50% probability
        if random.random() > 0.9:
            actor["nearby_dialogue"] = "health down"

        if random.random() > 0.9:
            actor["contact_dialogue"] = "health down"

        if random.random() > 0.5:
            actor["vel_x"] = random.choice([-1, 1])

        if random.random() > 0.5:
            actor["vel_y"] = random.choice([-1, 1])

        actors.append(actor)

    actors.append({
            "name": "player",
            "x": random.randint(*x_range),
            "y": random.randint(*y_range),
            "block": bool(random.getrandbits(1)),
            "view": "p"
        })
    return actors

import json

def write_to_json_file(data, filename):
    with open(filename, 'w') as f:
        json.dump(data, f, indent=4)

# Generate actors

def write_random_data(n, filename):
    directions = ['n', 'w', 's', 'e']
    with open(filename, 'w') as f:
        for _ in range(n):
            f.write(random.choice(directions) + '\n')

# Write to JSON file
def main():
    actors = generate_actors(10000, (0, 500), (0, 500), ['a', 'b', 'c', 'd', 'e','f','g','j'])
    write_to_json_file({"actors": actors}, './resources/scenes/level_1.scene')
    

main()
# write_random_data(1000, './input.txt')