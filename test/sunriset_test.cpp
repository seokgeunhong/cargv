
#include "sunriset_version.h"
#include "sunriset.h"
#include "gtest/gtest.h"

#include <stdio.h>


#define _c(a)    (sizeof(a)/sizeof((a)[0]))
#define EXPECT_APPROX(a, b, delta) \
    EXPECT_LE(fabs((a)-(b)), (delta))


class Test_sunriset : public testing::Test {
protected:
    virtual void SetUp()
    {
    }

};

TEST_F(Test_sunriset, day_len_seoul)
{
    EXPECT_APPROX(day_length(2018, 2, 9, 0, 37.56667), 10.6107, 0.000001);
    EXPECT_APPROX(day_civil_twilight_length(
        2018, 2, 9, 0, 37.56667), 11.51068, 0.000001);
    EXPECT_APPROX(day_nautical_twilight_length(
        2018, 2, 9, 0, 37.56667), 12.541088, 0.000001);
    EXPECT_APPROX(day_astronomical_twilight_length(
        2018, 2, 9, 0, 37.56667), 13.558356, 0.000001);
}

TEST_F(Test_sunriset, sun_rise_set_seoul)
{
    double rise, set;

    EXPECT_EQ(sun_rise_set(2018, 2, 9, 126.978056, 37.56667, &rise, &set), 0);
    EXPECT_APPROX(rise, -1.527985, 0.000001);
    EXPECT_APPROX(set, 9.070172, 0.000001);

    EXPECT_EQ(civil_twilight(2018, 2, 9, 126.978056, 37.56667, &rise, &set), 0);
    EXPECT_APPROX(rise, -1.978296, 0.000001);
    EXPECT_APPROX(set, 9.520483, 0.000001);

    EXPECT_EQ(nautical_twilight(
        2018, 2, 9, 126.978056, 37.56667, &rise, &set), 0);
    EXPECT_APPROX(rise, -2.493765, 0.000001);
    EXPECT_APPROX(set, 10.035952, 0.000001);

    EXPECT_EQ(astronomical_twilight(
        2018, 2, 9, 126.978056, 37.56667, &rise, &set), 0);
    EXPECT_APPROX(rise, -3.002559, 0.000001);
    EXPECT_APPROX(set, 10.544746, 0.000001);
}

TEST_F(Test_sunriset, sun_rise_set_nordkapp)
{
    double rise, set;

    // Nordkapp, Norge(Norway)
    EXPECT_LT(sun_rise_set(2017, 12, 22, 25.48, 71.1, &rise, &set), 0);

    EXPECT_EQ(civil_twilight(2017, 12, 22, 25.48, 71.1, &rise, &set), 0);
    EXPECT_APPROX(rise, 8.685923, 0.000001);
    EXPECT_APPROX(set, 11.871380, 0.000001);

    EXPECT_EQ(nautical_twilight(2017, 12, 22, 25.48, 71.1, &rise, &set), 0);
    EXPECT_APPROX(rise, 6.578896, 0.000001);
    EXPECT_APPROX(set, 13.978406, 0.000001);

    EXPECT_EQ(astronomical_twilight(2017, 12, 22, 25.48, 71.1, &rise, &set), 0);
    EXPECT_APPROX(rise, 5.150469, 0.000001);
    EXPECT_APPROX(set, 15.406834, 0.000001);
}

TEST_F(Test_sunriset, north_pole)
{
    double rise, set;

    // any day
    EXPECT_EQ(day_length(2018, 2, 9, .0, 90.0), .0);
    EXPECT_EQ(day_civil_twilight_length(2018, 2, 9, .0, 90.0), .0);
    EXPECT_EQ(day_nautical_twilight_length(2018, 2, 9, .0, 90.0), .0);
    EXPECT_EQ(day_astronomical_twilight_length(2018, 2, 9, .0, 90.0), 24.0);
    EXPECT_LT(sun_rise_set(2018, 2, 9, .0, 90.0, &rise, &set), 0);
    EXPECT_LT(civil_twilight(2018, 2, 9, .0, 90.0, &rise, &set), 0);
    EXPECT_LT(nautical_twilight(2018, 2, 9, .0, 90.0, &rise, &set), 0);
    EXPECT_GT(astronomical_twilight(2018, 2, 9, .0, 90.0, &rise, &set), 0);

    // winter solstice
    EXPECT_EQ(day_length(2018, 12, 22, .0, 90.0), .0);
    EXPECT_EQ(day_civil_twilight_length(2018, 12, 22, .0, 90.0), .0);
    EXPECT_EQ(day_nautical_twilight_length(2018, 12, 22, .0, 90.0), .0);
    EXPECT_EQ(day_astronomical_twilight_length(2018, 12, 22, .0, 90.0), .0);
    EXPECT_LT(sun_rise_set(2018, 12, 22, .0, 90.0, &rise, &set), 0);
    EXPECT_LT(civil_twilight(2018, 12, 22, .0, 90.0, &rise, &set), 0);
    EXPECT_LT(nautical_twilight(2018, 12, 22, .0, 90.0, &rise, &set), 0);
    EXPECT_LT(astronomical_twilight(2018, 12, 22, .0, 90.0, &rise, &set), 0);

    // summer solstice
    EXPECT_EQ(day_length(2018, 6, 21, .0, 90.0), 24.0);
    EXPECT_EQ(day_civil_twilight_length(2018, 6, 21, .0, 90.0), 24.0);
    EXPECT_EQ(day_nautical_twilight_length(2018, 6, 21, .0, 90.0), 24.0);
    EXPECT_EQ(day_astronomical_twilight_length(2018, 6, 21, .0, 90.0), 24.0);
    EXPECT_GT(sun_rise_set(2018, 6, 21, .0, 90.0, &rise, &set), 0);
    EXPECT_GT(civil_twilight(2018, 6, 21, .0, 90.0, &rise, &set), 0);
    EXPECT_GT(nautical_twilight(2018, 6, 21, .0, 90.0, &rise, &set), 0);
    EXPECT_GT(astronomical_twilight(2018, 6, 21, .0, 90.0, &rise, &set), 0);
}

TEST_F(Test_sunriset, south_pole)
{
    double rise, set;

    // any day
    EXPECT_EQ(day_length(2018, 5, 9, .0, -90.0), .0);
    EXPECT_EQ(day_civil_twilight_length(2018, 5, 9, .0, -90.0), .0);
    EXPECT_EQ(day_nautical_twilight_length(2018, 5, 9, .0, -90.0), .0);
    EXPECT_EQ(day_astronomical_twilight_length(2018, 4, 5, .0, -90.0), 24.0);
    EXPECT_LT(sun_rise_set(2018, 5, 9, .0, -90.0, &rise, &set), 0);
    EXPECT_LT(civil_twilight(2018, 5, 9, .0, -90.0, &rise, &set), 0);
    EXPECT_LT(nautical_twilight(2018, 5, 9, .0, -90.0, &rise, &set), 0);
    EXPECT_GT(astronomical_twilight(2018, 5, 9, .0, -90.0, &rise, &set), 0);

    // summer solstice
    EXPECT_EQ(day_length(2018, 12, 22, .0, -90.0), 24.0);
    EXPECT_EQ(day_civil_twilight_length(2018, 12, 22, .0, -90.0), 24.0);
    EXPECT_EQ(day_nautical_twilight_length(2018, 12, 22, .0, -90.0), 24.0);
    EXPECT_EQ(day_astronomical_twilight_length(2018, 12, 22, .0, -90.0), 24.0);
    EXPECT_GT(sun_rise_set(2018, 12, 22, .0, -90.0, &rise, &set), 0);
    EXPECT_GT(civil_twilight(2018, 12, 22, .0, -90.0, &rise, &set), 0);
    EXPECT_GT(nautical_twilight(2018, 12, 22, .0, -90.0, &rise, &set), 0);
    EXPECT_GT(astronomical_twilight(2018, 12, 22, .0, -90.0, &rise, &set), 0);

    // winter solstice
    EXPECT_EQ(day_length(2018, 6, 21, .0, -90.0), .0);
    EXPECT_EQ(day_civil_twilight_length(2018, 6, 21, .0, -90.0), .0);
    EXPECT_EQ(day_nautical_twilight_length(2018, 6, 21, .0, -90.0), .0);
    EXPECT_EQ(day_astronomical_twilight_length(2018, 6, 21, .0, -90.0), .0);
    EXPECT_LT(sun_rise_set(2018, 6, 21, .0, -90.0, &rise, &set), 0);
    EXPECT_LT(civil_twilight(2018, 6, 21, .0, -90.0, &rise, &set), 0);
    EXPECT_LT(nautical_twilight(2018, 6, 21, .0, -90.0, &rise, &set), 0);
    EXPECT_LT(astronomical_twilight(2018, 6, 21, .0, -90.0, &rise, &set), 0);
}
