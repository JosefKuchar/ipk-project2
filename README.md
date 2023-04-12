# IPK Project 2: IPK Server for Remote Calculator

This README is the main documentation for this project.

## Usage

```
ipkcpd -h <host> -p <port> -m <mode>
```

## Requirements

- `gcc`
- `make`

For testing

- `python3`

## Make targets

- `make` Builds the project and creates `ipkcpd` binary in project root
- `make run_tcp` Builds project and runs server in TCP mode with default (examples) arguments
- `make run_udp` Builds project and runs server in UDP mode with default (example) arguments
- `make test` Runs tests.
- `make zip` Creates final ZIP file for assignment submission
- `make clean` Cleans temporary files (e.g object files)

## Project structure

These are the notable files and folders.

- `/examples` Example inputs
- `ipkcpd.cc` Entry point
- `args.cc`, `args.hpp` Argument parsing module
- `server.cc`, `server.hpp` Abstract base class, factory for servers
- `tcp-server.cc`, `tcp-server.hpp` TCP server implementation
- `udp-server.cc`, `udp-server.hpp` UDP server implementation
- `parser.cc`, `parser.hpp` Expression parser implementation
- `test.py` Tests

## Code

The application is written in C++20. server is abstract class and factory for specific (currently TCP and UDP) implementations. Arguments are parsed in their own module. Project aims to be simple and extensible.

## Testing

Testing was done with custom tests written in Python 3 with unittest library.
It tests tcp, udp comunication and argument parsing. Tests live in `test.py`, they have it's own help, which can be viewed with `python3 test.py -h`.

The application was tested as a whole. It was also confirmed that it compiles and runs on the reference VM.

Testing proof is located in `test.log` file.

## License

License in located in **LICENSE** file.

## References

- Changelog format - https://keepachangelog.com/cs/1.0.0/
- General project instructions - https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master
- Project instructions https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Project%202/iota
- Protocol specification https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Project%201/Protocol.md
- C++ reference https://en.cppreference.com/w/
