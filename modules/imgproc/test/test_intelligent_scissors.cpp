// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

#include "test_precomp.hpp"
//#include "opencv2/imgproc/segmentation.hpp"

namespace opencv_test { namespace {


Mat getTestImageGray()
{
    static Mat m;
    if (m.empty())
    {
        m = imread(findDataFile("shared/lena.png"), IMREAD_GRAYSCALE);
    }
    return m.clone();
}

Mat getTestImageColor()
{
    static Mat m;
    if (m.empty())
    {
        m = imread(findDataFile("shared/lena.png"), IMREAD_COLOR);
    }
    return m.clone();
}

Mat getTestImage1()
{
    static Mat m;
    if (m.empty())
    {
        m.create(Size(200, 100), CV_8UC1);
        m.setTo(Scalar::all(128));
        Rect roi(50, 30, 100, 40);
        m(roi).setTo(Scalar::all(0));
#if 0
        imshow("image", m);
        waitKey();
#endif
    }
    return m.clone();
}

Mat getTestImage2()
{
    static Mat m;
    if (m.empty())
    {
        m.create(Size(200, 100), CV_8UC1);
        m.setTo(Scalar::all(128));
        Rect roi(40, 30, 100, 40);
        m(roi).setTo(Scalar::all(255));
#if 0
        imshow("image", m);
        waitKey();
#endif
    }
    return m.clone();
}

Mat getTestImage3()
{
    static Mat m;
    if (m.empty())
    {
        m.create(Size(200, 100), CV_8UC1);
        m.setTo(Scalar::all(128));
        Scalar color(0,0,0,0);
        line(m, Point(30, 50), Point(50, 50), color, 1);
        line(m, Point(50, 50), Point(80, 30), color, 1);
        line(m, Point(150, 50), Point(80, 30), color, 1);
        line(m, Point(150, 50), Point(180, 50), color, 1);

        line(m, Point(80, 10), Point(80, 90), Scalar::all(200), 1);
        line(m, Point(100, 10), Point(100, 90), Scalar::all(200), 1);
        line(m, Point(120, 10), Point(120, 90), Scalar::all(200), 1);
#if 0
        imshow("image", m);
        waitKey();
#endif
    }
    return m.clone();
}

Mat getTestImage4()
{
    static Mat m;
    if (m.empty())
    {
        m.create(Size(200, 100), CV_8UC1);
        for (int y = 0; y < m.rows; y++)
        {
            for (int x = 0; x < m.cols; x++)
            {
                float dx = (float)(x - 100);
                float dy = (float)(y - 100);
                float d = sqrtf(dx * dx + dy * dy);
                m.at<uchar>(y, x) = saturate_cast<uchar>(100 + 100 * sin(d / 10 * CV_PI));
            }
        }
#if 0
        imshow("image", m);
        waitKey();
#endif
    }
    return m.clone();
}

Mat getTestImage5()
{
    static Mat m;
    if (m.empty())
    {
        m.create(Size(200, 100), CV_8UC1);
        for (int y = 0; y < m.rows; y++)
        {
            for (int x = 0; x < m.cols; x++)
            {
                float dx = (float)(x - 100);
                float dy = (float)(y - 100);
                float d = sqrtf(dx * dx + dy * dy);
                m.at<uchar>(y, x) = saturate_cast<uchar>(x / 2 + 100 * sin(d / 10 * CV_PI));
            }
        }
#if 0
        imshow("image", m);
        waitKey();
#endif
    }
    return m.clone();
}

void show(const Mat& img, const std::vector<Point> pts)
{
    if (cvtest::debugLevel >= 10)
    {
        Mat dst = img.clone();
        std::vector< std::vector<Point> > contours;
        contours.push_back(pts);
        polylines(dst, contours, false, Scalar::all(255));
        imshow("dst", dst);
        waitKey();
    }
}

void getContours(segmentation::IntelligentScissorsMB& tool,
                 const Point& target_point,
                 std::vector<Point>& pts,
                 std::vector<Point>& reference_pts,
                 const bool backward = false)
{
    tool.getContour(target_point, pts, backward);

    const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    CV_Assert(test_info);
    const std::string name = std::string(cvtest::TS::ptr()->get_data_path() + "imgproc/" + test_info->test_case_name() + "-" + test_info->name() + (backward ? "-backward" : "") + ".xml");

#ifdef GENERATE_TEST_DATA
    {
        cv::FileStorage fs(name, cv::FileStorage::WRITE);
        fs << "pts" << pts;
    }
#endif

    FileStorage fs(name, FileStorage::READ);
    read(fs["pts"], reference_pts, std::vector<Point>());
}

void getAndCheckContour(segmentation::IntelligentScissorsMB& tool,
                        const Point& target_point,
                        std::vector<Point>& pts,
                        const bool backward = false)
{
    std::vector<Point> reference_pts;
    getContours(tool, target_point, pts, reference_pts, backward);
    EXPECT_EQ(pts, reference_pts);
}

TEST(Imgproc_IntelligentScissorsMB, rect)
{
    segmentation::IntelligentScissorsMB tool;

    tool.applyImage(getTestImage1());

    Point source_point(50, 30);
    tool.buildMap(source_point);

    Point target_point(100, 30);
    std::vector<Point> pts;
    getAndCheckContour(tool, target_point, pts);

    tool.applyImage(getTestImage2());

    tool.buildMap(source_point);

    std::vector<Point> pts2;
    getAndCheckContour(tool, target_point, pts2, true/*backward*/);

    EXPECT_EQ(pts.size(), pts2.size());
}

TEST(Imgproc_IntelligentScissorsMB, lines)
{
    segmentation::IntelligentScissorsMB tool;
    Mat image = getTestImage3();
    tool.applyImage(image);

    Point source_point(30, 50);
    tool.buildMap(source_point);

    Point target_point(150, 50);
    std::vector<Point> pts;
    getAndCheckContour(tool, target_point, pts);

    EXPECT_EQ((size_t)121, pts.size());
    show(image, pts);
}

TEST(Imgproc_IntelligentScissorsMB, circles)
{
    segmentation::IntelligentScissorsMB tool;
    tool.setGradientMagnitudeMaxLimit(10);

    Mat image = getTestImage4();
    tool.applyImage(image);

    Point source_point(50, 50);
    tool.buildMap(source_point);

    Point target_point(150, 50);
    std::vector<Point> pts;
    getAndCheckContour(tool, target_point, pts);

    EXPECT_EQ((size_t)101, pts.size());
    show(image, pts);
}

TEST(Imgproc_IntelligentScissorsMB, circles_gradient)
{
    segmentation::IntelligentScissorsMB tool;
    Mat image = getTestImage5();
    tool.applyImage(image);

    Point source_point(50, 50);
    tool.buildMap(source_point);

    Point target_point(150, 50);
    std::vector<Point> pts;
    getAndCheckContour(tool, target_point, pts);

    EXPECT_EQ((size_t)101, pts.size());
    show(image, pts);
}

#define PTS_SIZE_EPS 2
#define PTS_EPS 1

TEST(Imgproc_IntelligentScissorsMB, grayscale)
{
    segmentation::IntelligentScissorsMB tool;

    Mat image = getTestImageGray();
    tool.applyImage(image);

    Point source_point(275, 63);
    tool.buildMap(source_point);

    Point target_point(413, 155);
    std::vector<Point> pts, reference_pts;
    getContours(tool, target_point, pts, reference_pts);
    if (pts.size() == reference_pts.size())
    {
        EXPECT_LE(cv::norm(pts, reference_pts, cv::NORM_INF), PTS_EPS);
    }
    else
    {
        size_t gold = 206;
        EXPECT_GE(pts.size(), gold - PTS_SIZE_EPS);
        EXPECT_LE(pts.size(), gold + PTS_SIZE_EPS);
        EXPECT_EQ(reference_pts.size(), gold);
    }
    show(image, pts);
}

TEST(Imgproc_IntelligentScissorsMB, check_features_grayscale_1_0_0_zerro_crossing_with_limit)
{
    segmentation::IntelligentScissorsMB tool;
    tool.setEdgeFeatureZeroCrossingParameters(64);
    tool.setWeights(1.0f, 0.0f, 0.0f);

    Mat image = getTestImageGray();
    tool.applyImage(image);

    Point source_point(275, 63);
    tool.buildMap(source_point);

    Point target_point(413, 155);
    std::vector<Point> pts, reference_pts;
    getContours(tool, target_point, pts, reference_pts);
    if (pts.size() == reference_pts.size())
    {
        EXPECT_LE(cv::norm(pts, reference_pts, cv::NORM_INF), PTS_EPS);
    }
    else
    {
        size_t gold = 207;
        EXPECT_GE(pts.size(), gold - PTS_SIZE_EPS);
        EXPECT_LE(pts.size(), gold + PTS_SIZE_EPS);
        EXPECT_EQ(reference_pts.size(), gold);
    }
    show(image, pts);
}

TEST(Imgproc_IntelligentScissorsMB, check_features_grayscale_1_0_0_canny)
{
    segmentation::IntelligentScissorsMB tool;
    tool.setEdgeFeatureCannyParameters(50, 100);
    tool.setWeights(1.0f, 0.0f, 0.0f);

    Mat image = getTestImageGray();
    tool.applyImage(image);

    Point source_point(275, 63);
    tool.buildMap(source_point);

    Point target_point(413, 155);
    std::vector<Point> pts, reference_pts;
    getContours(tool, target_point, pts, reference_pts);
    if (pts.size() == reference_pts.size())
    {
        EXPECT_LE(cv::norm(pts, reference_pts, cv::NORM_INF), PTS_EPS);
    }
    else
    {
        size_t gold = 201;
        EXPECT_GE(pts.size(), gold - PTS_SIZE_EPS);
        EXPECT_LE(pts.size(), gold + PTS_SIZE_EPS);
        EXPECT_EQ(reference_pts.size(), gold);
    }
    show(image, pts);
}

TEST(Imgproc_IntelligentScissorsMB, check_features_grayscale_0_1_0)
{
    segmentation::IntelligentScissorsMB tool;
    tool.setWeights(0.0f, 1.0f, 0.0f);

    Mat image = getTestImageGray();
    tool.applyImage(image);

    Point source_point(275, 63);
    tool.buildMap(source_point);

    Point target_point(413, 155);
    std::vector<Point> pts, reference_pts;
    getContours(tool, target_point, pts, reference_pts);
    if (pts.size() == reference_pts.size())
    {
        EXPECT_LE(cv::norm(pts, reference_pts, cv::NORM_INF), PTS_EPS);
    }
    else
    {
        size_t gold = 166;
        EXPECT_GE(pts.size(), gold - PTS_SIZE_EPS);
        EXPECT_LE(pts.size(), gold + PTS_SIZE_EPS);
        EXPECT_EQ(reference_pts.size(), gold);
    }
    show(image, pts);
}

TEST(Imgproc_IntelligentScissorsMB, check_features_grayscale_0_0_1)
{
    segmentation::IntelligentScissorsMB tool;
    tool.setWeights(0.0f, 0.0f, 1.0f);

    Mat image = getTestImageGray();
    tool.applyImage(image);

    Point source_point(275, 63);
    tool.buildMap(source_point);

    Point target_point(413, 155);
    std::vector<Point> pts, reference_pts;
    getContours(tool, target_point, pts, reference_pts);
    if (pts.size() == reference_pts.size())
    {
        EXPECT_LE(cv::norm(pts, reference_pts, cv::NORM_INF), PTS_EPS);
    }
    else
    {
        size_t gold = 197;
        EXPECT_GE(pts.size(), gold - PTS_SIZE_EPS);
        EXPECT_LE(pts.size(), gold + PTS_SIZE_EPS);
        EXPECT_EQ(reference_pts.size(), gold);
    }
    show(image, pts);
}

TEST(Imgproc_IntelligentScissorsMB, color)
{
    segmentation::IntelligentScissorsMB tool;

    Mat image = getTestImageColor();
    tool.applyImage(image);

    Point source_point(275, 63);
    tool.buildMap(source_point);

    Point target_point(413, 155);
    std::vector<Point> pts, reference_pts;
    getContours(tool, target_point, pts, reference_pts);
    if (pts.size() == reference_pts.size())
    {
        EXPECT_LE(cv::norm(pts, reference_pts, cv::NORM_INF), PTS_EPS);
    }
    else
    {
        size_t gold = 205;
        EXPECT_GE(pts.size(), gold - PTS_SIZE_EPS);
        EXPECT_LE(pts.size(), gold + PTS_SIZE_EPS);
        EXPECT_EQ(reference_pts.size(), gold);
    }
    show(image, pts);
}

TEST(Imgproc_IntelligentScissorsMB, color_canny)
{
    segmentation::IntelligentScissorsMB tool;
    tool.setEdgeFeatureCannyParameters(32, 100);

    Mat image = getTestImageColor();
    tool.applyImage(image);

    Point source_point(275, 63);
    tool.buildMap(source_point);

    Point target_point(413, 155);
    std::vector<Point> pts, reference_pts;
    getContours(tool, target_point, pts, reference_pts);
    if (pts.size() == reference_pts.size())
    {
        EXPECT_LE(cv::norm(pts, reference_pts, cv::NORM_INF), PTS_EPS);
    }
    else
    {
        size_t gold = 200;
        EXPECT_GE(pts.size(), gold - PTS_SIZE_EPS);
        EXPECT_LE(pts.size(), gold + PTS_SIZE_EPS);
        EXPECT_EQ(reference_pts.size(), gold);
    }
    show(image, pts);
}


TEST(Imgproc_IntelligentScissorsMB, color_custom_features_invalid)
{
    segmentation::IntelligentScissorsMB tool;
    ASSERT_ANY_THROW(tool.applyImageFeatures(noArray(), noArray(), noArray()));
}

TEST(Imgproc_IntelligentScissorsMB, color_custom_features_edge)
{
    segmentation::IntelligentScissorsMB tool;

    Mat image = getTestImageColor();

    Mat canny_edges;
    Canny(image, canny_edges, 32, 100, 5);
    Mat binary_edge_feature;
    cv::threshold(canny_edges, binary_edge_feature, 254, 1, THRESH_BINARY_INV);
    tool.applyImageFeatures(binary_edge_feature, noArray(), noArray(), image);

    Point source_point(275, 63);
    tool.buildMap(source_point);

    Point target_point(413, 155);
    std::vector<Point> pts, reference_pts;
    getContours(tool, target_point, pts, reference_pts);
    if (pts.size() == reference_pts.size())
    {
        EXPECT_LE(cv::norm(pts, reference_pts, cv::NORM_INF), PTS_EPS);
    }
    else
    {
        size_t gold = 201;
        EXPECT_GE(pts.size(), gold - PTS_SIZE_EPS);
        EXPECT_LE(pts.size(), gold + PTS_SIZE_EPS);
        EXPECT_EQ(reference_pts.size(), gold);
    }
    show(image, pts);
}

TEST(Imgproc_IntelligentScissorsMB, color_custom_features_all)
{
    segmentation::IntelligentScissorsMB tool;

    tool.setWeights(0.9f, 0.0f, 0.1f);

    Mat image = getTestImageColor();

    Mat canny_edges;
    Canny(image, canny_edges, 50, 100, 5);
    Mat binary_edge_feature; // 0, 1 values
    cv::threshold(canny_edges, binary_edge_feature, 254, 1, THRESH_BINARY_INV);

    Mat_<Point2f> gradient_direction(image.size(), Point2f(0, 0));  // normalized
    Mat_<float> gradient_magnitude(image.size(), 0);  // cost function
    tool.applyImageFeatures(binary_edge_feature, gradient_direction, gradient_magnitude);

    Point source_point(275, 63);
    tool.buildMap(source_point);

    Point target_point(413, 155);
    std::vector<Point> pts, reference_pts;
    getContours(tool, target_point, pts, reference_pts);
    if (pts.size() == reference_pts.size())
    {
        EXPECT_LE(cv::norm(pts, reference_pts, cv::NORM_INF), PTS_EPS);
    }
    else
    {
        size_t gold = 201;
        EXPECT_GE(pts.size(), gold - PTS_SIZE_EPS);
        EXPECT_LE(pts.size(), gold + PTS_SIZE_EPS);
        EXPECT_EQ(reference_pts.size(), gold);
    }
    show(image, pts);
}

TEST(Imgproc_IntelligentScissorsMB, color_custom_features_edge_magnitude)
{
    segmentation::IntelligentScissorsMB tool;

    tool.setWeights(0.9f, 0.0f, 0.1f);

    Mat image = getTestImageColor();

    Mat canny_edges;
    Canny(image, canny_edges, 50, 100, 5);
    Mat binary_edge_feature; // 0, 1 values
    cv::threshold(canny_edges, binary_edge_feature, 254, 1, THRESH_BINARY_INV);

    Mat_<float> gradient_magnitude(image.size(), 0);  // cost function
    tool.applyImageFeatures(binary_edge_feature, noArray(), gradient_magnitude);

    Point source_point(275, 63);
    tool.buildMap(source_point);

    Point target_point(413, 155);
    std::vector<Point> pts, reference_pts;
    getContours(tool, target_point, pts, reference_pts);
    if (pts.size() == reference_pts.size())
    {
        EXPECT_LE(cv::norm(pts, reference_pts, cv::NORM_INF), PTS_EPS);
    }
    else
    {
        size_t gold = 201;
        EXPECT_GE(pts.size(), gold - PTS_SIZE_EPS);
        EXPECT_LE(pts.size(), gold + PTS_SIZE_EPS);
        EXPECT_EQ(reference_pts.size(), gold);
    }
    show(image, pts);
}


}} // namespace
