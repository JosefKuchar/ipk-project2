"""Tests for server implementation"""

import unittest
import socket


class TestTCP(unittest.TestCase):
    """TCP tests"""

    def send_message(self, message):
        """Send a message to the server and return the response"""
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(("127.0.0.1", 1234))
        sock.sendall(message)
        response = b""
        while True:
            data = sock.recv(1024)
            if not data:
                break
            response += data
        sock.close()
        return response

    def test_hello_bye(self):
        """HELLO BYE"""
        self.assertEqual(self.send_message(b"HELLO\nBYE\n"), b"HELLO\nBYE\n")

    def test_hello_hello(self):
        """HELLO HELLO"""
        self.assertEqual(self.send_message(b"HELLO\nHELLO\n"), b"HELLO\nBYE\n")

    def test_bye(self):
        """BYE"""
        self.assertEqual(self.send_message(b"BYE\n"), b"BYE\n")

    def test_invalid(self):
        """ABC"""
        self.assertEqual(self.send_message(b"ABC\n"), b"BYE\n")

    def test_hello_invalid(self):
        """HELLO ABC"""
        self.assertEqual(self.send_message(b"HELLO\nABC\n"), b"HELLO\nBYE\n")

    def test_addition(self):
        """HELLO SOLVE (+ 1 2) BYE"""
        self.assertEqual(self.send_message(
            b"HELLO\nSOLVE (+ 1 2)\nBYE\n"), b"HELLO\nRESULT 3\nBYE\n")

    def test_subtraction(self):
        """HELLO SOLVE (- 80 52) BYE"""
        self.assertEqual(self.send_message(
            b"HELLO\nSOLVE (- 80 52)\nBYE\n"), b"HELLO\nRESULT 28\nBYE\n")

    def test_multiplication(self):
        """HELLO SOLVE (* 2 3) BYE"""
        self.assertEqual(self.send_message(
            b"HELLO\nSOLVE (* 2 3)\nBYE\n"), b"HELLO\nRESULT 6\nBYE\n")

    def test_division(self):
        """HELLO SOLVE (/ 10 2) BYE"""
        self.assertEqual(self.send_message(
            b"HELLO\nSOLVE (/ 10 2)\nBYE\n"), b"HELLO\nRESULT 5\nBYE\n")

    def test_division_by_zero(self):
        """HELLO SOLVE (/ 10 0) BYE"""
        self.assertEqual(self.send_message(
            b"HELLO\nSOLVE (/ 10 0)\n"), b"HELLO\nBYE\n")

    def test_large_solve(self):
        """HELLO SOLVE (+(1 1 ... 1)) BYE"""
        self.assertEqual(self.send_message(
            b"HELLO\nSOLVE (+" + (b" 1" * 1000) + b")\nBYE\n"), b"HELLO\nRESULT 1000\nBYE\n")

    def test_complex_solve(self):
        """HELLO SOLVE (+ 1 (* 2 3) (/ 8 4)) BYE"""
        self.assertEqual(self.send_message(
            b"HELLO\nSOLVE (+ 1 (* 2 3) (/ 8 4))\nBYE\n"), b"HELLO\nRESULT 9\nBYE\n")

    def test_invalid_expression(self):
        """HELLO SOLVE (1 2 3)"""
        self.assertEqual(self.send_message(
            b"HELLO\nSOLVE (1 2 3)\n"), b"HELLO\nBYE\n")

    def test_invalid_expression2(self):
        """HELLO SOLVE (1 2 3))"""
        self.assertEqual(self.send_message(
            b"HELLO\nSOLVE (+ 1 2 3))\n"), b"HELLO\nBYE\n")

    def test_invalid_expression3(self):
        """HELLO SOLVE (1 2 3"""
        self.assertEqual(self.send_message(
            b"HELLO\nSOLVE (+ 1 2 3\n"), b"HELLO\nBYE\n")

    def test_invalid_expression4(self):
        """HELLO SOLVE (+ 1)"""
        self.assertEqual(self.send_message(
            b"HELLO\nSOLVE (+ 1)\n"), b"HELLO\nBYE\n")

    def test_invalid_expression5(self):
        """HELLO SOLVE (+ 1 2 3))"""
        self.assertEqual(self.send_message(
            b"HELLO\nSOLVE (+ 1 2 3))\n"), b"HELLO\nBYE\n")

    def test_invalid_expression6(self):
        """HELLO SOLVE (+ 1a2 3)"""
        self.assertEqual(self.send_message(
            b"HELLO\nSOLVE (+ 1a2 3)\n"), b"HELLO\nBYE\n")

    def test_negative_result(self):
        """HELLO SOLVE (- 1 2)"""
        self.assertEqual(self.send_message(
            b"HELLO\nSOLVE (- 1 2)\n"), b"HELLO\nBYE\n")


class TestUDP(unittest.TestCase):
    """UDP tests"""

    def send_message(self, message):
        """Send a message to the server and return the response"""
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.sendto(message, ("127.0.0.1", 1235))
        response, _ = sock.recvfrom(1024)
        sock.close()
        return response

    def test_invalid_opcode(self):
        """Invalid opcode"""
        self.assertEqual(self.send_message(b"\1\1a"),
                         b'\x01\x01\x0eInvalid opcode')

    def test_invalid_length(self):
        """Invalid length"""
        self.assertEqual(self.send_message(b"\0\0a"),
                         b'\x01\x01\x0eInvalid length')

    def test_invalid_length2(self):
        """Invalid length"""
        self.assertEqual(self.send_message(b"\0\255a"),
                         b'\x01\x01\x0eInvalid length')

    def test_expression(self):
        """(+ 1 2)"""
        self.assertEqual(self.send_message(b"\0\7(+ 1 2)"), b'\x01\x00\x013')

    def test_invalid(self):
        """ABC"""
        self.assertEqual(self.send_message(b"\0\3ABC"),
                         b'\x01\x01\x1bError evaluating expression')

    def test_negative_result(self):
        """(- 1 2)"""
        self.assertEqual(self.send_message(b"\0\7(- 1 2)"),
                         b'\x01\x01\x1bError evaluating expression')


if __name__ == "__main__":
    unittest.main()
