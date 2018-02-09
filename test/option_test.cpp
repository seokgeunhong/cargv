
#include "sunriset_option.h"
#include "gtest/gtest.h"

#include <stdio.h>


#define _c(a)    (sizeof(a)/sizeof((a)[0]))
#define EXPECT_APPROX(a, b, delta) \
    EXPECT_LE(fabs((a)-(b)), (delta))


class Test_option : public testing::Test {
protected:
    virtual void SetUp()
    {
    }

    static char *_cmd;
    static const cline_opt _opts[];

    struct sunriset_option_t option;
    int err;
};

char *Test_option::_cmd = "sunriset";


TEST_F(Test_option, noarg)
{
    static char *argv[] = {_cmd};
    ASSERT_EQ(parse_args(&option, _c(argv), argv), 0);
    EXPECT_EQ(option.version, 0);
    EXPECT_EQ(option.help, 0);

    // today
    time_t now;
    time(&now);
    tm *t = localtime(&now);
    EXPECT_EQ(option.date.year, t->tm_year + 1900);
    EXPECT_EQ(option.date.month, t->tm_mon + 1);
    EXPECT_EQ(option.date.day, t->tm_mday);

    // Seoul
    EXPECT_EQ(option.latitude.deg, 37000000);
    EXPECT_EQ(option.latitude.min, 340000);
    EXPECT_EQ(option.latitude.sec, 0);
}

TEST_F(Test_option, opt_h)
{
    static char *argv[] = {_cmd, "-h"};
    ASSERT_EQ(parse_args(&option, _c(argv), argv), 0);
    EXPECT_EQ(option.help, 1);
}

TEST_F(Test_option, opt_help)
{
    static char *argv[] = {_cmd, "--help"};
    ASSERT_EQ(parse_args(&option, _c(argv), argv), 0);
    EXPECT_EQ(option.help, 1);
}

TEST_F(Test_option, opt_version)
{
    static char *argv[] = {_cmd, "--version"};
    ASSERT_EQ(parse_args(&option, _c(argv), argv), 0);
    EXPECT_EQ(option.version, 1);
}

TEST_F(Test_option, opt_unknown)
{
    static char *argv[] = {_cmd, "-h", "--babo"};
    cline_parser parser;
    ASSERT_EQ(parse_args(&option, _c(argv), argv), CLINE_ERR_OPTION);
}

TEST_F(Test_option, opt_date)
{
    static char *argv[] = {_cmd, "--date", "1976-6-17"};

    ASSERT_EQ(parse_args(&option, _c(argv), argv), 0);
    EXPECT_EQ(option.date.year, 1976);
    EXPECT_EQ(option.date.month, 6);
    EXPECT_EQ(option.date.day, 17);
}

TEST_F(Test_option, opt_date_override)
{
    static char *argv[] = {_cmd, "--date", "1976-6-17", "--year", "2018"};

    ASSERT_EQ(parse_args(&option, _c(argv), argv), 0);
    EXPECT_EQ(option.date.year, 2018);
    EXPECT_EQ(option.date.month, 6);
    EXPECT_EQ(option.date.day, 17);
}

TEST_F(Test_option, opt_date_wrong)
{
    static char *argv[] = {_cmd, "--date", "1976-6--17"};

    ASSERT_EQ(parse_args(&option, _c(argv), argv), CLINE_ERR_VALUE);
}

TEST_F(Test_option, opt_date_missing)
{
    static char *argv[] = {_cmd, "--date"};
    ASSERT_EQ(parse_args(&option, _c(argv), argv), CLINE_ERR_VALUE_REQUIRED);
}

TEST_F(Test_option, opt_latitude)
{
    static char *argv[] = {_cmd, "--latitude", "-18.333333"};

    ASSERT_EQ(parse_args(&option, _c(argv), argv), 0);
    EXPECT_APPROX(cline_get_latitude_degree(&option.latitude), -18.333333, 0.000001);
}

TEST_F(Test_option, opt_latitude_wrong)
{
    static char *argv[] = {_cmd, "--latitude", "-18.3333-"};
    ASSERT_EQ(parse_args(&option, _c(argv), argv), CLINE_ERR_VALUE);
}

TEST_F(Test_option, opt_latitude_missing)
{
    static char *argv[] = {_cmd, "--latitude"};
    ASSERT_EQ(parse_args(&option, _c(argv), argv), CLINE_ERR_VALUE_REQUIRED);
}
