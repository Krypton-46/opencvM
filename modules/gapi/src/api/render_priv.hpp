// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
//
// Copyright (C) 2018 Intel Corporation


#ifndef OPENCV_RENDER_PRIV_HPP
#define OPENCV_RENDER_PRIV_HPP

#include <opencv2/gapi/render/render.hpp>

namespace cv
{
namespace gapi
{
namespace wip
{
namespace draw
{

// FIXME only for tests
GAPI_EXPORTS void cvtNV12ToYUV(const cv::Mat& y, const cv::Mat& uv, cv::Mat& yuv);
GAPI_EXPORTS void cvtYUVToNV12(const cv::Mat& yuv, cv::Mat& y, cv::Mat& uv);

class IBitmaskCreator
{
public:
    virtual int createMask(cv::Mat&) = 0;
    virtual cv::Size computeMaskSize() = 0;
    virtual void setMaskParams(const cv::gapi::wip::draw::Text& text) = 0;
    virtual ~IBitmaskCreator() = default;
};

template<typename T>
struct make_mask_creator
{
    template <typename... Args>
    static std::unique_ptr<IBitmaskCreator> create(Args&&... args)
    {
        return std::unique_ptr<IBitmaskCreator>(new T(std::forward<Args>(args)...));
    }
};

class FreeTypeBitmaskCreator;
template <>
struct make_mask_creator<FreeTypeBitmaskCreator>
{
    template <typename... Args>
    static std::unique_ptr<IBitmaskCreator> create(Args&&... args)
    {
#ifdef HAVE_FREETYPE
        return std::unique_ptr<IBitmaskCreator>(new FreeTypeBitmaskCreator(std::forward<Args>(args)...));
#endif
        cv::util::suppress_unused_warning(args...);
        throw std::runtime_error("Freetype not found");
        return nullptr;
    }
};

} // namespace draw
} // namespace wip
} // namespace gapi
} // namespace cv

#endif // OPENCV_RENDER_PRIV_HPP
