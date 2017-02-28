/*
 * file:   calcOpticalFlowPyrLK.cpp
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
#include <opencv2/video.hpp>
using namespace cv;
using namespace matlab;
using namespace bridge;

/*
 * calcOpticalFlowPyrLK
 * void calcOpticalFlowPyrLK(Mat prevImg, Mat nextImg, Mat prevPts, Mat nextPts, Mat status, Mat err, Size winSize=Size(21,21), int maxLevel=3, TermCriteria criteria=TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 30, 0.01), int flags=0, double minEigThreshold=1e-4);
 * Gateway routine
 *   nlhs - number of return arguments
 *   plhs - pointers to return arguments
 *   nrhs - number of input arguments
 *   prhs - pointers to input arguments
 */
void mexFunction(int nlhs, mxArray* plhs[],
                 int nrhs, const mxArray* prhs[]) {

  // parse the inputs
  ArgumentParser parser("calcOpticalFlowPyrLK");
  parser.addVariant("calcOpticalFlowPyrLK", 4, 5, "winSize", "maxLevel", "criteria", "flags", "minEigThreshold");
  MxArrayVector sorted = parser.parse(MxArrayVector(prhs, prhs+nrhs));

  // setup
  BridgeVector inputs(sorted.begin(), sorted.end());
  BridgeVector outputs(3);

  
  // unpack the arguments
  Mat prevImg = inputs[0].toMat();
  Mat nextImg = inputs[1].toMat();
  Mat prevPts = inputs[2].toMat();
  Mat nextPts = inputs[3].toMat();
  Size winSize = inputs[4].empty() ? (Size) Size(21,21) : inputs[4].toSize();
  int maxLevel = inputs[5].empty() ? (int) 3 : inputs[5].toInt();
  TermCriteria criteria = inputs[6].empty() ? (TermCriteria) TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 30, 0.01) : inputs[6].toTermCriteria();
  int flags = inputs[7].empty() ? (int) 0 : inputs[7].toInt();
  double minEigThreshold = inputs[8].empty() ? (double) 1e-4 : inputs[8].toDouble();
  Mat status;
  Mat err;

  // call the opencv function
  // [out =] namespace.fun(src1, ..., srcn, dst1, ..., dstn, opt1, ..., optn);
  try {
    cv::calcOpticalFlowPyrLK(prevImg, nextImg, prevPts, nextPts, status, err, winSize, maxLevel, criteria, flags, minEigThreshold);
  } catch(cv::Exception& e) {
    error(std::string("cv::exception caught: ").append(e.what()).c_str());
  } catch(std::exception& e) {
    error(std::string("std::exception caught: ").append(e.what()).c_str());
  } catch(...) {
    error("Uncaught exception occurred in calcOpticalFlowPyrLK");
  }
  
  // assign the outputs into the bridge
  outputs[0] = nextPts;
  outputs[1] = status;
  outputs[2] = err;


  // push the outputs back to matlab
  for (size_t n = 0; n < static_cast<size_t>(std::max(nlhs,1)); ++n) {
    plhs[n] = outputs[n].toMxArray().releaseOwnership();
  }
}