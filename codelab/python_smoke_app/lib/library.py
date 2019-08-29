"""Example Python Library
"""
from yapf.yapflib.yapf_api import FormatCode


def get_answer():
    return 42


def format_code(s):
    return FormatCode(s)
