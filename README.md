# work_21-nevertheless_she_persisted
Dylan Hu - Work 21: Nevertheless, She Persisted

## Description
This project uses two executables to create and use named pipes in order to send data between two running programs. This project compiles two executables. 

The first acts as the "server", continuously connecting to clients via a three-way handshake. It then recieves any data, reverses it, and sends it back to the client.

The second acts as the "client", connecting to the server using a three-way handshake. It prompts the user for input, and relays it and the response to and from the server.

Both programs handle SIGINTs by cleaning up named pipes, if necessary.

## Compilation and Execution
Use `make` to compile all needed executables. Run `./server` in one terminal, then `./client` in another.

#### Exiting
Exit using ctrl+c for both programs. This exits the program gracefully by removing named pipes if necessary.
