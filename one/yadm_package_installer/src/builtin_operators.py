from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from typing import Any

import sh

import one.yadm_package_installer.src.operators_ast


def config_setting(operators: "list", name: "str", values: "dict[str, str]"):
    operators += [
        one.yadm_package_installer.src.operators_ast.ConfigSettingOperator(
            name, values)
    ]


def select(condition: "dict[str, Any]"):
    return one.yadm_package_installer.src.operators_ast.SelectMacro(condition)


def apt_install(operators: "list",
                name: "str",
                packages: "list[str]" = [],
                deps: "list[str]" = []):
    operators += [
        one.yadm_package_installer.src.operators_ast.AptInstallOperator(
            name, deps, packages)
    ]


def bash_exec(operators: "list",
              name: "str",
              commands: "list[str]" = [],
              sudo: "bool" = False,
              deps: "list[str]" = []):
    operators += [
        one.yadm_package_installer.src.operators_ast.BashExecOperator(
            name, deps, commands, sudo)
    ]
