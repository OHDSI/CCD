//
// Created by Jianxiao Yang on 2019-12-19.
//

#ifndef GPUMODELSPECIFICSCOX_HPP
#define GPUMODELSPECIFICSCOX_HPP


// #define USE_VECTOR
#undef USE_VECTOR

// #define GPU_DEBUG
#undef GPU_DEBUG
//#define USE_LOG_SUM
#define TIME_DEBUG

//#include <Rcpp.h>

#include "BaseGpuModelSpecifics.hpp"
#include "Iterators.h"
#include "CubScan.h"

namespace bsccs{

    namespace compute = boost::compute;

//    namespace detail {
//
//        namespace constant {
//            static const int updateXBetaBlockSize = 256; // 512; // Appears best on K40
//            static const int updateAllXBetaBlockSize = 32;
//            int exactCLRBlockSize = 32;
//            int exactCLRSyncBlockSize = 32;
//            static const int maxBlockSize = 256;
//        }; // namespace constant
//    }; // namespace detail

    template <class BaseModel, typename RealType>
    class GpuModelSpecificsCox :
            public BaseGpuModelSpecifics<BaseModel, RealType> {
    public:

#ifdef CYCLOPS_DEBUG_TIMING
        using ModelSpecifics<BaseModel, RealType>::duration;
#endif
        using BaseGpuModelSpecifics<BaseModel, RealType>::ctx;
        using BaseGpuModelSpecifics<BaseModel, RealType>::device;
        using BaseGpuModelSpecifics<BaseModel, RealType>::queue;
        using BaseGpuModelSpecifics<BaseModel, RealType>::neededFormatTypes;
        using BaseGpuModelSpecifics<BaseModel, RealType>::double_precision;

        using BaseGpuModelSpecifics<BaseModel, RealType>::modelData;
        using BaseGpuModelSpecifics<BaseModel, RealType>::hX;
//        using BaseGpuModelSpecifics<BaseModel, RealType>::hPid;
//        using BaseGpuModelSpecifics<BaseModel, RealType>::K;
//        using BaseGpuModelSpecifics<BaseModel, RealType>::J;
        using BaseGpuModelSpecifics<BaseModel, RealType>::N;
        using BaseGpuModelSpecifics<BaseModel, RealType>::offsExpXBeta;
        using BaseGpuModelSpecifics<BaseModel, RealType>::hXBeta;
//        using BaseGpuModelSpecifics<BaseModel, RealType>::hY;
//        using BaseGpuModelSpecifics<BaseModel, RealType>::hOffs;
        using BaseGpuModelSpecifics<BaseModel, RealType>::denomPid;
//        using BaseGpuModelSpecifics<BaseModel, RealType>::denomPid2;
//        using BaseGpuModelSpecifics<BaseModel, RealType>::numerPid;
//        using BaseGpuModelSpecifics<BaseModel, RealType>::numerPid2;
//        using BaseGpuModelSpecifics<BaseModel, RealType>::hNWeight;
//        using BaseGpuModelSpecifics<BaseModel, RealType>::hKWeight;
        using BaseGpuModelSpecifics<BaseModel, RealType>::accDenomPid;
//        using BaseGpuModelSpecifics<BaseModel, RealType>::accNumerPid;
//        using BaseGpuModelSpecifics<BaseModel, RealType>::accNumerPid2;

        using BaseGpuModelSpecifics<BaseModel, RealType>::dKWeight;
//        using BaseGpuModelSpecifics<BaseModel, RealType>::dNWeight;
        using BaseGpuModelSpecifics<BaseModel, RealType>::dY;
//        using BaseGpuModelSpecifics<BaseModel, RealType>::dOffs;
        using BaseGpuModelSpecifics<BaseModel, RealType>::dId;
//        using BaseGpuModelSpecifics<BaseModel, RealType>::dBeta;
        using BaseGpuModelSpecifics<BaseModel, RealType>::dXBeta;
        using BaseGpuModelSpecifics<BaseModel, RealType>::dExpXBeta;
        using BaseGpuModelSpecifics<BaseModel, RealType>::dDenominator;
//        using BaseGpuModelSpecifics<BaseModel, RealType>::dDenominator2;
        using BaseGpuModelSpecifics<BaseModel, RealType>::dAccDenominator;
        using BaseGpuModelSpecifics<BaseModel, RealType>::dColumns;

        std::vector<double> hBuffer;
        std::vector<double> hBuffer1;

        int tpb = 256; // threads-per-block  // Appears best on K40
        int PSC_K = 32;
        int PSC_WG_SIZE = 256;

        GpuModelSpecificsCox(const ModelData<RealType>& input,
                             const std::string& deviceName)
        : BaseGpuModelSpecifics<BaseModel, RealType>(input, deviceName),
          dBuffer(ctx), dBuffer1(ctx){

            std::cerr << "ctor GpuModelSpecificsCox" << std::endl;

        }

        virtual ~GpuModelSpecificsCox() {
            std::cerr << "dtor GpuModelSpecificsCox" << std::endl;
        }

        virtual void deviceInitialization() {
            BaseGpuModelSpecifics<BaseModel, RealType>::deviceInitialization();
            hXBetaKnown = true;
            dXBetaKnown = true;
            buildAllKernels(neededFormatTypes);
        }


        virtual void computeRemainingStatistics(bool useWeights) {

            std::cerr << "GPU::cRS called" << std::endl;

            // Currently RS only computed on CPU and then copied
            ModelSpecifics<BaseModel, RealType>::computeRemainingStatistics(useWeights);

#ifdef CYCLOPS_DEBUG_TIMING
            auto start = bsccs::chrono::steady_clock::now();
#endif
            /*
            if (algorithmType == AlgorithmType::MM) {
                compute::copy(std::begin(hBeta), std::end(hBeta), std::begin(dBeta), queue);
            }
            */

            if (BaseModel::likelihoodHasDenominator) {
                compute::copy(std::begin(offsExpXBeta), std::end(offsExpXBeta), std::begin(dExpXBeta), queue);
                compute::copy(std::begin(denomPid), std::end(denomPid), std::begin(dDenominator), queue);
            }


#ifdef CYCLOPS_DEBUG_TIMING
            auto end = bsccs::chrono::steady_clock::now();
            ///////////////////////////"
            duration["compRSG          "] += bsccs::chrono::duration_cast<chrono::TimingUnits>(end - start).count();;
#endif

        }

        virtual void updateXBeta(double realDelta, int index, bool useWeights) {

#ifdef GPU_DEBUG
            ModelSpecifics<BaseModel, WeightType>::updateXBeta(realDelta, index, useWeights);
#endif // GPU_DEBUG

#ifdef CYCLOPS_DEBUG_TIMING
            auto start = bsccs::chrono::steady_clock::now();
#endif

            FormatType formatType = hX.getFormatType(index);
            auto& kernel = kernelUpdateXBeta[formatType];
//         auto& column = columns[index];
//         const auto taskCount = column.getTaskCount();
            const auto taskCount = dColumns.getTaskCount(index);

//            std::cout << "taskCount: " << taskCount << " dDenominator.size(): " << dDenominator.size() << '\n';
            kernel.set_arg(0, dColumns.getDataOffset(index)); // offX
            kernel.set_arg(1, dColumns.getIndicesOffset(index)); // offK
            kernel.set_arg(2, taskCount); // N
            if (double_precision) {
                kernel.set_arg(3, realDelta);
            } else {
                auto fDelta = (float)realDelta;
                kernel.set_arg(3, fDelta);
            }
            kernel.set_arg(4, dColumns.getData()); // *X
            kernel.set_arg(5, dColumns.getIndices()); // *K
//            kernel.set_arg(6, dY);
//            kernel.set_arg(7, dXBeta);
//            kernel.set_arg(8, dExpXBeta);
//            kernel.set_arg(9, dDenominator);
//            kernel.set_arg(10, dId);

            size_t workGroups = taskCount / detail::constant::updateXBetaBlockSize;
            if (taskCount % detail::constant::updateXBetaBlockSize != 0) {
                ++workGroups;
            }
            const size_t globalWorkSize = workGroups * detail::constant::updateXBetaBlockSize;

            queue.enqueue_1d_range_kernel(kernel, 0, globalWorkSize, detail::constant::updateXBetaBlockSize);
            queue.finish();

            hXBetaKnown = false; // dXBeta was just updated

#ifdef CYCLOPS_DEBUG_TIMING
            auto end = bsccs::chrono::steady_clock::now();
            ///////////////////////////"
            auto name = "updateXBetaG" + getFormatTypeExtension(formatType) + "  ";
            duration[name] += bsccs::chrono::duration_cast<chrono::TimingUnits>(end - start).count();
#endif

            // copy results to host
            hXBeta.resize(dExpXBeta.size());
            compute::copy(std::begin(dXBeta), std::end(dXBeta), std::begin(hXBeta), queue);

            offsExpXBeta.resize(dExpXBeta.size());
            compute::copy(std::begin(dExpXBeta), std::end(dExpXBeta), std::begin(offsExpXBeta), queue);

            denomPid.resize(dDenominator.size());
            compute::copy(std::begin(dDenominator), std::end(dDenominator), std::begin(denomPid), queue);

            // print results
//            std::cout << "dXBeta: ";
//            for (auto x:hXBeta) {
//                std::cout << x << " ";
//            }
//            std::cout << "\n";
//            std::cout << "dExpXBeta: ";
//            for (auto x:offsExpXBeta) {
//                std::cout << x << " ";
//            }
//            std::cout << "\n";
//            std::cout << "dDenominator: ";
//            for (auto x:denomPid) {
//                std::cout << x << " ";
//            }
//            std::cout << "\n";

//            ModelSpecifics<BaseModel, RealType>::computeAccumlatedDenominator(useWeights);
            computeAccumlatedDenominator(useWeights);

#ifdef GPU_DEBUG
            // Compare results:
        detail::compare(hXBeta, dXBeta, "xBeta not equal");
        detail::compare(offsExpXBeta, dExpXBeta, "expXBeta not equal");
        detail::compare(denomPid, dDenominator, "denominator not equal");
#endif // GPU_DEBUG
        }

        virtual void computeAccumlatedDenominator(bool useWeights) {

#ifdef CYCLOPS_DEBUG_TIMING
            auto start = bsccs::chrono::steady_clock::now();
#endif
            CubScan(&denomPid[0], &accDenomPid[0], N);
	    /*
	    int tc = dDenominator.size();
            const auto taskCount = tc;
            size_t workGroups = taskCount / detail::constant::updateXBetaBlockSize;
            if (taskCount % detail::constant::updateXBetaBlockSize != 0) {
                ++workGroups;
            }
            const size_t globalWorkSize = workGroups * detail::constant::updateXBetaBlockSize;
            const auto intervalSize = 8192;
//            const auto maxInterval = 144;
            if (dAccDenominator.size() < dDenominator.size()) {
                dAccDenominator.resize(dDenominator.size(), queue);
            }
            if (dBuffer.size() < dDenominator.size()) {
                dBuffer.resize(dDenominator.size(), queue);
            }
            if (dBuffer1.size() < workGroups) {
                dBuffer1.resize(workGroups, queue);
            }

            // first level
            auto& kernel = kernelScanLev1;
            kernel.set_arg(0, dDenominator);
            kernel.set_arg(1, dAccDenominator);
            kernel.set_arg(2, dBuffer); // partial_scan_buffer
            kernel.set_arg(3, taskCount);
            kernel.set_arg(4, intervalSize);
            kernel.set_arg(5, dBuffer1); // interval_results

//            std::cout << " GlobalWorkSize: " << globalWorkSize << " workGroups: " << workGroups << " BlockSize: " << detail::constant::updateXBetaBlockSize << " taskCount: " << taskCount;
            queue.enqueue_1d_range_kernel(kernel, 0, globalWorkSize, detail::constant::updateXBetaBlockSize);
            queue.finish();

//            // check intermediate results
//            hBuffer.resize(dBuffer.size());
//            compute::copy(std::begin(dBuffer), std::end(dBuffer), std::begin(hBuffer), queue);
//            std::cout << "dBuffer: ";
//            for (auto x:hBuffer) {
//                std::cout << x << " ";
//            }
//            std::cout << "\n";
//            hBuffer1.resize(dBuffer1.size());
//            compute::copy(std::begin(dBuffer1), std::end(dBuffer1), std::begin(hBuffer1), queue);
//            std::cout << "dBuffer1: ";
//            for (auto x:hBuffer1) {
//                std::cout << x << " ";
//            }
//            std::cout << "\n";

            // second level
            auto& kernel1 = kernelScanLev2;
            kernel1.set_arg(0, dDenominator);
            kernel1.set_arg(1, dAccDenominator);
            kernel1.set_arg(2, dBuffer1); // interval_results
            kernel1.set_arg(3, dBuffer1); // partial_scan_buffer
            kernel1.set_arg(4, taskCount);
            kernel1.set_arg(5, intervalSize);

//            size_t workGroups = taskCount / detail::constant::updateXBetaBlockSize;
//            if (taskCount % detail::constant::updateXBetaBlockSize != 0) {
//                ++workGroups;
//            }
//            const size_t globalWorkSize = workGroups * detail::constant::updateXBetaBlockSize;
            queue.enqueue_1d_range_kernel(kernel1, 0, detail::constant::updateXBetaBlockSize, detail::constant::updateXBetaBlockSize);
            queue.finish();

//            // check intermediate results
//            hBuffer.resize(dBuffer.size());
//            compute::copy(std::begin(dBuffer), std::end(dBuffer), std::begin(hBuffer), queue);
//            std::cout << "dBuffer: ";
//            for (auto x:hBuffer) {
//                std::cout << x << " ";
//            }
//            std::cout << "\n";
//            hBuffer1.resize(dBuffer1.size());
//            compute::copy(std::begin(dBuffer1), std::end(dBuffer1), std::begin(hBuffer1), queue);
//            std::cout << "dBuffer1: ";
//            for (auto x:hBuffer1) {
//                std::cout << x << " ";
//            }
//            std::cout << "\n";

            // final update
            auto& kernel2 = kernelScanUpd;
            kernel2.set_arg(0, dDenominator);
            kernel2.set_arg(1, dAccDenominator);
            kernel2.set_arg(2, taskCount);
            kernel2.set_arg(3, intervalSize);
            kernel2.set_arg(4, dBuffer1); // interval_results
            kernel2.set_arg(5, dBuffer); // partial_scan_buffer

//            size_t workGroups = taskCount / detail::constant::updateXBetaBlockSize;
//            if (taskCount % detail::constant::updateXBetaBlockSize != 0) {
//                ++workGroups;
//            }
//            const size_t globalWorkSize = workGroups * detail::constant::updateXBetaBlockSize;
            queue.enqueue_1d_range_kernel(kernel2, 0, globalWorkSize, detail::constant::updateXBetaBlockSize);
            queue.finish();

//            // check intermediate results
//            hBuffer.resize(dBuffer.size());
//            compute::copy(std::begin(dBuffer), std::end(dBuffer), std::begin(hBuffer), queue);
//            std::cout << "dBuffer: ";
//            for (auto x:hBuffer) {
//                std::cout << x << " ";
//            }
//            std::cout << "\n";
//            hBuffer1.resize(dBuffer1.size());
//            compute::copy(std::begin(dBuffer1), std::end(dBuffer1), std::begin(hBuffer1), queue);
//            std::cout << "dBuffer1: ";
//            for (auto x:hBuffer1) {
//                std::cout << x << " ";
//            }
//            std::cout << "\n";

	    */
#ifdef CYCLOPS_DEBUG_TIMING
            auto end = bsccs::chrono::steady_clock::now();
            ///////////////////////////"
            duration["accumlatedDenomG "] += bsccs::chrono::duration_cast<chrono::TimingUnits>(end - start).count();
#endif

	    /*
            // copy results to host
            accDenomPid.resize(dAccDenominator.size());
            compute::copy(std::begin(dAccDenominator), std::end(dAccDenominator), std::begin(accDenomPid), queue);//            std::cout << "dDenominator: ";

            std::cout << "accDenominator: ";
            for (auto x:accDenomPid) {
                std::cout << x << " ";
            }
            std::cout << "\n";
	    */

            // naive scan
//            auto& kernel = kernelComputeAccumlatedDenominator;
//
//            int tc = dDenominator.size();
//            const auto taskCount = tc;
//
////            kernel.set_arg(0, dColumns.getDataOffset()); // offX
////            kernel.set_arg(1, dColumns.getIndicesOffset()); // offK
//            kernel.set_arg(0, taskCount);
//            kernel.set_arg(1, dDenominator);
//            if (dAccDenominator.size() < dDenominator.size()) {
//                dAccDenominator.resize(dDenominator.size(), queue);
//            }
//            kernel.set_arg(2, dAccDenominator);
//            if (dBuffer.size() < dDenominator.size()) {
//                dBuffer.resize(dDenominator.size(), queue);
//            }
//            std::cout << "dDenominator.size(): " << dDenominator.size() << '\n';
//            kernel.set_arg(3, dBuffer);
//            kernel.set_arg(4, dId);
//
//            size_t workGroups = taskCount / detail::constant::updateXBetaBlockSize;
//            if (taskCount % detail::constant::updateXBetaBlockSize != 0) {
//                ++workGroups;
//            }
//            const size_t globalWorkSize = workGroups * detail::constant::updateXBetaBlockSize;
//            queue.enqueue_1d_range_kernel(kernel, 0, globalWorkSize, detail::constant::updateXBetaBlockSize);
//            queue.finish();
        }

        virtual const std::vector<double> getXBeta() {
            if (!hXBetaKnown) {
                compute::copy(std::begin(dXBeta), std::end(dXBeta), std::begin(hXBeta), queue);
                hXBetaKnown = true;
            }
            return ModelSpecifics<BaseModel, RealType>::getXBeta();
        }

        virtual const std::vector<double> getXBetaSave() {
            return ModelSpecifics<BaseModel, RealType>::getXBetaSave();
        }

        virtual void saveXBeta() {
            if (!hXBetaKnown) {
                compute::copy(std::begin(dXBeta), std::end(dXBeta), std::begin(hXBeta), queue);
                hXBetaKnown = true;
            }
            ModelSpecifics<BaseModel, RealType>::saveXBeta();
        }

        virtual void zeroXBeta() {

            //std::cerr << "GPU::zXB called" << std::endl;

            ModelSpecifics<BaseModel, RealType>::zeroXBeta(); // touches hXBeta

            dXBetaKnown = false;
        }

        virtual void axpyXBeta(const double beta, const int j) {

            //std::cerr << "GPU::aXB called" << std::endl;

            ModelSpecifics<BaseModel, RealType>::axpyXBeta(beta, j); // touches hXBeta

            dXBetaKnown = false;
        }

    private:
        void buildAllUpdateXBetaKernels(const std::vector<FormatType>& neededFormatTypes) {
            for (FormatType formatType : neededFormatTypes) {
                buildUpdateXBetaKernel(formatType);
            }
        }

        void buildAllComputeAccumlatedDenominatorKernels() {
            //for (FormatType formatType : neededFormatTypes) {
            buildComputeAccumlatedDenominatorKernel(true);
            buildComputeAccumlatedDenominatorKernel(false);
            //}
        }

        void buildAllScanLev1Kernels() {
            buildScanLev1Kernel();
        }

        void buildAllScanLev2Kernels() {
            buildScanLev2Kernel();
        }

        void buildAllScanUpdKernels() {
            buildScanUpdKernel();
        }

//        void buildAllGradientHessianKernels(const std::vector<FormatType>& neededFormatTypes) {
//            int b = 0;
//            for (FormatType formatType : neededFormatTypes) {
//                buildGradientHessianKernel(formatType, true); ++b;
//                buildGradientHessianKernel(formatType, false); ++b;
//            }
//        }

        std::string getFormatTypeExtension(FormatType formatType) {
            switch (formatType) {
                case DENSE:
                    return "Den";
                case SPARSE:
                    return "Spa";
                case INDICATOR:
                    return "Ind";
                case INTERCEPT:
                    return "Icp";
                default: return "";
            }
        }

        SourceCode writeCodeForUpdateXBetaKernel(FormatType formatType);
        SourceCode writeCodeForComputeAccumlatedDenominatorKernel(bool useWeights);
        SourceCode writecodeForScanLev1Kernel();
        SourceCode writecodeForScanLev2Kernel();
        SourceCode writecodeForScanUpdKernel();
//        SourceCode writeCodeForGradientHessianKernel(FormatType formatType, bool useWeights, bool isNvidia);

        void buildUpdateXBetaKernel(FormatType formatType) {

            std::stringstream options;
//            options << "-DREAL=" << (sizeof(real) == 8 ? "double" : "float");
            if (double_precision) {
                options << "-DREAL=double -DTMP_REAL=double -DTPB=" << tpb;
            } else {
                options << "-DREAL=float -DTMP_REAL=float -DTPB=" << tpb;
            }
            options << " -cl-mad-enable";

            auto source = writeCodeForUpdateXBetaKernel(formatType);

//            std::cerr << source.body << std::endl;

            auto program = compute::program::build_with_source(source.body, ctx, options.str());
            std::cout << "built updateXBeta program \n";
            auto kernel = compute::kernel(program, source.name);

            // Rcpp::stop("uXB");

            // Run-time constant arguments.
            kernel.set_arg(6, dY);
            kernel.set_arg(7, dXBeta);
            kernel.set_arg(8, dExpXBeta);
            kernel.set_arg(9, dDenominator);
            kernel.set_arg(10, dId);

            kernelUpdateXBeta[formatType] = std::move(kernel);
        }

        void buildComputeAccumlatedDenominatorKernel(bool useWeights) {

            std::stringstream options;
            if (double_precision) {
                options << "-DREAL=double -DTMP_REAL=double -DTPB=" << tpb;
            } else {
                options << "-DREAL=float -DTMP_REAL=float -DTPB=" << tpb;
            }
            options << " -cl-mad-enable";

            auto source = writeCodeForComputeAccumlatedDenominatorKernel(useWeights);

//            std::cerr << source.body << std::endl;

            auto program = compute::program::build_with_source(source.body, ctx, options.str());
            std::cout << "built accDenominator program \n";
            auto kernel = compute::kernel(program, source.name);

            kernelComputeAccumlatedDenominator = std::move(kernel);
        }

        void buildScanLev1Kernel() {

            std::stringstream options;
            if (double_precision) {
                options << "-DREAL=double -DTMP_REAL=double -Dpsc_K=" << PSC_K << " -Dpsc_WG_SIZE=" << PSC_WG_SIZE;
            } else {
                options << "-DREAL=float -DTMP_REAL=float -Dpsc_K=" << PSC_K<< " -Dpsc_WG_SIZE=" << PSC_WG_SIZE;
            }
            options << " -cl-mad-enable";

            auto source = writecodeForScanLev1Kernel();

//            std::cerr << source.body << std::endl;

            auto program = compute::program::build_with_source(source.body, ctx, options.str());
            std::cout << "built scan_lev1 program \n";
            auto kernel = compute::kernel(program, source.name);

            kernelScanLev1 = std::move(kernel);
        }

        void buildScanLev2Kernel() {

            std::stringstream options;
            if (double_precision) {
                options << "-DREAL=double -DTMP_REAL=double -Dpsc_K=" << PSC_K << " -Dpsc_WG_SIZE=" << PSC_WG_SIZE;
            } else {
                options << "-DREAL=float -DTMP_REAL=float -Dpsc_K=" << PSC_K<< " -Dpsc_WG_SIZE=" << PSC_WG_SIZE;
            }
            options << " -cl-mad-enable";

            auto source = writecodeForScanLev2Kernel();

//            std::cerr << source.body << std::endl;

            auto program = compute::program::build_with_source(source.body, ctx, options.str());
            std::cout << "built scan_lev2 program \n";
            auto kernel = compute::kernel(program, source.name);

            kernelScanLev2 = std::move(kernel);
        }

        void buildScanUpdKernel() {

            std::stringstream options;
            if (double_precision) {
                options << "-DREAL=double -DTMP_REAL=double -Dpsc_K=" << PSC_K << " -Dpsc_WG_SIZE=" << PSC_WG_SIZE;
            } else {
                options << "-DREAL=float -DTMP_REAL=float -Dpsc_K=" << PSC_K<< " -Dpsc_WG_SIZE=" << PSC_WG_SIZE;
            }
            options << " -cl-mad-enable";

            auto source = writecodeForScanUpdKernel();

//            std::cerr << source.body << std::endl;

            auto program = compute::program::build_with_source(source.body, ctx, options.str());
            std::cout << "built scan_final_update program \n";
            auto kernel = compute::kernel(program, source.name);

            kernelScanUpd = std::move(kernel);
        }


//        void buildGradientHessianKernel(FormatType formatType, bool useWeights) {
//
//            std::stringstream options;
//
//            if (double_precision) {
//#ifdef USE_VECTOR
//                options << "-DREAL=double -DTMP_REAL=double2 -DTPB=" << tpb;
//#else
//                options << "-DREAL=double -DTMP_REAL=double -DTPB=" << tpb;
//#endif // USE_VECTOR
//            } else {
//#ifdef USE_VECTOR
//                options << "-DREAL=float -DTMP_REAL=float2 -DTPB=" << tpb;
//#else
//                options << "-DREAL=float -DTMP_REAL=float -DTPB=" << tpb;
//#endif // USE_VECTOR
//            }
//            options << " -cl-mad-enable -cl-fast-relaxed-math"; // " -cl-mad-enable"?
//
////            if (double_precision) {
////                options << "-DREAL=double -DTMP_REAL=double -DTPB=" << tpb;
////            } else {
////                options << "-DREAL=float -DTMP_REAL=float -DTPB=" << tpb;
////            }
////            options << " -cl-mad-enable";
//
////         compute::vector<compute::double2_> buf(10, ctx);
////
////         compute::double2_ sum = compute::double2_{0.0, 0.0};
////         compute::reduce(buf.begin(), buf.end(), &sum, queue);
////
////         std::cerr << sum << std::endl;
////
////         auto cache = compute::program_cache::get_global_cache(ctx);
////         auto list = cache->get_keys();
////         std::cerr << "list size = " << list.size() << std::endl;
////         for (auto a : list) {
////             std::cerr << a.first << ":" << a.second << std::endl;
////             auto p = cache->get(a.first, a.second);
////             if (p) {
////                 std::cerr << p->source() << std::endl;
////             }
////         }
////
////         Rcpp::stop("out");
//
//            const auto isNvidia = compute::detail::is_nvidia_device(queue.get_device());
//
////         std::cerr << queue.get_device().name() << " " << queue.get_device().vendor() << std::endl;
////         std::cerr << "isNvidia = " << isNvidia << std::endl;
////         Rcpp::stop("out");
//
//            auto source = writeCodeForGradientHessianKernel(formatType, useWeights, isNvidia);
//
//            /*
//            if (algorithmType == AlgorithmType::MM) {
//                std::cout << "wrote MM source\n";
//                source = writeCodeForMMGradientHessianKernel(formatType, useWeights, isNvidia);
//            }
//            */
//
//            // std::cerr << options.str() << std::endl;
//            // std::cerr << source.body << std::endl;
//
//            std::cout << "formatType: " << formatType << " isNvidia: " << isNvidia << '\n';
//            auto program = compute::program::build_with_source(source.body, ctx, options.str());
//            std::cout << "program built \n";
//            auto kernel = compute::kernel(program, source.name);
//            std::cout << "kernal built \n";
//
//            // Rcpp::stop("cGH");
//
//            // Run-time constant arguments.
//            kernel.set_arg(5, dY);
//            kernel.set_arg(6, dXBeta);
//            kernel.set_arg(7, dExpXBeta);
//            kernel.set_arg(8, dDenominator);
//            kernel.set_arg(9, dBuffer);  // TODO Does not seem to stick
//            kernel.set_arg(10, dId);
//            kernel.set_arg(11, dKWeight); // TODO Does not seem to stick
//
////            source = writeCodeForMMGradientHessianKernel(formatType, useWeights, isNvidia);
////            program = compute::program::build_with_source(source.body, ctx, options.str());
////            auto kernelMM = compute::kernel(program, source.name);
////            kernelMM.set_arg(5, dY);
////            kernelMM.set_arg(6, dXBeta);
////            kernelMM.set_arg(7, dExpXBeta);
////            kernelMM.set_arg(8, dDenominator);
////            kernelMM.set_arg(9, dBuffer);  // TODO Does not seem to stick
////            kernelMM.set_arg(10, dId);
////            kernelMM.set_arg(11, dKWeight); // TODO Does not seem to stick
//
//            if (useWeights) {
//                kernelGradientHessianWeighted[formatType] = std::move(kernel);
////                kernelGradientHessianMMWeighted[formatType] = std::move(kernelMM);
//            } else {
//                kernelGradientHessianNoWeight[formatType] = std::move(kernel);
////                kernelGradientHessianMMNoWeight[formatType] = std::move(kernelMM);
//            }
//        }


        void buildAllKernels(const std::vector<FormatType>& neededFormatTypes) {
//            buildAllGradientHessianKernels(neededFormatTypes);
//            std::cout << "built gradhessian kernels \n";
            buildAllUpdateXBetaKernels(neededFormatTypes);
            std::cout << "built updateXBeta kernels \n";
            buildAllComputeAccumlatedDenominatorKernels();
            std::cout << "built accumulatedDenominator kernels \n";
            buildAllScanLev1Kernels();
            std::cout << "built ScanLev1 kernels \n";
            buildAllScanLev2Kernels();
            std::cout << "built ScanLev2 kernels \n";
            buildAllScanUpdKernels();
            std::cout << "built ScanUpd kernels \n";
        }

        std::map<FormatType, compute::kernel> kernelUpdateXBeta;
        compute::kernel kernelComputeAccumlatedDenominator;
        compute::kernel kernelScanLev1;
        compute::kernel kernelScanLev2;
        compute::kernel kernelScanUpd;
//        std::map<FormatType, compute::kernel> kernelGradientHessianWeighted;
//        std::map<FormatType, compute::kernel> kernelGradientHessianNoWeight;

        compute::vector<RealType> dBuffer;
        compute::vector<RealType> dBuffer1;
        bool hXBetaKnown;
        bool dXBetaKnown;
    };
} // namespace bsccs

#include "KernelsCox.hpp"

#endif //GPUMODELSPECIFICSCOX_HPP