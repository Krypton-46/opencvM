// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html


#include <iostream>
#include "volume_impl.hpp"
#include "tsdf_functions.hpp"
#include "opencv2/imgproc.hpp"

namespace cv
{

Volume::Impl::Impl(VolumeSettings settings) :
    voxelSize(settings.getVoxelSize()),
    voxelSizeInv(1.0f / voxelSize),
    raycastStepFactor(settings.getRaycastStepFactor())
{
    std::cout << "Volume::Impl::Impl()" << std::endl;
    this->settings = settings;
    Matx44f _pose;
    settings.getVolumePose(_pose);
    this->pose = Affine3f(_pose);
}

// TSDF

TsdfVolume::TsdfVolume(VolumeSettings settings) :
    Volume::Impl(settings)
{
    std::cout << "TsdfVolume::TsdfVolume()" << std::endl;

    this->settings = settings;

    CV_Assert(settings.getMaxWeight() < 255);
    // Unlike original code, this should work with any volume size
    // Not only when (x,y,z % 32) == 0
    Vec3i resolution;
    settings.getVolumeResolution(resolution);
    volResolution = Point3i(resolution);
    volSize = Point3f(volResolution) * voxelSize;
    truncDist = std::max(settings.getTruncatedDistance(), 2.1f * voxelSize);

    // (xRes*yRes*zRes) array
    // Depending on zFirstMemOrder arg:
    // &elem(x, y, z) = data + x*zRes*yRes + y*zRes + z;
    // &elem(x, y, z) = data + x + y*xRes + z*xRes*yRes;
    int xdim, ydim, zdim;
    if (settings.getMaxWeight())
    {
        xdim = volResolution.z * volResolution.y;
        ydim = volResolution.z;
        zdim = 1;
    }
    else
    {
        xdim = 1;
        ydim = volResolution.x;
        zdim = volResolution.x * volResolution.y;
    }
    volDims = Vec4i(xdim, ydim, zdim);
    volStrides = Vec4i(xdim, ydim, zdim);
    this->neighbourCoords = Vec8i(
        volDims.dot(Vec4i(0, 0, 0)),
        volDims.dot(Vec4i(0, 0, 1)),
        volDims.dot(Vec4i(0, 1, 0)),
        volDims.dot(Vec4i(0, 1, 1)),
        volDims.dot(Vec4i(1, 0, 0)),
        volDims.dot(Vec4i(1, 0, 1)),
        volDims.dot(Vec4i(1, 1, 0)),
        volDims.dot(Vec4i(1, 1, 1))
    );

    volume = Mat(1, volResolution.x * volResolution.y * volResolution.z, rawType<TsdfVoxel>());

    reset();
}
TsdfVolume::~TsdfVolume() {}

void TsdfVolume::integrate(OdometryFrame frame, InputArray pose)
{
    std::cout << "TsdfVolume::integrate()" << std::endl;

    CV_TRACE_FUNCTION();
    Depth depth;
    frame.getDepth(depth);
    CV_Assert(depth.type() == DEPTH_TYPE);
    CV_Assert(!depth.empty());
    // TODO: remove this dependence from OdometryFrame
    depth = depth * settings.getDepthFactor();

    Matx33f intr;
    settings.getCameraIntrinsics(intr);
    Intr intrinsics(intr);
    Vec6f newParams((float)depth.rows, (float)depth.cols,
        intrinsics.fx, intrinsics.fy,
        intrinsics.cx, intrinsics.cy);
    if (!(frameParams == newParams))
    {
        frameParams = newParams;
        pixNorms = preCalculationPixNorm(depth.size(), intrinsics);
    }
    Matx44f cameraPose = pose.getMat();
    integrateVolumeUnit(truncDist, voxelSize, settings.getMaxWeight(), (this->pose).matrix, volResolution, volStrides, depth,
        settings.getDepthFactor(), cameraPose, intrinsics, pixNorms, volume);

}
void TsdfVolume::integrate(InputArray frame, InputArray pose)
{
    std::cout << "TsdfVolume::integrate()" << std::endl;

    CV_TRACE_FUNCTION();
    Depth depth = frame.getMat();
    CV_Assert(depth.type() == DEPTH_TYPE);
    CV_Assert(!depth.empty());

    Matx33f intr;
    settings.getCameraIntrinsics(intr);
    Intr intrinsics(intr);
    Vec6f newParams((float)depth.rows, (float)depth.cols,
        intrinsics.fx, intrinsics.fy,
        intrinsics.cx, intrinsics.cy);
    if (!(frameParams == newParams))
    {
        frameParams = newParams;
        pixNorms = preCalculationPixNorm(depth.size(), intrinsics);
    }
    Matx44f cameraPose = pose.getMat();

    integrateVolumeUnit(truncDist, voxelSize, settings.getMaxWeight(), (this->pose).matrix, volResolution, volStrides, depth,
        settings.getDepthFactor(), cameraPose, intrinsics, pixNorms, volume);
}

void TsdfVolume::raycast(const Matx44f& cameraPose, int height, int width, OutputArray _points, OutputArray _normals) const
{
    std::cout << "TsdfVolume::raycast()" << std::endl;
    raycastVolumeUnit(settings, cameraPose, volume, _points, _normals);
}

void TsdfVolume::fetchNormals() const {}
void TsdfVolume::fetchPointsNormals() const {}

void TsdfVolume::reset()
{
    CV_TRACE_FUNCTION();

    volume.forEach<VecTsdfVoxel>([](VecTsdfVoxel& vv, const int* /* position */)
        {
            TsdfVoxel& v = reinterpret_cast<TsdfVoxel&>(vv);
            v.tsdf = floatToTsdf(0.0f); v.weight = 0;
        });
}
int TsdfVolume::getVisibleBlocks() const { return 1; }
size_t TsdfVolume::getTotalVolumeUnits() const { return 1; }




// HASH_TSDF

HashTsdfVolume::HashTsdfVolume(VolumeSettings settings) :
    Volume::Impl(settings)
{ this->settings = settings; }
HashTsdfVolume::~HashTsdfVolume() {}

void HashTsdfVolume::integrate(OdometryFrame frame, InputArray pose) { std::cout << "HashTsdfVolume::integrate()" << std::endl; }
void HashTsdfVolume::integrate(InputArray frame, InputArray pose) { std::cout << "HashTsdfVolume::integrate()" << std::endl; }
void HashTsdfVolume::raycast(const Matx44f& cameraPose, int height, int width, OutputArray _points, OutputArray _normals) const { std::cout << "HashTsdfVolume::raycast()" << std::endl; }

void HashTsdfVolume::fetchNormals() const {}
void HashTsdfVolume::fetchPointsNormals() const {}

void HashTsdfVolume::reset() {}
int HashTsdfVolume::getVisibleBlocks() const { return 1; }
size_t HashTsdfVolume::getTotalVolumeUnits() const { return 1; }

// COLOR_TSDF

ColorTsdfVolume::ColorTsdfVolume(VolumeSettings settings) :
    Volume::Impl(settings)
{ this->settings = settings; }
ColorTsdfVolume::~ColorTsdfVolume() {}

void ColorTsdfVolume::integrate(OdometryFrame frame, InputArray pose) { std::cout << "ColorTsdfVolume::integrate()" << std::endl; }
void ColorTsdfVolume::integrate(InputArray frame, InputArray pose) { std::cout << "ColorTsdfVolume::integrate()" << std::endl; }
void ColorTsdfVolume::raycast(const Matx44f& cameraPose, int height, int width, OutputArray _points, OutputArray _normals) const { std::cout << "ColorTsdfVolume::raycast()" << std::endl; }

void ColorTsdfVolume::fetchNormals() const {}
void ColorTsdfVolume::fetchPointsNormals() const {}

void ColorTsdfVolume::reset() {}
int ColorTsdfVolume::getVisibleBlocks() const { return 1; }
size_t ColorTsdfVolume::getTotalVolumeUnits() const { return 1; }

}
