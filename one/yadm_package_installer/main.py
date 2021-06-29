import ast
import functools
import pathlib

from absl import app
from absl import flags
from absl import logging

import one.yadm_package_installer.src.builtin_operators

FLAGS = flags.FLAGS

flags.DEFINE_string("input", None, "The input compile_commands.json file path.")

flags.mark_flag_as_required("input")


def main(argv):
    del argv  # Not used.

    description_file_path = pathlib.Path(FLAGS.input)
    with description_file_path.open("r") as f:
        s = ast.parse(f.read())
    # TODO(zhangshuai.ustc): verify the ast only contains the constants declaration & function calls
    logging.debug(ast.dump(s))
    f = compile(s, FLAGS.input, "exec")

    operators = []
    eval(
        f, None, {
            "config_setting":
                functools.partial(
                    one.yadm_package_installer.src.builtin_operators.
                    config_setting, operators),
            "select":
                one.yadm_package_installer.src.builtin_operators.select,
            "apt_install":
                functools.partial(
                    one.yadm_package_installer.src.builtin_operators.
                    apt_install, operators),
            "bash_exec":
                functools.partial(
                    one.yadm_package_installer.src.builtin_operators.bash_exec,
                    operators)
        })
    logging.info("operators=%s", operators)


if __name__ == '__main__':
    app.run(main)
