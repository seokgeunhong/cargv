
#include "sunriset_option.h"
#include "gtest/gtest.h"

#include <stdio.h>
#include <math.h>


#define _c(a)    (sizeof(a)/sizeof((a)[0]))
#define EXPECT_APPROX(a, b, delta) \
    EXPECT_LE(fabs((a)-(b)), (delta))


class Test_option : public testing::Test {
protected:
    virtual void SetUp()
    {
    }

    static const char *_cmd;
    static const cline_opt _opts[];

    struct sunriset_option_t option;
    int err;
};

const char *Test_option::_cmd = "sunriset";


TEST_F(Test_option, noarg)
{
    static const char *argv[] = {_cmd};
    ASSERT_EQ(parse_args(&option, _c(argv), (char **)argv), 0);
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
    EXPECT_EQ(option.position.latitude.deg, 37000000);
    EXPECT_EQ(option.position.latitude.min, 34000000);
    EXPECT_EQ(option.position.latitude.sec, 0);
    EXPECT_EQ(option.position.longitude.deg, 126000000);
    EXPECT_EQ(option.position.longitude.min, 58000000);
    EXPECT_EQ(option.position.longitude.sec, 0);
}

TEST_F(Test_option, opt_h)
{
    static const char *argv[] = {_cmd, "-h"};
    ASSERT_EQ(parse_args(&option, _c(argv), (char **)argv), 0);
    EXPECT_EQ(option.help, 1);
}

TEST_F(Test_option, opt_help)
{
    static const char *argv[] = {_cmd, "--help"};
    ASSERT_EQ(parse_args(&option, _c(argv), (char **)argv), 0);
    EXPECT_EQ(option.help, 1);
}

TEST_F(Test_option, opt_version)
{
    static const char *argv[] = {_cmd, "--version"};
    ASSERT_EQ(parse_args(&option, _c(argv), (char **)argv), 0);
    EXPECT_EQ(option.version, 1);
}

TEST_F(Test_option, opt_unknown)
{
    static const char *argv[] = {_cmd, "-h", "--babo"};

    ASSERT_EQ(parse_args(&option, _c(argv), (char **)argv), CLINE_ERR_OPTION);
}

TEST_F(Test_option, opt_date)
{
    static const char *argv[] = {_cmd, "--date", "1976-6-17"};

    ASSERT_EQ(parse_args(&option, _c(argv), (char **)argv), 0);
    EXPECT_EQ(option.date.year, 1976);
    EXPECT_EQ(option.date.month, 6);
    EXPECT_EQ(option.date.day, 17);
}

TEST_F(Test_option, opt_date_override)
{
    static const char *argv[] = {_cmd, "--date", "1976-6-17", "--year", "2018"};

    ASSERT_EQ(parse_args(&option, _c(argv), (char **)argv), 0);
    EXPECT_EQ(option.date.year, 2018);
    EXPECT_EQ(option.date.month, 6);
    EXPECT_EQ(option.date.day, 17);
}

TEST_F(Test_option, opt_date_wrong)
{
    static const char *argv[] = {_cmd, "--date", "1976-6--17"};

    ASSERT_EQ(parse_args(&option, _c(argv), (char **)argv), CLINE_ERR_VALUE);
}

TEST_F(Test_option, opt_date_missing)
{
    static const char *argv[] = {_cmd, "--date"};
    ASSERT_EQ(parse_args(&option, _c(argv), (char **)argv), CLINE_ERR_VALUE_REQUIRED);
}

TEST_F(Test_option, opt_pos)
{
    static const char *argv[] = {_cmd, "--position", "-18.333333+37.56667"};

    ASSERT_EQ(parse_args(&option, _c(argv), (char **)argv), 0);
    EXPECT_APPROX(cline_get_degree(&option.position.latitude), -18.333333, 0.000001);
    EXPECT_APPROX(cline_get_degree(&option.position.longitude), 37.56667, 0.000001);
}
