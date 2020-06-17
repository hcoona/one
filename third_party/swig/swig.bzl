# Ported from https://github.com/tensorflow/tensorflow/blob/v1.14.0/tensorflow/tensorflow.bzl
# Changes:
# 1. Remove tensorflow specific flags, etc.
# 2. Remove the `Lib` folder from swig_include_dirs.
# 3. Expose raw python swig genrule.
# 4. Add Java swig rule.

##### Utilities #####

load("@rules_java//java:defs.bzl", "java_library")
load("@rules_cc//cc:defs.bzl", "cc_binary")

def _get_repository_roots(ctx, files):
    """Returns abnormal root directories under which files reside.

    When running a ctx.action, source files within the main repository are all
    relative to the current directory; however, files that are generated or exist
    in remote repositories will have their root directory be a subdirectory,
    e.g. bazel-out/local-fastbuild/genfiles/external/jpeg_archive. This function
    returns the set of these devious directories, ranked and sorted by popularity
    in order to hopefully minimize the number of I/O system calls within the
    compiler, because includes have quadratic complexity.
    """
    result = {}
    for f in files.to_list():
        root = f.root.path
        if root:
            if root not in result:
                result[root] = 0
            result[root] -= 1
        work = f.owner.workspace_root
        if work:
            if root:
                root += "/"
            root += work
        if root:
            if root not in result:
                result[root] = 0
            result[root] -= 1
    return [k for v, k in sorted([(v, k) for k, v in result.items()])]

def _get_transitive_headers(hdrs, deps):
    """Obtain the header files for a target and its transitive dependencies.

    Args:
      hdrs: a list of header files
      deps: a list of targets that are direct dependencies

    Returns:
      a collection of the transitive headers
    """
    return depset(
        hdrs,
        transitive = [dep[CcInfo].compilation_context.headers for dep in deps],
    )

##### Python Rules #####

# Bazel rules for building swig files.
def _py_wrap_cc_impl(ctx):
    srcs = ctx.files.srcs
    if len(srcs) != 1:
        fail("Exactly one SWIG source file label must be specified.", "srcs")
    module_name = ctx.attr.module_name
    src = ctx.files.srcs[0]
    inputs = _get_transitive_headers([src] + ctx.files.swig_includes, ctx.attr.deps)
    inputs = depset(ctx.files._swiglib, transitive = [inputs])
    inputs = depset(ctx.files.toolchain_deps, transitive = [inputs])
    swig_include_dirs = depset(_get_repository_roots(ctx, inputs))
    swig_include_dirs = depset(
        [d for d in sorted([f.dirname for f in ctx.files._swiglib]) if not d.endswith("Lib")],
        transitive = [swig_include_dirs],
    )
    args = [
        "-c++",
        "-python",
        "-module",
        module_name,
        "-o",
        ctx.outputs.cc_out.path,
        "-outdir",
        ctx.outputs.py_out.dirname,
    ]
    args += ["-l" + f.path for f in ctx.files.swig_includes]
    args += ["-I" + i for i in swig_include_dirs.to_list()]
    args += [src.path]
    outputs = [ctx.outputs.cc_out, ctx.outputs.py_out]
    ctx.actions.run(
        executable = ctx.executable._swig,
        arguments = args,
        inputs = inputs.to_list(),
        outputs = outputs,
        mnemonic = "PythonSwig",
        progress_message = "SWIGing " + src.path,
    )
    return struct(files = depset(outputs))

_py_wrap_cc = rule(
    attrs = {
        "srcs": attr.label_list(
            mandatory = True,
            allow_files = True,
        ),
        "swig_includes": attr.label_list(
            allow_files = True,
        ),
        "deps": attr.label_list(
            allow_files = True,
            providers = [CcInfo],
        ),
        "toolchain_deps": attr.label_list(
            allow_files = True,
        ),
        "module_name": attr.string(mandatory = True),
        "py_module_name": attr.string(mandatory = True),
        "_swig": attr.label(
            default = Label("@swig//:swig"),
            executable = True,
            cfg = "host",
        ),
        "_swiglib": attr.label(
            default = Label("@swig//:python_templates"),
            allow_files = True,
        ),
    },
    outputs = {
        "cc_out": "%{module_name}.cc",
        "py_out": "%{py_module_name}.py",
    },
    implementation = _py_wrap_cc_impl,
)

def py_wrap_cc(
        name,
        srcs,
        swig_includes = [],
        deps = [],
        copts = [],
        version_script = None,
        **kwargs):
    """Builds a Python extension module."""
    module_name = name.split("/")[-1]

    # Convert a rule name such as foo/bar/baz to foo/bar/_baz.so
    # and use that as the name for the rule producing the .so file.
    cc_library_name = "/".join(name.split("/")[:-1] + ["_" + module_name + ".so"])
    extra_deps = []
    _py_wrap_cc(
        name = name + "_py_wrap",
        srcs = srcs,
        module_name = module_name,
        py_module_name = name,
        swig_includes = swig_includes,
        toolchain_deps = ["@bazel_tools//tools/cpp:current_cc_toolchain"],
        deps = deps + extra_deps,
    )
    cc_binary(
        name = cc_library_name,
        srcs = [module_name + ".cc"],
        copts = copts + [
            "-Wno-self-assign",
            "-Wno-sign-compare",
            "-Wno-write-strings",
        ],
        linkstatic = True,
        linkshared = True,
        deps = deps + extra_deps,
        **kwargs
    )
    native.py_library(
        name = name,
        srcs = [":" + name + ".py"],
        srcs_version = "PY2AND3",
        data = [":" + cc_library_name],
    )

##### Raw Python SWIG Rules #####

# Bazel rules for building swig files.
def _py_swig_genrule_impl(ctx):
    srcs = ctx.files.srcs
    if len(srcs) != 1:
        fail("Exactly one SWIG source file label must be specified.", "srcs")
    module_name = ctx.attr.module_name
    src = ctx.files.srcs[0]
    inputs = depset([src] + ctx.files.swig_includes)
    inputs = depset(ctx.files._swiglib, transitive = [inputs])
    inputs = depset(ctx.files.toolchain_deps, transitive = [inputs])
    swig_include_dirs = depset(_get_repository_roots(ctx, inputs))
    swig_include_dirs = depset(
        [d for d in sorted([f.dirname for f in ctx.files._swiglib]) if not d.endswith("Lib")],
        transitive = [swig_include_dirs],
    )
    args = [
        "-c++",
        "-python",
        "-module",
        module_name,
        "-o",
        ctx.outputs.cc_out.path,
        "-outdir",
        ctx.outputs.py_out.dirname,
    ]
    args += ["-l" + f.path for f in ctx.files.swig_includes]
    args += ["-I" + i for i in swig_include_dirs.to_list()]
    args += [src.path]
    outputs = [ctx.outputs.cc_out, ctx.outputs.py_out]
    ctx.actions.run(
        executable = ctx.executable._swig,
        arguments = args,
        inputs = inputs.to_list(),
        outputs = outputs,
        mnemonic = "PythonSwig",
        progress_message = "SWIGing " + src.path,
    )
    return struct(files = depset(outputs))

_py_swig_genrule = rule(
    attrs = {
        "srcs": attr.label_list(
            mandatory = True,
            allow_files = True,
        ),
        "swig_includes": attr.label_list(
            allow_files = True,
        ),
        "toolchain_deps": attr.label_list(
            allow_files = True,
        ),
        "module_name": attr.string(mandatory = True),
        "py_module_name": attr.string(mandatory = True),
        "cc_file_name": attr.string(),
        "_swig": attr.label(
            default = Label("@swig//:swig"),
            executable = True,
            cfg = "host",
        ),
        "_swiglib": attr.label(
            default = Label("@swig//:python_templates"),
            allow_files = True,
        ),
    },
    outputs = {
        "cc_out": "%{cc_file_name}",
        "py_out": "%{py_module_name}.py",
    },
    implementation = _py_swig_genrule_impl,
)

def py_swig_genrule(
        name,
        srcs,
        swig_includes = [],
        deps = [],
        module_name = None,
        cc_file_name = None,
        **kwargs):
    _py_swig_genrule(
        name = name,
        srcs = srcs,
        swig_includes = swig_includes,
        module_name = module_name,
        py_module_name = module_name,
        cc_file_name = cc_file_name,
    )

##### Java Rules #####

def _java_wrap_cc_impl(ctx):
    srcs = ctx.files.srcs
    if len(srcs) != 1:
        fail("Exactly one SWIG source file label must be specified.", "srcs")
    module_name = ctx.attr.module_name
    package_name = ctx.attr.package_name
    src = ctx.files.srcs[0]
    inputs = _get_transitive_headers([src] + ctx.files.swig_includes, ctx.attr.deps)
    inputs = depset(ctx.files._swiglib, transitive = [inputs])
    inputs = depset(ctx.files.toolchain_deps, transitive = [inputs])
    swig_include_dirs = depset(_get_repository_roots(ctx, inputs))
    swig_include_dirs = depset(
        [d for d in sorted([f.dirname for f in ctx.files._swiglib]) if not d.endswith("Lib")],
        transitive = [swig_include_dirs],
    )
    args = [
        "-c++",
        "-java",
        "-module",
        module_name,
        "-package",
        package_name,
        "-o",
        ctx.outputs.cc_out.path,
        "-outdir",
        ctx.outputs.java_out.dirname,
    ]
    args += ["-l" + f.path for f in ctx.files.swig_includes]
    args += ["-I" + i for i in swig_include_dirs.to_list()]
    args += [src.path]
    outputs = [
        ctx.outputs.cc_out,
        ctx.outputs.java_out,
        ctx.outputs.javajni_out,
    ] + ctx.outputs.java_outputs
    ctx.actions.run(
        executable = ctx.executable._swig,
        arguments = args,
        inputs = inputs.to_list(),
        outputs = outputs,
        mnemonic = "JavaSwig",
        progress_message = "SWIGing " + src.path,
    )
    return struct(files = depset(outputs))

_java_wrap_cc = rule(
    attrs = {
        "srcs": attr.label_list(
            mandatory = True,
            allow_files = True,
        ),
        "swig_includes": attr.label_list(
            allow_files = True,
        ),
        "deps": attr.label_list(
            allow_files = True,
            providers = [CcInfo],
        ),
        "java_outputs": attr.output_list(
            mandatory = True,
            allow_empty = False,
        ),
        "toolchain_deps": attr.label_list(
            allow_files = True,
        ),
        "module_name": attr.string(mandatory = True),
        "package_name": attr.string(mandatory = True),
        "java_module_name": attr.string(mandatory = True),
        "_swig": attr.label(
            default = Label("@swig//:swig"),
            executable = True,
            cfg = "host",
        ),
        "_swiglib": attr.label(
            default = Label("@swig//:java_templates"),
            allow_files = True,
        ),
    },
    outputs = {
        "cc_out": "%{module_name}.cc",
        "java_out": "%{java_module_name}.java",
        "javajni_out": "%{java_module_name}JNI.java",
    },
    implementation = _java_wrap_cc_impl,
)

def java_wrap_cc(
        name,
        package_name,
        srcs,
        swig_includes = [],
        deps = [],
        copts = [],
        java_outputs = [],
        **kwargs):
    """Builds a Python extension module."""
    module_name = name.split("/")[-1]

    # Convert a rule name such as foo/bar/baz to foo/bar/_baz.so
    # and use that as the name for the rule producing the .so file.
    cc_library_name = "/".join(name.split("/")[:-1] + ["_" + module_name + ".so"])
    _java_wrap_cc(
        name = name + "_java_wrap",
        srcs = srcs,
        module_name = module_name,
        package_name = package_name,
        java_module_name = name,
        swig_includes = swig_includes,
        toolchain_deps = ["@bazel_tools//tools/cpp:current_cc_toolchain"],
        deps = deps,
        java_outputs = java_outputs,
    )
    cc_binary(
        name = cc_library_name,
        srcs = [module_name + ".cc"],
        copts = copts + [
            "-Wno-self-assign",
            "-Wno-sign-compare",
            "-Wno-write-strings",
        ],
        linkstatic = True,
        linkshared = True,
        deps = deps,
        **kwargs
    )
    java_library(
        name = name,
        srcs = [
            ":" + name + ".java",
            ":" + name + "JNI.java",
        ] + java_outputs,
        data = [":" + cc_library_name],
    )
