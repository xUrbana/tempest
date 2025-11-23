# Tempest Weather Station Data Processor

This project provides a C++ library and applications for processing data from a Tempest weather station, along with a Python binding for easy integration into Python projects.

## Features

*   **C++ Library:** `libtempest` - Core logic for receiving and parsing Tempest weather station UDP packets.
*   **Python Module:** `pytempest` - A Python wrapper around the C++ library, allowing Python applications to easily interact with Tempest data.
*   **`tempest-print` application:** A C++ command-line application that receives Tempest data and prints parsed observations to standard output.
*   **`tempest-db` application:** A C++ command-line application that receives Tempest data and stores parsed observations into a PostgreSQL database.

## Dependencies

### Build-time Dependencies

*   **C++ Compiler:** C++23 compatible (e.g., GCC 13+, Clang 16+).
*   **CMake:** Version 3.31.6 or higher for building the C++ components.
*   **pybind11:** For creating Python bindings.
*   **nlohmann/json:** Header-only library for JSON parsing.
*   **spdlog:** Header-only library for fast, flexible logging.
*   **libpqxx:** C++ API for PostgreSQL.

### Runtime Dependencies

*   **PostgreSQL:** Required by the `tempest-db` application.
*   **Boost:** Used for `asio` in the UDP receiver.

### Python Dependencies

*   **scikit-build-core:** Build backend for the Python package.
*   **pybind11:** For Python bindings (runtime dependency for the Python module).

## Building the C++ Applications

The project uses CMake for building the C++ components.

1.  **Build the project:**
    ```bash
    ./build.sh
    ```
    This will produce executables in the `build/apps/print/` (e.g., `tempest-print`) and `build/apps/db/` (e.g., `tempest-db`) directories.

## Installing the Python Module

The `pytempest` Python module can be installed using `pip`. This process will automatically build the C++ extension using `scikit-build-core`.

1.  **Ensure you have `pip` and a Python development environment:**
    ```bash
    python3 -m pip install --upgrade pip
    ```
2.  **Install the Python package from the project root:**
    ```bash
    pip install .
    ```

## Usage

### Python Library (`pytempest`)

Once installed, you can import and use the `tempest` module in your Python scripts.

```python
import pytempest

# Initialize the Tempest client
t = pytempest.Tempest()

# Add a handler (a function that will be called with each new observation)
# The observation object 'o' will have attributes like 'air_temp_f', 'pressure_inhg', etc.
t.add_handler(lambda o: print(f"Current Air Temperature: {o.air_temp_f}°F"))

# Run the client to start receiving data
# This will block until the application is stopped (e.g., Ctrl+C)
t.run()
```

### `tempest-print` Application (C++)

This application simply prints incoming Tempest observations to the console.

```bash
./build/apps/print/tempest-print
```

### `tempest-db` Application (C++)

This application connects to a PostgreSQL database and stores incoming Tempest observations. It requires environment variables to connect to the database.

**Environment Variables for PostgreSQL Connection:**

*   `TEMPEST_DB_HOST`: PostgreSQL host (default: `localhost`)
*   `TEMPEST_DB_PORT`: PostgreSQL port (default: `5432`)
*   `TEMPEST_DB_NAME`: Database name (default: `tempest`)
*   `TEMPEST_DB_USER`: Username (default: `tempest`)
*   `TEMPEST_DB_PASSWORD`: Password (default: `password`)

**Example usage:**

```bash
# Ensure your PostgreSQL server is running and accessible
# (Optional: Set environment variables if different from defaults)
TEMPEST_DB_HOST=my_db_host TEMPEST_DB_NAME=my_tempest_db ./build/apps/db/tempest-db
```

## Docker

The project includes a `Dockerfile` and `docker-compose.yaml` for containerized deployment, particularly for the `tempest-db` application with a PostgreSQL database.

**Building the Docker Image:**

First, ensure the C++ applications are built as described in "Building the C++ Applications" section. The `Dockerfile` expects the `tempest-db` executable to be present in the `build/apps/db/` directory.

```bash
docker build -t tempest-app .
```

**Running with Docker Compose:**

The `docker-compose.yaml` file defines two services: `tempest` (which runs your `tempest-db` application) and `db` (a PostgreSQL container).

```bash
docker-compose up
```

**Note:** The `docker-compose.yaml` uses `network_mode: host` for simplified networking. Be aware of the implications this has on port exposure and network isolation. The `tempest` service in `docker-compose.yaml` is configured to use a pre-built image from `ryankelley20/tempest:latest`. If you wish to use your locally built image, you will need to modify the `docker-compose.yaml` to reference `image: tempest-app` (or whatever tag you used during `docker build`) and optionally add a `build: .` context for the `tempest` service to build it on the fly.