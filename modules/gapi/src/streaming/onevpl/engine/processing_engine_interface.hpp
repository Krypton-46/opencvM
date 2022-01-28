// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
//
// Copyright (C) 2022 Intel Corporation

#ifndef GAPI_STREAMING_ONEVPL_ENGINE_PROCESSING_ENGINE_INTERFACE_HPP
#define GAPI_STREAMING_ONEVPL_ENGINE_PROCESSING_ENGINE_INTERFACE_HPP

#include "precomp.hpp"
#include <opencv2/gapi/media.hpp>
#include <opencv2/gapi/util/optional.hpp>

#include "streaming\onevpl\engine\preproc_defines.hpp"

#ifdef HAVE_INF_ENGINE
#include <inference_engine.hpp>

namespace cv {
namespace gapi {
namespace wip {

struct IProcessingEngine {
    virtual ~IProcessingEngine() = default;

    virtual cv::util::optional<pp_params>
        is_applicable(const cv::MediaFrame& in_frame) = 0;

    virtual pp_session
        initialize_preproc(const pp_params& params,
                           const InferenceEngine::InputInfo::CPtr& net_input) = 0;
    virtual cv::MediaFrame
        run_sync(const pp_session &sess, const cv::MediaFrame& in_frame,
                 const cv::util::optional<cv::Rect> &opt_roi = {}) = 0;
};
} // namespace wip
} // namespace gapi
} // namespace cv
#endif // HAVE_INF_ENGINE
#endif // GAPI_STREAMING_ONEVPL_ENGINE_PROCESSING_ENGINE_INTERFACE_HPP
