"""Test Example Library
"""
import unittest

from one.codelab.python_smoke_app.lib.library import get_answer
from one.codelab.python_smoke_app.lib.library import format_code


class TestLibrary(unittest.TestCase):

    def test_get_answer(self):
        self.assertEqual(42, get_answer())

    def test_format_code(self):
        result = format_code("f ( a = 1, b = 2 )")
        self.assertTrue(result[1])
        self.assertEqual("f(a=1, b=2)\n", result[0])


if __name__ == "__main__":
    unittest.main()
