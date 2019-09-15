
#include "cargv.h"
#include "cargv_version.h"
#include "gtest/gtest.h"

#include <stdio.h>
#include <math.h>
#include <time.h>



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
    EXPECT_EQ(cargv_text(&cargv, "TEST", t, 1), 1);
    EXPECT_STREQ(t[0], "abc");
    EXPECT_EQ(cargv_text(&cargv, "TEST", t, 2), 2);
    EXPECT_STREQ(t[0], "abc");
    EXPECT_STREQ(t[1], "--text");
    EXPECT_EQ(cargv_text(&cargv, "TEST", t, 4), 3);
    EXPECT_STREQ(t[0], "abc");
    EXPECT_STREQ(t[1], "--text");
    EXPECT_STREQ(t[2], "def");

    EXPECT_EQ(cargv_shift(&cargv, 3), 3);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, oneof)
{
    static const char *args[] = { _name, "dog", "cat", "", };
    const char *v[4];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_oneof(&cargv, "ONEOF", "dog--cat", "--", v, 4), 2);
    EXPECT_STREQ(v[0], "dog");
    EXPECT_STREQ(v[1], "cat");
    EXPECT_EQ(cargv_oneof(&cargv, "ONEOF", "|dog|cat|", "|", v, 4), 2);
    EXPECT_STREQ(v[0], "dog");
    EXPECT_STREQ(v[1], "cat");
    EXPECT_EQ(cargv_oneof(&cargv, "ONEOF", "cow|dog", "|", v, 4), 1);
    EXPECT_STREQ(v[0], "dog");
    EXPECT_EQ(cargv_oneof(&cargv, "ONEOF", "cow|cat", "|", v, 4), 0);
    EXPECT_EQ(cargv_oneof(&cargv, "ONEOF", "||", "|", v, 4), 0);
    EXPECT_EQ(cargv_shift(&cargv, 2), 2);
    EXPECT_EQ(cargv_oneof(&cargv, "ONEOF", "dog||cat", "||", v, 4), 0);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, sint)
{
    static const char *args[] = { _name,
        "32", "-1", "+1000", "-0", "-_0_",
        "9223372036854775807", "9,223,372,036,854,775,807",
    };
    static const cargv_int_t vals[] = {
        32, -1, 1000, 0, 0,
        INT64_C(9223372036854775807), INT64_C(9223372036854775807),
    };
    cargv_int_t a;
    cargv_int_t const *v = vals;

    ASSERT_EQ(_c(args)-1, _c(vals));
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_int(&cargv, "TEST", &a, 1), 1);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        EXPECT_EQ(a, *v);
        ++v;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, int_error)
{
    static const char *args[] = { _name,
        "1a", "-1,2.3", "+,", "++1"
    };
    cargv_int_t a;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_int(&cargv, "TEST", &a, 1), 0);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, sint_overflow)
{
    static const char *args[] = { _name,
        "9223372036854775808",
        "-9223372036854775809",
        "18446744073709551616",
    };
    cargv_int_t a;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_int(&cargv, "TEST", &a, 1), CARGV_VAL_OVERFLOW);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
    testing::internal::GetCapturedStderr();
}

TEST_F(Test_cargv, uint)
{
    static const char *args[] = { _name,
        "32", "0", "32768", "18446744073709551615",
        "3,2", "0.", "_32__76_8__", "18,446,744,073,709,551,615",
    };
    static const cargv_uint_t vals[] = {
        32, 0, 32768, UINT64_C(18446744073709551615),
        32, 0, 32768, UINT64_C(18446744073709551615),
    };
    cargv_uint_t a;
    cargv_uint_t const *v = vals;

    ASSERT_EQ(_c(args)-1, _c(vals));
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
    static const char *args[] = { _name,
        "1a", "1,2.3", ",", "...", "_._._.,"
    };
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
        "18446744073709551616", "-100",
        "18,446,744,073,709,551,616", "-_100"
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
        "10101010", "+1976-06-17", "1976/6/17", "-45-01-23",
        "-1010-01", "1001/01", "-100/2", "+1-2-3",
        "+2019", "0000", "0", "-100", "1",
        "--0102", "--02-03", "--04/05",
    };
    cargv_date_t vals[] = {
        {1010,10,10}, {1976,6,17}, {1976,6,17}, {-45,1,23},
        {-1010,1,0}, {1001,1,0}, {-100,2,0}, {1,2,3},
        {2019,0,0}, {0,0,0}, {0,0,0}, {-100,0,0}, {1,0,0},
        {CARGV_YEAR_DEFAULT,1,2}, {CARGV_YEAR_DEFAULT,2,3}, {CARGV_YEAR_DEFAULT,4,5},
    };
    cargv_date_t a;
    cargv_date_t *v = vals;

    ASSERT_EQ(_c(args)-1, _c(vals));
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

TEST_F(Test_cargv, date_error)
{
    static const char *args[] = { _name,
        "2018-01-01-",  // redundant character
        "2018-001-01",  // >2 digits for month
        "2018-01-001",  // >2 digits for day
        "2018-11--01",  // 2 `-`s
        "2018+02-29",   // wrong character `+`
        "2018-0a-29",   // wrong character `a`
        "201802-29",    // >4 digits for year
        "201811",       // YYYYMM is not allowed
        "10000-12-24",  // >4 digits for year
        "2018-13-01", "2018-02-29", "2019-00-01", "2019-12-00",
        "---19", "--19-03", "--+10-12",
    };
    static const int results[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        CARGV_VAL_OVERFLOW, CARGV_VAL_OVERFLOW,
        CARGV_VAL_OVERFLOW, CARGV_VAL_OVERFLOW,
        0, CARGV_VAL_OVERFLOW, 0,
    };
    cargv_date_t a;
    const int *r = results;

    testing::internal::CaptureStderr();
    ASSERT_EQ(_c(args)-1, _c(results));
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

TEST_F(Test_cargv, time)
{
    static const char *args[] = { _name,
        "0", "4", "0:0", "12:34", "24:00", "14:59:27",
        "0Z", "4Z", "00:00+00", "24:00+1", "14:59:27-9", "23:59-09:20",
    };
    static const cargv_time_t vals[] = {
        {0,0,0,0,*CARGV_TZ_LOCAL},
        {4,0,0,0,*CARGV_TZ_LOCAL},
        {0,0,0,0,*CARGV_TZ_LOCAL},
        {12,34,0,0,*CARGV_TZ_LOCAL},
        {24,0,0,0,*CARGV_TZ_LOCAL},
        {14,59,27,0,*CARGV_TZ_LOCAL},
        {0,0,0,0,{0,0}},
        {4,0,0,0,{0,0}},
        {0,0,0,0,{0,0}},
        {24,0,0,0,{1,0}},
        {14,59,27,0,{-9,0}},
        {23,59,0,0,{-9,-20}},
    };
    cargv_time_t a;
    cargv_time_t const *v = vals;

    ASSERT_EQ(_c(args)-1, _c(vals));
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_time(&cargv, "TEST", &a, 1), 1);
        EXPECT_EQ(a.hour, v->hour);
        EXPECT_EQ(a.minute, v->minute);
        EXPECT_EQ(a.second, v->second);
        EXPECT_EQ(a.milisecond, v->milisecond);
        //EXPECT_EQ(a.tz.sign, v->tz.sign);
        EXPECT_EQ(a.tz.hour, v->tz.hour);
        EXPECT_EQ(a.tz.minute, v->tz.minute);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        ++v;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, time_error)
{
    static const char *args[] = { _name,
        "+2:00Z", "-2:00Z",  // +- not allowed
        "123:00Z", "12:345Z", "12:34:567Z", "12:34:Z", "12:34:56:Z",
        "12:", "12:34:", "12:34:56:", "12/34",
        "12-34", "24:01Z", "25:00Z", "0:60Z", "01:02:60Z",  //Overflow
    };
    static const int results[] = {
        0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        CARGV_VAL_OVERFLOW, CARGV_VAL_OVERFLOW, CARGV_VAL_OVERFLOW,
        CARGV_VAL_OVERFLOW, CARGV_VAL_OVERFLOW,
    };
    cargv_time_t a;
    int const *r = results;

    testing::internal::CaptureStderr();
    ASSERT_EQ(_c(args)-1, _c(results));
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

TEST_F(Test_cargv, timezone)
{
    static const char *args[] = { _name,
        "Z", "+00", "+1", "+09:20", "-9:3",
    };
    const cargv_timezone_t vals[] = {
        *CARGV_UTC, *CARGV_UTC, {1,0}, {9,20}, {-9,-3},
    };
    cargv_timezone_t a;
    cargv_timezone_t const *v = vals;

    ASSERT_EQ(_c(args)-1, _c(vals));
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_timezone(&cargv, "TEST", &a, 1), 1);
        //EXPECT_EQ(a.sign, v->sign);
        EXPECT_EQ(a.hour, v->hour);
        EXPECT_EQ(a.minute, v->minute);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        ++v;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, timezone_error)
{
    static const char *args[] = { _name,
        "z", "+", "-", "+001:00", "+01:000",
        "+200",  // needs 4 digits.
        "+13", "+1201", "-13", "-1201", "-00:60",
    };
    static const int results[] = {
        0, 0, 0, 0, 0, 0,
        CARGV_VAL_OVERFLOW, CARGV_VAL_OVERFLOW, CARGV_VAL_OVERFLOW,
        CARGV_VAL_OVERFLOW, CARGV_VAL_OVERFLOW,
    };
    cargv_timezone_t a;
    int const *r = results;

    testing::internal::CaptureStderr();
    ASSERT_EQ(_c(args)-1, _c(results));
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_timezone(&cargv, "TEST", &a, 1), *r);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        ++r;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
    testing::internal::GetCapturedStderr();
}

TEST_F(Test_cargv, datetime)
{
    static const char *args[] = { _name,
        "+1976-06-17T08:00+9:30",
        "45-01-23 1:2:3Z",
        "45-01-23 1:2:3",
        "45-01-23",
        "11T12",
        "11T12Z",
        "--2-29",
        "--0102T0:0+0",
        "--02-03 24:00-1",
        "--04/05T23:59:59-09:20",
        "10",
        "10Z",
        "23:59:59",
    };
    static const cargv_datetime_t vals[] = {
        {1976,6,17,8,0,0,0,{9,30}},
        {45,1,23,1,2,3,0,*CARGV_UTC},
        {45,1,23,1,2,3,0,*CARGV_TZ_LOCAL},
        {45,1,23,CARGV_HOUR_DEFAULT,CARGV_MINUTE_DEFAULT,CARGV_SECOND_DEFAULT,
         CARGV_MILISECOND_DEFAULT,*CARGV_TZ_LOCAL},
        {11,CARGV_MONTH_DEFAULT,CARGV_DAY_DEFAULT,12,0,0,0,*CARGV_TZ_LOCAL},
        {11,CARGV_MONTH_DEFAULT,CARGV_DAY_DEFAULT,12,0,0,0,*CARGV_UTC},
        {CARGV_YEAR_DEFAULT,2,29,CARGV_HOUR_DEFAULT,CARGV_MINUTE_DEFAULT,
         CARGV_SECOND_DEFAULT,CARGV_MILISECOND_DEFAULT,*CARGV_TZ_LOCAL},
        {CARGV_YEAR_DEFAULT,1,2,0,0,0,0,*CARGV_UTC},
        {CARGV_YEAR_DEFAULT,2,3,24,0,0,0,{-1,0}},
        {CARGV_YEAR_DEFAULT,4,5,23,59,59,0,{-9,-20}},
        {10,0,0,CARGV_HOUR_DEFAULT,CARGV_MINUTE_DEFAULT,CARGV_SECOND_DEFAULT,
         CARGV_MILISECOND_DEFAULT,*CARGV_TZ_LOCAL},
        {CARGV_YEAR_DEFAULT,CARGV_MONTH_DEFAULT,CARGV_DAY_DEFAULT,10,0,0,0,*CARGV_UTC},
        {CARGV_YEAR_DEFAULT,CARGV_MONTH_DEFAULT,CARGV_DAY_DEFAULT,23,59,59,0,*CARGV_TZ_LOCAL},
    };
    cargv_datetime_t a;
    const cargv_datetime_t *v = vals;

    ASSERT_EQ(_c(args)-1, _c(vals));
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_datetime(&cargv, "TEST", &a, 1), 1);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        EXPECT_EQ(a.year, v->year);
        EXPECT_EQ(a.month, v->month);
        EXPECT_EQ(a.day, v->day);
        EXPECT_EQ(a.hour, v->hour);
        EXPECT_EQ(a.minute, v->minute);
        EXPECT_EQ(a.second, v->second);
        EXPECT_EQ(a.milisecond, v->milisecond);
        //EXPECT_EQ(a.tz.sign, v->tz.sign);
        EXPECT_EQ(a.tz.hour, v->tz.hour);
        EXPECT_EQ(a.tz.minute, v->tz.minute);
        ++v;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, datetime_error)
{
    static const char *args[] = { _name,
        "+1976-06-17T",
        "+1976-06-17Z",
        "+1976-13-17T",
        "+1976-06-17+0900",
        "+1976-06-17T08:00+9:30a",
        "T08:00+9:30",
        "+9:30",
        "10000",
        "2019-13-23",
        "25:00+9:30",
        "+1976-13-17T08:00+9:30",
        "+2019-2-29T08:00+9:30",
    };
    static const int results[] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        CARGV_VAL_OVERFLOW, CARGV_VAL_OVERFLOW, CARGV_VAL_OVERFLOW,
        CARGV_VAL_OVERFLOW,
    };
    cargv_datetime_t a;
    int const *r = results;

    testing::internal::CaptureStderr();
    ASSERT_EQ(_c(args)-1, _c(results));
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_datetime(&cargv, "TEST", &a, 1), *r);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        ++r;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
    testing::internal::GetCapturedStderr();
}

TEST_F(Test_cargv, convert_localtime)
{
    static const cargv_datetime_t srcs[] = {
        {1976,6,17,8,10,0,0,{9,30}},
        {1976,6,17,8,10,0,0,{9,30}},
        {2019,1,1,23,59,0,0,{-12,0}},
        {2019,1,1,12,0,0,0,{12,0}},
        {2019,12,31,12,0,0,0,{-12,0}},
        {2016,3,1,8,10,0,0,{9,30}},   // leap year
        {2016,2,29,18,40,0,0,{-9,-30}},   // leap year
        {2019,3,1,8,10,0,0,{9,30}},
        {2019,2,28,18,40,0,0,{-9,-30}},
    };
    static const cargv_datetime_t dsts[] = {
        {1976,6,16,22,40,0,0,*CARGV_UTC},
        {1976,6,16,13,10,0,0,{-9,-30}},
        {2019,1,2,23,59,0,0,{12,0}},
        {2018,12,31,12,0,0,0,{-12,0}},
        {2020,1,1,12,0,0,0,{12,0}},
        {2016,2,29,22,40,0,0,*CARGV_UTC},
        {2016,3,1,4,10,0,0,*CARGV_UTC},
        {2019,2,28,22,40,0,0,*CARGV_UTC},
        {2019,3,1,4,10,0,0,*CARGV_UTC},
    };
    cargv_datetime_t v;

    ASSERT_EQ(_c(srcs), _c(dsts));
    for (cargv_datetime_t const *s = srcs, *d = dsts;
         s - srcs < _c(srcs);
         ++s, ++d) {
        EXPECT_EQ(cargv_convert_localtime(&v, s, &d->tz), CARGV_OK);
        EXPECT_EQ(v.year, d->year);
        EXPECT_EQ(v.month, d->month);
        EXPECT_EQ(v.day, d->day);
        EXPECT_EQ(v.hour, d->hour);
        EXPECT_EQ(v.minute, d->minute);
        EXPECT_EQ(v.second, d->second);
        EXPECT_EQ(v.milisecond, d->milisecond);
        //EXPECT_EQ(v.tz.sign, d->tz.sign);
        EXPECT_EQ(v.tz.hour, d->tz.hour);
        EXPECT_EQ(v.tz.minute, d->tz.minute);
    }
}

TEST_F(Test_cargv, convert_localtime_error)
{
    static const cargv_datetime_t srcs[] = {
        {-9999,1,1,4,0,0,0,{+9,0}},
        {9999,12,31,18,0,0,0,{-9,0}},
    };
    cargv_datetime_t v;

    for (cargv_datetime_t const *s = srcs; s - srcs < _c(srcs); ++s) {
        EXPECT_EQ(cargv_convert_localtime(&v, s, CARGV_UTC), CARGV_VAL_OVERFLOW);
    }
}

TEST_F(Test_cargv, degree)
{
    static const char *args[] = { _name,
        "-32", "+9103", "+32.3957", "-13239.5", "+0793333.33", "-0",
    };
    cargv_degree_t v[6];

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    EXPECT_EQ(cargv_degree(&cargv, "DEG", v, 6), 6);
    EXPECT_EQ(v[0].degree, -32000000);
    EXPECT_EQ(v[0].minute, 0);
    EXPECT_EQ(v[0].second, 0);
    EXPECT_EQ(v[1].degree, 91000000);
    EXPECT_EQ(v[1].minute, 3000000);
    EXPECT_EQ(v[1].second, 0);
    EXPECT_EQ(v[2].degree, 32395700);
    EXPECT_EQ(v[2].minute, 0);
    EXPECT_EQ(v[2].second, 0);
    EXPECT_EQ(v[3].degree, -132000000);
    EXPECT_EQ(v[3].minute, -39500000);
    EXPECT_EQ(v[3].second, 0);
    EXPECT_EQ(v[4].degree, 79000000);
    EXPECT_EQ(v[4].minute, 33000000);
    EXPECT_EQ(v[4].second, 33330000);
    EXPECT_EQ(v[5].degree, 0);
    EXPECT_EQ(v[5].minute, 0);
    EXPECT_EQ(v[5].second, 0);
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
    EXPECT_EQ(v[0].latitude.degree, -32395700);
    EXPECT_EQ(v[0].latitude.minute, 0);
    EXPECT_EQ(v[0].latitude.second, 0);
    EXPECT_EQ(v[0].longitude.degree, 132000000);
    EXPECT_EQ(v[0].longitude.minute,  39570000);
    EXPECT_EQ(v[0].longitude.second,  0);
    EXPECT_EQ(v[1].latitude.degree, 62300000);
    EXPECT_EQ(v[1].latitude.minute, 0);
    EXPECT_EQ(v[1].latitude.second, 0);
    EXPECT_EQ(v[1].longitude.degree, 0);
    EXPECT_EQ(v[1].longitude.minute, 0);
    EXPECT_EQ(v[1].longitude.second, 0);
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
