/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Copyright (C) 2013, OpenCV Foundation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "precomp.hpp"
#include <stdio.h>

/*
   This is a translation to C++ from the Matlab's LMSolve package by Miroslav Balda.
   Here is the original copyright:
   ============================================================================

   Copyright (c) 2007, Miroslav Balda
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

       * Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in
         the documentation and/or other materials provided with the distribution

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

namespace cv {

static void subMatrix(const Mat& src, Mat& dst,
                      const Mat& mask)
{
    CV_Assert(src.type() == CV_64F && dst.type() == CV_64F);
    int m = src.rows, n = src.cols;
    int i1 = 0, j1 = 0;
    for(int i = 0; i < m; i++)
    {
        if(mask.at<uchar>(i))
        {
            const double* srcptr = src.ptr<double>(i);
            double* dstptr = dst.ptr<double>(i1++);

            for(int j = j1 = 0; j < n; j++)
            {
                if(n < m || mask.at<uchar>(j))
                    dstptr[j1++] = srcptr[j];
            }
        }
    }
}

class LMSolverImpl CV_FINAL : public LMSolver
{
public:
    LMSolverImpl(const Ptr<LMSolver::Callback>& _cb, int _maxIters, double _eps = FLT_EPSILON)
        : cb(_cb), eps(_eps), maxIters(_maxIters)
    {
    }

    int run(InputOutputArray param0) const CV_OVERRIDE
    {
        return LMSolver::run(param0, noArray(), 0,
            TermCriteria(TermCriteria::COUNT | TermCriteria::EPS, maxIters, eps), DECOMP_SVD,
            [&](Mat& param, Mat* err, Mat* J)->bool
            {
                return cb->compute(param, err ? _OutputArray(*err) : _OutputArray(),
                                   J ? _OutputArray(*J) : _OutputArray());
            });
    }

    void setMaxIters(int iters) CV_OVERRIDE { CV_Assert(iters > 0); maxIters = iters; }
    int getMaxIters() const CV_OVERRIDE { return maxIters; }

    Ptr<LMSolver::Callback> cb;
    double eps;
    int maxIters;
};


Ptr<LMSolver> LMSolver::create(const Ptr<LMSolver::Callback>& cb, int maxIters, double eps)
{
    return makePtr<LMSolverImpl>(cb, maxIters, eps);
}

static int LMSolver_run(InputOutputArray _param0, InputArray _mask,
                        int nerrs, const TermCriteria& termcrit,
                        int solveMethod, bool LtoR,
                        std::function<bool (Mat&, Mat*, Mat*)>* cb,
                        std::function<bool (Mat&, Mat*, Mat*, double*)>* cb_alt)
{
    //DEBUG
    //static int ctr = 0;
    //ctr++;
    //std::cout << "old ctr: " << ctr << std::endl;

    int lambdaLg10 = -3;
    Mat mask = _mask.getMat();
    Mat param0 = _param0.getMat();
    Mat x, xd, r, rd, J, A, Ap, v, temp_d, d, Am, vm, dm;
    int p0type = param0.type();
    int maxIters = termcrit.type & TermCriteria::COUNT ? termcrit.maxCount : 1000;
    double epsx = termcrit.type & TermCriteria::EPS ? termcrit.epsilon : 0, epsf = epsx;

    CV_Assert( (param0.cols == 1 || param0.rows == 1) && (p0type == CV_32F || p0type == CV_64F));
    CV_Assert( cb || cb_alt );

    int lx = param0.rows + param0.cols - 1;
    param0.convertTo(x, CV_64F);
    d.create(lx, 1, CV_64F);

    CV_Assert(!mask.data ||
              (mask.depth() == CV_8U &&
               (mask.cols == 1 || mask.rows == 1) &&
               (mask.rows + mask.cols - 1 == lx)));
    int lxm = mask.data ? countNonZero(mask) : lx;
    if (lxm < lx) {
        Am.create(lxm, lxm, CV_64F);
        vm.create(lxm, 1, CV_64F);
    }

    if( x.cols != 1 )
        transpose(x, x);

    A.create(lx, lx, CV_64F);
    v.create(lx, 1, CV_64F);

    if (nerrs > 0) {
        J.create(nerrs, lx, CV_64F);
        r.create(nerrs, 1, CV_64F);
        rd.create(nerrs, 1, CV_64F);
    }

    double S = 0;
    int nfJ = 1;
    if (cb_alt) {
        if( !(*cb_alt)(x, &v, &A, &S) )
            return -1;
        completeSymm(A, LtoR);
    } else {
        if( !(*cb)(x, &r, &J) )
            return -1;
        S = norm(r, NORM_L2SQR);
        mulTransposed(J, A, true);
        gemm(J, r, 1, noArray(), 0, v, GEMM_1_T);
    }

    int i, iter = 0;

    for( ;; )
    {
        CV_Assert( A.type() == CV_64F && A.rows == lx );
        A.copyTo(Ap);
        double lambda = exp(lambdaLg10*log(10.));
        for( i = 0; i < lx; i++ )
            Ap.at<double>(i, i) *= (1 + lambda);
        if (lxm < lx) {
            // remove masked-out rows & cols from JtJ and JtErr
            subMatrix(Ap, Am, mask);
            subMatrix(v, vm, mask);
            solve(Am, vm, dm, solveMethod);
            int j = 0;
            // 'unpack' the param delta
            for(i = j = 0; i < lx; i++)
                d.at<double>(i) = mask.at<uchar>(i) != 0 ? dm.at<double>(j++) : 0.;
        } else {
            solve(Ap, v, d, solveMethod);
        }
        subtract(x, d, xd);

        //DEBUG
        //if (ctr == 1003)
        //{
        //    std::cout << "S: " << S << std::endl;
        //    std::cout << "Ap.diag(): " << Ap.diag() << std::endl;
        //    std::cout << "lambda: " << lambda << std::endl;
        //    std::cout << "v: " << v << std::endl;
        //    std::cout << "d: " << d << std::endl;
        //}

        double Sd = 0.;

        if (cb_alt) {
            if( !(*cb_alt)(xd, 0, 0, &Sd) )
                return -1;
        } else {
            if( !(*cb)(xd, &rd, 0) )
                return -1;
            Sd = norm(rd, NORM_L2SQR);
        }

        nfJ++;
        if( Sd < S )
        {
            nfJ++;
            S = Sd;
            lambdaLg10 = MAX(lambdaLg10-1, -16);
            iter++;
            std::swap(x, xd);
            if (cb_alt) {
                v.setZero();
                A.setZero();
                Sd = 0.;
                if( !(*cb_alt)(x, &v, &A, &Sd) )
                    return -1;
                completeSymm(A, LtoR);
            } else {
                r.setZero();
                J.setZero();
                if( !(*cb)(x, &r, &J) )
                    return -1;
                mulTransposed(J, A, true);
                gemm(J, r, 1, noArray(), 0, v, GEMM_1_T);
            }
        } else {
            iter += lambdaLg10 == 16;
            lambdaLg10 = MIN(lambdaLg10+1, 16);
        }

        bool proceed = iter < maxIters && norm(d, NORM_INF) >= epsx && S >= epsf*epsf;

        //DEBUG
        //printf("%c %d %d, err=%g, lg10(lambda)=%d\n",
        //       (proceed ? ' ' : '*'), iter, nfJ, S, lambdaLg10);
        /*
        if(lxm < lx)
        {
            printf("lambda=%g. delta:", lambda);
            int j;
            for(i = j = 0; i < lx; i++) {
                double delta = d.at<double>(i);
                j += delta != 0;
                if(j < 10)
                    printf(" %.2g", delta);
            }
            printf("\n");
            printf("%c %d %d, err=%g, param[0]=%g, d[0]=%g, lg10(lambda)=%d\n",
                   (proceed ? ' ' : '*'), iter, nfJ, S, x.at<double>(0), d.at<double>(0), lambdaLg10);
        }
        */
        if(!proceed)
            break;
    }

    if( param0.size() != x.size() )
        transpose(x, x);

    x.convertTo(param0, p0type);
    if( iter == maxIters )
        iter = -iter;

    return iter;
}

int LMSolver::run(InputOutputArray param, InputArray mask, int nerrs,
                  const TermCriteria& termcrit, int solveMethod,
                  LMSolver::LongCallback cb)
{
    return LMSolver_run(param, mask, nerrs, termcrit, solveMethod, true, &cb, 0);
}

int LMSolver::runAlt(InputOutputArray param, InputArray mask,
                     const TermCriteria& termcrit, int solveMethod, bool LtoR,
                     LMSolver::AltCallback cb_alt)
{
    return LMSolver_run(param, mask, 0, termcrit, solveMethod, LtoR, 0, &cb_alt);
}


// ----------------------------------------------------------------------------


// from Ceres, equation energy change:
// eq. energy = 1/2 * (residuals + J * step)^2 =
// 1/2 * ( residuals^2 + 2 * residuals^T * J * step + (J*step)^T * J * step)
// eq. energy change = 1/2 * residuals^2 - eq. energy =
// residuals^T * J * step + 1/2 * (J*step)^T * J * step =
// (residuals^T * J + 1/2 * step^T * J^T * J) * step =
// step^T * ((residuals^T * J)^T + 1/2 * (step^T * J^T * J)^T) =
// 1/2 * step^T * (2 * J^T * residuals + J^T * J * step) =
// 1/2 * step^T * (2 * J^T * residuals + (J^T * J + LMDiag - LMDiag) * step) =
// 1/2 * step^T * (2 * J^T * residuals + (J^T * J + LMDiag) * step - LMDiag * step) =
// 1/2 * step^T * (J^T * residuals - LMDiag * step) =
// 1/2 * x^T * (jtb - lmDiag^T * x)
static double calcJacCostChangeLm(const cv::Mat_<double>& jtb, const cv::Mat_<double>& x, const cv::Mat_<double>& lmDiag)
{
    return cv::sum(x.mul(jtb - lmDiag.mul(x)))[0] * 0.5;
}


BaseLevMarq::Report BaseLevMarq::optimize()
{
    //DEBUG
    //static int ctr = 0;
    //ctr++;
    //std::cout << "ctr: " << ctr << std::endl;

    pImpl->prepareVars();

    double energy = 0.0;
    if (!pImpl->calcFunc(energy, /*useProbeVars*/ false, /*calcEnergy*/ true, /*calcJacobian*/ true) || energy < 0)
    {
        CV_LOG_INFO(NULL, "Error while calculating energy function");
        return BaseLevMarq::Report(false, 0, 0); // not found
    }

    double oldEnergy = energy;

    CV_LOG_INFO(NULL, "#s" << " energy: " << energy);

    // diagonal clamping options
    const double minDiag = 1e-6;
    const double maxDiag = 1e32;
    // limit lambda inflation
    const double maxLambda = 1e32;

    // finish reasons
    bool tooLong = false; // => not found
    bool bigLambda = false; // => not found
    bool smallGradient = false; // => found
    bool smallStep = false; // => found
    bool smallEnergyDelta = false; // => found
    bool smallEnergy = false; // => found

    // column scale inverted, for jacobian scaling
    Mat_<double> di;

    // do the jacobian conditioning improvement used in Ceres
    if (jacobiScaling)
    {
        // L2-normalize each jacobian column
        // vec d = {d_j = sum(J_ij^2) for each column j of J} = get_diag{ J^T * J }
        // di = { 1/(1+sqrt(d_j)) }, extra +1 to avoid div by zero
        Mat_<double> ds;
        const Mat_<double> diag = pImpl->getDiag();
        cv::sqrt(diag, ds);
        di = 1.0 / (ds + 1.0);
    }

    double lmUpFactor = initialLmUpFactor;
    double lambdaLevMarq = initialLambdaLevMarq;

    unsigned int iter = 0;
    bool done = false;
    while (!done)
    {
        // At this point we should have jtj and jtb built

        CV_LOG_INFO(NULL, "#LM#s" << " energy: " << energy);

        // do the jacobian conditioning improvement used in Ceres
        if (jacobiScaling)
        {
            pImpl->doJacobiScaling(di);
        }

        const Mat_<double> jtb = pImpl->getJtb();

        double gradientMax = cv::norm(jtb, NORM_INF);

        // Save original diagonal of jtj matrix for LevMarq
        const Mat_<double> diag = pImpl->getDiag();

        // Solve using LevMarq and get delta transform
        bool enoughLm = false;

        while (!enoughLm && !done)
        {
            // form LevMarq matrix
            Mat_<double> lmDiag, jtjDiag;
            lmDiag = diag * lambdaLevMarq;
            if (clampDiagonal)
                lmDiag = cv::min(cv::max(lmDiag, minDiag), maxDiag);
            jtjDiag = lmDiag + diag;
            pImpl->setDiag(jtjDiag);

            CV_LOG_INFO(NULL, "linear solve...");

            // use double or convert everything to float
            Mat_<double> x((int)jtb.rows, 1);
            bool solved = pImpl->solve(x);

            //DEBUG
            //if (ctr == 1003)
            //{
            //    std::cout << "energy: " << energy << std::endl;
            //    std::cout << "jtjDiag: " << jtjDiag << std::endl;
            //    std::cout << "lambda: " << lambdaLevMarq << std::endl;
            //    std::cout << "jtb: " << jtb << std::endl;
            //    std::cout << "x: " << x << std::endl;
            //}

            CV_LOG_INFO(NULL, (solved ? "OK" : "FAIL"));

            double costChange = 0.0;
            double jacCostChange = 0.0;
            double stepQuality = 0.0;
            double xNorm = 0.0;
            if (solved)
            {
                // what energy drop should be according to local model estimation
                jacCostChange = calcJacCostChangeLm(jtb, x, lmDiag);

                // x norm
                xNorm = cv::norm(x, stepNormInf ? NORM_INF : NORM_L2SQR);

                // undo jacobi scaling
                if (jacobiScaling)
                {
                    x = x.mul(di);
                }

                // remember, we move against the gradient direction
                x = -x;

                // calc energy with current delta x
                pImpl->currentOplusXToProbe(x);

                bool success = pImpl->calcFunc(energy, /*useProbeVars*/ true, /*calcEnergy*/ true, /*calcJacobian*/ false);
                if (!success || energy < 0 || isnan(energy))
                {
                    CV_LOG_INFO(NULL, "Error while calculating energy function");
                    return BaseLevMarq::Report(false, iter, oldEnergy); // not found
                }

                costChange = oldEnergy - energy;

                stepQuality = costChange / jacCostChange;

                CV_LOG_INFO(NULL, "#LM#" << iter
                    << " energy: " << energy
                    << " deltaEnergy: " << costChange
                    << " deltaEqEnergy: " << jacCostChange
                    << " max(J^T*b): " << gradientMax
                    << (stepNormInf ? " normInf(x): " : " norm2(x): ") << xNorm
                    << " deltaEnergy/energy: " << costChange / energy);
            }

            // zero cost change is treated like an algorithm failure if checkRelEnergyChange is off
            if (!solved || costChange < 0 || (!checkRelEnergyChange && abs(costChange) < DBL_EPSILON))
            {
                // failed to optimize, increase lambda and repeat

                lambdaLevMarq *= lmUpFactor;
                if (upDouble)
                    lmUpFactor *= 2.0;

                CV_LOG_INFO(NULL, "LM goes up, lambda: " << lambdaLevMarq << ", old energy: " << oldEnergy);
            }
            else
            {
                // optimized successfully, decrease lambda and set variables for next iteration
                enoughLm = true;

                if (useStepQuality)
                    lambdaLevMarq *= std::max(1.0 / initialLmDownFactor, 1.0 - pow(2.0 * stepQuality - 1.0, 3));
                else
                    lambdaLevMarq *= 1.0 / initialLmDownFactor;
                lmUpFactor = initialLmUpFactor;

                smallGradient = (gradientMax < minGradientTolerance);
                smallStep = (xNorm < stepNormTolerance);
                smallEnergyDelta = (costChange / energy < relEnergyDeltaTolerance);
                smallEnergy = (energy < smallEnergyTolerance);

                pImpl->acceptProbe();

                CV_LOG_INFO(NULL, "#" << iter << " energy: " << energy);

                oldEnergy = energy;

                CV_LOG_INFO(NULL, "LM goes down, lambda: " << lambdaLevMarq << " step quality: " << stepQuality);
            }

            iter++;

            tooLong = (iter >= maxIterations);
            bigLambda = (lambdaLevMarq >= maxLambda);

            done = tooLong || bigLambda;
            done = done || (checkMinGradient && smallGradient);
            done = done || (checkStepNorm && smallStep);
            done = done || (checkRelEnergyChange && smallEnergyDelta);
            done = done || (smallEnergy);
        }

        // calc jacobian for next iteration
        if (!done)
        {
            double dummy;
            if (!pImpl->calcFunc(dummy, /*useProbeVars*/ false, /*calcEnergy*/ false, /*calcJacobian*/ true))
            {
                CV_LOG_INFO(NULL, "Error while calculating jacobian");
                return BaseLevMarq::Report(false, iter, oldEnergy); // not found
            }
        }
    }

    bool found = (smallGradient || smallStep || smallEnergyDelta || smallEnergy);

    CV_LOG_INFO(NULL, "Finished: " << (found ? "" : "not ") << "found");
    std::string fr = "Finish reason: ";
    if (checkMinGradient && smallGradient)
        CV_LOG_INFO(NULL, fr + "gradient max val dropped below threshold");
    if (checkStepNorm && smallStep)
        CV_LOG_INFO(NULL, fr + "step size dropped below threshold");
    if (checkRelEnergyChange && smallEnergyDelta)
        CV_LOG_INFO(NULL, fr + "relative energy change between iterations dropped below threshold");
    if (smallEnergy)
        CV_LOG_INFO(NULL, fr + "energy dropped below threshold");
    if (tooLong)
        CV_LOG_INFO(NULL, fr + "max number of iterations reached");
    if (bigLambda)
        CV_LOG_INFO(NULL, fr + "lambda has grown above the threshold, the trust region is too small");

    return BaseLevMarq::Report(found, iter, oldEnergy);
}


struct LevMarqDenseLinearImpl : public BaseLevMarq::Impl
{
    // all variables including fixed ones
    size_t nVars;
    size_t allVars;
    Mat_<double> jtj, jtb;
    Mat_<double> probeX, currentX;
    // for oplus operation
    Mat_<double> delta;

    // "Long" callback: f(x, &b, &J) -> bool
    // Produces jacobian and residuals for each energy term
    LevMarqDenseLinear::LongCallback cb;
    // "Normal" callback: f(x, &jtb, &jtj, &energy) -> bool
    // Produces J^T*J and J^T*b directly instead of J and b
    LevMarqDenseLinear::NormalCallback cb_alt;

    Mat_<uchar> mask;
    // full matrices containing all vars including fixed ones
    // used only when mask is not empty
    Mat_<double> jtjFull, jtbFull;

    // used only with long callback
    Mat_<double> jLong, bLong;
    size_t nerrs;
    // used only with alt. callback
    // What part of symmetric matrix is to copy to another part
    bool LtoR;
    // What method to use for linear system solving
    int solveMethod;

    LevMarqDenseLinearImpl(int nvars_, LevMarqDenseLinear::LongCallback callback_, InputArray mask_, int nerrs_, int solveMethod_) :
        LevMarqDenseLinearImpl(noArray(), nvars_, callback_, nullptr, nerrs_, false, mask_, solveMethod_)
    { }
    LevMarqDenseLinearImpl(int nvars_, LevMarqDenseLinear::NormalCallback callback_, InputArray mask_, bool LtoR_, int solveMethod_) :
        LevMarqDenseLinearImpl(noArray(), nvars_, nullptr, callback_, 0, LtoR_, mask_, solveMethod_)
    { }
    LevMarqDenseLinearImpl(InputOutputArray param_, LevMarqDenseLinear::LongCallback callback_, InputArray mask_, int nerrs_, int solveMethod_) :
        LevMarqDenseLinearImpl(param_, 0, callback_, nullptr, nerrs_, false, mask_, solveMethod_)
    { }
    LevMarqDenseLinearImpl(InputOutputArray param_, LevMarqDenseLinear::NormalCallback callback_, InputArray mask_, bool LtoR_, int solveMethod_) :
        LevMarqDenseLinearImpl(param_, 0, nullptr, callback_, 0, LtoR_, mask_, solveMethod_)
    { }

    LevMarqDenseLinearImpl(InputOutputArray currentX_, int nvars,
        LevMarqDenseLinear::LongCallback cb_ = nullptr,
        LevMarqDenseLinear::NormalCallback cb_alt_ = nullptr,
        size_t nerrs_ = 0,
        bool LtoR_ = false,
        InputArray mask_ = noArray(),
        int solveMethod_ = DECOMP_SVD) :
        BaseLevMarq::Impl(),
        // these fields will be initialized at prepareVars()
        jtj(),
        jtb(),
        probeX(),
        delta(),
        jtjFull(),
        jtbFull(),
        jLong(),
        bLong()
    {
        if (!currentX_.empty())
        {
            CV_Assert(currentX_.type() == CV_64F);
            CV_Assert(currentX_.rows() == 1 || currentX_.cols() == 1);
            this->allVars = currentX_.size().area();
            this->currentX = currentX_.getMat().reshape(1, (int)this->allVars);
        }
        else
        {
            CV_Assert(nvars > 0);
            this->allVars = nvars;
            this->currentX = Mat_<double>((int)this->allVars, 1);
        }

        CV_Assert(cb_ || cb_alt_ && !(cb_ && cb_alt_));
        this->cb = cb_;
        this->cb_alt = cb_alt_;

        this->nerrs = nerrs_;
        this->LtoR = LtoR_;
        this->solveMethod = solveMethod_;

        if (!mask_.empty())
        {
            CV_Assert(mask_.depth() == CV_8U);
            CV_Assert(mask_.size() == currentX_.size());
            int maskSize = mask_.size().area();
            this->mask.create(maskSize, 1);
            mask_.copyTo(this->mask);
        }
        else
            this->mask = Mat_<uchar>();

        this->nVars = this->mask.empty() ? this->allVars : countNonZero(this->mask);
        CV_Assert(this->nVars > 0);
    }


    static void subMatrix(const Mat_<double>& src, Mat_<double>& dst, const Mat_<uchar>& mask)
    {
        CV_Assert(src.type() == CV_64F && dst.type() == CV_64F);
        int m = src.rows, n = src.cols;
        int i1 = 0, j1 = 0;
        for (int i = 0; i < m; i++)
        {
            if (mask(i))
            {
                const double* srcptr = src[i];
                double* dstptr = dst[i1++];

                for (int j = j1 = 0; j < n; j++)
                {
                    if (n < m || mask(j))
                        dstptr[j1++] = srcptr[j];
                }
            }
        }
    }

    virtual bool calcFunc(double& energy, bool useProbeVars = false, bool calcEnergy = true, bool calcJacobian = false) CV_OVERRIDE
    {
        Mat_<double> xd = useProbeVars ? probeX : currentX;

        double sd = 0.0;
        Mat jtbp, jtjp, jLongp;
        if (calcJacobian)
        {
            jtbp = (!mask.empty()) ? jtbFull : jtb;
            jtjp = (!mask.empty()) ? jtjFull : jtj;

            jtbp.setZero();
            jtjp.setZero();

            if (!cb_alt)
            {
                jLong.setZero();
                jLongp = jLong;
            }
        }

        if (cb_alt)
        {
            bool r = calcJacobian ? cb_alt(xd, jtbp, jtjp, sd) : cb_alt(xd, noArray(), noArray(), sd);
            if (!r)
                return false;
        }
        else
        {
            bLong.setZero();
            bool r = calcJacobian ? cb(xd, bLong, jLongp) : cb(xd, bLong, noArray());
            if (!r)
                return false;
        }

        if (calcJacobian)
        {
            if (cb_alt)
            {
                completeSymm(jtjp, LtoR);
            }
            else
            {
                mulTransposed(jLongp, jtjp, true);
                gemm(jLongp, bLong, 1, noArray(), 0, jtbp, GEMM_1_T);
            }
        }

        if (calcEnergy)
        {
            if (cb_alt)
            {
                energy = sd;
            }
            else
            {
                energy = norm(bLong, NORM_L2SQR);
            }
        }

        if (!mask.empty())
        {
            subMatrix(jtjFull, jtj, mask);
            subMatrix(jtbFull, jtb, mask);
        }

        return true;
    }

    // adds x to current variables and writes result to probe vars
    virtual void currentOplusXToProbe(const Mat_<double>& x) CV_OVERRIDE
    {
        // 'unpack' the param delta
        int j = 0;
        if (!mask.empty())
        {
            for (int i = 0; i < allVars; i++)
            {
                delta.at<double>(i) = (mask.at<uchar>(i) != 0) ? x(j++) : 0.0;
            }
        }
        else
            delta = x;

        probeX = currentX + delta;
    }

    virtual void prepareVars() CV_OVERRIDE
    {
        jtj = Mat_<double>((int)nVars, (int)nVars);
        jtb = Mat_<double>((int)nVars, 1);

        probeX = currentX.clone();
        delta = Mat_<double>((int)allVars, 1);

        // Allocate vars for use with mask
        if (!mask.empty())
        {
            jtjFull = Mat_<double>((int)allVars, (int)allVars);
            jtbFull = Mat_<double>((int)allVars, 1);
        }

        if (nerrs)
        {
            jLong = Mat_<double>((int)nerrs, (int)allVars);
            bLong = Mat_<double>((int)nerrs, 1, CV_64F);
        }
    }

    virtual const Mat_<double> getDiag() CV_OVERRIDE
    {
        return jtj.diag().clone();
    }

    virtual const Mat_<double> getJtb() CV_OVERRIDE
    {
        return jtb;
    }

    virtual void setDiag(const Mat_<double>& d) CV_OVERRIDE
    {
        d.copyTo(jtj.diag());
    }

    virtual void doJacobiScaling(const Mat_<double>& di) CV_OVERRIDE
    {
        // J := J * d_inv, d_inv = make_diag(di)
        // J^T*J := (J * d_inv)^T * J * d_inv = diag(di) * (J^T * J) * diag(di) = eltwise_mul(J^T*J, di*di^T)
        // J^T*b := (J * d_inv)^T * b = d_inv^T * J^T*b = eltwise_mul(J^T*b, di)
        // scaling J^T*J
        for (int i = 0; i < (int)nVars; i++)
        {
            double* jtjrow = jtj.ptr<double>(i);
            for (int j = 0; j < (int)nVars; j++)
            {
                jtjrow[j] *= di(i) * di(j);
            }
        }
        // scaling J^T*b
        for (int i = 0; i < (int)nVars; i++)
        {
            jtb(i) *= di(i);
        }
    }

    virtual bool solve(Mat_<double>& x) CV_OVERRIDE
    {
        return cv::solve(jtj, jtb, x, solveMethod);
    }

    virtual void acceptProbe() CV_OVERRIDE
    {
        probeX.copyTo(currentX);
    }
};


/*
Ptr<BaseLevMarq> createLegacyLevMarq(InputOutputArray currentX,
                                     int maxIter,
                                     LevMarqDenseLinear::LongCallback cb,
                                     LevMarqDenseLinear::AltCallback cb_alt,
                                     size_t nerrs,
                                     bool LtoR,
                                     InputArray mask,
                                     int solveMethod)
{
    auto solver = makePtr<LegacyLevMarq>(currentX, cb, cb_alt, nerrs, LtoR, mask, solveMethod);

    solver->initialLambdaLevMarq = 0.001;
    solver->initialLmUpFactor = 10.0;
    solver->initialLmDownFactor = 10.0;
    solver->upDouble = false;
    solver->useStepQuality = false;
    solver->clampDiagonal = false;
    solver->checkRelEnergyChange = false;
    solver->stepNormInf = true;
    solver->checkMinGradient = false;
    // old LMSolver calculates successful iterations only, this one calculates all iterations
    solver->maxIterations = (unsigned int)(maxIter * 2.1);
    solver->checkStepNorm = true;
    solver->stepNormTolerance = (double)FLT_EPSILON;
    solver->smallEnergyTolerance = (double)FLT_EPSILON * (double)FLT_EPSILON;

    return solver;
}
*/

/*
class LevMarqDenseLinearImpl CV_FINAL : public LevMarqDenseLinear
{
public:
    LevMarqDenseLinearImpl(const Ptr<LevMarqDenseLinear::Callback>& _cb, int _maxIters, double _eps = FLT_EPSILON)
        : cb(_cb), cbNormal(), eps(_eps), maxIters(_maxIters)
    { }
    LevMarqDenseLinearImpl(const Ptr<LevMarqDenseLinear::NormalCallback>& _cb, int _maxIters, double _eps = FLT_EPSILON)
        : cb(), cbNormal(_cb), eps(_eps), maxIters(_maxIters)
    { }

    int run(InputOutputArray param0) const CV_OVERRIDE
    {
        if (cb)
        {
            return LevMarqDenseLinear::run(param0, noArray(), 0,
                TermCriteria(TermCriteria::COUNT | TermCriteria::EPS, maxIters, eps), DECOMP_SVD,
                [&](Mat& param, Mat* err, Mat* J)->bool
                {
                    return cb->compute(param, err ? _OutputArray(*err) : _OutputArray(),
                                       J ? _OutputArray(*J) : _OutputArray());
                });
        }
        else if (cbNormal)
        {
            return LevMarqDenseLinear::runAlt(param0, noArray(),
                TermCriteria(TermCriteria::COUNT | TermCriteria::EPS, maxIters, eps), DECOMP_SVD, true,
                [&](Mat& param, Mat* JtErr, Mat* JtJ, double* errnorm)->bool
                {
                    double dummy;
                    return cbNormal->compute(param, JtErr ? OutputArray(*JtErr) : noArray(),
                                             JtJ ? OutputArray(*JtJ) : noArray(),
                                             errnorm ? *errnorm : dummy);
                }
            );
        }
        else
        {
            CV_Error(CV_StsBadArg, "No callback given");
        }
    }

};


int LevMarqDenseLinear::run(InputOutputArray param, InputArray mask,
                            int nerrs, const TermCriteria& termCrit, int solveMethod,
                            LevMarqDenseLinear::LongCallback callb)
{
    auto lm = createLegacyLevMarq(param, termCrit.maxCount, callb, nullptr, nerrs, false, mask, solveMethod);
    // implementing the same epsilon checks as in legacy code
    lm->stepNormTolerance = termCrit.epsilon;
    lm->smallEnergyTolerance = termCrit.epsilon * termCrit.epsilon;
    return lm->optimize();
}

int LevMarqDenseLinear::runAlt(InputOutputArray param, InputArray mask,
                               const TermCriteria& termCrit, int solveMethod, bool LtoR,
                               LevMarqDenseLinear::AltCallback callb)
{
    auto lm = createLegacyLevMarq(param, termCrit.maxCount, nullptr, callb, 0, LtoR, mask, solveMethod);
    // implementing the same epsilon checks as in legacy code
    lm->stepNormTolerance = termCrit.epsilon;
    lm->smallEnergyTolerance = termCrit.epsilon * termCrit.epsilon;
    return lm->optimize();
}
*/

LevMarqDenseLinear::LevMarqDenseLinear(int nvars, LongCallback callback, InputArray mask, int nerrs, int solveMethod) :
    BaseLevMarq(makePtr<LevMarqDenseLinearImpl>(nvars, callback, mask, nerrs, solveMethod))
{ }
LevMarqDenseLinear::LevMarqDenseLinear(int nvars, NormalCallback callback, InputArray mask, bool LtoR, int solveMethod) :
    BaseLevMarq(makePtr<LevMarqDenseLinearImpl>(nvars, callback, mask, LtoR, solveMethod))
{ }
LevMarqDenseLinear::LevMarqDenseLinear(InputOutputArray param, LongCallback callback, InputArray mask, int nerrs, int solveMethod) :
    BaseLevMarq(makePtr<LevMarqDenseLinearImpl>(param, callback, mask, nerrs, solveMethod))
{ }
LevMarqDenseLinear::LevMarqDenseLinear(InputOutputArray param, NormalCallback callback, InputArray mask, bool LtoR, int solveMethod) :
    BaseLevMarq(makePtr<LevMarqDenseLinearImpl>(param, callback, mask, LtoR, solveMethod))
{ }

BaseLevMarq::Report LevMarqDenseLinear::run(InputOutputArray param)
{
    CV_Assert(!param.empty() && (param.type() == CV_64F) && (param.rows() == 1 || param.cols() == 1));
    pImpl.dynamicCast<LevMarqDenseLinearImpl>()->currentX = param.getMat().reshape(1, param.size().area());
    return optimize();
}

}
