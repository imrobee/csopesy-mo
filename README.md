# CSOPESY‑MO 🖥️

A command‑line CPU scheduling simulation implemented in C, supporting the following classic algorithms:

- **FCFS** – First‑Come, First‑Served  
- **SJF** – Shortest Job First (non-preemptive)  
- **SRTF** – Shortest Remaining Time First (preemptive)  
- **RR** – Round-Robin  

---

## 📁 Repository Structure

/
├── src/ # C source files for each scheduler
├── include/ # Header files
├── testcases/ # Sample input & expected output
├── Makefile # Build scripts (make release, make clean)
├── testscript.sh # Script to batch-test scheduler implementations
└── README.md # This file

---

## ⚙️ Requirements

- GCC (C compiler)
- GNU Make

---

📌 Authors & Contributors
- Daniel Gavrie Clemente
- Dominic Angelo Roque
- Felix Melford Mangawang
- Jan Robee Feliciano


## Getting Started with the Emulator

1. Download the files from this repository or clone the repository to your machine
2. Compile the program
3. Run the application

### Usage

Once the program is running, you can use the following commands to interact with the emulator and process scheduler:

-   **`initialize`**  
    Initializes the system and prepares the environment for further operations. This must be run before starting any session or scheduler test.

    ```bash
     initialize
    ```

    Output:  
    ` Processor configuration initialized.`

-   **`screen -s <session_name>`**  
    Starts a new session with the given name.

    ```bash
     screen -s example_session
    ```

-   **`screen -r <session_name>`**  
    Reattaches to an existing session.

    ```bash
     screen -r example_session
    ```

    Output:  
    Reattaches and switches to the session `example_session`.

-   **`screen -ls`**  
    Lists all active sessions.

    ```bash
     screen -ls
    ```

-   **`scheduler-test`**  
    Starts the creation of dummy processes for testing the scheduling functionality.

    ```bash
     scheduler-test
    ```

    Output:  
    ` Creating dummy processes`

-   **`scheduler-stop`**  
    Stops the creation of dummy processes.

    ```bash
     scheduler-stop
    ```

    Output:  
    ` Stopping creation of dummy processes`

-   **`report-util`**  
    Generates and displays a utilization report of the system.

    ```bash
     report-util
    ```

-   **`clear`**  
    Clears the screen and re-displays the header.

    ```bash
     clear
    ```

-   **`exit`**  
    Exits the current session if inside one, or exits the emulator if not in a session.

    ```bash
     exit
    ```

    Output:  
    If in a session: ` Exiting session...`  
    If outside a session: ` Exiting emulator...`

-   **`help`**  
    Displays the list of available commands.

    ```bash
     help
    ```

-   **`process-smi`**
    Prints simple information of the process. Applicable when attached to a process.

    ```bash
     process-smi
    ```

---
