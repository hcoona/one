from __future__ import division
from __future__ import print_function
from __future__ import absolute_import

import ast

from absl import logging
from absl.testing import absltest
from rules_python.python.runfiles import runfiles


class LoaderTest(absltest.TestCase):
    def test_smoke_test_01(self):
        r = runfiles.Create()
        with open(
                r.Rlocation(
                    "com_github_hcoona_one/one/yadm_package_installer/test/smoke_test_01.sky"
                ), "r") as f:
            contents = f.read()
        s = ast.parse(contents, "smoke_test_01.sky")
        logging.info(ast.dump(s))
        f = compile(s, "smoke_test_01.sky", "exec")

        apt_install_called = False
        global_called_list = []

        def _apt_install(name, packages, deps=[]):
            logging.info("name=%s, packages=%s", name, packages)
            nonlocal apt_install_called
            nonlocal global_called_list
            apt_install_called = True
            global_called_list += [{
                "type": "apt_install",
                "name": name,
                "packages": packages,
                "deps": deps,
            }]

        eval(f, None, {
            "global_called_list": global_called_list,
            "apt_install": _apt_install
        })

        logging.info("%s", global_called_list)

        self.assertTrue(apt_install_called)


if __name__ == '__main__':
    absltest.main()
