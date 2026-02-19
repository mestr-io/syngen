import re
import os


def extract_list(file_path, list_name):
    with open(file_path, "r") as f:
        content = f.read()

    # Matches patterns like `list_name = (` or `list_name = OrderedDict(`
    # This is a heuristic approach, assuming the structure from the viewed files.
    # For `person`: keys are names, values are weights.
    # For `lorem`: just a tuple of strings.

    if "OrderedDict" in content:
        # Extract names from OrderedDict keys
        # Pattern: "Name", weight
        # We need to find the block for list_name
        pattern = re.compile(
            f"{list_name}\s*=\s*OrderedDict\s*\(\s*\((.*?)\)\s*\)", re.DOTALL
        )
        match = pattern.search(content)
        if not match:
            print(f"Could not find {list_name} in {file_path}")
            return []

        block = match.group(1)
        # Extract strings inside quotes that are followed by a comma and a number
        # Example: ("Aaron", 0.006741589),
        names = re.findall(r"\(?\"([A-Za-z]+)\",\s*[\d\.]+", block)
        return names

    else:
        # Tuple extraction for lorem
        pattern = re.compile(f"{list_name}\s*=\s*\((.*?)\)", re.DOTALL)
        match = pattern.search(content)
        if not match:
            print(f"Could not find {list_name} in {file_path}")
            return []

        block = match.group(1)
        words = re.findall(r"\"([a-zA-Z\s]+)\"", block)
        return words


def write_header(data_map, output_file):
    with open(output_file, "w") as f:
        f.write("#ifndef FAKER_DATA_H\n")
        f.write("#define FAKER_DATA_H\n\n")

        for name, items in data_map.items():
            f.write(f"static const char *{name}[] = {{\n")
            for item in items:
                f.write(f'    "{item}",\n')
            f.write("};\n\n")
            f.write(f"static const int {name}_count = {len(items)};\n\n")

        f.write("#endif // FAKER_DATA_H\n")


if __name__ == "__main__":
    person_file = "temp_faker/faker/providers/person/en_US/__init__.py"
    lorem_file = "temp_faker/faker/providers/lorem/en_US/__init__.py"

    first_names_male = extract_list(person_file, "first_names_male")
    first_names_female = extract_list(person_file, "first_names_female")
    last_names = extract_list(person_file, "last_names")
    words = extract_list(lorem_file, "word_list")

    data = {
        "faker_first_names_male": first_names_male,
        "faker_first_names_female": first_names_female,
        "faker_last_names": last_names,
        "faker_words": words,
    }

    output_path = "src/faker/faker_data.h"
    write_header(data, output_path)
    print(
        f"Generated {output_path} with {len(first_names_male)} male names, {len(first_names_female)} female names, {len(last_names)} last names, and {len(words)} words."
    )
