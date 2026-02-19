# Project Plan: Syngen (Synthetic Slack Export Generator)

## 1. Objective
Create a C99 CLI utility that generates synthetic Slack export data (zipped JSON) for testing visualization tools.
The tool mimics the structure of a real Slack export, filling it with plausible fake data (users, channels, messages).

## 2. CLI Specification
The program will be invoked as follows:
```bash
syngen -c <channels> -m <messages> -u <users> <output_filename>
```

**Arguments:**
-   `-c`: Number of channels to generate (default: 25).
-   `-m`: Total number of messages to generate (default: 1000).
-   `-u`: Number of users to generate (default: 10).
-   `<output_filename>`: The name of the output zip file (e.g., `export.zip`).

## 3. Architecture

### 3.1. External Dependencies
-   **cJSON**: A lightweight, portable C library for creating JSON objects. We will embed `cJSON.c` and `cJSON.h` directly into the source tree to ensure the project is self-contained and easy to build.
-   **Zip Utility**: The program will utilize the system's `zip` command via `system()` calls to package the generated directory.

### 3.2. Modules
1.  **`main.c`**: Entry point. Handles argument parsing (using `getopt`), initializes seeds, and orchestrates the generation process.
2.  **`faker` Module**:
    -   `faker_data.h`: Contains static arrays of names (first/last) and lorem ipsum words.
    -   `faker.c`: Functions to return random names, emails, IDs (`U...`, `C...`), and text blocks.
3.  **`structs`**:
    -   `User`: Represents a synthetic user (id, name, profile).
    -   `Channel`: Represents a channel (id, name, members list).
    -   `Message`: Represents a message (ts, user_id, text).
4.  **`generator` Module**:
    -   `gen_users`: Creates $N$ users with consistent metadata.
    -   `gen_channels`: Creates $N$ channels.
    -   `gen_messages`: Distributes $M$ messages across channels and users using a Gaussian distribution to simulate realistic activity concentrations.
5.  **`writer` Module**:
    -   Uses `cJSON` to serialize structs to disk.
    -   Manages directory creation (`mkdir -p`).

## 4. Implementation Steps

### Phase 1: Setup & Dependencies
- [x] Initialize project structure (`src`, `include`, `vendor`).
- [x] Import `cJSON` (source files).
- [x] Create `Makefile`.
- [x] Extract data from Python `faker` library and convert to `src/faker_data.h`.
#### Testing
- [x] Compile empty project.
- [x] Verify `faker_data.h` contains valid C arrays and data is accessible.

### Phase 2: Core Data Structures & Faker
- [x] Define `User`, `Channel`, `Message` structs.
- [x] Implement `faker` functions (random name, random ID, random timestamp, random text).
#### Testing
- [x] Unit tests for `User`, `Channel`, and `Message` constructors/initializers.
- [x] Unit tests for `faker` functions (verify output format and randomness).

### Phase 3: Generation Logic
- [x] Implement User generation (populate `users.json` structure).
- [x] Implement Channel generation (populate `channels.json` structure).
- [x] Implement Message generation with Gaussian distribution logic (assigning messages to channels/users non-uniformly).
#### Testing
- [x] Verify `gen_users` produces unique IDs and correct count.
- [x] Verify `gen_channels` produces correct count.
- [x] Verify `gen_messages` distribution (check statistical spread).

### Phase 4: Output & Serialization
- [x] Implement JSON writing using `cJSON`.
- [x] Implement file system operations (creating directories for each channel).
- [x] Implement the `users.json` and `channels.json` writers.
- [x] Implement the daily message file writer (e.g., `channel/YYYY-MM-DD.json`).
#### Testing
- [x] Verify directory structure creation.
- [x] Verify JSON files are valid (linting) and contain expected fields.

### Phase 5: CLI & Packaging
- [x] Implement `main` with `getopt` for `-c`, `-m`, `-u`, and filename arguments.
- [x] Add the zip compression step at the end.
- [x] Final compilation and testing against the `slack-export-viewer`.
#### Testing
- [x] End-to-end test: Run `syngen` and unzip the output to check integrity.

## 5. Data Strategy
-   **Names**: We will extract ~1000 common names from the `faker` library.
-   **Text**: We will use a standard "Lorem Ipsum" word list to generate sentences.
-   **Timestamps**: We will generate timestamps spanning a recent 30-day window.
