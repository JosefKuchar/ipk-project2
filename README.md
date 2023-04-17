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

## Implementation details

The application is written in C++20. Server is abstract class and factory for specific (currently TCP and UDP) implementations. Arguments are parsed in their own module. Project aims to be simple and extensible.

### Parsing queries

Individual queries are parsed with top-down recursive descent parser. This implementation is in `parser.cc`. ABNF grammar was manually converted to LL grammar:

```
Query -> ( Op SP Expr SP Expr OptExpr ) END .
Op -> + .
Op -> - .
Op -> * .
Op -> / .
Expr -> ( Op SP Expr SP Expr OptExpr ) .
Expr -> NUMBER .
OptExpr -> SP Expr OptExpr .
OptExpr -> .
```

### Handling multiple clients

In TCP mode each client is handled in separate thread. In UDP mode all requests all handled by single thread.

### TCP specifics

Messages are matched with REGEX patterns. Server correctly implements handling multiple messages in one `read` call and also single message split to multiple reads.

## Testing

Testing was done with custom tests written in Python 3 with unittest library.
It tests both tcp and udp comunication. Tests live in `test.py`.

The application was tested as a whole. It was also confirmed that it compiles and runs on the reference VM.

Parser is tested mainly with TCP tests. There is no need to re-test same things in UDP, so in UDP there are only sanity checks.

Test outputs:

### TCP testing scenarios

| Input                                              | Expected output                | Actual output                  |
| -------------------------------------------------- | ------------------------------ | ------------------------------ |
| `b"HELLO\nBYE\n"`                                  | `b"HELLO\nBYE\n"`              | `b"HELLO\nBYE\n"`              |
| `b"HELLO\nHELLO\n"`                                | `b"HELLO\nBYE\n"`              | `b"HELLO\nBYE\n"`              |
| `b"BYE\n"`                                         | `b"BYE\n"`                     | `b"BYE\n"`                     |
| `b"ABC\n"`                                         | `b"BYE\n"`                     | `b"BYE\n"`                     |
| `b"HELLO\nABC\n"`                                  | `b"HELLO\nBYE\n"`              | `b"HELLO\nBYE\n"`              |
| `b"HELLO\nSOLVE (+ 1 2)\nBYE\n"`                   | `b"HELLO\nRESULT 3\nBYE\n"`    | `b"HELLO\nRESULT 3\nBYE\n"`    |
| `b"HELLO\nSOLVE (- 80 52)\nBYE\n"`                 | `b"HELLO\nRESULT 28\nBYE\n"`   | `b"HELLO\nRESULT 28\nBYE\n"`   |
| `b"HELLO\nSOLVE (* 2 3)\nBYE\n"`                   | `b"HELLO\nRESULT 6\nBYE\n"`    | `b"HELLO\nRESULT 6\nBYE\n"`    |
| `b"HELLO\nSOLVE (/ 10 2)\nBYE\n"`                  | `b"HELLO\nRESULT 5\nBYE\n"`    | `b"HELLO\nRESULT 5\nBYE\n"`    |
| `b"HELLO\nSOLVE (+" + (b" 1" * 1000) + b")\nBYE\n` | `b"HELLO\nRESULT 1000\nBYE\n"` | `b"HELLO\nRESULT 1000\nBYE\n"` |
| `b"HELLO\nSOLVE (+ 1 (* 2 3) (/ 8 4))\nBYE\n"`     | `b"HELLO\nRESULT 9\nBYE\n"`    | `b"HELLO\nRESULT 9\nBYE\n"`    |
| `b"HELLO\nSOLVE (1 2 3)\n"`                        | `b"HELLO\nBYE\n"`              | `b"HELLO\nBYE\n"`              |
| `b"HELLO\nSOLVE (+ 1 2 3))\n"`                     | `b"HELLO\nBYE\n"`              | `b"HELLO\nBYE\n"`              |
| `b"HELLO\nSOLVE (+ 1 2 3\n"`                       | `b"HELLO\nBYE\n"`              | `b"HELLO\nBYE\n"`              |
| `b"HELLO\nSOLVE (+ 1)\n"`                          | `b"HELLO\nBYE\n"`              | `b"HELLO\nBYE\n"`              |
| `b"HELLO\nSOLVE (+ 1 2 3))\n"`                     | `b"HELLO\nBYE\n"`              | `b"HELLO\nBYE\n"`              |
| `b"HELLO\nSOLVE (+ 1a2 3)\n"`                      | `b"HELLO\nBYE\n"`              | `b"HELLO\nBYE\n"`              |
| `b"HELLO\nSOLVE (- 1 2)\n"`                        | `b"HELLO\nBYE\n"`              | `b"HELLO\nBYE\n"`              |

### UDP testing scenarios

| Input            | Expected output                              | Actual output                                |
| ---------------- | -------------------------------------------- | -------------------------------------------- |
| `b"\1\1a"`       | `b'\x01\x01\x0eInvalid opcode'`              | `b'\x01\x01\x0eInvalid opcode'`              |
| `b"\0\0a"`       | `b'\x01\x01\x0eInvalid length'`              | `b'\x01\x01\x0eInvalid length'`              |
| `b"\0\255a"`     | `b'\x01\x01\x0eInvalid length'`              | `b'\x01\x01\x0eInvalid length'`              |
| `b"\0\7(+ 1 2)"` | `b'\x01\x00\x013'`                           | `b'\x01\x00\x013'`                           |
| `b"\0\3ABC"`     | `b'\x01\x01\x1bError evaluating expression'` | `b'\x01\x01\x1bError evaluating expression'` |
| `b"\0\7(- 1 2)"` | `b'\x01\x00\x02-1'`                          | `b'\x01\x00\x02-1'`                          |

### Test outputs

```
test_addition (__main__.TestTCP)
HELLO SOLVE (+ 1 2) BYE ... ok
test_bye (__main__.TestTCP)
BYE ... ok
test_complex_solve (__main__.TestTCP)
HELLO SOLVE (+ 1 (* 2 3) (/ 8 4)) BYE ... ok
test_division (__main__.TestTCP)
HELLO SOLVE (/ 10 2) BYE ... ok
test_division_by_zero (__main__.TestTCP)
HELLO SOLVE (/ 10 0) BYE ... ok
test_hello_bye (__main__.TestTCP)
HELLO BYE ... ok
test_hello_hello (__main__.TestTCP)
HELLO HELLO ... ok
test_hello_invalid (__main__.TestTCP)
HELLO ABC ... ok
test_invalid (__main__.TestTCP)
ABC ... ok
test_invalid_expression (__main__.TestTCP)
HELLO SOLVE (1 2 3) ... ok
test_invalid_expression2 (__main__.TestTCP)
HELLO SOLVE (1 2 3)) ... ok
test_invalid_expression3 (__main__.TestTCP)
HELLO SOLVE (1 2 3 ... ok
test_invalid_expression4 (__main__.TestTCP)
HELLO SOLVE (+ 1) ... ok
test_invalid_expression5 (__main__.TestTCP)
HELLO SOLVE (+ 1 2 3)) ... ok
test_invalid_expression6 (__main__.TestTCP)
HELLO SOLVE (+ 1a2 3) ... ok
test_large_solve (__main__.TestTCP)
HELLO SOLVE (+ (1 1 ... 1)) BYE ... ok
test_multiplication (__main__.TestTCP)
HELLO SOLVE (* 2 3) BYE ... ok
test_negative_result (__main__.TestTCP)
HELLO SOLVE (- 1 2) ... ok
test_subtraction (__main__.TestTCP)
HELLO SOLVE (- 80 52) BYE ... ok
test_expression (__main__.TestUDP)
(+ 1 2) ... ok
test_invalid (__main__.TestUDP)
ABC ... ok
test_invalid_length (__main__.TestUDP)
Invalid length ... ok
test_invalid_length2 (__main__.TestUDP)
Invalid length ... ok
test_invalid_opcode (__main__.TestUDP)
Invalid opcode ... ok
test_negative_result (__main__.TestUDP)
(- 1 2) ... ok
```

## License

License in located in **LICENSE** file.

## References

- Changelog format - https://keepachangelog.com/cs/1.0.0/
- General project instructions - https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master
- Project instructions https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Project%202/iota
- Protocol specification https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Project%201/Protocol.md
- C++ reference https://en.cppreference.com/w/
