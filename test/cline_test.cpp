
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
    {"--date\0",            CLINE_DATE      },
    {"--geo\0",             CLINE_GEOCOORD  },
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

TEST_F(Test_cline, opt_geocoord_iso6709)
{
    static char *argv[] = {_cmd,
        "--geo", "+32.3957+132.3957",
        "--geo", "-18.933333-13239.5/",
        "--geo", "+3239.57+0793333.334/",
        "--geo", "+32+9103",
        "--geo", "+32.3-0",
    };

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "--geo");
    EXPECT_STREQ(val.arg, "+32.3957+132.3957");
    EXPECT_EQ(val.geocoord.latitude.deg, 32395700);
    EXPECT_EQ(val.geocoord.latitude.min, 0);
    EXPECT_EQ(val.geocoord.latitude.sec, 0);
    EXPECT_EQ(cline_get_degree(&val.geocoord.latitude), 32.3957);
    EXPECT_EQ(val.geocoord.longitude.deg, 132395700);
    EXPECT_EQ(val.geocoord.longitude.min, 0);
    EXPECT_EQ(val.geocoord.longitude.sec, 0);
    EXPECT_EQ(cline_get_degree(&val.geocoord.longitude), 132.3957);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "--geo");
    EXPECT_STREQ(val.arg, "-18.933333-13239.5/");
    EXPECT_EQ(val.geocoord.latitude.deg, -18933333);
    EXPECT_EQ(val.geocoord.latitude.min, 0);
    EXPECT_EQ(val.geocoord.latitude.sec, 0);
    EXPECT_EQ(cline_get_degree(&val.geocoord.latitude), -18.933333);
    EXPECT_EQ(val.geocoord.longitude.deg, -132000000);
    EXPECT_EQ(val.geocoord.longitude.min, -39500000);
    EXPECT_EQ(val.geocoord.longitude.sec, 0);
    EXPECT_APPROX(cline_get_degree(&val.geocoord.longitude), -132.658333, 0.000001);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "--geo");
    EXPECT_STREQ(val.arg, "+3239.57+0793333.334/");
    EXPECT_EQ(val.geocoord.latitude.deg, 32000000);
    EXPECT_EQ(val.geocoord.latitude.min, 39570000);
    EXPECT_EQ(val.geocoord.latitude.sec, 0);
    EXPECT_APPROX(cline_get_degree(&val.geocoord.latitude), 32.6595, 0.000001);
    EXPECT_EQ(val.geocoord.longitude.deg, 79000000);
    EXPECT_EQ(val.geocoord.longitude.min, 33000000);
    EXPECT_EQ(val.geocoord.longitude.sec, 33330000);
    EXPECT_APPROX(cline_get_degree(&val.geocoord.longitude), 79.559259, 0.000001);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "--geo");
    EXPECT_STREQ(val.arg, "+32+9103");
    EXPECT_EQ(val.geocoord.latitude.deg, 32000000);
    EXPECT_EQ(val.geocoord.latitude.min, 0);
    EXPECT_EQ(val.geocoord.latitude.sec, 0);
    EXPECT_EQ(cline_get_degree(&val.geocoord.latitude), 32.0);
    EXPECT_EQ(val.geocoord.longitude.deg, 91000000);
    EXPECT_EQ(val.geocoord.longitude.min,  3000000);
    EXPECT_EQ(val.geocoord.longitude.sec, 0);
    EXPECT_APPROX(cline_get_degree(&val.geocoord.longitude), 91.05, 0.000001);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_OK);
    EXPECT_STREQ(name, "--geo");
    EXPECT_STREQ(val.arg, "+32.3-0");
    EXPECT_EQ(val.geocoord.latitude.deg, 32300000);
    EXPECT_EQ(val.geocoord.latitude.min, 0);
    EXPECT_EQ(val.geocoord.latitude.sec, 0);
    EXPECT_EQ(cline_get_degree(&val.geocoord.latitude), 32.3);
    EXPECT_EQ(val.geocoord.longitude.deg, 0);
    EXPECT_EQ(val.geocoord.longitude.min, 0);
    EXPECT_EQ(val.geocoord.longitude.sec, 0);
    EXPECT_EQ(cline_get_degree(&val.geocoord.longitude), .0);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_END);
}

TEST_F(Test_cline, opt_geocoord_iso6709_err)
{
    static char *argv[] = {_cmd,
        "--geo", "+132.3957+32.3957",
        "--geo", "18.933333-13239.57",
        "--geo", "+32.390793333.334",
        "--geo", "+18.3333-13287.57",
        "--geo", "+18.3333-232.57",
    };

    cline_parser parser;
    ASSERT_EQ(cline_init(&parser, _c(_opts), _opts, _c(argv), argv), CLINE_OK);

    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_ERR_VALUE);
    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_ERR_VALUE);
    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_ERR_VALUE);
    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_ERR_VALUE);
    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_ERR_VALUE);
    EXPECT_EQ(cline_read(&parser, &opt, &name, &val), CLINE_END);
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
