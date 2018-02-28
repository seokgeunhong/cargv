Build `cargv` with bazel
=====

This project is written using [bazel](http://www.bazel.build/) 0.10.0.

## Build static library:

```bazel build //:static```

`libcargv.a` or `cargv.lib` is found under `bazel-bin/`.

## Run all tests:

```bazel test //:test-all```
