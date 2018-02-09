
#include "cline.h"
#include "gtest/gtest.h"

#include <stdio.h>
#include <math.h>


#define _c(a)    (sizeof(a)/sizeof((a)[0]))
#define EXPECT_APPROX(a, b, delta) \
    EXPECT_LE(fabs((a)-(b)), (delta))


class Test_cline : public testing::Test {
protected:
    virtual void SetUp()
    {
    }

    static char *_cmd;
    static const cline_opt _opts[];

    const cline_opt *opt;
    const char *name;
    struct cline_value val;
};

char *Test_cline::_cmd = "cline_test";

const cline_opt Test_cline::_opts[] = {
    {"-h\0--help\0",        CLINE_BOOL      },
    {"-v\0--verbose\0",     CLINE_BOOL      },
    {"-n\0--integer\0",     CLINE_INTEGER   },
    {"--text\0",            CLINE_TEXT      },
    {"--latitude\0",        CLINE_LATITUDE  },
    {"--date\0",            CLINE_DATE      },
};


TEST_F(Test_cline, noarg)
{
    static char *argv[] = {_cmd};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    for (int i = 0; i < 10; ++i)
        EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_END);
}

TEST_F(Test_cline, opt_flag1)
{
    static char *argv[] = {_cmd, "-h"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "-h");
}

TEST_F(Test_cline, opt_flag2)
{
    static char *argv[] = {_cmd, "--help"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "--help");
}

TEST_F(Test_cline, opt_unknown1)
{
    static char *argv[] = {_cmd, "-help"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);
    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_ERR_OPTION);
}

TEST_F(Test_cline, opt_unknown2)
{
    static char *argv[] = {_cmd, "-h", "--babo"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);
    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_ERR_OPTION);
}

TEST_F(Test_cline, opt_val_missing)
{
    static char *argv[] = {_cmd, "-n"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_ERR_VALUE_REQUIRED);
    EXPECT_STREQ(name, "-n");
    EXPECT_STREQ(val.arg, NULL);
    EXPECT_EQ(val.integer, 0);
}

TEST_F(Test_cline, opt_text)
{
    static char *argv[] = {_cmd,
        "--text", "abc",
        "--text", "--",
        "-v",
    };

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "--text");
    EXPECT_STREQ(val.arg, "abc");
    EXPECT_STREQ(val.text, "abc");

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "--text");
    EXPECT_STREQ(val.arg, "--");
    EXPECT_STREQ(val.text, "--");

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "-v");

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_END);
}

TEST_F(Test_cline, opt_int)
{
    static char *argv[] = {_cmd, "-n", "32"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "-n");
    EXPECT_STREQ(val.arg, "32");
    EXPECT_EQ(val.integer, 32);
}

TEST_F(Test_cline, opt_int_err)
{
    static char *argv[] = {_cmd, "-n", "32.333"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_ERR_VALUE);
    EXPECT_STREQ(name, "-n");
    EXPECT_STREQ(val.arg, "32.333");
}

TEST_F(Test_cline, opt_date)
{
    static char *argv[] = {_cmd,
        "--date", "1976-06-017",
        "--date", "1976/6-17",
    };

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "--date");
    EXPECT_STREQ(val.arg, "1976-06-017");
    EXPECT_EQ(val.date.year, 1976);
    EXPECT_EQ(val.date.month, 6);
    EXPECT_EQ(val.date.day, 17);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "--date");
    EXPECT_STREQ(val.arg, "1976/6-17");
    EXPECT_EQ(val.date.year, 1976);
    EXPECT_EQ(val.date.month, 6);
    EXPECT_EQ(val.date.day, 17);
}

TEST_F(Test_cline, opt_date_err)
{
    static char *argv[] = {_cmd,
        "--date", "1976-6--17",
        "--date", "-1976-6-17",
        "--date", "1976-6/-17",
        "--date", "1976-6-17-",
    };

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_ERR_VALUE);
    EXPECT_STREQ(name, "--date");
    EXPECT_STREQ(val.arg, "1976-6--17");

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_ERR_VALUE);
    EXPECT_STREQ(name, "--date");
    EXPECT_STREQ(val.arg, "-1976-6-17");

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_ERR_VALUE);
    EXPECT_STREQ(name, "--date");
    EXPECT_STREQ(val.arg, "1976-6/-17");

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_ERR_VALUE);
    EXPECT_STREQ(name, "--date");
    EXPECT_STREQ(val.arg, "1976-6-17-");
}

TEST_F(Test_cline, opt_latitude_iso)
{
    static char *argv[] = {_cmd,
        "--latitude", "+32.3957",
        "--latitude", "-18.933333",
        "--latitude", "+3239.57",
        "--latitude", "-1833.3334",
        "--latitude", "-183333.334",
    };

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "--latitude");
    EXPECT_STREQ(val.arg, "+32.3957");
    EXPECT_EQ(val.latitude.deg, 32395700);
    EXPECT_EQ(val.latitude.min, 0);
    EXPECT_EQ(val.latitude.sec, 0);
    EXPECT_EQ(cline_get_latitude_degree(&val.latitude), 32.3957);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "--latitude");
    EXPECT_STREQ(val.arg, "-18.933333");
    EXPECT_EQ(val.latitude.deg, -18933333);
    EXPECT_EQ(val.latitude.min, 0);
    EXPECT_EQ(val.latitude.sec, 0);
    EXPECT_APPROX(cline_get_latitude_degree(&val.latitude), -18.933333, 0.000001);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "--latitude");
    EXPECT_STREQ(val.arg, "+3239.57");
    EXPECT_EQ(val.latitude.deg, 32000000);
    EXPECT_EQ(val.latitude.min, 395700);
    EXPECT_EQ(val.latitude.sec, 0);
    EXPECT_APPROX(cline_get_latitude_degree(&val.latitude), 32.6595, 0.0001);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "--latitude");
    EXPECT_STREQ(val.arg, "-1833.3334");
    EXPECT_EQ(val.latitude.deg, -18000000);
    EXPECT_EQ(val.latitude.min, -333334);
    EXPECT_EQ(val.latitude.sec, 0);
    EXPECT_APPROX(cline_get_latitude_degree(&val.latitude), -18.555556, 0.000001);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "--latitude");
    EXPECT_STREQ(val.arg, "-183333.334");
    EXPECT_EQ(val.latitude.deg, -18000000);
    EXPECT_EQ(val.latitude.min, -330000);
    EXPECT_EQ(val.latitude.sec, -3333);
    EXPECT_APPROX(cline_get_latitude_degree(&val.latitude), -18.559258, 0.000001);
}

TEST_F(Test_cline, sep)
{
    static char *argv[] = {_cmd, "-h", "--", "-v", "abc"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "-h");
    EXPECT_STREQ(val.arg, NULL);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_END);

    // EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    // EXPECT_EQ(opt, nullptr);
    // EXPECT_STREQ(name, NULL);
    // EXPECT_STREQ(val, "-v");

    // EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    // EXPECT_EQ(opt, nullptr);
    // EXPECT_STREQ(name, NULL);
    // EXPECT_STREQ(val, "abc");

    // EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_END);
}

TEST_F(Test_cline, no_sep)
{
    static char *argv[] = {_cmd, "-h", "abc", "-v", "123"};

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "-h");
    EXPECT_STREQ(val.arg, NULL);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_END);

    // EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    // EXPECT_EQ(opt, nullptr);
    // EXPECT_STREQ(name, NULL);
    // EXPECT_STREQ(val, "abc");

    // EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    // EXPECT_EQ(opt, nullptr);
    // EXPECT_STREQ(name, NULL);
    // EXPECT_STREQ(val, "-v");

    // EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    // EXPECT_EQ(opt, nullptr);
    // EXPECT_STREQ(name, NULL);
    // EXPECT_STREQ(val, "123");

    // EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_END);
}
