import io
import os
import pathlib

import yaml
from absl import app
from absl import flags
from absl import logging
from jinja2 import Environment
from jinja2 import FileSystemLoader
from jinja2 import select_autoescape
from rules_python.python.runfiles import runfiles

FLAGS = flags.FLAGS

flags.DEFINE_string("input", None, "The input enum yaml file path.")
flags.DEFINE_string("output_dir", None,
                    "The output generated files base directory.")

flags.mark_flag_as_required("input")
flags.mark_flag_as_required("output_dir")


def main(argv):
    del argv  # Not used.

    logging.get_absl_handler().use_absl_log_file()

    runfile = runfiles.Create()
    env = Environment(loader=FileSystemLoader([
        os.path.dirname(
            runfile.Rlocation(
                "com_github_hcoona_one/one/codegen/enum_cpp_template.cc.jinja")),
        os.path.dirname(
            runfile.Rlocation(
                "com_github_hcoona_one/one/codegen/enum_cpp_template.h.jinja")),
    ]),
                      autoescape=select_autoescape())

    impl_template = env.get_template("enum_cpp_template.cc.jinja")
    header_template = env.get_template("enum_cpp_template.h.jinja")

    with io.open(FLAGS.input, "r", encoding="utf-8") as input_file:
        context = yaml.safe_load(input_file)

    output_directory_path = pathlib.Path(FLAGS.output_dir)
    output_directory_path.mkdir(parents=True, exist_ok=True)
    header_template.stream(context).dump(str(output_directory_path / "a.h"))


if __name__ == '__main__':
    app.run(main)
