# `sunriset` workspace
# ====================

# [googletest](https://github.com/google/googletest/)
# ------------
#
# While `http_archive()` is recommended over `git_repository()` in the guide,
# the lastest release, 1.8.0 on Aug 2016, contains no bazel script, and have
# build problem between mingw and pthread.
#
# Both problems are fixed after a year from the last release, and it looks
# like that they're busy doing some refactoring[1][], so;
#
# [1]: https://github.com/google/googletest/projects/1

# This could be `master`, 'release-1.18.0', or just a commit sha1.
commit='cac5d7ce1a50c5c890a0b20fcaddd2360ec10f8f'   # Jan 11 2018

http_archive(
    name='com_google_googletest',
    url='https://github.com/google/googletest/archive/'+commit+'.zip',
    strip_prefix='googletest-'+commit,
)
