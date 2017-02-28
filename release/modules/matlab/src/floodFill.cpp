/*
 * file:   floodFill.cpp
 * author: A trusty code generator
 * date:   Tue, 28 Feb 2017 10:45:57
 *
 * This file was autogenerated, do not modify.
 * See LICENSE for full modification and redistribution details.
 * Copyright 2017 The OpenCV Foundation
 */
#include <string>
#include <vector>
#include <cassert>
#include <exception>
#include <opencv2/matlab/bridge.hpp>
#include <opencv2/imgproc.hpp>
using namespace cv;
using namespace matlab;
using namespace bridge;

/*
 * floodFill
 * int floodFill(Mat image, Mat mask, Point seedPoint, Scalar newVal, Rect* rect=0, Scalar loDiff=Scalar(), Scalar upDiff=Scalar(), int flags=4);
 * Gateway routine
 *   nlhs - number of return arguments
 *   plhs - pointers to return arguments
 *   nrhs - number of input arguments
 *   prhs - pointers to input arguments
 */
void mexFunction(int nlhs, mxArray* plhs[],
                 int nrhs, const mxArray* prhs[]) {

  // parse the inputs
  ArgumentParser parser("floodFill");
  parser.addVariant("floodFill", 4, 3, "loDiff", "upDiff", "flags");
  MxArrayVector sorted = parser.parse(MxArrayVector(prhs, prhs+nrhs));

  // setup
  BridgeVector inputs(sorted.begin(), sorted.end());
  BridgeVector outputs(4);

  
  // unpack the arguments
  Mat image = inputs[0].toMat();
  Mat mask = inputs[1].toMat();
  Point seedPoint = inputs[2].toPoint();
  Scalar newVal = inputs[3].toScalar();
  Scalar loDiff = inputs[4].empty() ? (Scalar) Scalar() : inputs[4].toScalar();
  Scalar upDiff = inputs[5].empty() ? (Scalar) Scalar() : inputs[5].toScalar();
  int flags = inputs[6].empty() ? (int) 4 : inputs[6].toInt();
  Rect rect;
  int retval;

  // call the opencv function
  // [out =] namespace.fun(src1, ..., srcn, dst1, ..., dstn, opt1, ..., optn);
  try {
    retval = cv::floodFill(image, mask, seedPoint, newVal, &rect, loDiff, upDiff, flags);
  } catch(cv::Exception& e) {
    error(std::string("cv::exception caught: ").append(e.what()).c_str());
  } catch(std::exception& e) {
    error(std::string("std::exception caught: ").append(e.what()).c_str());
  } catch(...) {
    error("Uncaught exception occurred in floodFill");
  }
  
  // assign the outputs into the bridge
  outputs[0] = retval;
  outputs[1] = image;
  outputs[2] = mask;
  outputs[3] = rect;


  // push the outputs back to matlab
  for (size_t n = 0; n < static_cast<size_t>(std::max(nlhs,1)); ++n) {
    plhs[n] = outputs[n].toMxArray().releaseOwnership();
  }
}