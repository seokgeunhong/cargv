Build `sunriset` with bazel
=====

This project is written using [bazel](http://www.bazel.build/) 0.10.0.

### Build binary

```bazel build //:sunriset```

Executable `sunriset` or `sunriset.exe` is found under `bazel-bin/`.

### Run all tests:

```bazel test //:test-all```
