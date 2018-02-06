
#include "cline.h"
#include "gtest/gtest.h"

#include <stdio.h>


#define _c(a)    (sizeof(a)/sizeof((a)[0]))


class Test_cline : public testing::Test {
protected:
    virtual void SetUp()
    {
    }

    static char *_cmd;
    static const cline_opt _opts[];

    const cline_opt *opt;
    const char *name;
    const char *val;
};

char *Test_cline::_cmd = "cline_test";

const cline_opt Test_cline::_opts[] = {
    {"-h\0--help\0",       0},
    {"-v\0--verbose\0",    0},
    {"-x\0--longitude\0",  1},
    {"-y\0--latitude\0",   1},
    {"-z\0--altitude\0",   1},
};


TEST_F(Test_cline, noarg)
{
    static char *argv[] = {_cmd};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    for (int i = 0; i < 10; ++i)
        EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_DONE);
}

TEST_F(Test_cline, opt_h)
{
    static char *argv[] = {_cmd, "-h"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_EQ(opt, &_opts[0]);
    EXPECT_STREQ(name, "-h");
    EXPECT_STREQ(val, "");
}

TEST_F(Test_cline, opt_help)
{
    static char *argv[] = {_cmd, "--help"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_EQ(opt, &_opts[0]);
    EXPECT_STREQ(name, "--help");
    EXPECT_STREQ(val, "");
}

TEST_F(Test_cline, opt_unknown1)
{
    static char *argv[] = {_cmd, "-help"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);
    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_ERR_BAD_OPTION);
}

TEST_F(Test_cline, opt_unknown2)
{
    static char *argv[] = {_cmd, "-h", "--babo"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);
    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_ERR_BAD_OPTION);
}

TEST_F(Test_cline, opt_x)
{
    static char *argv[] = {_cmd, "-x", "32.3957"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_EQ(opt, &_opts[2]);
    EXPECT_STREQ(name, "-x");
    EXPECT_STREQ(val, "32.3957");
}

TEST_F(Test_cline, opt_x_err)
{
    static char *argv[] = {_cmd, "-x"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_ERR_VAL_MISSING);
    EXPECT_EQ(opt, &_opts[2]);
    EXPECT_STREQ(name, "-x");
    EXPECT_STREQ(val, "");
}

TEST_F(Test_cline, sep)
{
    static char *argv[] = {_cmd, "-h", "--", "-v", "abc"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_EQ(opt, &_opts[0]);
    EXPECT_STREQ(name, "-h");
    EXPECT_STREQ(val, "");

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_DONE);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_EQ(opt, nullptr);
    EXPECT_STREQ(name, "");
    EXPECT_STREQ(val, "-v");

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_EQ(opt, nullptr);
    EXPECT_STREQ(name, "");
    EXPECT_STREQ(val, "abc");

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_DONE);
}

TEST_F(Test_cline, no_sep)
{
    static char *argv[] = {_cmd, "-h", "abc", "-v", "123"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_EQ(opt, &_opts[0]);
    EXPECT_STREQ(name, "-h");
    EXPECT_STREQ(val, "");

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_DONE);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_EQ(opt, nullptr);
    EXPECT_STREQ(name, "");
    EXPECT_STREQ(val, "abc");

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_EQ(opt, nullptr);
    EXPECT_STREQ(name, "");
    EXPECT_STREQ(val, "-v");

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_EQ(opt, nullptr);
    EXPECT_STREQ(name, "");
    EXPECT_STREQ(val, "123");

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_DONE);
}

TEST_F(Test_cline, sep_as_val)
{
    static char *argv[] = {_cmd, "-x", "--", "-v"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_EQ(opt, &_opts[2]);
    EXPECT_STREQ(name, "-x");
    EXPECT_STREQ(val, "--");

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_EQ(opt, &_opts[1]);
    EXPECT_STREQ(name, "-v");
    EXPECT_STREQ(val, "");

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_DONE);
    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_DONE);
}
