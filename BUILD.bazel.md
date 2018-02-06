Build `sunriset` with bazel
===========================

### Run all tests:

```bazel test //:test```

### Build command line executable:

```bazel build //:main```

### Build static library:

```bazel build //:lib```

### Available targets:

* `//:main` - executable binary, `sunriset` or `sunriset.exe`
* `//:lib` - static library, `sunriset.a` or `sunriset.lib`
* `//:test` - test-all
