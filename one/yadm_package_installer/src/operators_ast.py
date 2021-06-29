from __future__ import division
from __future__ import print_function
from __future__ import absolute_import

from typing import Any
from typing import Union

StrListT = Union["SelectMacro", "list[str]"]


def format_string_list(l: "StrListT"):
    if (isinstance(l, SelectMacro)):
        return "Select"

    if len(l) == 0:
        return ""

    return "\"" + "\" \"".join(l) + "\""


def format_string_any_dict(d: "dict[str, Any]"):
    # TODO(zhangshuai.ustc): implement it.
    return d.__repr__()


class Operator:
    """An interface for all operators."""
    def __init__(self, name: "str"):
        self.name = name

    def __repr__(self):
        return "(\"Operator\" (\"name\" %s))" % self.name


class Macro:
    """An interface for all macros."""
    pass


class ConfigSettingOperator(Operator):
    """ConfigSetting AST node."""
    def __init__(self, name: "str", values: "dict[str, Any]"):
        super(ConfigSettingOperator, self).__init__(name)
        self.values = values

    def __repr__(self):
        return "(\"ConfigSettingOperator\" (\"name\" %s) (\"values\" [%s]))" % (
            self.name, format_string_any_dict(self.values))


class SelectMacro(Macro):
    """Select macro AST node."""
    def __init__(self, condition: "dict[str, Any]"):
        super(Macro, self).__init__()
        self.condition = condition


class ActionOperatorBase(Operator):
    """The base definition for action operators."""
    def __init__(self, name: "str", deps: StrListT):
        super(ActionOperatorBase, self).__init__(name)
        self.deps = deps

    def __repr__(self):
        return "(\"ActionOperatorBase\" (\"name\" %s) (\"deps\" [%s]))" % (
            self.name, format_string_list(self.deps))


class AptInstallOperator(ActionOperatorBase):
    """An AST node for apt install."""
    def __init__(self, name: "str", deps: StrListT, packages: StrListT):
        super(AptInstallOperator, self).__init__(name, deps)
        self.packages = packages

    def __repr__(self):
        return "(\"AptInstallOperator\" (\"name\" %s) (\"deps\" [%s]) (\"packages\" [%s]))" % (
            self.name, format_string_list(
                self.deps), format_string_list(self.packages))


class BashExecOperator(ActionOperatorBase):
    """An AST node for bash execution."""
    def __init__(self, name: "str", deps: StrListT, commands: StrListT,
                 sudo: "bool"):
        super(BashExecOperator, self).__init__(name, deps)
        self.commands = commands
        self.sudo = sudo

    def __repr__(self):
        return "(\"BashExecOperator\" (\"name\" %s) (\"deps\" [%s]) (\"commands\" [%s]) (\"sudo\" %s))" % (
            self.name, format_string_list(
                self.deps), format_string_list(self.commands), self.sudo)
