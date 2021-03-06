
#include "cargv/cargv.h"
#include "cargv/cargv_version.h"
#include "gtest/gtest.h"

#include <stdio.h>
#include <math.h>
#include <time.h>



#define _c(a)    (ptrdiff_t(sizeof(a)/sizeof((a)[0])))


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
        "9223372036854775807", "-9223372036854775808",
        "9,223,372,036,854,775,807", "-9_223_372_036_854_775_808",
    };
    static const cargv_int_t expected[] = {
        32, -1, 1000, 0, 0,
        INT64_C(9223372036854775807), -INT64_C(9223372036854775807)-1,
        INT64_C(9223372036854775807), -INT64_C(9223372036854775807)-1,
    };
    cargv_int_t v;
    cargv_int_t const *e = expected;

    ASSERT_EQ(_c(args)-1, _c(expected));
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_int(&cargv, "TEST", &v, 1), 1);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        EXPECT_EQ(v, *e);
        ++e;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, sint_error)
{
    static const char *args[] = { _name,
        "1a", "-1,2.3", "+,", "++1"
    };
    cargv_int_t v;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_int(&cargv, "TEST", &v, 1), 0);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, sint_overflow)
{
    static const char *args[] = { _name,
        "9223372036854775808",
        "-9223372036854775809",
        "18446744073709551616",
    };
    cargv_int_t v;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_int(&cargv, "TEST", &v, 1), CARGV_VAL_OVERFLOW);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, uint)
{
    static const char *args[] = { _name,
        "32", "0", "32768", "18446744073709551615",
        "3,2", "0.", "_32__76_8__", "18,446,744,073,709,551,615",
    };
    static const cargv_uint_t expected[] = {
        32, 0, 32768, UINT64_C(18446744073709551615),
        32, 0, 32768, UINT64_C(18446744073709551615),
    };
    cargv_uint_t v;
    cargv_uint_t const *e = expected;

    ASSERT_EQ(_c(args)-1, _c(expected));
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_uint(&cargv, "TEST", &v, 1), 1);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        EXPECT_EQ(v, *e);
        ++e;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, uint_error)
{
    static const char *args[] = { _name,
        "1a", "1,2.3", ",", "...", "_._._.,"
    };
    cargv_uint_t v;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_uint(&cargv, "TEST", &v, 1), 0);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, uint_overflow)
{
    static const char *args[] = { _name,
        "18446744073709551616", "-100",
        "18,446,744,073,709,551,616", "-_100"
    };
    cargv_uint_t v;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_uint(&cargv, "TEST", &v, 1), CARGV_VAL_OVERFLOW);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, date)
{
    static const char *args[] = { _name,
        "10101010", "+1999-06-3", "1999/6/3", "-45-01-23",
        "-1010-01", "1001/01", "-100/2", "+1-2-3",
        "+2019", "0000", "0", "-100", "1",
        "--0102", "--02-03", "--04/05",
    };

    #define _HMSZ_DEFAULT CARGV_HOUR_DEFAULT,CARGV_MINUTE_DEFAULT,CARGV_SECOND_DEFAULT,CARGV_MILISECOND_DEFAULT,*CARGV_TZ_LOCAL
    static const cargv_datetime_t expected[] = {
        {1010,10,10,_HMSZ_DEFAULT},
        {1999,6,3,_HMSZ_DEFAULT},
        {1999,6,3,_HMSZ_DEFAULT},
        {-45,1,23,_HMSZ_DEFAULT},
        {-1010,1,0,_HMSZ_DEFAULT},
        {1001,1,0,_HMSZ_DEFAULT},
        {-100,2,0,_HMSZ_DEFAULT},
        {1,2,3,_HMSZ_DEFAULT},
        {2019,0,0,_HMSZ_DEFAULT},
        {0,0,0,_HMSZ_DEFAULT},
        {0,0,0,_HMSZ_DEFAULT},
        {-100,0,0,_HMSZ_DEFAULT},
        {1,0,0,_HMSZ_DEFAULT},
        {CARGV_YEAR_DEFAULT,1,2,_HMSZ_DEFAULT},
        {CARGV_YEAR_DEFAULT,2,3,_HMSZ_DEFAULT},
        {CARGV_YEAR_DEFAULT,4,5,_HMSZ_DEFAULT},
    };
    #undef _HMSZ_DEFAULT
    
    cargv_datetime_t v;
    const cargv_datetime_t *e = expected;

    ASSERT_EQ(_c(args)-1, _c(expected));
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_date(&cargv, "TEST", &v, 1), 1);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        EXPECT_EQ(v.year, e->year);
        EXPECT_EQ(v.month, e->month);
        EXPECT_EQ(v.day, e->day);
        ++e;
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
        "2018-0a-29",   // wrong character `v`
        "201802-29",    // >4 digits for year
        "201811",       // YYYYMM is not allowed
        "10000-12-24",  // >4 digits for year
        "---19", "--+10-12",
    };
    cargv_datetime_t v;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_date(&cargv, "TEST", &v, 1), 0);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, date_overflow)
{
    static const char *args[] = { _name,
        "2018-13-01", "2018-02-29", "2019-00-01", "2019-12-00", "--19-03",
    };
    cargv_datetime_t v;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_date(&cargv, "TEST", &v, 1), CARGV_VAL_OVERFLOW);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, time)
{
    static const char *args[] = { _name,
        "0", "4", "0:0", "12:34", "24:00", "14:59:27",
        "0Z", "4Z", "00:00+00", "24:00+1", "14:59:27-9", "23:59-09:20",
    };
    #define _YMD_DEFAULT CARGV_YEAR_DEFAULT,CARGV_MONTH_DEFAULT,CARGV_DAY_DEFAULT
    static const cargv_datetime_t expected[] = {
        {_YMD_DEFAULT, 0,0,0,0,*CARGV_TZ_LOCAL},
        {_YMD_DEFAULT, 4,0,0,0,*CARGV_TZ_LOCAL},
        {_YMD_DEFAULT, 0,0,0,0,*CARGV_TZ_LOCAL},
        {_YMD_DEFAULT, 12,34,0,0,*CARGV_TZ_LOCAL},
        {_YMD_DEFAULT, 24,0,0,0,*CARGV_TZ_LOCAL},
        {_YMD_DEFAULT, 14,59,27,0,*CARGV_TZ_LOCAL},
        {_YMD_DEFAULT, 0,0,0,0,{0,0}},
        {_YMD_DEFAULT, 4,0,0,0,{0,0}},
        {_YMD_DEFAULT, 0,0,0,0,{0,0}},
        {_YMD_DEFAULT, 24,0,0,0,{1,0}},
        {_YMD_DEFAULT, 14,59,27,0,{-9,0}},
        {_YMD_DEFAULT, 23,59,0,0,{-9,-20}},
    };
    #undef _YMD_DEFAULT
    
    cargv_datetime_t v;
    cargv_datetime_t const *e = expected;

    ASSERT_EQ(_c(args)-1, _c(expected));
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_time(&cargv, "TEST", &v, 1), 1);
        EXPECT_EQ(v.hour, e->hour);
        EXPECT_EQ(v.minute, e->minute);
        EXPECT_EQ(v.second, e->second);
        EXPECT_EQ(v.milisecond, e->milisecond);
        EXPECT_EQ(v.tz.hour, e->tz.hour);
        EXPECT_EQ(v.tz.minute, e->tz.minute);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        ++e;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, time_error)
{
    static const char *args[] = { _name,
        "+2:00Z", "-2:00Z",  // +- not allowed
        "123:00Z", "12:345Z", "12:34:567Z", "12:34:Z", "12:34:56:Z",
        "12:", "12:34:", "12:34:56:", "12/34",
    };
    cargv_datetime_t v;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_time(&cargv, "TEST", &v, 1), 0);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, time_overflow)
{
    static const char *args[] = { _name,
        "12-34", "24:01Z", "25:00Z", "0:60Z", "01:02:60Z",
    };
    cargv_datetime_t v;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_time(&cargv, "TEST", &v, 1), CARGV_VAL_OVERFLOW);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, timezone)
{
    static const char *args[] = { _name,
        "Z", "+00", "+1", "+09:20", "-9:3", "+14:00", "-12", "+1359", "-11:59",
    };
    const cargv_timezone_t expected[] = {
        *CARGV_UTC, *CARGV_UTC, {1,0}, {9,20}, {-9,-3},
        {14,0}, {-12,0}, {13,59}, {-11,-59},
    };
    cargv_timezone_t v;
    cargv_timezone_t const *e = expected;

    ASSERT_EQ(_c(args)-1, _c(expected));
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_timezone(&cargv, "TEST", &v, 1), 1);
        EXPECT_EQ(v.hour, e->hour);
        EXPECT_EQ(v.minute, e->minute);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        ++e;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, timezone_error)
{
    static const char *args[] = { _name,
        "z", "+", "-", "+001:00", "+01:000",
        "+200",  // needs 4 digits.
    };
    cargv_timezone_t v;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_timezone(&cargv, "TEST", &v, 1), 0);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, timezone_overflow)
{
    static const char *args[] = { _name,
        "+15", "+1401", "-13", "-1201", "-00:60",
    };
    cargv_timezone_t v;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_timezone(&cargv, "TEST", &v, 1), CARGV_VAL_OVERFLOW);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, datetime)
{
    static const char *args[] = { _name,
        "+1999-06-3T08:00+9:30",
        "+1999-06-3T08:00+14:0",
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
    static const cargv_datetime_t expected[] = {
        {1999,6,3,8,0,0,0,{9,30}},
        {1999,6,3,8,0,0,0,{14,0}},
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
        {CARGV_YEAR_DEFAULT,CARGV_MONTH_DEFAULT,CARGV_DAY_DEFAULT,10,0,0,0,
         *CARGV_UTC},
        {CARGV_YEAR_DEFAULT,CARGV_MONTH_DEFAULT,CARGV_DAY_DEFAULT,23,59,59,0,
         *CARGV_TZ_LOCAL},
    };
    cargv_datetime_t v;
    const cargv_datetime_t *e = expected;

    ASSERT_EQ(_c(args)-1, _c(expected));
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_datetime(&cargv, "TEST", &v, 1), 1);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        EXPECT_EQ(v.year, e->year);
        EXPECT_EQ(v.month, e->month);
        EXPECT_EQ(v.day, e->day);
        EXPECT_EQ(v.hour, e->hour);
        EXPECT_EQ(v.minute, e->minute);
        EXPECT_EQ(v.second, e->second);
        EXPECT_EQ(v.milisecond, e->milisecond);
        EXPECT_EQ(v.tz.hour, e->tz.hour);
        EXPECT_EQ(v.tz.minute, e->tz.minute);
        ++e;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, datetime_error)
{
    static const char *args[] = { _name,
        "+1999-06-3T",  // No time
        "+1999-06-3Z",  // No time
        "+1999-13-3T",  // No time
        "+1999-06-3+0900",  // No time
        "+1999-06-3T08:00+9:30a",  // Redundant character
        "T08:00+9:30",   // No date
        "+9:30",         // No time
        "10000",         // ?
    };
    cargv_datetime_t v;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_datetime(&cargv, "TEST", &v, 1), 0);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, datetime_overflow)
{
    static const char *args[] = { _name,
        "2019-13-23",
        "25:00+9:30",
        "+1999-13-17T08:00+9:30",
        "+2019-2-29T08:00+9:30",
    };
    cargv_datetime_t v;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);  //_name
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_datetime(&cargv, "TEST", &v, 1), CARGV_VAL_OVERFLOW);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);
}

TEST_F(Test_cargv, local_datetime)
{
    static const cargv_datetime_t srcs[] = {
        {1999,6,3,8,10,0,0,{9,30}},
        {1999,6,3,8,10,0,0,{9,30}},
        {1999,6,3,18,10,0,0,{-9,-30}},
        {2019,1,1,23,59,0,0,{-12,0}},
        {2019,1,1,12,0,0,0,{12,0}},
        {2019,12,31,12,0,0,0,{-12,0}},
        {2016,3,1,8,10,0,0,{9,30}},   // leap year
        {2016,2,29,18,40,0,0,{-9,-30}},   // leap year
        {2019,3,1,8,10,0,0,{9,30}},
        {2019,2,28,18,40,0,0,{-9,-30}},
        {2019,3,1,12,0,0,0,{14,0}},
    };
    static const cargv_datetime_t dsts[] = {
        {1999,6,2,22,40,0,0,*CARGV_UTC},
        {1999,6,2,13,10,0,0,{-9,-30}},
        {1999,6,4,3,40,0,0,*CARGV_UTC},
        {2019,1,2,23,59,0,0,{12,0}},
        {2018,12,31,12,0,0,0,{-12,0}},
        {2020,1,1,12,0,0,0,{12,0}},
        {2016,2,29,22,40,0,0,*CARGV_UTC},
        {2016,3,1,4,10,0,0,*CARGV_UTC},
        {2019,2,28,22,40,0,0,*CARGV_UTC},
        {2019,3,1,4,10,0,0,*CARGV_UTC},
        {2019,2,28,22,0,0,0,*CARGV_UTC},
    };
    cargv_datetime_t v;

    ASSERT_EQ(_c(srcs), _c(dsts));
    for (cargv_datetime_t const *s = srcs, *d = dsts;
         s - srcs < _c(srcs);
         ++s, ++d) {
        EXPECT_EQ(cargv_local_datetime(&v, s, &d->tz), CARGV_OK);
        EXPECT_EQ(v.year, d->year);
        EXPECT_EQ(v.month, d->month);
        EXPECT_EQ(v.day, d->day);
        EXPECT_EQ(v.hour, d->hour);
        EXPECT_EQ(v.minute, d->minute);
        EXPECT_EQ(v.second, d->second);
        EXPECT_EQ(v.milisecond, d->milisecond);
        EXPECT_EQ(v.tz.hour, d->tz.hour);
        EXPECT_EQ(v.tz.minute, d->tz.minute);
    }
}

TEST_F(Test_cargv, local_datetime_overflow)
{
    static const cargv_datetime_t srcs[] = {
        {-9999,1,1,4,0,0,0,{+9,0}},
        {9999,12,31,18,0,0,0,{-9,0}},
    };
    cargv_datetime_t v;

    for (cargv_datetime_t const *s = srcs; s - srcs < _c(srcs); ++s) {
        EXPECT_EQ(cargv_local_datetime(&v, s, CARGV_UTC), CARGV_VAL_OVERFLOW);
    }
}

TEST_F(Test_cargv, degree)
{
    static const char *args[] = { _name,
        "+1", "-32", "-0", "+132", "+9103", "+32.3957", "-13239.5",
        "+0793333.33", "-0793333.33",
    };
    static const cargv_degree_t expected[] = {
        {1,0,0,0,0,0},
        {-32,0,0,0,0,0},
        {0,0,0,0,0,0},
        {132,0,0,0,0,0},
        {91,0,3,0,0,0},
        {32,395700,0,0,0,0},
        {-132,0,-39,-500000,0,0},
        {79,0,33,0,33,330000},
        {-79,-0,-33,-0,-33,-330000},
    };
    cargv_degree_t v;
    const cargv_degree_t *e = expected;

    ASSERT_EQ(_c(args)-1, _c(expected));
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_degree(&cargv, "TEST", &v, 1), 1);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        EXPECT_EQ(v.degree, e->degree);
        EXPECT_EQ(v.microdegree, e->microdegree);
        EXPECT_EQ(v.minute, e->minute);
        EXPECT_EQ(v.microminute, e->microminute);
        EXPECT_EQ(v.second, e->second);
        EXPECT_EQ(v.microsecond, e->microsecond);
        ++e;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, degree_error)
{
    static const char *args[] = { _name,
        "0", "13239.5", "+72.12.12", "+720.12.12",
    };
    cargv_degree_t v;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_degree(&cargv, "TEST", &v, 1), 0);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, degree_overflow)
{
    static const char *args[] = { _name,
        "-361", "+1163", "-112278,01",
    };
    cargv_degree_t v;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_degree(&cargv, "TEST", &v, 1), CARGV_VAL_OVERFLOW);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, get_degree)
{
    static const cargv_degree_t vals[] = {
        {1,0,0,0,0,0},
        {-32,0,0,0,0,0},
        {0,0,0,0,0,0},
        {132,0,0,0,0,0},
        {91,0,3,0,0,0},
        {32,395700,0,0,0,0},
        {-132,0,-39,-500000,0,0},
        {79,0,33,0,33,330000},
        {-79,-0,-33,-0,-33,-330000},
    };
    static const cargv_real_t expected[] = {
        1.0, -32.0, .0, 132.0, 91.0+3.0/60.0,
        32.395700,
        -132.0-39.5/60.0,
        79.0+33.0/60.0+33.33/3600.0,
        -79.0-33.0/60.0-33.33/3600.0,
    };
    const cargv_real_t *e = expected;

    ASSERT_EQ(_c(vals), _c(expected));
    for (const cargv_degree_t *v = vals; v-vals < _c(vals); ++v, ++e) {
        EXPECT_EQ(cargv_get_degree(v), *e);
    }
}

TEST_F(Test_cargv, geocoord)
{
    static const char *args[] = { _name,
        "-32.3957+13239.57/",
        "+62.3-0",
        "+3734+12658/",  // Seoul
        "+3747-12225/",  // San Francisco
        "+3955+11623/",  // Beijing
    };
    static const cargv_geocoord_t expected[] = {
        {{-32,-395700,0,0,0,0},{132,0,39,570000,0,0}},
        {{62,300000,0,0,0,0},{0,0,0,0,0,0}},
        *CARGV_SEOUL,
        *CARGV_SAN_FRANCISCO,
        *CARGV_BEIJING,
    };
    cargv_geocoord_t v;
    const cargv_geocoord_t *e = expected;

    ASSERT_EQ(_c(args)-1, _c(expected));
    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_geocoord(&cargv, "TEST", &v, 1), 1);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
        EXPECT_EQ(v.latitude.degree, e->latitude.degree);
        EXPECT_EQ(v.latitude.microdegree, e->latitude.microdegree);
        EXPECT_EQ(v.latitude.minute, e->latitude.minute);
        EXPECT_EQ(v.latitude.microminute, e->latitude.microminute);
        EXPECT_EQ(v.latitude.second, e->latitude.second);
        EXPECT_EQ(v.latitude.microsecond, e->latitude.microsecond);
        EXPECT_EQ(v.longitude.degree, e->longitude.degree);
        EXPECT_EQ(v.longitude.microdegree, e->longitude.microdegree);
        EXPECT_EQ(v.longitude.minute, e->longitude.minute);
        EXPECT_EQ(v.longitude.microminute, e->longitude.microminute);
        EXPECT_EQ(v.longitude.second, e->longitude.second);
        EXPECT_EQ(v.longitude.microsecond, e->longitude.microsecond);
        ++e;
    }
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, geocoord_error)
{
    static const char *args[] = { _name,
        "-1/", "-1+1/?", "+32.2838.334-0",
    };
    cargv_geocoord_t v;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_geocoord(&cargv, "TEST", &v, 1), 0);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}

TEST_F(Test_cargv, geocoord_overflow)
{
    static const char *args[] = { _name,
        "-1914141+0/", "+18.3-232.56",
    };
    cargv_geocoord_t v;

    ASSERT_EQ(cargv_init(&cargv, _name, _c(args), args), CARGV_OK);
    EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    testing::internal::CaptureStderr();
    while (cargv_len(&cargv) > 0) {
        EXPECT_EQ(cargv_geocoord(&cargv, "TEST", &v, 1), CARGV_VAL_OVERFLOW);
        EXPECT_EQ(cargv_shift(&cargv, 1), 1);
    }
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(cargv_shift(&cargv, 1), 0);  // Ensure empty
}
