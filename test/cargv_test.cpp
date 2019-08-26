
#include "cargv.h"
#include "cargv_version.h"
#include "gtest/gtest.h"

#include <stdio.h>
#include <math.h>


#define _c(a)    (sizeof(a)/sizeof((a)[0]))


class Test_cargv : public testing::Test {
protected:
    virtual void SetUp()
    {
    }

    static const char *_name;

    struct cargv_t cargv;
};

const char *Test_cargv::_name = "cargv-test";


TEST(Test_cargv_lib, version)
{
    cargv_version_t ver;

    EXPECT_EQ(cargv_version(&ver), CARGV_VERSION);
    EXPECT_EQ(ver.major, CARGV_VERSION_MAJOR);
    EXPECT_EQ(ver.minor, CARGV_VERSION_MINOR);
    EXPECT_EQ(ver.patch, CARGV_VERSION_PATCH);
    EXPECT_EQ(ver.state, CARGV_VERSION_STATE_DEV);

    EXPECT_STREQ(cargv_version_string(), CARGV_VERSION_STRING);
}

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

TEST_F(Test_cargv, oneof)
{
    static const char *argv[] = {_name, "dog", "cat", ""};
    const char *v[4];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_oneof(&cargv, "ONEOF", "dog--cat", "--", v, 4), 2);
    EXPECT_EQ(cargv_oneof(&cargv, "ONEOF", "cow|cat", "|", v, 4), 0);
    EXPECT_EQ(cargv_oneof(&cargv, "ONEOF", "||", "|", v, 4), 0);
    EXPECT_EQ(cargv_oneof(&cargv, "ONEOF", "|dog|cat|", "|", v, 4), 2);
    EXPECT_EQ(cargv_shift(&cargv, 2), 2);
    EXPECT_EQ(cargv_oneof(&cargv, "ONEOF", "dog||cat", "||", v, 4), 0);
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
        "18446744073709551616",
    };
    cargv_int_t val[1];

    testing::internal::CaptureStderr();
    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_int(&cargv, "INT", val, 1), 1);
    EXPECT_EQ(val[0], INT64_C(9223372036854775807));
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_int(&cargv, "INT", val, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_int(&cargv, "INT", val, 1), 1);
    EXPECT_EQ(val[0], INT64_C(-9223372036854775807)-1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_int(&cargv, "INT", val, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_int(&cargv, "INT", val, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
    testing::internal::GetCapturedStderr();
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
    testing::internal::CaptureStderr();
    EXPECT_EQ(cargv_uint(&cargv, "UINT", val, 1), 1);
    EXPECT_EQ(val[0], UINT64_C(18446744073709551615));
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_uint(&cargv, "UINT", val, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_uint(&cargv, "UINT", val, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
    testing::internal::GetCapturedStderr();
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
        "2018-11--01", "2018+02-29", "201802-29"
    };
    cargv_date_t v[1];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_date(&cargv, "DATE", v, 1), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_date(&cargv, "DATE", v, 1), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_date(&cargv, "DATE", v, 1), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, date_overflow)
{
    static const char *argv[] = {_name,
        "10000-12-24", "2018-00-01", "2018-13-01", "2018-11-00",
        "2018-02-29", "201802-01-01"
    };
    cargv_date_t v[1];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    testing::internal::CaptureStderr();
    EXPECT_EQ(cargv_date(&cargv, "DATE", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_date(&cargv, "DATE", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_date(&cargv, "DATE", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_date(&cargv, "DATE", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_date(&cargv, "DATE", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_date(&cargv, "DATE", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
    testing::internal::GetCapturedStderr();
}

TEST_F(Test_cargv, time_utc)
{
    static const char *argv[] = {_name,
        "4Z", "00:00Z", "24:00Z", "14:59:27Z", "23:59Z"
    };
    int carg = _c(argv);
    cargv_time_t v[_c(argv)];

    ASSERT_EQ(cargv_init(&cargv, _name, carg, argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1); --carg;
    EXPECT_EQ(cargv_time(&cargv, "TEST", v, 1), 1);
    EXPECT_EQ(v[0].hour, 4);
    EXPECT_EQ(v[0].minute, 0);
    EXPECT_EQ(v[0].second, 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1); --carg;
    EXPECT_EQ(cargv_time(&cargv, "TEST", v, carg+1), carg);
    EXPECT_EQ(v[0].hour, 0);
    EXPECT_EQ(v[0].minute, 0);
    EXPECT_EQ(v[0].second, 0);
    EXPECT_EQ(v[1].hour, 24);
    EXPECT_EQ(v[1].minute, 0);
    EXPECT_EQ(v[1].second, 0);
    EXPECT_EQ(v[2].hour, 14);
    EXPECT_EQ(v[2].minute, 59);
    EXPECT_EQ(v[2].second, 27);
    EXPECT_EQ(v[3].hour, 23);
    EXPECT_EQ(v[3].minute, 59);
    EXPECT_EQ(v[3].second, 0);
    EXPECT_EQ(cargv_shift(&cargv, carg), carg);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, time_zone)
{
    static const char *argv[] = {_name,
        "4+00", "00:00+09:20", "7:20:34+9:30", "18:40-0930"
    };
    int carg = _c(argv);
    cargv_time_t v[_c(argv)];

    ASSERT_EQ(cargv_init(&cargv, _name, carg, argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1); --carg;
    EXPECT_EQ(cargv_time(&cargv, "TEST", v, 1), 1);
    EXPECT_EQ(v[0].hour, 4);
    EXPECT_EQ(v[0].minute, 0);
    EXPECT_EQ(v[0].second, 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1); --carg;
    EXPECT_EQ(cargv_time(&cargv, "TEST", v, carg+1), carg);
    EXPECT_EQ(v[0].hour, -10);
    EXPECT_EQ(v[0].minute, 40);
    EXPECT_EQ(v[0].second, 0);
    EXPECT_EQ(v[1].hour, -3);
    EXPECT_EQ(v[1].minute, 50);
    EXPECT_EQ(v[1].second, 34);
    EXPECT_EQ(v[2].hour, 28);
    EXPECT_EQ(v[2].minute, 10);
    EXPECT_EQ(v[2].second, 0);
    EXPECT_EQ(cargv_shift(&cargv, carg), carg);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, time_err)
{
    static const char *argv[] = {_name,
        "-2:00",
    };
    int carg = _c(argv);
    cargv_time_t v[1];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1); --carg;
    while (carg > 0) {
        EXPECT_EQ(cargv_time(&cargv, "TEST", v, 1), 0);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1); --carg;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, time_overflow)
{
    static const char *argv[] = {_name,
        "24:01", "25:00", "0:60", "0900"
    };
    int carg = _c(argv);
    cargv_time_t v[1];

    testing::internal::CaptureStderr();
    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1); --carg;
    while (carg > 0) {
        EXPECT_EQ(cargv_time(&cargv, "TEST", v, 1), CARGV_VAL_OVERFLOW);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1); --carg;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
    testing::internal::GetCapturedStderr();
}

TEST_F(Test_cargv, time_zone_overflow)
{
    static const char *argv[] = {_name,
        "4+13", "4+1201", "4-13", "4-1201", "12-00:60", "1+200"
    };
    int carg = _c(argv);
    cargv_time_t v[1];

    testing::internal::CaptureStderr();
    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1); --carg;
    while (carg > 0) {
        EXPECT_EQ(cargv_time(&cargv, "TEST", v, 1), CARGV_VAL_OVERFLOW);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1); --carg;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
    testing::internal::GetCapturedStderr();
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
        "-361", "+1163", "-112278.01", "13239.5", "0", "+720.12.12"
    };
    cargv_degree_t v[1];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(argv), argv), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    testing::internal::CaptureStderr();
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
    EXPECT_EQ(cargv_degree(&cargv, "DEG", v, 1), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
    testing::internal::GetCapturedStderr();
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
    testing::internal::CaptureStderr();
    EXPECT_EQ(cargv_geocoord(&cargv, "GEOCOORD", v, 1), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_geocoord(&cargv, "GEOCOORD", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_geocoord(&cargv, "GEOCOORD", v, 1), CARGV_VAL_OVERFLOW);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_geocoord(&cargv, "GEOCOORD", v, 1), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
    testing::internal::GetCapturedStderr();
}
