/*
 * file:   getStructuringElement.cpp
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
 * getStructuringElement
 * Mat getStructuringElement(int shape, Size ksize, Point anchor=Point(-1,-1));
 * Gateway routine
 *   nlhs - number of return arguments
 *   plhs - pointers to return arguments
 *   nrhs - number of input arguments
 *   prhs - pointers to input arguments
 */
void mexFunction(int nlhs, mxArray* plhs[],
                 int nrhs, const mxArray* prhs[]) {

  // parse the inputs
  ArgumentParser parser("getStructuringElement");
  parser.addVariant("getStructuringElement", 2, 1, "anchor");
  MxArrayVector sorted = parser.parse(MxArrayVector(prhs, prhs+nrhs));

  // setup
  BridgeVector inputs(sorted.begin(), sorted.end());
  BridgeVector outputs(1);

  
  // unpack the arguments
  int shape = inputs[0].toInt();
  Size ksize = inputs[1].toSize();
  Point anchor = inputs[2].empty() ? (Point) Point(-1,-1) : inputs[2].toPoint();
  Mat retval;

  // call the opencv function
  // [out =] namespace.fun(src1, ..., srcn, dst1, ..., dstn, opt1, ..., optn);
  try {
    retval = cv::getStructuringElement(shape, ksize, anchor);
  } catch(cv::Exception& e) {
    error(std::string("cv::exception caught: ").append(e.what()).c_str());
  } catch(std::exception& e) {
    error(std::string("std::exception caught: ").append(e.what()).c_str());
  } catch(...) {
    error("Uncaught exception occurred in getStructuringElement");
  }
  
  // assign the outputs into the bridge
  outputs[0] = retval;


  // push the outputs back to matlab
  for (size_t n = 0; n < static_cast<size_t>(std::max(nlhs,1)); ++n) {
    plhs[n] = outputs[n].toMxArray().releaseOwnership();
  }
}