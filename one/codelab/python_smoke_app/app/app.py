"""Example Python Application
"""
from __future__ import print_function

from one.codelab.python_smoke_app.lib.library import get_answer
from one.codelab.python_smoke_app.lib.library import format_code

if __name__ == "__main__":
    print("The answer is: %d" % get_answer())
    print("The formatted code: %s" % format_code("f ( a = 1, b = 2 )")[0])
