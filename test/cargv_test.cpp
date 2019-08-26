
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
    static const char *args[] = { _name, };

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_opt(&cargv, "--help"), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, opt)
{
    static const char *args[] = { _name, "-h", "--help", };

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
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
    static const char *args[] = { _name, "-", "--", };

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
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
    static const char *args[] = { _name, "abc", "--text", "def", };
    const char *t[4];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
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
    static const char *args[] = { _name, "dog", "cat", "", };
    const char *v[4];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
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
    static const char *args[] = { _name, "32", "-1", "+1000", "1a", "1", };
    cargv_int_t val[4];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
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
    static const char *args[] = { _name,
        "9223372036854775807", "9223372036854775808",
        "-9223372036854775808", "-9223372036854775809",
        "18446744073709551616",
    };
    cargv_int_t val[1];

    testing::internal::CaptureStderr();
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
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
    static const char *args[] = { _name,
        "32", "0", "32768", "18446744073709551615", };
    static const cargv_uint_t vals[] = {
        32, 0, 32768, UINT64_C(18446744073709551615),
    };
    cargv_uint_t a;
    cargv_uint_t const *v = vals;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_uint(&cargv, "TEST", &a, 1), 1);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        EXPECT_EQ(a, *v);
        ++v;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, uint_error)
{
    static const char *args[] = { _name, "1a", };
    cargv_uint_t a;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_uint(&cargv, "TEST", &a, 1), 0);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, uint_overflow)
{
    static const char *args[] = { _name,
        "18446744073709551616", "-100"
    };
    cargv_uint_t a;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_uint(&cargv, "TEST", &a, 1), CARGV_VAL_OVERFLOW);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
    testing::internal::GetCapturedStderr();
}

TEST_F(Test_cargv, date)
{
    static const char *args[] = { _name,
        "10101010", "+1976-06-17", "1976/6/17", "-1-12-24",
        "-1010-01", "1001/01", "-100/2", "+1-12",
        "+2019", "0000", "-100",
        "--0102", "--02-03", "--04/05",
    };
    static const cargv_date_t vals[] = {
        {1010,10,10}, {1976,6,17}, {1976,6,17}, {-1,12,24},
        {-1010,1,0}, {1001,1,0}, {-100,2,0}, {1,12,0},
        {2019,0,0}, {0,0,0}, {-100,0,0},
        {0,1,2}, {0,2,3}, {0,4,5},
    };
    cargv_date_t a;
    cargv_date_t const *v = vals;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_date(&cargv, "TEST", &a, 1), 1);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        EXPECT_EQ(a.year, v->year);
        EXPECT_EQ(a.month, v->month);
        EXPECT_EQ(a.day, v->day);
        ++v;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, date_err)
{
    static const char *args[] = { _name,
        "2018-11--01", "2018+02-29", "201802-29",
        "201811",  /* YYYYMM is not allowed */
        "10000-12-24", "2018-13-01", "2018-02-29", "201802-01-01",
        "---19", "--19-03", "--+10-12",
    };
    static const int results[] = {
        0, 0, CARGV_VAL_OVERFLOW,
        CARGV_VAL_OVERFLOW,
        CARGV_VAL_OVERFLOW,
        CARGV_VAL_OVERFLOW,
        CARGV_VAL_OVERFLOW,
        CARGV_VAL_OVERFLOW,
        0, CARGV_VAL_OVERFLOW, 0,
    };
    cargv_date_t a;
    const int *r = results;

    testing::internal::CaptureStderr();
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_date(&cargv, "TEST", &a, 1), *r);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        ++r;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
    testing::internal::GetCapturedStderr();
}

TEST_F(Test_cargv, time_utc)
{
    static const char *args[] = { _name,
        "4Z", "00:00Z", "24:00Z", "14:59:27Z", "23:59Z"
    };
    const cargv_time_t vals[] = {
        {4,0,0}, {0,0,0}, {24,0,0}, {14,59,27}, {23,59,0},
    };
    cargv_time_t a;
    cargv_time_t const *v = vals;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_time(&cargv, "TEST", &a, 1), 1);
        EXPECT_EQ(a.hour, v->hour);
        EXPECT_EQ(a.minute, v->minute);
        EXPECT_EQ(a.second, v->second);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        ++v;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, time_zone)
{
    static const char *args[] = { _name,
        "4+00", "00:00+09:20", "7:20:34+9:30", "18:40-0930"
    };
    const cargv_time_t vals[] = {
        {4,0,0}, {-10,40,0}, {-3,50,34}, {28,10,0},
    };
    cargv_time_t a;
    cargv_time_t const *v = vals;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_time(&cargv, "TEST", &a, 1), 1);
        EXPECT_EQ(a.hour, v->hour);
        EXPECT_EQ(a.minute, v->minute);
        EXPECT_EQ(a.second, v->second);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        ++v;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, time_err)
{
    static const char *args[] = { _name,
        "-2:00",
        "24:01", "25:00", "0:60", "0900",
    };
    const int results[] = {
        0,
        CARGV_VAL_OVERFLOW, CARGV_VAL_OVERFLOW, CARGV_VAL_OVERFLOW,
        0
    };
    cargv_time_t a;
    int const *r = results;

    testing::internal::CaptureStderr();
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_time(&cargv, "TEST", &a, 1), *r);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        ++r;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
    testing::internal::GetCapturedStderr();
}

TEST_F(Test_cargv, time_zone_overflow)
{
    static const char *args[] = { _name,
        "4+13", "4+1201", "4-13", "4-1201", "12-00:60", "1+200"
    };
    int carg = _c(args);
    cargv_time_t v[1];

    testing::internal::CaptureStderr();
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
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
    static const char *args[] = { _name,
        "-32", "+9103", "+32.3957", "-13239.5", "+0793333.334", "-0",
    };
    cargv_degree_t v[6];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
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
    static const char *args[] = { _name,
        "-361", "+1163", "-112278.01", "13239.5", "0", "+720.12.12"
    };
    cargv_degree_t v[1];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
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
    static const char *args[] = { _name,
        "-32.3957+13239.57/",
        "+62.3-0",
    };
    cargv_geocoord_t v[5];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
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
    static const char *args[] = { _name,
        "-1+1/?", "-1914141+0/", "+18.3-232.56", "+32.2838.334-0",
    };
    cargv_geocoord_t v[1];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
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
