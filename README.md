# Low Level Synchronization: Visitor Boat Simulation

This repository implements a multithreaded simulation of a visitor-boat system using low-level synchronization primitives in C (POSIX threads, mutexes, condition variables, and custom semaphores). The simulation models visitors arriving, sightseeing, and taking boat rides, with synchronization ensuring correct access to boats and proper session management.

## Features

- **Custom semaphore implementation** using `pthread_mutex_t` and `pthread_cond_t`.
- Simulates multiple boats and multiple visitors, each as a separate thread.
- Each visitor performs sightseeing before attempting to get a boat ride.
- Each boat waits for a visitor, then simulates a ride and becomes available again.
- Proper synchronization to prevent race conditions and deadlocks.
- Clean resource management (all threads joined, mutexes and barriers destroyed).

## Files

- `boating.c` - Main simulation logic, including visitor and boat thread functions.
- `semaphore.h` / `semaphore.c` - Custom semaphore structure and operations.
- `Makefile` - Build and run instructions.
- `LA7.pdf` - Assignment or project specification (for reference).

## How It Works

1. The simulation takes two command-line arguments: number of boats (`m`) and number of visitors (`n`).
2. Each visitor thread:
   - "Sightsees" for a random time.
   - Waits for a boat to become available.
   - Reserves a boat, communicates ride time, and synchronizes with the corresponding boat.
   - Leaves after the ride.
3. Each boat thread:
   - Waits for a visitor to request a ride.
   - Synchronizes with the visitor, simulates the ride, then becomes available again.
   - Exits when all visitors are done.

## Build Instructions

Ensure you have `gcc` and POSIX threads (`pthread`) installed.

```bash
make
```

## Run the Simulation

```bash
./boating <num_boats> <num_visitors>
```
- `<num_boats>`: Number of boats (must be between 5 and 10)
- `<num_visitors>`: Number of visitors (must be between 20 and 100)

Example:
```bash
./boating 5 30
```
Or using the Makefile shortcut:
```bash
make run
```

## Clean Up

Remove the compiled binary with:
```bash
make clean
```

## Example Output

```
Boat      1    Ready
Boat      2    Ready
...
Visitor   1    Starts sightseeing for  85 minutes
...
Visitor   1    Ready to ride a boat (ride time = 25)
Visitor   1    Finds boat  1
Boat      1    Start of ride for visitor   1
Boat      1    End of ride for visitor   1 (ride time = 25)
Visitor   1    Leaving
...
```

## Assignment Specification

See `LA7.pdf` for the original assignment description and requirements.

## License

This project is provided for educational purposes. Please review `LA7.pdf` for any specific usage restrictions or attribution requirements.
