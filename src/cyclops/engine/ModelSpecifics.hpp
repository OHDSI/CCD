/*
 * ModelSpecifics.hpp
 *
 *  Created on: Jul 13, 2012
 *      Author: msuchard
 */

#ifndef MODELSPECIFICS_HPP_
#define MODELSPECIFICS_HPP_

#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <numeric>
#include <limits>

#include <boost/iterator/permutation_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/zip_iterator.hpp>

#include "ModelSpecifics.h"
#include "Iterators.h"

#include "Recursions.hpp"
#include "ParallelLoops.h"
#include "Ranges.h"

//#include "R.h"
//#include "Rcpp.h" // TODO Remove

#ifdef CYCLOPS_DEBUG_TIMING
	#include "Timing.h"
	namespace bsccs {
		const std::string DenseIterator::name = "Den";
		const std::string IndicatorIterator::name = "Ind";
		const std::string SparseIterator::name = "Spa";
		const std::string InterceptIterator::name = "Icp";
	}
#endif

//#define OLD_WAY
//#define NEW_WAY1
#define NEW_WAY2

//#define USE_BIGNUM
#define USE_LONG_DOUBLE

namespace bsccs {

#ifdef USE_BIGNUM
	typedef bigNum DDouble;
#else
	#ifdef USE_LONG_DOUBLE
		typedef long double DDouble;
	#else
		typedef double DDouble;
	#endif
#endif

#if defined(DEBUG_COX) || defined(DEBUG_COX_MIN)
    using std::cerr;
    using std::endl;
#endif

#define NEW_LOOPS

namespace helper {
//
//     auto getRangeAll(const int length) ->
//             boost::iterator_range<
//                 decltype(boost::make_counting_iterator(0))
//             > {
//         return  {
//             boost::make_counting_iterator(0),
//             boost::make_counting_iterator(length)
//         };
//     }
//
//     template <class IteratorTag>
//     auto getRangeDenominator(const IntVectorPtr& mat, const int N, IteratorTag) ->  void {
//     	std::cerr << "Not yet implemented." << std::endl;
//     	std::exit(-1);
//     }
//
//     auto getRangeDenominator(const IntVectorPtr& mat, const int N, DenseTag) ->
//             boost::iterator_range<
//                 decltype(boost::make_counting_iterator(0))
//             > {
//         return {
//             boost::make_counting_iterator(0),
//             boost::make_counting_iterator(N)
//         };
//     }
//
//     auto getRangeDenominator(const IntVectorPtr& mat, const int N, SparseTag) ->
//             boost::iterator_range<
//                 decltype(mat->begin())
//             > {
//         return {
//             std::begin(*mat), std::end(*mat)
//         };
//     }
//
//     auto getRangeDenominator(const IntVectorPtr& mat, const int N, IndicatorTag) ->
//             boost::iterator_range<
//                 decltype(mat->begin())
//             > {
//         return {
//             std::begin(*mat), std::end(*mat)
//         };
//     }
//
//     template <class IteratorTag>
//     auto getRangeNumerator(const IntVectorPtr& mat, const int N, IteratorTag) ->  void {
//     	std::cerr << "Not yet implemented." << std::endl;
//     	std::exit(-1);
//     }
//
//     auto getRangeNumerator(const IntVectorPtr& mat, const int N, DenseTag) ->
//             boost::iterator_range<
//                 decltype(boost::make_counting_iterator(0))
//             > {
//         return {
//             boost::make_counting_iterator(0),
//             boost::make_counting_iterator(N)
//         };
//     }
//
//     auto getRangeNumerator(const IntVectorPtr& mat, const int N, SparseTag) ->
//             boost::iterator_range<
//                 decltype(mat->begin())
//             > {
//         return {
//             std::begin(*mat), std::end(*mat)
//         };
//     }
//
//     auto getRangeNumerator(const IntVectorPtr& mat, const int N, IndicatorTag) ->
//             boost::iterator_range<
//                 decltype(mat->begin())
//             > {
//         return {
//             std::begin(*mat), std::end(*mat)
//         };
//     }
//
//     template <class IteratorTag>
//     auto getRangeCOOX(const CompressedDataMatrix& mat, const int index, IteratorTag) -> void {
//     	std::cerr << "Not yet implemented." << std::endl;
//     	std::exit(-1);
//     }
//
//     auto getRangeCOOX(const CompressedDataMatrix& mat, const int index, DenseTag) ->
//             boost::iterator_range<
//                 boost::zip_iterator<
//                     boost::tuple<
//                         decltype(boost::make_counting_iterator(0)), // TODO Not quite right
//                         decltype(boost::make_counting_iterator(0)),
//                         decltype(begin(mat.getDataVector(index)))
//                     >
//                 >
//             > {
//         auto i = boost::make_counting_iterator(0); // TODO Not quite right
//         auto j = boost::make_counting_iterator(0);
//         auto x = begin(mat.getDataVector(index));
//
//         const size_t K = mat.getNumberOfRows();
//
//         return {
//             boost::make_zip_iterator(
//                 boost::make_tuple(i, j, x)),
//             boost::make_zip_iterator(
//                 boost::make_tuple(i + K, j + K, x + K))
//         };
//     }
//
// 	template <class IteratorTag>
//     auto getRangeX(const CompressedDataMatrix& mat, const int index, IteratorTag) -> void {
//     	std::cerr << "Not yet implemented." << std::endl;
//     	std::exit(-1);
//     }

    template <typename RealType>
	auto getRangeX(const CompressedDataMatrix<RealType>& mat, const int index, InterceptTag) ->
	    //            aux::zipper_range<
	    boost::iterator_range<
	        boost::zip_iterator<
	            boost::tuple<
	                decltype(boost::make_counting_iterator(0))
	            >
	        >
	    > {

	    auto i = boost::make_counting_iterator(0);
	        const size_t K = mat.getNumberOfRows();

	        return {
	            boost::make_zip_iterator(
	                boost::make_tuple(i)),
	                boost::make_zip_iterator(
	                    boost::make_tuple(i + K))
	        };
	    }

    template <typename RealType>
    auto getRangeX(const CompressedDataMatrix<RealType>& mat, const int index, DenseTag) ->
//            aux::zipper_range<
 						boost::iterator_range<
 						boost::zip_iterator<
 						boost::tuple<
	            decltype(boost::make_counting_iterator(0)),
            	decltype(begin(mat.getDataVector(index)))
            >
            >
            > {

        auto i = boost::make_counting_iterator(0);
        auto x = begin(mat.getDataVector(index));
		const size_t K = mat.getNumberOfRows();

        return {
            boost::make_zip_iterator(
                boost::make_tuple(i, x)),
            boost::make_zip_iterator(
                boost::make_tuple(i + K, x + K))
        };
    }

    template <typename RealType>
    auto getRangeX(const CompressedDataMatrix<RealType>& mat, const int index, SparseTag) ->
//            aux::zipper_range<
						boost::iterator_range<
 						boost::zip_iterator<
 						boost::tuple<
	            decltype(begin(mat.getCompressedColumnVector(index))),
            	decltype(begin(mat.getDataVector(index)))
            >
            >
            > {

        auto i = begin(mat.getCompressedColumnVector(index));
        auto x = begin(mat.getDataVector(index));
		const size_t K = mat.getNumberOfEntries(index);

        return {
            boost::make_zip_iterator(
                boost::make_tuple(i, x)),
            boost::make_zip_iterator(
                boost::make_tuple(i + K, x + K))
        };
    }

    template <typename RealType>
    auto getRangeX(const CompressedDataMatrix<RealType>& mat, const int index, IndicatorTag) ->
//            aux::zipper_range<
						boost::iterator_range<
 						boost::zip_iterator<
 						boost::tuple<
	            decltype(begin(mat.getCompressedColumnVector(index)))
	          >
	          >
            > {

        auto i = begin(mat.getCompressedColumnVector(index));
		const size_t K = mat.getNumberOfEntries(index);

        return {
            boost::make_zip_iterator(
                boost::make_tuple(i)),
            boost::make_zip_iterator(
                boost::make_tuple(i + K))
        };
    }

} // namespace helper


template <class BaseModel,typename RealType>
ModelSpecifics<BaseModel,RealType>::ModelSpecifics(const ModelData<RealType>& input)
	: AbstractModelSpecifics(input), BaseModel(),
   modelData(input),
   hX(modelData.getX()),
   hY(input.getYVectorRef()),
   hOffs(input.getTimeVectorRef())
 //  hPidOriginal(input.getPidVectorRef()), hPid(const_cast<int*>(hPidOriginal.data())),
 //  boundType(MmBoundType::METHOD_2)
//  	threadPool(4,4,1000)
// threadPool(0,0,10)
	{
    	// Do nothing
	// TODO Memory allocation here

#ifdef CYCLOPS_DEBUG_TIMING
	auto now = bsccs::chrono::system_clock::now();
	auto now_c = bsccs::chrono::system_clock::to_time_t(now);
	std::cout << std::endl << "Start: " << std::ctime(&now_c) << std::endl;
#endif


}

template <class BaseModel, typename RealType>
AbstractModelSpecifics* ModelSpecifics<BaseModel,RealType>::clone() const {
	return new ModelSpecifics<BaseModel,RealType>(modelData);
}

template <class BaseModel, typename RealType>
double ModelSpecifics<BaseModel,RealType>::getGradientObjective(bool useCrossValidation) {

		auto& xBeta = getXBeta();

		RealType criterion = 0;
		if (useCrossValidation) {
			for (int i = 0; i < K; i++) {
				criterion += xBeta[i] * hY[i] * hKWeight[i];
			}
		} else {
			for (int i = 0; i < K; i++) {
				criterion += xBeta[i] * hY[i];
			}
		}
		return static_cast<double> (criterion);
	}

template <class BaseModel, typename RealType>
void ModelSpecifics<BaseModel,RealType>::printTiming() {

#ifdef CYCLOPS_DEBUG_TIMING

	std::cout << std::endl;
	for (auto& d : duration) {
		std::cout << d.first << " " << d.second << std::endl;
	}
	std::cout << "NEW LOOPS" << std::endl;

#endif
}

template <class BaseModel,typename RealType>
ModelSpecifics<BaseModel,RealType>::~ModelSpecifics() {
	// TODO Memory release here

#ifdef CYCLOPS_DEBUG_TIMING

    printTiming();

	auto now = bsccs::chrono::system_clock::now();
	auto now_c = bsccs::chrono::system_clock::to_time_t(now);
	std::cout << std::endl << "End:   " << std::ctime(&now_c) << std::endl;
#endif

}

template <class BaseModel, typename RealType> template <class IteratorType>
void ModelSpecifics<BaseModel,RealType>::incrementNormsImpl(int index) {

    // TODO should use row-access
	IteratorType it(hX, index);
	for (; it; ++it) {
		const int k = it.index();
		const RealType x = it.value();

		norm[k] += std::abs(x);
	}
}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::initializeMmXt() {

#ifdef CYCLOPS_DEBUG_TIMING
    auto start = bsccs::chrono::steady_clock::now();
#endif

    hXt = hX.transpose();

#ifdef CYCLOPS_DEBUG_TIMING
auto end = bsccs::chrono::steady_clock::now();
///////////////////////////"
duration["transpose        "] += bsccs::chrono::duration_cast<chrono::TimingUnits>(end - start).count();
#endif
}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::initializeMM(
        MmBoundType boundType,
        const std::vector<bool>& fixBeta
    ) {

#ifdef CYCLOPS_DEBUG_TIMING
    auto start = bsccs::chrono::steady_clock::now();
#endif

    // std::cout << "Computing norms..." << std::endl;
    norm.resize(K);
    zeroVector(&norm[0], K);

    for (int j = 0; j < J; ++j) {
    	if (
    	true
    	// !fixBeta[j]
    	) {
			switch(hX.getFormatType(j)) {
				case INDICATOR :
					incrementNormsImpl<IndicatorIterator<RealType>>(j);
					break;
				case SPARSE :
					incrementNormsImpl<SparseIterator<RealType>>(j);
					break;
				case DENSE :
					incrementNormsImpl<DenseIterator<RealType>>(j);
					break;
				case INTERCEPT :
					incrementNormsImpl<InterceptIterator<RealType>>(j);
					break;
			}
        }
    }

    if (boundType == MmBoundType::METHOD_1) {
        std::cerr << "boundType: METHOD_1" << std::endl;


    } else if (boundType == MmBoundType::METHOD_2) {
        std::cerr << "boundType: METHOD_2" << std::endl;

        double total = 0;

        for (int j = 0; j < J; ++j) {
            if (!fixBeta[j]) {
                typedef std::pair<double,double> Range;

                std::vector<Range> range(modelData.getNumberOfPatients(), Range(
                        -std::numeric_limits<double>::max(),
                        std::numeric_limits<double>::max()
                ));

                modelData.binaryReductionByStratum(range, j, [](Range& r, double x) {
                    return Range(
                        std::max(x, r.first),
                        std::min(x, r.second)
                    );
                });

                double curvature = 0.0;
                for (Range r : range) {
                    curvature += r.first - r.second;
                }
                curvature /= 4;

                total += curvature;
            }
        }

        if (curvature.size() == 0) {
            curvature.resize(J);
        }

        for (int j = 0; j < J; ++j) {
            curvature[j] = total;
        }
    }

#ifdef CYCLOPS_DEBUG_TIMING
    auto end = bsccs::chrono::steady_clock::now();
    ///////////////////////////"
    duration["norms            "] += bsccs::chrono::duration_cast<chrono::TimingUnits>(end - start).count();
#endif

// 	struct timeval time1, time2;
// 	gettimeofday(&time1, NULL);
//
//     std::cout << "Constructing Xt..." << std::endl;

//     if (!hXt) {
//         initializeMmXt();
//     }

//     gettimeofday(&time2, NULL);
// 	double duration = //calculateSeconds(time1, time2);
// 		time2.tv_sec - time1.tv_sec +
// 			(double)(time2.tv_usec - time1.tv_usec) / 1000000.0;
//
// 	std::cout << "Done with MM initialization" << std::endl;
// 	std::cout << duration << std::endl;
	// Rcpp::stop("out");

}

template <class BaseModel,typename RealType>
const std::vector<double> ModelSpecifics<BaseModel,RealType>::getXBeta() {
    return std::vector<double>(std::begin(hXBeta), std::end(hXBeta));
}

template <class BaseModel,typename RealType>
const std::vector<double> ModelSpecifics<BaseModel,RealType>::getXBetaSave() {
    return std::vector<double>(std::begin(hXBetaSave), std::end(hXBetaSave));
}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::zeroXBeta() {
	std::fill(std::begin(hXBeta), std::end(hXBeta), 0.0);
}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::saveXBeta() {
	auto& xBeta = getXBeta();
	if (hXBetaSave.size() < xBeta.size()) {
		hXBetaSave.resize(xBeta.size());
	}
	std::copy(std::begin(xBeta), std::end(xBeta), std::begin(hXBetaSave));
}



template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::computeXBeta(double* beta, bool useWeights) {

    if (!hXt) {
        initializeMmXt();
    }

#ifdef CYCLOPS_DEBUG_TIMING
    auto start = bsccs::chrono::steady_clock::now();
#endif

    switch(hXt->getFormatType(0)) {
    case INDICATOR :
        computeXBetaImpl<IndicatorIterator<RealType>>(beta);
        break;
    case SPARSE :
        computeXBetaImpl<SparseIterator<RealType>>(beta);
        break;
    case DENSE :
        computeXBetaImpl<DenseIterator<RealType>>(beta);
        break;
    case INTERCEPT:
        break;
}


#ifdef CYCLOPS_DEBUG_TIMING
auto end = bsccs::chrono::steady_clock::now();
///////////////////////////"
duration["computeXBeta     "] += bsccs::chrono::duration_cast<chrono::TimingUnits>(end - start).count();
#endif

}

template <class BaseModel,typename RealType> template <class IteratorType>
void ModelSpecifics<BaseModel,RealType>::computeXBetaImpl(double *beta) {

    for (int k = 0; k < K; ++k) {
        RealType sum = 0.0;
        IteratorType it(*hXt, k);
        for (; it; ++it) {
            const auto j = it.index();
            sum += it.value() * beta[j];
        }
        hXBeta[k] = sum;
        // TODO Add back in for fastest LR
//         const auto exb = std::exp(sum);
//         offsExpXBeta[k] = exb;
//         denomPid[k] = 1.0 + exb;
    }

    // std::cerr << "did weird stuff to denomPid" << std::endl;
}

template <class BaseModel,typename RealType>
bool ModelSpecifics<BaseModel,RealType>::allocateXjY(void) { return BaseModel::precomputeGradient; }

template <class BaseModel,typename RealType>
bool ModelSpecifics<BaseModel,RealType>::allocateXjX(void) { return BaseModel::precomputeHessian; }

template <class BaseModel,typename RealType>
bool ModelSpecifics<BaseModel,RealType>::sortPid(void) { return BaseModel::sortPid; }

template <class BaseModel,typename RealType>
bool ModelSpecifics<BaseModel,RealType>::initializeAccumulationVectors(void) { return BaseModel::cumulativeGradientAndHessian; }

template <class BaseModel,typename RealType>
bool ModelSpecifics<BaseModel,RealType>::allocateNtoKIndices(void) { return BaseModel::hasNtoKIndices; }

template <class BaseModel,typename RealType>
bool ModelSpecifics<BaseModel,RealType>::hasResetableAccumulators(void) { return BaseModel::hasResetableAccumulators; }

template <class BaseModel,typename RealType> template <class IteratorType>
void ModelSpecifics<BaseModel,RealType>::axpy(RealType* y, const RealType alpha, const int index) {
	IteratorType it(hX, index);
	for (; it; ++it) {
		const int k = it.index();
		y[k] += alpha * it.value();
	}
}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::axpyXBeta(const double beta, const int j) {

#ifdef CYCLOPS_DEBUG_TIMING
    auto start = bsccs::chrono::steady_clock::now();
#endif

	if (beta != 0.0) {
		switch (hX.getFormatType(j)) {
		case INDICATOR:
			axpy < IndicatorIterator<RealType> > (hXBeta.data(), beta, j);
			break;
		case INTERCEPT:
		    axpy < InterceptIterator<RealType> > (hXBeta.data(), beta, j);
		    break;
		case DENSE:
			axpy < DenseIterator<RealType> > (hXBeta.data(), beta, j);
			break;
		case SPARSE:
			axpy < SparseIterator<RealType> > (hXBeta.data(), beta, j);
			break;
		}
	}

#ifdef CYCLOPS_DEBUG_TIMING
	auto end = bsccs::chrono::steady_clock::now();
	///////////////////////////"
	duration["axpy             "] += bsccs::chrono::duration_cast<chrono::TimingUnits>(end - start).count();
#endif

}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::setWeights(double* inWeights, bool useCrossValidation) {
	// Set K weights
	if (hKWeight.size() != K) {
		hKWeight.resize(K);
	}
	if (useCrossValidation) {
		for (size_t k = 0; k < K; ++k) {
			hKWeight[k] = inWeights[k];
		}
	} else {
		std::fill(hKWeight.begin(), hKWeight.end(), static_cast<RealType>(1));
	}

	if (initializeAccumulationVectors()) {
		setPidForAccumulation(inWeights);
	}

	// Set N weights (these are the same for independent data models
	if (hNWeight.size() < N + 1) { // Add +1 for extra (zero-weight stratum)
		hNWeight.resize(N + 1);
	}

	std::fill(hNWeight.begin(), hNWeight.end(), static_cast<RealType>(0));
	for (size_t k = 0; k < K; ++k) {
		RealType event = BaseModel::observationCount(hY[k])*hKWeight[k];
		incrementByGroup(hNWeight.data(), hPid, k, event);
	}

#ifdef DEBUG_COX
	cerr << "Done with set weights" << endl;
#endif

}

template<class BaseModel, typename RealType>
void ModelSpecifics<BaseModel, RealType>::computeXjY(bool useCrossValidation) {
	for (size_t j = 0; j < J; ++j) {
		hXjY[j] = 0;

		GenericIterator<RealType> it(hX, j);

		if (useCrossValidation) {
			for (; it; ++it) {
				const int k = it.index();
				if (BaseModel::exactTies && hNWeight[BaseModel::getGroup(hPid, k)] > 1) {
					// Do not precompute
				} else {
					hXjY[j] += it.value() * hY[k] * hKWeight[k];
				}
			}
		} else {
			for (; it; ++it) {
				const int k = it.index();
				if (BaseModel::exactTies && hNWeight[BaseModel::getGroup(hPid, k)] > 1) {
					// Do not precompute
				} else {
					hXjY[j] += it.value() * hY[k];
				}
			}
		}
#ifdef DEBUG_COX
		cerr << "j: " << j << " = " << hXjY[j]<< endl;
#endif
	}
}

template<class BaseModel, typename RealType>
void ModelSpecifics<BaseModel, RealType>::computeXjX(bool useCrossValidation) {
	for (size_t j = 0; j < J; ++j) {
		hXjX[j] = 0;
		GenericIterator<RealType> it(hX, j);

		if (useCrossValidation) {
			for (; it; ++it) {
				const int k = it.index();
				if (BaseModel::exactTies && hNWeight[BaseModel::getGroup(hPid, k)] > 1) {
					// Do not precompute
				} else {
					hXjX[j] += it.value() * it.value() * hKWeight[k];
				}
			}
		} else {
			for (; it; ++it) {
				const int k = it.index();
				if (BaseModel::exactTies && hNWeight[BaseModel::getGroup(hPid, k)] > 1) {
					// Do not precompute
				} else {
					hXjX[j] += it.value() * it.value();
				}
			}
		}
	}
}

template<class BaseModel, typename RealType>
void ModelSpecifics<BaseModel, RealType>::computeNtoKIndices(bool useCrossValidation) {

	hNtoK.resize(N+1);
	int n = 0;
	for (size_t k = 0; k < K;) {
		hNtoK[n] = k;
		int currentPid = hPid[k];
		do {
			++k;
		} while (k < K && currentPid == hPid[k]);
		++n;
	}
	hNtoK[n] = K;
}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::computeFixedTermsInLogLikelihood(bool useCrossValidation) {
	if(BaseModel::likelihoodHasFixedTerms) {
		logLikelihoodFixedTerm = static_cast<RealType>(0);
	    bool hasOffs = hOffs.size() > 0;
		if(useCrossValidation) {
			for(size_t i = 0; i < K; i++) {
			    auto offs = hasOffs ? hOffs[i] : static_cast<RealType>(0);
				logLikelihoodFixedTerm += BaseModel::logLikeFixedTermsContrib(hY[i], offs, offs) * hKWeight[i];
			}
		} else {
			for(size_t i = 0; i < K; i++) {
			    auto offs = hasOffs ? hOffs[i] : static_cast<RealType>(0);
				logLikelihoodFixedTerm += BaseModel::logLikeFixedTermsContrib(hY[i], offs, offs); // TODO SEGV in Poisson model
			}
		}
	}
}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::computeFixedTermsInGradientAndHessian(bool useCrossValidation) {
	if (sortPid()) {
		doSortPid(useCrossValidation);
	}
	if (allocateXjY()) {
		computeXjY(useCrossValidation);
	}
	if (allocateXjX()) {
		computeXjX(useCrossValidation);
	}
	if (allocateNtoKIndices()) {
		computeNtoKIndices(useCrossValidation);
	}
}

template <class BaseModel,typename RealType>
double ModelSpecifics<BaseModel,RealType>::getLogLikelihood(bool useCrossValidation) {

#ifdef CYCLOPS_DEBUG_TIMING
	auto start = bsccs::chrono::steady_clock::now();
#endif

//     auto rangeNumerator = helper::getRangeAll(K);
//
//     RealType logLikelihood = useCrossValidation ?
//     		variants::reduce(
//                 rangeNumerator.begin(), rangeNumerator.end(), static_cast<RealType>(0.0),
//                 AccumulateLikeNumeratorKernel<BaseModel,RealType,int,true>(begin(hY), begin(hXBeta), begin(hKWeight)),
//                 SerialOnly()
//     		) :
//     		variants::reduce(
//                 rangeNumerator.begin(), rangeNumerator.end(), static_cast<RealType>(0.0),
//                 AccumulateLikeNumeratorKernel<BaseModel,RealType,int,false>(begin(hY), begin(hXBeta), begin(hKWeight)),
//                 SerialOnly()
//     		);

    auto rangeNumerator = helper::getRangeAllNumerators<RealType>(K, hY, hXBeta, hKWeight);

    RealType logLikelihood = useCrossValidation ?
    		variants::reduce(
                rangeNumerator.begin(), rangeNumerator.end(), static_cast<RealType>(0.0),
                TestAccumulateLikeNumeratorKernel<BaseModel,RealType,true>(),
                SerialOnly()
    		) :
    		variants::reduce(
                rangeNumerator.begin(), rangeNumerator.end(), static_cast<RealType>(0.0),
                TestAccumulateLikeNumeratorKernel<BaseModel,RealType,false>(),
                SerialOnly()
    		);

//     std::cerr << logLikelihood << " == " << logLikelihood2 << std::endl;

    if (BaseModel::likelihoodHasDenominator) {

//         auto rangeDenominator = helper::getRangeAll(N);
//
//         auto kernelDenominator = (BaseModel::cumulativeGradientAndHessian) ?
//                 AccumulateLikeDenominatorKernel<BaseModel,RealType,int>(begin(hNWeight), begin(accDenomPid)) :
//                 AccumulateLikeDenominatorKernel<BaseModel,RealType,int>(begin(hNWeight), begin(denomPid));
//
//         logLikelihood -= variants::reduce(
//                 rangeDenominator.begin(), rangeDenominator.end(),
//                 static_cast<RealType>(0.0),
//                 kernelDenominator,
//                 SerialOnly()
//         );

		auto rangeDenominator = (BaseModel::cumulativeGradientAndHessian) ?
				helper::getRangeAllDenominators<RealType>(N, accDenomPid, hNWeight) :
				helper::getRangeAllDenominators<RealType>(N, denomPid, hNWeight);

		logLikelihood -= variants::reduce(
				rangeDenominator.begin(), rangeDenominator.end(),
				static_cast<RealType>(0.0),
				TestAccumulateLikeDenominatorKernel<BaseModel,RealType>(),
				SerialOnly()
		);

//         std::cerr << logLikelihood << " == " << logLikelihood2 << std::endl;
    }

	if (BaseModel::likelihoodHasFixedTerms) {
		logLikelihood += logLikelihoodFixedTerm;
	}

#ifdef CYCLOPS_DEBUG_TIMING
	auto end = bsccs::chrono::steady_clock::now();
	///////////////////////////"
	duration["compLogLike      "] += bsccs::chrono::duration_cast<chrono::TimingUnits>(end - start).count();
#endif

	return static_cast<double>(logLikelihood);
}

template <class BaseModel,typename RealType>
double ModelSpecifics<BaseModel,RealType>::getPredictiveLogLikelihood(double* weights) {

    std::vector<RealType> saveKWeight;
	if(BaseModel::cumulativeGradientAndHessian)	{

 		saveKWeight = hKWeight; // make copy

// 		std::vector<int> savedPid = hPidInternal; // make copy
// 		std::vector<int> saveAccReset = accReset; // make copy
		setPidForAccumulation(weights);
		computeRemainingStatistics(true); // compute accDenomPid

    }

	// Compile-time switch for models with / with-out PID (hasIndependentRows)
	auto range = helper::getRangeAllPredictiveLikelihood<RealType>(K, hY, hXBeta,
		(BaseModel::cumulativeGradientAndHessian) ? accDenomPid : denomPid,
		weights, hPid, std::integral_constant<bool, BaseModel::hasIndependentRows>());

	auto kernel = TestPredLikeKernel<BaseModel,RealType>();

	RealType logLikelihood = variants::reduce(
			range.begin(), range.end(), static_cast<RealType>(0.0),
			kernel,
			SerialOnly()
		);

	if (BaseModel::cumulativeGradientAndHessian) {

// 		hPidInternal = savedPid; // make copy; TODO swap
// 		accReset = saveAccReset; // make copy; TODO swap
		setPidForAccumulation(&saveKWeight[0]);
		computeRemainingStatistics(true);
	}

	return static_cast<double>(logLikelihood);
}   // END OF DIFF

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::getPredictiveEstimates(double* y, double* weights){

	// TODO Check with SM: the following code appears to recompute hXBeta at large expense
//	std::vector<RealType> xBeta(K,0.0);
//	for(int j = 0; j < J; j++){
//		GenericIterator it(modelData, j);
//		for(; it; ++it){
//			const int k = it.index();
//			xBeta[k] += it.value() * hBeta[j] * weights[k];
//		}
//	}
	if (weights) {
		for (size_t k = 0; k < K; ++k) {
			if (weights[k]) {
				y[k] = BaseModel::predictEstimate(hXBeta[k]);
			}
		}
	} else {
		for (size_t k = 0; k < K; ++k) {
			y[k] = BaseModel::predictEstimate(hXBeta[k]);
		}
	}
	// TODO How to remove code duplication above?
}

// TODO The following function is an example of a double-dispatch, rewrite without need for virtual function
template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::computeGradientAndHessian(int index, double *ogradient,
		double *ohessian, bool useWeights) {

#ifdef CYCLOPS_DEBUG_TIMING
#ifndef CYCLOPS_DEBUG_TIMING_LOW
	auto start = bsccs::chrono::steady_clock::now();
#endif
#endif

	if (hX.getNumberOfNonZeroEntries(index) == 0) {
	    *ogradient = 0.0; *ohessian = 0.0;
	    return;
	}

	// Run-time dispatch, so virtual call should not effect speed
	if (useWeights) {
		switch (hX.getFormatType(index)) {
			case INDICATOR :
				computeGradientAndHessianImpl<IndicatorIterator<RealType>>(index, ogradient, ohessian, weighted);
				break;
			case SPARSE :
				computeGradientAndHessianImpl<SparseIterator<RealType>>(index, ogradient, ohessian, weighted);
				break;
			case DENSE :
				computeGradientAndHessianImpl<DenseIterator<RealType>>(index, ogradient, ohessian, weighted);
				break;
			case INTERCEPT :
				computeGradientAndHessianImpl<InterceptIterator<RealType>>(index, ogradient, ohessian, weighted);
				break;
		}
	} else {
		switch (hX.getFormatType(index)) {
			case INDICATOR :
				computeGradientAndHessianImpl<IndicatorIterator<RealType>>(index, ogradient, ohessian, unweighted);
				break;
			case SPARSE :
				computeGradientAndHessianImpl<SparseIterator<RealType>>(index, ogradient, ohessian, unweighted);
				break;
			case DENSE :
				computeGradientAndHessianImpl<DenseIterator<RealType>>(index, ogradient, ohessian, unweighted);
				break;
			case INTERCEPT :
				computeGradientAndHessianImpl<InterceptIterator<RealType>>(index, ogradient, ohessian, unweighted);
				break;
		}
	}

#ifdef CYCLOPS_DEBUG_TIMING
#ifndef CYCLOPS_DEBUG_TIMING_LOW
	auto end = bsccs::chrono::steady_clock::now();
	///////////////////////////"
	duration["compGradAndHess  "] += bsccs::chrono::duration_cast<chrono::TimingUnits>(end - start).count();
#endif
#endif

}

template <class RealType>
std::pair<RealType, RealType> operator+(
        const std::pair<RealType, RealType>& lhs,
        const std::pair<RealType, RealType>& rhs) {

    return { lhs.first + rhs.first, lhs.second + rhs.second };
}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::computeMMGradientAndHessian(
        std::vector<GradientHessian>& gh,
        const std::vector<bool>& fixBeta,
        bool useWeights) {

    if (norm.size() == 0) {
        initializeMM(boundType, fixBeta);
    }

#ifdef CYCLOPS_DEBUG_TIMING
#ifndef CYCLOPS_DEBUG_TIMING_LOW
    auto start = bsccs::chrono::steady_clock::now();
#endif
#endif

    for (int index = 0; index < J; ++index) {
        double *ogradient = &(gh[index].first);
        double *ohessian  = &(gh[index].second);

        if (fixBeta[index]) {
            *ogradient = 0.0; *ohessian = 0.0;
        } else {

        // Run-time dispatch, so virtual call should not effect speed
        if (useWeights) {
            switch (hX.getFormatType(index)) {
            case INDICATOR :
                computeMMGradientAndHessianImpl<IndicatorIterator<RealType>>(index, ogradient, ohessian, weighted);
                break;
            case SPARSE :
                computeMMGradientAndHessianImpl<SparseIterator<RealType>>(index, ogradient, ohessian, weighted);
                break;
            case DENSE :
                computeMMGradientAndHessianImpl<DenseIterator<RealType>>(index, ogradient, ohessian, weighted);
                break;
            case INTERCEPT :
                computeMMGradientAndHessianImpl<InterceptIterator<RealType>>(index, ogradient, ohessian, weighted);
                break;
            }
        } else {
            switch (hX.getFormatType(index)) {
            case INDICATOR :
                computeMMGradientAndHessianImpl<IndicatorIterator<RealType>>(index, ogradient, ohessian, unweighted);
                break;
            case SPARSE :
                computeMMGradientAndHessianImpl<SparseIterator<RealType>>(index, ogradient, ohessian, unweighted);
                break;
            case DENSE :
                computeMMGradientAndHessianImpl<DenseIterator<RealType>>(index, ogradient, ohessian, unweighted);
                break;
            case INTERCEPT :
                computeMMGradientAndHessianImpl<InterceptIterator<RealType>>(index, ogradient, ohessian, unweighted);
                break;
            }
        }
        }
    }

#ifdef CYCLOPS_DEBUG_TIMING
#ifndef CYCLOPS_DEBUG_TIMING_LOW
    auto end = bsccs::chrono::steady_clock::now();
    ///////////////////////////"
    duration["compMMGradAndHess"] += bsccs::chrono::duration_cast<chrono::TimingUnits>(end - start).count();
#endif
#endif

}

template <class BaseModel,typename RealType> template <class IteratorType, class Weights>
void ModelSpecifics<BaseModel,RealType>::computeMMGradientAndHessianImpl(int index, double *ogradient,
                                                                           double *ohessian, Weights w) {

#ifdef CYCLOPS_DEBUG_TIMING
#ifdef CYCLOPS_DEBUG_TIMING_LOW
    auto start = bsccs::chrono::steady_clock::now();
#endif
#endif

    RealType gradient = static_cast<RealType>(0);
    RealType hessian = static_cast<RealType>(0);

    IteratorType it(hX, index);
    for (; it; ++it) {
        const int k = it.index();

        BaseModel::template incrementMMGradientAndHessian<IteratorType, Weights>(
                gradient, hessian, offsExpXBeta[k],
                denomPid[BaseModel::getGroup(hPid, k)], hNWeight[BaseModel::getGroup(hPid, k)],
                it.value(), hXBeta[k], hY[k], norm[k]);
    }

    //hessian = 40 * modelData.getNumberOfStrata() * modelData.getNumberOfColumns() / 4.0; // curvature[index];

    // hessian *= curvature[index];

    //std::cerr << "g: " << gradient << " h: " << hessian << " f: " << hXjY[index] << std::endl;

    if (BaseModel::precomputeGradient) { // Compile-time switch
        gradient -= hXjY[index];
    }

//    std::cerr << hXjY[index] << std::endl;

    if (BaseModel::precomputeHessian) { // Compile-time switch
        hessian += static_cast<RealType>(2.0) * hXjX[index];
    }

    *ogradient = static_cast<double>(gradient);
    *ohessian = static_cast<double>(hessian);

    // std::cerr << index << " " << gradient << " " << hessian << std::endl;

#ifdef CYCLOPS_DEBUG_TIMING
#ifdef CYCLOPS_DEBUG_TIMING_LOW
    auto end = bsccs::chrono::steady_clock::now();
    ///////////////////////////"
    auto name = "compGradHessMM" + IteratorType::name + "";
    duration[name] += bsccs::chrono::duration_cast<chrono::TimingUnits>(end - start).count();
#endif
#endif
}

template <class BaseModel,typename RealType> template <class IteratorType, class Weights>
void ModelSpecifics<BaseModel,RealType>::computeGradientAndHessianImpl(int index, double *ogradient,
		double *ohessian, Weights w) {

#ifdef CYCLOPS_DEBUG_TIMING
#ifdef CYCLOPS_DEBUG_TIMING_LOW
	auto start = bsccs::chrono::steady_clock::now();
#endif
#endif

	RealType gradient = static_cast<RealType>(0);
	RealType hessian = static_cast<RealType>(0);

	if (BaseModel::cumulativeGradientAndHessian) { // Compile-time switch

#ifdef DEBUG_COX2
	    RealType lastG = gradient;
	    RealType lastH = hessian;
#endif

    	if (sparseIndices[index] == nullptr || sparseIndices[index]->size() > 0) {

		// TODO
		// x. Fill numerators <- 0
		// x. Compute non-zero numerators
		// x. Segmented scan of numerators
		// x. Transformation/reduction of [begin,end)

		IteratorType it(sparseIndices[index].get(), N);


		RealType accNumerPid  = static_cast<RealType>(0);
		RealType accNumerPid2 = static_cast<RealType>(0);

// 		const RealType* data = modelData.getDataVector(index);

        // find start relavent accumulator reset point
        auto reset = begin(accReset);
        while( *reset < it.index() ) {
            ++reset;
        }

		for (; it; ) {
			int i = it.index();

			if (*reset <= i) {
			    accNumerPid  = static_cast<RealType>(0.0);
			    accNumerPid2 = static_cast<RealType>(0.0);
			    ++reset;
			}


//			const RealType x = it.value();

//			const RealType x = (IteratorType::isIndicator) ? 1.0 :
//				(IteratorType::isSparse) ? *data : data[i];
// 			const RealType x = 1.0;

			const auto numerator1 = numerPid[i];
			const auto numerator2 = numerPid2[i];

//     		const RealType numerator1 = BaseModel::gradientNumeratorContrib(x, offsExpXBeta[i], hXBeta[i], hY[i]);
//     		const RealType numerator2 = BaseModel::gradientNumerator2Contrib(x, offsExpXBeta[i]);

     		accNumerPid += numerator1;
     		accNumerPid2 += numerator2;

//#define DEBUG_COX2

#ifdef DEBUG_COX2
#endif
			// Compile-time delegation
			BaseModel::incrementGradientAndHessian(it,
					w, // Signature-only, for iterator-type specialization
					&gradient, &hessian, accNumerPid, accNumerPid2,
					accDenomPid[i], hNWeight[i],
                             0.0,
                             //it.value(),
                             hXBeta[i], hY[i]);
					// When function is in-lined, compiler will only use necessary arguments
#ifdef DEBUG_COX2
			using namespace std;

			if (lastG != gradient || lastH != hessian) {

			cerr << "w: " << i << " " << hNWeight[i] << " " << numerator1 << ":" <<
				    accNumerPid << ":" << accNumerPid2 << ":" << accDenomPid[i];

			cerr << " -> g:" << gradient << " h:" << hessian << endl;
			}

			lastG = gradient; lastH = hessian;
#endif
			++it;

			if (IteratorType::isSparse) {
// 				++data;
				const int next = it ? it.index() : N;
				for (++i; i < next; ++i) {
#ifdef DEBUG_COX
			cerr << "q: " << i << " " << hNWeight[i] << " " << 0 << ":" <<
					accNumerPid << ":" << accNumerPid2 << ":" << accDenomPid[i];
#endif
                    if (*reset <= i) {
			            accNumerPid  = static_cast<RealType>(0.0);
        			    accNumerPid2 = static_cast<RealType>(0.0);
		        	    ++reset;
                   }

					BaseModel::incrementGradientAndHessian(it,
							w, // Signature-only, for iterator-type specialization
							&gradient, &hessian, accNumerPid, accNumerPid2,
							accDenomPid[i], hNWeight[i], static_cast<RealType>(0), hXBeta[i], hY[i]);
							// When function is in-lined, compiler will only use necessary arguments
#ifdef DEBUG_COX
			cerr << " -> g:" << gradient << " h:" << hessian << endl;
#endif

				}
			}
		}
		}

#ifdef DEBUG_COX2
    Rcpp::stop("out");
#endif

	} else if (BaseModel::hasIndependentRows) {

		auto range = helper::independent::getRangeX(hX, index,
		        offsExpXBeta, hXBeta, hY, denomPid, hNWeight,
		        typename IteratorType::tag());

		const auto result = variants::reduce(range.begin(), range.end(), Fraction<RealType>(0,0),
		    TransformAndAccumulateGradientAndHessianKernelIndependent<BaseModel,IteratorType, Weights, RealType, int>(),
 	        SerialOnly()
// 		RcppParallel()
		);


// 		const auto result2 = variants::reduce(range.begin(), range.end(), Fraction<RealType>(0,0),
// 		    TransformAndAccumulateGradientAndHessianKernelIndependent<BaseModel,IteratorType, Weights, RealType, int>(),
// // 			SerialOnly()
// 			RcppParallel()
// 		);


// 		std::cerr << result.real() << " " << result.imag()	<< std::endl;
// 		std::cerr << result2.real() << " " << result2.imag()	<< std::endl << std::endl;

		gradient = result.real();
		hessian = result.imag();

	} else {

// #ifdef OLD_WAY
//
// 		auto range = helper::getRangeDenominator(sparseIndices[index], N, typename IteratorType::tag());
//
// 		auto kernel = AccumulateGradientAndHessianKernel<BaseModel,IteratorType, Weights, real, int>(
// 							begin(numerPid), begin(numerPid2), begin(denomPid),
// 							begin(hNWeight), begin(hXBeta), begin(hY));
//
// 		Fraction<real> result = variants::reduce(range.begin(), range.end(), Fraction<real>(0,0), kernel,
// 		 SerialOnly()
// 	//     info
// 		);
//
// 		gradient = result.real();
// 		hessian = result.imag();
//
// #endif
//
// #ifdef NEW_WAY2

		auto rangeKey = helper::dependent::getRangeKey(hX, index, hPid,
		        typename IteratorType::tag());

        auto rangeXNumerator = helper::dependent::getRangeX(hX, index, offsExpXBeta,
                typename IteratorType::tag());

        auto rangeGradient = helper::dependent::getRangeGradient(sparseIndices[index].get(), N, // runtime error: reference binding to null pointer of type 'struct vector'
                denomPid, hNWeight,
                typename IteratorType::tag());

		const auto result = variants::trial::nested_reduce(
		        rangeKey.begin(), rangeKey.end(),
		        rangeXNumerator.begin(), rangeGradient.begin(),
		        std::pair<RealType,RealType>{0,0}, Fraction<RealType>{0,0},
                TestNumeratorKernel<BaseModel,IteratorType,RealType>(), // Inner transform-reduce
		       	TestGradientKernel<BaseModel,IteratorType,Weights>()); // Outer transform-reduce

		gradient = result.real();
		hessian = result.imag();
// #endif

//       std::cerr << std::endl
//            << result.real() << " " << result.imag() << std::endl
//            << result2.real() << " " << result2.imag() << std::endl
// 		   		 << result3.real() << " " << result3.imag() << std::endl;

//  		::Rf_error("break");

    } // not Cox

	//std::cerr << "g: " << gradient << " h: " << hessian << " f: " << hXjY[index] << std::endl;

	if (BaseModel::precomputeGradient) { // Compile-time switch
		gradient -= hXjY[index];
	}

	if (BaseModel::precomputeHessian) { // Compile-time switch
		hessian += static_cast<RealType>(2.0) * hXjX[index];
	}

 	*ogradient = static_cast<double>(gradient);
	*ohessian = static_cast<double>(hessian);

#ifdef CYCLOPS_DEBUG_TIMING
#ifdef CYCLOPS_DEBUG_TIMING_LOW
	auto end = bsccs::chrono::steady_clock::now();
	///////////////////////////"
	auto name = "compGradHess" + IteratorType::name + "  ";
	duration[name] += bsccs::chrono::duration_cast<chrono::TimingUnits>(end - start).count();
#endif
#endif

 }

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::computeFisherInformation(int indexOne, int indexTwo,
		double *oinfo, bool useWeights) {

	if (useWeights) {
// 		std::cerr << "Weights are not yet implemented in Fisher Information calculations" << std::endl;
// 		exit(-1);
		throw new std::logic_error("Weights are not yet implemented in Fisher Information calculations");
	} else { // no weights
		switch (hX.getFormatType(indexOne)) {
			case INDICATOR :
				dispatchFisherInformation<IndicatorIterator<RealType>>(indexOne, indexTwo, oinfo, weighted);
				break;
			case SPARSE :
				dispatchFisherInformation<SparseIterator<RealType>>(indexOne, indexTwo, oinfo, weighted);
				break;
			case DENSE :
				dispatchFisherInformation<DenseIterator<RealType>>(indexOne, indexTwo, oinfo, weighted);
				break;
			case INTERCEPT :
				dispatchFisherInformation<InterceptIterator<RealType>>(indexOne, indexTwo, oinfo, weighted);
				break;
		}
	}
}

template <class BaseModel, typename RealType> template <typename IteratorTypeOne, class Weights>
void ModelSpecifics<BaseModel,RealType>::dispatchFisherInformation(int indexOne, int indexTwo, double *oinfo, Weights w) {
	switch (hX.getFormatType(indexTwo)) {
		case INDICATOR :
			computeFisherInformationImpl<IteratorTypeOne,IndicatorIterator<RealType>>(indexOne, indexTwo, oinfo, w);
			break;
		case SPARSE :
			computeFisherInformationImpl<IteratorTypeOne,SparseIterator<RealType>>(indexOne, indexTwo, oinfo, w);
			break;
		case DENSE :
			computeFisherInformationImpl<IteratorTypeOne,DenseIterator<RealType>>(indexOne, indexTwo, oinfo, w);
			break;
		case INTERCEPT :
			computeFisherInformationImpl<IteratorTypeOne,InterceptIterator<RealType>>(indexOne, indexTwo, oinfo, w);
			break;
	}
//	std::cerr << "End of dispatch" << std::endl;
}


template<class BaseModel, typename RealType> template<class IteratorType>
SparseIterator<RealType> ModelSpecifics<BaseModel, RealType>::getSubjectSpecificHessianIterator(int index) {

	if (hessianSparseCrossTerms.find(index) == hessianSparseCrossTerms.end()) {
		// Make new
//		std::vector<int>* indices = new std::vector<int>();
        auto indices = make_shared<std::vector<int> >();
//		std::vector<real>* values = new std::vector<real>();
        auto values = make_shared<std::vector<RealType> >();
//		CompressedDataColumn* column = new CompressedDataColumn(indices, values,
//				SPARSE);
    	CDCPtr column = bsccs::make_shared<CompressedDataColumn<RealType>>(indices, values, SPARSE);
		hessianSparseCrossTerms.insert(std::make_pair(index,
// 		    CompressedDataColumn(indices, values, SPARSE)));
		    column));

		IteratorType itCross(hX, index);
		for (; itCross;) {
			RealType value = 0.0;
			int currentPid = hPid[itCross.index()];  // TODO Need to fix for stratified Cox
			do {
				const int k = itCross.index();
				value += BaseModel::gradientNumeratorContrib(itCross.value(),
						offsExpXBeta[k], hXBeta[k], hY[k]);
				++itCross;
			} while (itCross && currentPid == hPid[itCross.index()]); // TODO Need to fix for stratified Cox
			indices->push_back(currentPid);
			values->push_back(value);
		}
	}
	return SparseIterator<RealType>(*hessianSparseCrossTerms[index]);

}

template <class BaseModel, typename RealType> template <class IteratorTypeOne, class IteratorTypeTwo, class Weights>
void ModelSpecifics<BaseModel,RealType>::computeFisherInformationImpl(int indexOne, int indexTwo, double *oinfo, Weights w) {

	IteratorTypeOne itOne(hX, indexOne);
	IteratorTypeTwo itTwo(hX, indexTwo);
	PairProductIterator<IteratorTypeOne,IteratorTypeTwo,RealType> it(itOne, itTwo);

	RealType information = static_cast<RealType>(0);
	for (; it.valid(); ++it) {
		const int k = it.index();
		// Compile-time delegation

		BaseModel::incrementFisherInformation(it,
				w, // Signature-only, for iterator-type specialization
				&information,
				offsExpXBeta[k],
				0.0, 0.0, // numerPid[k], numerPid2[k], // remove
				denomPid[BaseModel::getGroup(hPid, k)],
				hKWeight[k], it.value(), hXBeta[k], hY[k]); // When function is in-lined, compiler will only use necessary arguments
	}

	if (BaseModel::hasStrataCrossTerms) {

		// Check if index is pre-computed
//#define USE_DENSE
#ifdef USE_DENSE
		if (hessianCrossTerms.find(indexOne) == hessianCrossTerms.end()) {
			// Make new
			std::vector<real> crossOneTerms(N);
			IteratorTypeOne crossOne(modelData, indexOne);
			for (; crossOne; ++crossOne) {
				const int k = crossOne.index();
				incrementByGroup(crossOneTerms.data(), hPid, k,
						BaseModel::gradientNumeratorContrib(crossOne.value(), offsExpXBeta[k], hXBeta[k], hY[k]));
			}
			hessianCrossTerms[indexOne];
//			std::cerr << std::accumulate(crossOneTerms.begin(), crossOneTerms.end(), 0.0) << std::endl;
			hessianCrossTerms[indexOne].swap(crossOneTerms);
		}
		std::vector<real>& crossOneTerms = hessianCrossTerms[indexOne];

		// TODO Remove code duplication
		if (hessianCrossTerms.find(indexTwo) == hessianCrossTerms.end()) {
			std::vector<real> crossTwoTerms(N);
			IteratorTypeTwo crossTwo(modelData, indexTwo);
			for (; crossTwo; ++crossTwo) {
				const int k = crossTwo.index();
				incrementByGroup(crossTwoTerms.data(), hPid, k,
						BaseModel::gradientNumeratorContrib(crossTwo.value(), offsExpXBeta[k], hXBeta[k], hY[k]));
			}
			hessianCrossTerms[indexTwo];
//			std::cerr << std::accumulate(crossTwoTerms.begin(), crossTwoTerms.end(), 0.0) << std::endl;
			hessianCrossTerms[indexTwo].swap(crossTwoTerms);
		}
		std::vector<real>& crossTwoTerms = hessianCrossTerms[indexTwo];

		// TODO Sparse loop
		real cross = 0.0;
		for (int n = 0; n < N; ++n) {
			cross += crossOneTerms[n] * crossTwoTerms[n] / (denomPid[n] * denomPid[n]);
		}
//		std::cerr << cross << std::endl;
		information -= cross;
#else
		SparseIterator<RealType> sparseCrossOneTerms = getSubjectSpecificHessianIterator<IteratorTypeOne>(indexOne);
		SparseIterator<RealType> sparseCrossTwoTerms = getSubjectSpecificHessianIterator<IteratorTypeTwo>(indexTwo);
		PairProductIterator<SparseIterator<RealType>,SparseIterator<RealType>,RealType> itSparseCross(sparseCrossOneTerms, sparseCrossTwoTerms);

		RealType sparseCross = static_cast<RealType>(0);
		for (; itSparseCross.valid(); ++itSparseCross) {
			const int n = itSparseCross.index();
			sparseCross += itSparseCross.value() / (denomPid[n] * denomPid[n]);
		}
		information -= sparseCross;
#endif
	}

	*oinfo = static_cast<double>(information);
}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::computeNumeratorForGradient(int index) {

#ifdef CYCLOPS_DEBUG_TIMING
#ifndef CYCLOPS_DEBUG_TIMING_LOW
	auto start = bsccs::chrono::steady_clock::now();
#endif
#endif

	if (BaseModel::cumulativeGradientAndHessian) {
//
// 		// Run-time delegation
// 		switch (modelData.getFormatType(index)) {
// 			case INDICATOR :
// 				incrementNumeratorForGradientImpl<IndicatorIterator>(index);
// 				break;
// 			case SPARSE :
// 				incrementNumeratorForGradientImpl<SparseIterator>(index);
// 				break;
// 			case DENSE :
// 				incrementNumeratorForGradientImpl<DenseIterator>(index);
// 				break;
// 			case INTERCEPT :
// 				incrementNumeratorForGradientImpl<InterceptIterator>(index);
// 				break;
// 			default : break;
// 				// throw error
// 		}
		switch (hX.getFormatType(index)) {
			case INDICATOR : {
				IndicatorIterator<RealType> it(*(sparseIndices)[index]);
				for (; it; ++it) { // Only affected entries
					numerPid[it.index()] = static_cast<RealType>(0.0);
				}
				incrementNumeratorForGradientImpl<IndicatorIterator<RealType>>(index);
				}
				break;
			case SPARSE : {
				SparseIterator<RealType> it(*(sparseIndices)[index]);
				for (; it; ++it) { // Only affected entries
					numerPid[it.index()] = static_cast<RealType>(0.0);
					if (BaseModel::hasTwoNumeratorTerms) { // Compile-time switch
						numerPid2[it.index()] = static_cast<RealType>(0.0); // TODO Does this invalid the cache line too much?
					}
				}
				incrementNumeratorForGradientImpl<SparseIterator<RealType>>(index); }
				break;
			case DENSE :
				zeroVector(numerPid.data(), N);
				if (BaseModel::hasTwoNumeratorTerms) { // Compile-time switch
					zeroVector(numerPid2.data(), N);
				}
				incrementNumeratorForGradientImpl<DenseIterator<RealType>>(index);
				break;
			case INTERCEPT :
				zeroVector(numerPid.data(), N);
				if (BaseModel::hasTwoNumeratorTerms) { // Compile-time switch
					zeroVector(numerPid2.data(), N);
				}
				incrementNumeratorForGradientImpl<InterceptIterator<RealType>>(index);
				break;
			default : break;
				// throw error
				//exit(-1);
		}
	}

#ifdef CYCLOPS_DEBUG_TIMING
#ifndef CYCLOPS_DEBUG_TIMING_LOW
	auto end = bsccs::chrono::steady_clock::now();
	///////////////////////////"
	duration["compNumForGrad   "] += bsccs::chrono::duration_cast<chrono::TimingUnits>(end - start).count();;
#endif
#endif

}

template <class BaseModel,typename RealType> template <class IteratorType>
void ModelSpecifics<BaseModel,RealType>::incrementNumeratorForGradientImpl(int index) {

#ifdef CYCLOPS_DEBUG_TIMING
#ifdef CYCLOPS_DEBUG_TIMING_LOW
	auto start = bsccs::chrono::steady_clock::now();
#endif
#endif

// #ifdef NEW_LOOPS

// 	auto zeroRange = helper::getRangeNumerator(sparseIndices[index], N, typename IteratorType::tag());
//
// 	auto zeroKernel = ZeroOutNumerator<BaseModel,IteratorType,real,int>(
// 			begin(numerPid), begin(numerPid2)
// 	);
//
// 	variants::for_each( // TODO Rewrite as variants::fill if rate-limiting
// 		zeroRange.begin(), zeroRange.end(),
// 		zeroKernel,
// // 		info
// // 		threadPool
// 		SerialOnly()
// // 		RcppParallel()
// 	);
//
// 	if (true) {
//
// 	auto computeRange = helper::getRangeX(modelData, index, typename IteratorType::tag());
//
// 	auto computeKernel = NumeratorForGradientKernel<BaseModel,IteratorType,real,int>(
// 					begin(numerPid), begin(numerPid2),
// 					begin(offsExpXBeta), begin(hXBeta),
// 					begin(hY),
// 					begin(hPid));
//
// 	variants::for_each(
// 		computeRange.begin(), computeRange.end(),
// 		computeKernel,
// // 		info
// //   		threadPool
//  		SerialOnly()
// //		RcppParallel() //  TODO Not thread-safe
// 		);
//
// 	} else {
// //	auto computeRange = helper::getRangeXDependent(modelData, index,
// //		numerPid, numerPid2, offsExpXBeta, hXBeta, hY, hPid,
// //// 		typename IteratorType::tag()
// //		DenseTag()
// //		);
// //
// //	auto info = C11Threads(4, 100);
//
//     // Let computeRange -> tuple<i, j, x>
//
// //    auto computeRangeCOO = helper::getRangeCOOX(modelData, index,
// //        typename IteratorType::tag());
// //
// //    variants::transform_segmented_reduce(
// //        computeRangeCOO.begin(), computeRangeCOO.end()
// //
// //    );
//
// 	}

// #else

	IteratorType it(hX, index);
	for (; it; ++it) {
		const int k = it.index();
		incrementByGroup(numerPid.data(), hPid, k,
				BaseModel::gradientNumeratorContrib(it.value(), offsExpXBeta[k], hXBeta[k], hY[k]));
		if (!IteratorType::isIndicator && BaseModel::hasTwoNumeratorTerms) {
			incrementByGroup(numerPid2.data(), hPid, k,
					BaseModel::gradientNumerator2Contrib(it.value(), offsExpXBeta[k]));
		}

#ifdef DEBUG_COX
using namespace std;
//			if (numerPid[BaseModel::getGroup(hPid, k)] > 0 && numerPid[BaseModel::getGroup(hPid, k)] < 1e-40) {
				cerr << "Increment" << endl;
				cerr << "hPid = "
				//<< hPid <<
				", k = " << k << ", index = " << BaseModel::getGroup(hPid, k) << endl;
				cerr << BaseModel::gradientNumeratorContrib(it.value(), offsExpXBeta[k], hXBeta[k], hY[k]) <<  " "
				<< it.value() << " " << offsExpXBeta[k] << " " << hXBeta[k] << " " << hY[k] << endl;
//				exit(-1);
//			}
#endif



	}

// #endif // NEW_LOOPS


#ifdef CYCLOPS_DEBUG_TIMING
#ifdef CYCLOPS_DEBUG_TIMING_LOW
	auto end = bsccs::chrono::steady_clock::now();
	///////////////////////////"
	auto name = "compNumGrad" + IteratorType::name + "   ";
	duration[name] += bsccs::chrono::duration_cast<chrono::TimingUnits>(end - start).count();
#endif
#endif

}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::updateXBeta(double delta, int index, bool useWeights) {

#ifdef CYCLOPS_DEBUG_TIMING
#ifndef CYCLOPS_DEBUG_TIMING_LOW
	auto start = bsccs::chrono::steady_clock::now();
#endif
#endif

	RealType realDelta = static_cast<RealType>(delta);

	// Run-time dispatch to implementation depending on covariate FormatType
	switch(hX.getFormatType(index)) {
		case INDICATOR :
			updateXBetaImpl<IndicatorIterator<RealType>>(realDelta, index, useWeights);
			break;
		case SPARSE :
			updateXBetaImpl<SparseIterator<RealType>>(realDelta, index, useWeights);
			break;
		case DENSE :
			updateXBetaImpl<DenseIterator<RealType>>(realDelta, index, useWeights);
			break;
		case INTERCEPT :
			updateXBetaImpl<InterceptIterator<RealType>>(realDelta, index, useWeights);
			break;
		default : break;
			// throw error
			//exit(-1);
	}

#ifdef CYCLOPS_DEBUG_TIMING
#ifndef CYCLOPS_DEBUG_TIMING_LOW
	auto end = bsccs::chrono::steady_clock::now();
	///////////////////////////"
	duration["updateXBeta      "] += bsccs::chrono::duration_cast<chrono::TimingUnits>(end - start).count();
#endif
#endif

}

template <class BaseModel,typename RealType> template <class IteratorType>
inline void ModelSpecifics<BaseModel,RealType>::updateXBetaImpl(RealType realDelta, int index, bool useWeights) {

#ifdef CYCLOPS_DEBUG_TIMING
#ifdef CYCLOPS_DEBUG_TIMING_LOW
	auto start = bsccs::chrono::steady_clock::now();
#endif
#endif

// #ifdef NEW_LOOPS

#if 1
	auto range = helper::getRangeX(hX, index, typename IteratorType::tag());

	auto kernel = UpdateXBetaKernel<BaseModel,IteratorType,RealType,int>(
					realDelta, begin(offsExpXBeta), begin(hXBeta),
					begin(hY),
					begin(hPid),
					begin(denomPid),
					begin(hOffs)
					);


	variants::for_each(
		range.begin(), range.end(),
		kernel,
// 		info
//          threadPool
// 		RcppParallel() // TODO Currently *not* thread-safe
          SerialOnly()
		);

#else

    if (BaseModel::hasIndependentRows) {

        auto range = helper::independent::getRangeXBeta(modelData, index,
                offsExpXBeta, hXBeta, denomPid, hOffs,
                typename IteratorType::tag());

        auto kernel = TestUpdateXBetaKernel<BaseModel,IteratorType,RealType>(realDelta);
        variants::for_each(
            range.begin(), range.end(),
            kernel,
            SerialOnly()
        );

    } else {

        auto rangeXBeta = helper::independent::getRangeXBeta(modelData, index,
            offsExpXBeta, hXBeta, denomPid, /* denom not used here */ hOffs,
            typename IteratorType::tag());

 		auto rangeKey = helper::dependent::getRangeKey(modelData, index, hPid,
		        typename IteratorType::tag());

		auto rangeDenominator = helper::dependent::getRangeDenominator(sparseIndices[index].get(), N,
		        denomPid, typename IteratorType::tag());

        auto kernel = TestUpdateXBetaKernelDependent<BaseModel,IteratorType,real>(realDelta);

        auto key = rangeKey.begin();
        auto end = rangeKey.end();
        auto inner = rangeXBeta.begin();
        auto outer = rangeDenominator.begin();

        const auto stop = end - 1;

        real result = 0;

        for (; key != stop; ++key, ++inner) {

            result = kernel(result, *inner);

            if (*key != *(key + 1)) {

                *outer = result + *outer;

                result = 0;
                ++outer;
            }
        }

        result = kernel(result, *inner);

        *outer = result + *outer;
    }

#endif

// 	std::cerr << std::endl << realDelta << std::endl;
//
// 	::Rf_error("return");



// #else
// 	IteratorType it(modelData, index);
// 	for (; it; ++it) {
// 		const int k = it.index();
// 		hXBeta[k] += realDelta * it.value(); // TODO Check optimization with indicator and intercept
// 		// Update denominators as well
// 		if (BaseModel::likelihoodHasDenominator) { // Compile-time switch
// 			real oldEntry = offsExpXBeta[k];
// 			real newEntry = offsExpXBeta[k] = BaseModel::getOffsExpXBeta(hOffs.data(), hXBeta[k], hY[k], k);
// 			incrementByGroup(denomPid, hPid, k, (newEntry - oldEntry));
// 		}
// 	}
//
// #endif

	computeAccumlatedDenominator(useWeights);

#ifdef CYCLOPS_DEBUG_TIMING
#ifdef CYCLOPS_DEBUG_TIMING_LOW
	auto end = bsccs::chrono::steady_clock::now();
	///////////////////////////"
	auto name = "updateXBeta" + IteratorType::name + "   ";
	duration[name] += bsccs::chrono::duration_cast<chrono::TimingUnits>(end - start).count();
#endif
#endif

}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::computeRemainingStatistics(bool useWeights) {

#ifdef CYCLOPS_DEBUG_TIMING
	auto start = bsccs::chrono::steady_clock::now();
#endif

	auto& xBeta = getXBeta();

	if (BaseModel::likelihoodHasDenominator) {
		fillVector(denomPid.data(), N, BaseModel::getDenomNullValue());
		for (size_t k = 0; k < K; ++k) {
			offsExpXBeta[k] = BaseModel::getOffsExpXBeta(hOffs.data(), xBeta[k], hY[k], k);
			incrementByGroup(denomPid.data(), hPid, k, offsExpXBeta[k]);
		}
		computeAccumlatedDenominator(useWeights); // WAS computeAccumlatedNumerDenom
	}

	// std::cerr << "finished MS.cRS" << std::endl;

#ifdef DEBUG_COX
	using namespace std;
	cerr << "Done with initial denominators" << endl;

	for (int i = 0; i < N; ++i) {
		cerr << denomPid[i] << " " << accDenomPid[i] << " " << numerPid[i] << endl;
	}
#endif

#ifdef CYCLOPS_DEBUG_TIMING
	auto end = bsccs::chrono::steady_clock::now();
	///////////////////////////"
	duration["compRS           "] += bsccs::chrono::duration_cast<chrono::TimingUnits>(end - start).count();;
#endif

}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::computeAccumlatedNumerator(bool useWeights) {

	if (BaseModel::likelihoodHasDenominator && //The two switches should ideally be separated
			BaseModel::cumulativeGradientAndHessian) { // Compile-time switch
		if (accNumerPid.size() != N) {
			accNumerPid.resize(N, static_cast<RealType>(0));
		}
		if (accNumerPid2.size() != N) {
			accNumerPid2.resize(N, static_cast<RealType>(0));
		}

		// segmented prefix-scan
		RealType totalNumer = static_cast<RealType>(0);
		RealType totalNumer2 = static_cast<RealType>(0);

		auto reset = begin(accReset);

		for (size_t i = 0; i < N; ++i) {

			if (static_cast<unsigned int>(*reset) == i) {
				totalNumer = static_cast<RealType>(0);
				totalNumer2 = static_cast<RealType>(0);
				++reset;
			}

			totalNumer += numerPid[i];
			totalNumer2 += numerPid2[i];
			accNumerPid[i] = totalNumer;
			accNumerPid2[i] = totalNumer2;
		}
	}
}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::computeAccumlatedDenominator(bool useWeights) {

	if (BaseModel::likelihoodHasDenominator && //The two switches should ideally be separated
		BaseModel::cumulativeGradientAndHessian) { // Compile-time switch
			if (accDenomPid.size() != (N + 1)) {
				accDenomPid.resize(N + 1, static_cast<RealType>(0));
			}
// 			if (accNumerPid.size() != N) {
// 				accNumerPid.resize(N, static_cast<RealType>(0));
// 			}
// 			if (accNumerPid2.size() != N) {
// 				accNumerPid2.resize(N, static_cast<RealType>(0));
// 			}

			// segmented prefix-scan
			RealType totalDenom = static_cast<RealType>(0);
// 			RealType totalNumer = static_cast<RealType>(0);
// 			RealType totalNumer2 = static_cast<RealType>(0);

			auto reset = begin(accReset);

			for (size_t i = 0; i < N; ++i) {
// TODO CHECK
				if (static_cast<unsigned int>(*reset) == i) { // TODO Check with SPARSE
					totalDenom = static_cast<RealType>(0);
// 					totalNumer = static_cast<RealType>(0);
// 					totalNumer2 = static_cast<RealType>(0);
					++reset;
				}

				totalDenom += denomPid[i];
// 				totalNumer += numerPid[i];
// 				totalNumer2 += numerPid2[i];
				accDenomPid[i] = totalDenom;
// 				accNumerPid[i] = totalNumer;
// 				accNumerPid2[i] = totalNumer2;
#if defined(DEBUG_COX) || defined(DEBUG_COX_MIN)
                using namespace std;
				cerr << denomPid[i] << " " << accDenomPid[i] << " (beta)" << endl;
#endif
			}
	}
}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::doSortPid(bool useCrossValidation) {
/* For Cox model:
 *
 * We currently assume that hZ[k] are sorted in decreasing order by k.
 *
 */
}


template <class BaseModel,typename RealType> template <typename AnyRealType>
void ModelSpecifics<BaseModel,RealType>::setPidForAccumulation(const AnyRealType* weights) {

    hPidInternal =  hPidOriginal; // Make copy
    hPid = hPidInternal.data(); // Point to copy
    accReset.clear();

    const int ignore = -1;

    // Find first non-zero weight
    size_t index = 0;
    while(weights != nullptr && weights[index] == 0.0 && index < K) {
        hPid[index] = ignore;
        index++;
    }

    int lastPid = hPid[index];
    AnyRealType lastTime = hOffs[index];
    AnyRealType lastEvent = hY[index];

    int pid = hPid[index] = 0;

    for (size_t k = index + 1; k < K; ++k) {
        if (weights == nullptr || weights[k] != 0.0) {
            int nextPid = hPid[k];

            if (nextPid != lastPid) { // start new strata
                pid++;
                accReset.push_back(pid);
                lastPid = nextPid;
            } else {

                if (lastEvent == 1.0 && lastTime == hOffs[k] && lastEvent == hY[k]) {
                    // In a tie, do not increment denominator
                } else {
                    pid++;
                }
            }
            lastTime = hOffs[k];
            lastEvent = hY[k];

            hPid[k] = pid;
        } else {
            hPid[k] = ignore;
        }
    }
    pid++;
    accReset.push_back(pid);

    // Save number of denominators
    N = pid;

    if (weights != nullptr) {
        for (size_t i = 0; i < K; ++i) {
            if (hPid[i] == ignore) hPid[i] = N; // do NOT accumulate, since loops use: i < N
        }
    }
    setupSparseIndices(N); // ignore pid == N (pointing to removed data strata)
}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::setupSparseIndices(const int max) {
    sparseIndices.clear(); // empty if full!

    for (size_t j = 0; j < J; ++j) {
        if (hX.getFormatType(j) == DENSE || hX.getFormatType(j) == INTERCEPT) {
            sparseIndices.push_back(NULL);
        } else {
            std::set<int> unique;
            const size_t n = hX.getNumberOfEntries(j);
            const int* indicators = hX.getCompressedColumnVector(j);
            for (size_t j = 0; j < n; j++) { // Loop through non-zero entries only
                const int k = indicators[j];
                const int i = hPid[k];  // TODO container-overflow #Generate some simulated data: #Fit the model
                if (i < max) {
                    unique.insert(i);
                }
            }
            auto indices = bsccs::make_shared<IndexVector>(unique.begin(), unique.end());
            sparseIndices.push_back(indices);
        }
    }
}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::deviceInitialization() {
    // Do nothing
}

template <class BaseModel,typename RealType>
void ModelSpecifics<BaseModel,RealType>::initialize(
        int iN,
        int iK,
        int iJ,
        const void*,
        double* iNumerPid,
        double* iNumerPid2,
        double* iDenomPid,
        //		int* iNEvents,
        double* iXjY,
        std::vector<std::vector<int>* >* iSparseIndices,
        const int* iPid_unused,
        double* iOffsExpXBeta,
        double* iXBeta,
        double* iOffs,
        double* iBeta,
        const double* iY_unused//,
    //		real* iWeights
) {
    N = iN;
    K = iK;
    J = iJ;
    offsExpXBeta.resize(K);
    hXBeta.resize(K);

    if (allocateXjY()) {
        hXjY.resize(J);
    }

    if (allocateXjX()) {
        hXjX.resize(J);
    }

    if (initializeAccumulationVectors()) {
        setPidForAccumulation(static_cast<double*>(nullptr)); // calls setupSparseIndices() before returning
    } else {
        // TODO Suspect below is not necessary for non-grouped data.
        // If true, then fill with pointers to CompressedDataColumn and do not delete in destructor
        setupSparseIndices(N); // Need to be recomputed when hPid change!
    }



    size_t alignedLength = getAlignedLength(N + 1);
    // 	numerDenomPidCache.resize(3 * alignedLength, 0);
    // 	numerPid = numerDenomPidCache.data();
    // 	denomPid = numerPid + alignedLength; // Nested in denomPid allocation
    // 	numerPid2 = numerPid + 2 * alignedLength;
    denomPid.resize(alignedLength);
    numerPid.resize(alignedLength);
    numerPid2.resize(alignedLength);

    deviceInitialization();

}

} // namespace

#endif /* MODELSPECIFICS_HPP_ */
//
