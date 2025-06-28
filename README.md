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
- (Optional) `diff`, `gprof` for profiling and test validation

---

## 🔧 Build & Run

1. Clone the repo:

   ```bash
   git clone https://github.com/imrobee/csopesy-mo.git
   cd csopesy-mo
Build the release version:

bash
Copy
Edit
make release
Executable is generated at build/scheduler or build/a.out

Clean artifacts:

bash
Copy
Edit
make clean
▶️ Usage
The scheduler reads input from stdin. Use input redirection:

./build/scheduler < testcases/input1.txt
Supported commands
make bench — compiles with timing info on execution

make bench-prof — compiles with gprof profiling flags

Test automation
Run tests via:

./testscript.sh
This script:

Prompts you to choose an algorithm

Runs all testcases and compares outputs

Requires diff to highlight mismatches

🧠 Input Format
Each test case must specify:

<algorithm> <quantum>
< n >
<pid> <arrival_time> <burst_time>
...
<algorithm>: FCFS, SJF, SRTF, or RR

<quantum>: a positive integer (only relevant for RR)

<n>: number of processes

Each line: process ID, arrival time, and burst time

Example
RR 4
3
P1 0 5
P2 1 3
P3 2 8
📊 Output
For each process, display:

Process ID

Arrival Time

Burst Time

Completion Time

Turnaround Time

Waiting Time

Also show average turnaround time and waiting time at the end.

📝 Algorithms
FCFS – schedules processes in order of arrival.

SJF – picks the job with shortest burst time (non-preemptive).

SRTF – preemptive variant: switches to shortest remaining job.

RR – cycles through processes in time slices (quantum).

📌 Authors & Contributors
Daniel Gavrie Clemente
Dominique Angelo Roque
Felix Melford Mangawang
Jan Robee Feliciano
