
#include "cargv.h"
#include "gtest/gtest.h"

#include <stdio.h>
#include <math.h>


#define _c(a)    (sizeof(a)/sizeof((a)[0]))
#define EXPECT_APPROX(a, b, delta) \
    EXPECT_LE(fabs((a)-(b)), (delta))


class Test_cargv : public testing::Test {
protected:
    virtual void SetUp()
    {
    }

    static const char *_name;

    struct cargv_t cargv;
};

const char *Test_cargv::_name = "cargv-test";


TEST_F(Test_cargv, noarg)
{
    static const char *argv[] = {_name,};

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_opt(&cargv, "--help"), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, opt)
{
    static const char *argv[] = {_name, "-h", "--help"};

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_opt(&cargv, "-*"), 1);
    EXPECT_EQ(cargv_opt(&cargv, "-h"), 1);
    EXPECT_EQ(cargv_opt(&cargv, "-hp"), 1);
    EXPECT_EQ(cargv_opt(&cargv, "-ah"), 1);
    EXPECT_EQ(cargv_opt(&cargv, "-h--help"), 1);
    EXPECT_EQ(cargv_opt(&cargv, "--help"), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_opt(&cargv, "-*"), 1);
    EXPECT_EQ(cargv_opt(&cargv, "-hp"), 0);
    EXPECT_EQ(cargv_opt(&cargv, "--help"), 1);
    EXPECT_EQ(cargv_opt(&cargv, "-h--help--another-help"), 1);
    EXPECT_EQ(cargv_opt(&cargv, "--cookoo--help"), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_opt(&cargv, "-*"), 0);
    EXPECT_EQ(cargv_opt(&cargv, "-h--help"), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, opt_empty)
{
    static const char *argv[] = {_name, "-", "--",};

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_opt(&cargv, "-*"), 0);
    EXPECT_EQ(cargv_opt(&cargv, "-h--help"), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_opt(&cargv, "-*"), 0);
    EXPECT_EQ(cargv_opt(&cargv, "-h--help"), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, text)
{
    static const char *argv[] = {_name, "abc", "--text", "def"};
    const char *t[4];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_text(&cargv, "T", t, 1), 1);
    EXPECT_STREQ(t[0], "abc");
    EXPECT_EQ(cargv_text(&cargv, "T", t, 2), 2);
    EXPECT_STREQ(t[0], "abc");
    EXPECT_STREQ(t[1], "--text");
    EXPECT_EQ(cargv_text(&cargv, "T", t, 4), 3);
    EXPECT_STREQ(t[0], "abc");
    EXPECT_STREQ(t[1], "--text");
    EXPECT_STREQ(t[2], "def");
    EXPECT_EQ(t[3], nullptr);

    EXPECT_EQ(cargv_shift(&cargv, 3), 3);
    //EXPECT_EQ(cargv_text(&cargv, "T", t, 4), CARGV_VAL_MISSING);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, key)
{
    static const char *argv[] = {_name, "h", "help", ""};
    const char *v[4];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_key(&cargv, "KEY", "-", v, 4), 0);
    EXPECT_EQ(cargv_key(&cargv, "KEY", "-h", v, 4), 1);
    EXPECT_EQ(cargv_key(&cargv, "KEY", "-hp", v, 4), 1);
    EXPECT_EQ(cargv_key(&cargv, "KEY", "-ah", v, 4), 1);
    EXPECT_EQ(cargv_key(&cargv, "KEY", "-h--help", v, 4), 2);
    EXPECT_EQ(cargv_key(&cargv, "KEY", "--help", v, 4), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_key(&cargv, "KEY", "-hp", v, 4), 0);
    EXPECT_EQ(cargv_key(&cargv, "KEY", "--help", v, 4), 1);
    EXPECT_EQ(cargv_key(&cargv, "KEY", "-h--help--another-help", v, 4), 1);
    EXPECT_EQ(cargv_key(&cargv, "KEY", "--cookoo--help", v, 4), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_key(&cargv, "KEY", "-h--help", v, 4), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, sint)
{
    static const char *argv[] = {_name, "32", "-1", "+1000", "1a", "1"};
    cargv_int_t val[4];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_int(&cargv, "INT", val, 1), 1);
    EXPECT_EQ(val[0], 32);
    EXPECT_EQ(cargv_int(&cargv, "INT", val, 4), 3);
    EXPECT_EQ(val[0], 32);
    EXPECT_EQ(val[1], -1);
    EXPECT_EQ(val[2], 1000);
    EXPECT_EQ(val[3], 0);
    EXPECT_EQ(cargv_shift(&cargv, 4), 4);
    EXPECT_EQ(cargv_int(&cargv, "INT", val, 1), 1);
    EXPECT_EQ(val[0], 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, sint_overflow)
{
    static const char *argv[] = {_name,
        "9223372036854775807", "9223372036854775808",
        "-9223372036854775808", "-9223372036854775809",
    };
    cargv_int_t val[1];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_int(&cargv, "INT", val, 1), 1);
    EXPECT_EQ(val[0], 9223372036854775807);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_int(&cargv, "INT", val, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_int(&cargv, "INT", val, 1), 1);
    EXPECT_EQ(val[0], -9223372036854775808);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_int(&cargv, "INT", val, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, uint)
{
    static const char *argv[] = {_name, "32", "0", "32768", "1a"};
    cargv_uint_t val[4];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_uint(&cargv, "UINT", val, 1), 1);
    EXPECT_EQ(val[0], 32);
    EXPECT_EQ(cargv_uint(&cargv, "UINT", val, 4), 3);
    EXPECT_EQ(val[0], 32);
    EXPECT_EQ(val[1], 0);
    EXPECT_EQ(val[2], 32768);
    EXPECT_EQ(val[3], 0);
    EXPECT_EQ(cargv_shift(&cargv, 4), 4);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, uint_overflow)
{
    static const char *argv[] = {_name,
        "18446744073709551615", "18446744073709551616", "-100"
    };
    cargv_uint_t val[1];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_uint(&cargv, "UINT", val, 1), 1);
    EXPECT_EQ(val[0], 18446744073709551615);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_uint(&cargv, "UINT", val, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_uint(&cargv, "UINT", val, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, date)
{
    static const char *argv[] = {_name,
        "+2018-06-17", "2018/6/17", "-0001-12-24",
    };
    cargv_date_t v[4];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_date(&cargv, "DATE", v, 1), 1);
    EXPECT_EQ(v[0].year, 2018);
    EXPECT_EQ(v[0].month, 6);
    EXPECT_EQ(v[0].day, 17);
    EXPECT_EQ(cargv_date(&cargv, "DATE", v, 4), 3);
    EXPECT_EQ(v[1].year, 2018);
    EXPECT_EQ(v[1].month, 6);
    EXPECT_EQ(v[1].day, 17);
    EXPECT_EQ(v[2].year, -1);
    EXPECT_EQ(v[2].month, 12);
    EXPECT_EQ(v[2].day, 24);
    EXPECT_EQ(v[3].year, 0);
    EXPECT_EQ(v[3].month, 0);
    EXPECT_EQ(v[3].day, 0);
    EXPECT_EQ(cargv_shift(&cargv, 3), 3);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, date_err)
{
    static const char *argv[] = {_name,
        "0000-12-24", "2018-00-01", "2018-13-01", "2018-11-00",
        "2018-11--01", "2018-02-29",
    };
    cargv_date_t v[1];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_date(&cargv, "DATE", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_date(&cargv, "DATE", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_date(&cargv, "DATE", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_date(&cargv, "DATE", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_date(&cargv, "DATE", v, 1), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_date(&cargv, "DATE", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, degree)
{
    static const char *argv[] = {_name,
        "-32", "+9103", "+32.3957", "-13239.5", "+0793333.334", "-0",
    };
    cargv_degree_t v[6];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_degree(&cargv, "DEG", v, 6), 6);
    EXPECT_EQ(v[0].deg, -32000000);
    EXPECT_EQ(v[0].min, 0);
    EXPECT_EQ(v[0].sec, 0);
    EXPECT_EQ(v[1].deg, 91000000);
    EXPECT_EQ(v[1].min, 3000000);
    EXPECT_EQ(v[1].sec, 0);
    EXPECT_EQ(v[2].deg, 32395700);
    EXPECT_EQ(v[2].min, 0);
    EXPECT_EQ(v[2].sec, 0);
    EXPECT_EQ(v[3].deg, -132000000);
    EXPECT_EQ(v[3].min, -39500000);
    EXPECT_EQ(v[3].sec, 0);
    EXPECT_EQ(v[4].deg, 79000000);
    EXPECT_EQ(v[4].min, 33000000);
    EXPECT_EQ(v[4].sec, 33330000);
    EXPECT_EQ(v[5].deg, 0);
    EXPECT_EQ(v[5].min, 0);
    EXPECT_EQ(v[5].sec, 0);
    EXPECT_EQ(cargv_shift(&cargv, 6), 6);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, degree_err)
{
    static const char *argv[] = {_name,
        "-361", "+1163", "-112278.01", "13239.5", "0",
    };
    cargv_degree_t v[1];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_degree(&cargv, "DEG", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_degree(&cargv, "DEG", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_degree(&cargv, "DEG", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_degree(&cargv, "DEG", v, 1), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_degree(&cargv, "DEG", v, 1), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, geocoord)
{
    static const char *argv[] = {_name,
        "-32.3957+13239.57/",
        "+62.3-0",
    };
    cargv_geocoord_t v[5];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_geocoord(&cargv, "GEOCOORD", v, 2), 2);
    EXPECT_EQ(v[0].latitude.deg, -32395700);
    EXPECT_EQ(v[0].latitude.min, 0);
    EXPECT_EQ(v[0].latitude.sec, 0);
    EXPECT_EQ(v[0].longitude.deg, 132000000);
    EXPECT_EQ(v[0].longitude.min,  39570000);
    EXPECT_EQ(v[0].longitude.sec,  0);
    EXPECT_EQ(v[1].latitude.deg, 62300000);
    EXPECT_EQ(v[1].latitude.min, 0);
    EXPECT_EQ(v[1].latitude.sec, 0);
    EXPECT_EQ(v[1].longitude.deg, 0);
    EXPECT_EQ(v[1].longitude.min, 0);
    EXPECT_EQ(v[1].longitude.sec, 0);
    EXPECT_EQ(cargv_shift(&cargv, 2), 2);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, geocoord_err)
{
    static const char *argv[] = {_name,
        "-1+1/?", "-1914141+0/", "+18.3-232.56", "+32.2838.334-0",
    };
    cargv_geocoord_t v[1];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_geocoord(&cargv, "GEOCOORD", v, 1), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_geocoord(&cargv, "GEOCOORD", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_geocoord(&cargv, "GEOCOORD", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_geocoord(&cargv, "GEOCOORD", v, 1), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}
