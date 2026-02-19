# Syngen - Synthetic Slack Export Generator

Syngen is a C99 CLI utility that generates synthetic Slack export data (zipped JSON) for testing visualization tools or analyzing Slack data structures without using sensitive real-world data.

It mimics the structure of a standard Slack export, populating it with realistic fake users, channels, and messages using a Gaussian distribution to simulate varying activity levels.

## Features

- Generates `users.json`, `channels.json`, and per-channel daily message files.
- **Gaussian Distribution**: Simulates realistic activity where some channels and users are more active than others.
- **Faker Integration**: Uses real names and "Lorem Ipsum" text derived from the Python `faker` library.
- **Self-contained**: Minimal external dependencies (standard libc + vendored cJSON).

## Prerequisites

- GCC (or any C99 compliant compiler)
- Make
- `zip` command-line utility (used by the program to create the final archive)
- `unzip` (only required for running the test suite)

## Building

To build the project, run:

```bash
make
```

This will compile the source code and place the executable in `bin/syngen`.

## Usage

```bash
./bin/syngen -c <channels> -m <messages> -u <users> <output_filename>
```

### Arguments

- `-c`: Number of channels to generate (default: 25).
- `-m`: Total number of messages to generate (default: 1000).
- `-u`: Number of users to generate (default: 10).
- `<output_filename>`: The name of the output zip file (e.g., `export.zip`).

### Example

Generate an export with 50 users, 20 channels, and 5000 messages:

```bash
./bin/syngen -c 20 -m 5000 -u 50 slack_fake.zip
```

## Running Tests

The project includes an integration test suite that verifies the generated directory structure and JSON content.

To run the tests:

```bash
make test
```

## Code Quality & Linting

The project uses `cppcheck` for static analysis and strict compiler flags for linting.

To check code quality:

```bash
make lint
```

This will run `cppcheck` if installed, or fall back to strict GCC/Clang warnings (`-Werror -pedantic -Wconversion -Wshadow`).

## Development

The project structure is as follows:

- `src/`: Source code.
  - `main.c`: Entry point and CLI parsing.
  - `faker/`: Data generation (names, text, IDs).
  - `generator/`: Logic for users, channels, and message distribution.
  - `export/`: JSON serialization and file writing.
- `include/`: Header files.
- `vendor/`: External libraries (cJSON).
- `tests/`: Integration test scripts.

### Regenerating Faker Data

The project uses a header file `src/faker/faker_data.h` containing static arrays of names and words. If you need to modify or regenerate this data:

1.  Clone the Python faker library into `temp_faker`.
2.  Run the extraction script:
    ```bash
    python3 extract_faker.py
    ```
