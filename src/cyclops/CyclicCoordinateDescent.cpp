/*
 * CyclicCoordinateDescent.cpp
 *
 *  Created on: May-June, 2010
 *      Author: msuchard
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>
#include <time.h>
#include <set>
#include <list>

#include "CyclicCoordinateDescent.h"
// #include "io/InputReader.h"
#include "Iterators.h"
#include "Timing.h"
//#include "priors/JointPrior.h"
// #include "io/ProgressLogger.h"

//#ifdef MY_RCPP_FLAG
//	#include <R.h>
//#else
//	void Rprintf(...) {
//		// Do nothing
//	}
//#endif

#ifndef MY_RCPP_FLAG
#define PI	3.14159265358979323851280895940618620443274267017841339111328125
#else
//#include "Rcpp.h"
#endif

namespace bsccs {

using namespace std;

// void compareIntVector(int* vec0, int* vec1, int dim, const char* name) {
// 	for (int i = 0; i < dim; i++) {
// 		if (vec0[i] != vec1[i]) {
// 			cerr << "Error at " << name << "[" << i << "]: ";
// 			cerr << vec0[i] << " != " << vec1[i] << endl;
// 			exit(0);
// 		}
// 	}
// }

CyclicCoordinateDescent::CyclicCoordinateDescent(
			//ModelData* reader,
			const ModelData& reader,			
			AbstractModelSpecifics& specifics,
			priors::JointPriorPtr prior,
			loggers::ProgressLoggerPtr _logger,
			loggers::ErrorHandlerPtr _error
		) : privateModelSpecifics(nullptr), modelSpecifics(specifics), jointPrior(prior), 
		 hXBeta(modelSpecifics.getXBeta()), hXBetaSave(modelSpecifics.getXBetaSave()), // TODO Remove
		hXI(reader), logger(_logger), error(_error) {
	N = hXI.getNumberOfPatients();
	K = hXI.getNumberOfRows();
	J = hXI.getNumberOfColumns();
	
// 	hXI = reader;
	hY = hXI.getYVector(); // TODO Delegate all data to ModelSpecifics
//	hOffs = reader->getOffsetVector();
	hPid = hXI.getPidVector();

	conditionId = hXI.getConditionId();

	updateCount = 0;
	likelihoodCount = 0;
	noiseLevel = NOISY;

	init(hXI.getHasOffsetCovariate());
}

CyclicCoordinateDescent* CyclicCoordinateDescent::clone() {	
	return new CyclicCoordinateDescent(*this);
} 

//template <typename T>
//struct GetType<T>; 

CyclicCoordinateDescent::CyclicCoordinateDescent(const CyclicCoordinateDescent& copy)
	: privateModelSpecifics(
			bsccs::unique_ptr<AbstractModelSpecifics>(
				copy.modelSpecifics.clone())), // deep copy
	  modelSpecifics(*privateModelSpecifics),  
	  hXBeta(modelSpecifics.getXBeta()), hXBetaSave(modelSpecifics.getXBetaSave()), // TODO Remove
// 	  jointPrior(priors::JointPriorPtr(copy.jointPrior->clone())), // deep copy
	  jointPrior(copy.jointPrior), // swallow
	  hXI(copy.hXI), // swallow
	  logger(copy.logger), error(copy.error) {
	        	        
	N = hXI.getNumberOfPatients();
	K = hXI.getNumberOfRows();
	J = hXI.getNumberOfColumns();	
	
	hY = hXI.getYVector(); // TODO Delegate all data to ModelSpecifics
	hPid = hXI.getPidVector();

	conditionId = hXI.getConditionId();

	updateCount = 0;
	likelihoodCount = 0;
	noiseLevel = copy.noiseLevel;

	init(hXI.getHasOffsetCovariate());			
}

CyclicCoordinateDescent::~CyclicCoordinateDescent(void) {

//	free(hPid);
//	free(hNEvents);
//	free(hY);
//	free(hOffs);
	
//	free(hBeta);
//	free(hXBeta);
//	free(hXBetaSave);
//	free(hDelta);
	
// #ifdef TEST_ROW_INDEX
// 	for (int j = 0; j < J; ++j) {
// 		if (hXColumnRowIndicators[j]) {
// 			free(hXColumnRowIndicators[j]);
// 		}
// 	}
// 	free(hXColumnRowIndicators);
// #endif

//	free(hXjY);
//	free(offsExpXBeta);
//	free(xOffsExpXBeta);
//	free(denomPid);  // Nested in numerPid allocation
//	free(numerPid);
//	free(t1);
	
// #ifdef NO_FUSE
// 	free(wPid);
// #endif
	
// 	if (hWeights) {
// 		free(hWeights);
// 	}

#ifdef SPARSE_PRODUCT
// 	for (std::vector<std::vector<int>* >::iterator it = sparseIndices.begin();
// 			it != sparseIndices.end(); ++it) {
// 		if (*it) {
// 			delete *it;
// 		}
// 	}
#endif
}

void CyclicCoordinateDescent::setNoiseLevel(NoiseLevels noise) {
	noiseLevel = noise;
}

string CyclicCoordinateDescent::getPriorInfo() {
	return jointPrior->getDescription();
}

void CyclicCoordinateDescent::setPrior(priors::JointPriorPtr newPrior) {
    jointPrior = newPrior;
}

void CyclicCoordinateDescent::resetBounds() {
	for (int j = 0; j < J; j++) {
		hDelta[j] = 2.0;
	}
}

void CyclicCoordinateDescent::init(bool offset) {
		
	// Set parameters and statistics space
	hDelta.resize(J, static_cast<double>(2.0));
	hBeta.resize(J, static_cast<double>(0.0));

// 	hXBeta = (real*) calloc(K, sizeof(real));
// 	hXBetaSave = (real*) calloc(K, sizeof(real));
	hXBeta.resize(K, static_cast<real>(0.0));
	hXBetaSave.resize(K, static_cast<real>(0.0));
	
	fixBeta.resize(J, false);
	
	// SHOULD BE HANDLED IN MODELDATA CONSTRUCTORS
// 	//Recode patient ids  TODO Delegate to grouped model
// 	int currentNewId = 0;
// 	int currentOldId = hPid[0];
// 	
// 	for(int i = 0; i < K; i++) {
// 		if (hPid[i] != currentOldId) {			
// 			currentOldId = hPid[i];
// 			currentNewId++;
// 		}
// 		hPid[i] = currentNewId;
// 	}
		
	// Init temporary variables
//	offsExpXBeta = (real*) malloc(sizeof(real) * K);
//	xOffsExpXBeta = (real*) malloc(sizeof(real) * K);

	// Put numer, numer2 and denom in single memory block, with first entries on 16-word boundary
// 	int alignedLength = getAlignedLength(N);
// 	numerPid = (real*) malloc(sizeof(real) * 3 * alignedLength);
// 	denomPid = numerPid + alignedLength; // Nested in denomPid allocation
// 	numerPid2 = numerPid + 2 * alignedLength;

//	hNEvents = (int*) malloc(sizeof(int) * N);
//	hXjY = (real*) malloc(sizeof(real) * J);
	hWeights.resize(0);
	
// #ifdef NO_FUSE
// 	wPid = (real*) malloc(sizeof(real) * alignedLength);
// #endif

	// TODO Suspect below is not necessary for non-grouped data.
	// If true, then fill with pointers to CompressedDataColumn and do not delete in destructor
// 	for (int j = 0; j < J; ++j) {
// 		if (hXI->getFormatType(j) == DENSE) {
// 			sparseIndices.push_back(NULL);
// 		} else {
// 			std::set<int> unique;
// 			const int n = hXI->getNumberOfEntries(j);
// 			const int* indicators = hXI->getCompressedColumnVector(j);
// 			for (int j = 0; j < n; j++) { // Loop through non-zero entries only
// 				const int k = indicators[j];
// 				const int i = hPid[k]; // ERROR HERE IN STRAT-COX, ALSO CONSIDER TIES, MOVE TO ENGINE???
// 				unique.insert(i);
// 			}
// 			std::vector<int>* indices = new std::vector<int>(unique.begin(),
// 					unique.end());
// 			sparseIndices.push_back(indices);
// 		}
// 	}

	useCrossValidation = false;
	validWeights = false;
	sufficientStatisticsKnown = false;
	fisherInformationKnown = false;
	varianceKnown = false;
	if (offset) {
		hBeta[0] = static_cast<double>(1);
		fixBeta[0] = true;
		xBetaKnown = false;
	} else {
		xBetaKnown = true; // all beta = 0 => xBeta = 0
	}
	doLogisticRegression = false;
	        
	modelSpecifics.initialize(N, K, J, &hXI, NULL, NULL, NULL,
			NULL, NULL,
			hPid, NULL,
			hXBeta.data(), NULL,
			NULL,
			hY
			);
}

int CyclicCoordinateDescent::getAlignedLength(int N) {
	return (N / 16) * 16 + (N % 16 == 0 ? 0 : 16);
}

void CyclicCoordinateDescent::computeNEvents() {  
	modelSpecifics.setWeights(
		hWeights.size() > 0 ? hWeights.data() : nullptr,
		useCrossValidation);
}

void CyclicCoordinateDescent::resetBeta(void) {
	for (int j = 0; j < J; j++) {
		hBeta[j] = 0.0;
	}
	for (int k = 0; k < K; k++) {
		hXBeta[k] = 0.0;
	}
	sufficientStatisticsKnown = false;
}

void CyclicCoordinateDescent::logResults(const char* fileName, bool withASE) {

	ofstream outLog(fileName);
	if (!outLog) {
	    std::ostringstream stream;
		stream << "Unable to open log file: " << fileName;
		error->throwError(stream);		
	}
	string sep(","); // TODO Make option

	outLog << "label"
			<< sep << "estimate"
			//<< sep << "score"
			;
	if (withASE) {
		outLog << sep << "ASE";
	}
	outLog << endl;

	for (int i = 0; i < J; i++) {		
		outLog << hXI.getColumn(i).getLabel()
//				<< sep << conditionId
				<< sep << hBeta[i];
		if (withASE) {
			double ASE = sqrt(getAsymptoticVariance(i,i));
			outLog << sep << ASE;
		}
		outLog << endl;
	}
	outLog.flush();
	outLog.close();
}

double CyclicCoordinateDescent::getPredictiveLogLikelihood(real* weights) {

	xBetaKnown = false;

	if (!xBetaKnown) {
		computeXBeta();
		xBetaKnown = true;
		sufficientStatisticsKnown = false;
	}

	if (!sufficientStatisticsKnown) {
		computeRemainingStatistics(true, 0); // TODO Remove index????
		sufficientStatisticsKnown = true;
	}

	getDenominators();

	return modelSpecifics.getPredictiveLogLikelihood(weights); // TODO Pass double
}

void CyclicCoordinateDescent::getPredictiveEstimates(real* y, real* weights) const {
	modelSpecifics.getPredictiveEstimates(y, weights);
}

int CyclicCoordinateDescent::getBetaSize(void) {
	return J;
}

int CyclicCoordinateDescent::getPredictionSize(void) const {
	return K;
}

bool CyclicCoordinateDescent::getIsRegularized(int i) const {
    return jointPrior->getIsRegularized(i);    
}

double CyclicCoordinateDescent::getBeta(int i) {
	if (!sufficientStatisticsKnown) {
		computeRemainingStatistics(true, i);
	}
	return static_cast<double>(hBeta[i]);
}

bool CyclicCoordinateDescent::getFixedBeta(int i) {
	return fixBeta[i];
}

void CyclicCoordinateDescent::setFixedBeta(int i, bool value) {
	fixBeta[i] = value;
}

void CyclicCoordinateDescent::checkAllLazyFlags(void) {
	if (!xBetaKnown) {
		computeXBeta();
		xBetaKnown = true;
		sufficientStatisticsKnown = false;
	}

	if (!validWeights) {
		computeNEvents();
		computeFixedTermsInLogLikelihood();
		computeFixedTermsInGradientAndHessian();
		validWeights = true;
	}

	if (!sufficientStatisticsKnown) {
		computeRemainingStatistics(true, 0); // TODO Check index?
		sufficientStatisticsKnown = true;
	}
}

double CyclicCoordinateDescent::getLogLikelihood(void) {

	checkAllLazyFlags();

	getDenominators();
	likelihoodCount += 1;

	return modelSpecifics.getLogLikelihood(useCrossValidation);
}

void CyclicCoordinateDescent::getDenominators() {
	// Do nothing
}

double convertVarianceToHyperparameter(double value) {
	return sqrt(2.0 / value);
}

double convertHyperparameterToVariance(double value) {
	return 2.0 / (value * value);
}

void CyclicCoordinateDescent::setHyperprior(double value) {
	jointPrior->setVariance(value);
}

//Hierarchical Support
void CyclicCoordinateDescent::setClassHyperprior(double value) {
	jointPrior->setVariance(1,value);
}

std::vector<double> CyclicCoordinateDescent::getHyperprior(void) const {
	return jointPrior->getVariance();
}

void CyclicCoordinateDescent::makeDirty(void) {
	xBetaKnown = false;
	validWeights = false;
	sufficientStatisticsKnown = false;
}

void CyclicCoordinateDescent::setPriorType(int iPriorType) {
	if (iPriorType < priors::NONE || iPriorType > priors::NORMAL) {
	    std::ostringstream stream;
		stream << "Unknown prior type";
		error->throwError(stream);		
	}
	priorType = iPriorType;
}

void CyclicCoordinateDescent::setBeta(const std::vector<double>& beta) {
	for (int j = 0; j < J; ++j) {
		hBeta[j] = beta[j]; // TODO Use std::copy
	}
	xBetaKnown = false;
	sufficientStatisticsKnown = false;
	fisherInformationKnown = false;
	varianceKnown = false;
}

void CyclicCoordinateDescent::setBeta(int i, double beta) {
#define PROCESS_IN_MS
#ifdef PROCESS_IN_MS
	double delta = beta - hBeta[i];
	updateXBeta(delta, i);
#else // Delay and then call computeSufficientStatistics
	SetBetaEntry entry(i, hBeta[i]);
	setBetaList.push_back(entry); // save old value and index
	hBeta[i] = beta;
	xBetaKnown = false;
#endif
	fisherInformationKnown = false;
	varianceKnown = false;
}

void CyclicCoordinateDescent::setWeights(real* iWeights) {

	if (iWeights == NULL) {
		if (hWeights.size() != 0) {			
			hWeights.resize(0);
		}
		
		// Turn off weights
		useCrossValidation = false;
		validWeights = false;
		sufficientStatisticsKnown = false;
	} else {

		if (hWeights.size() != K) {
			hWeights.resize(K); // = (real*) malloc(sizeof(real) * K);
		}
		for (int i = 0; i < K; ++i) {
			hWeights[i] = iWeights[i];
		}
		useCrossValidation = true;
		validWeights = false;
		sufficientStatisticsKnown = false;
	}
}

void CyclicCoordinateDescent::getWeights(std::vector<real>& weights) {
	if (weights.size() != K) {
		weights.resize(K);
	}

	if (hWeights.size() != K) {
		// If no weights have been set, return default weights.
		std::fill(weights.begin(), weights.end(), 1.0);
	} else {
		std::copy(hWeights.begin(), hWeights.end(), weights.begin());
	}
}

double CyclicCoordinateDescent::getLogPrior(void) {
	return jointPrior->logDensity(hBeta);
}

double CyclicCoordinateDescent::getObjectiveFunction(int convergenceType) {
	if (convergenceType == GRADIENT) {
		real criterion = 0;
		if (useCrossValidation) {
			for (int i = 0; i < K; i++) {
				criterion += hXBeta[i] * hY[i] * hWeights[i];
			}
		} else {
			for (int i = 0; i < K; i++) {
				criterion += hXBeta[i] * hY[i];
			}
		}
		return static_cast<double> (criterion);
	} else
	if (convergenceType == MITTAL) {
		return getLogLikelihood();
	} else
	if (convergenceType == LANGE) {
		return getLogLikelihood() + getLogPrior();
	} else {
    	std::ostringstream stream;
    	stream << "Invalid convergence type: " << convergenceType;
    	error->throwError(stream);	
    }
    return 0.0;
}

double CyclicCoordinateDescent::computeZhangOlesConvergenceCriterion(void) {
	double sumAbsDiffs = 0;
	double sumAbsResiduals = 0;
	if (useCrossValidation) {
		for (int i = 0; i < K; i++) {
			sumAbsDiffs += abs(hXBeta[i] - hXBetaSave[i]) * hWeights[i];
			sumAbsResiduals += abs(hXBeta[i]) * hWeights[i];
		}
	} else {
		for (int i = 0; i < K; i++) {
			sumAbsDiffs += abs(hXBeta[i] - hXBetaSave[i]);
			sumAbsResiduals += abs(hXBeta[i]);
		}
	}
	return sumAbsDiffs / (1.0 + sumAbsResiduals);
}

void CyclicCoordinateDescent::saveXBeta(void) {
	memcpy(hXBetaSave.data(), hXBeta.data(), K * sizeof(real));
}

void CyclicCoordinateDescent::update(const ModeFindingArguments& arguments) {
	
	const auto maxIterations = arguments.maxIterations;
	const auto convergenceType = arguments.convergenceType;
	const auto epsilon = arguments.tolerance;	
		
 	if (arguments.useKktSwindle && jointPrior->getSupportsKktSwindle()) {
		kktSwindle(arguments);		
	} else {
		findMode(maxIterations, convergenceType, epsilon);
	}
}

typedef std::tuple<
	int,    // index	
	double, // gradient
	bool    // force active
> ScoreTuple;

template <typename Iterator>
void CyclicCoordinateDescent::findMode(Iterator begin, Iterator end,
		const int maxIterations, const int convergenceType, const double epsilon) {
		
	std::fill(fixBeta.begin(), fixBeta.end(), true);
 	std::for_each(begin, end, [this] (ScoreTuple& tuple) {
 		fixBeta[std::get<0>(tuple)] = false;
 	});
	findMode(maxIterations, convergenceType, epsilon);
    // fixBeta is no longer valid
}

void CyclicCoordinateDescent::kktSwindle(const ModeFindingArguments& arguments) {

	const auto maxIterations = arguments.maxIterations;
	const auto convergenceType = arguments.convergenceType;
	const auto epsilon = arguments.tolerance;
	
	// Make sure internal state is up-to-date	
	checkAllLazyFlags();	
	
		
	std::list<ScoreTuple> activeSet;
	std::list<ScoreTuple> inactiveSet;
	std::list<int> excludeSet;		
				
	// Initialize sets
	int intercept = -1;
	if (hXI.getHasInterceptCovariate()) {
		intercept = hXI.getHasOffsetCovariate() ? 1 : 0;
	}
	
	for (int index = 0; index < J; ++index) {
		if (fixBeta[index]) {
			excludeSet.push_back(index);
		} else {
			if (index == intercept || // Always place intercept into active set
                !jointPrior->getSupportsKktSwindle(index)) {
// 				activeSet.push_back(index);
				activeSet.push_back(std::make_tuple(index, 0.0, true));
			} else {
				inactiveSet.push_back(std::make_tuple(index, 0.0, false));
			}
		}
	}
	
	bool done = false;
	int swindleIterationCount = 1;
	
	int initialActiveSize = activeSet.size();
	int perPassSize = arguments.swindleMultipler;
	
	while (!done) {
	
		if (noiseLevel >= QUIET) {
			std::ostringstream stream;
			stream << "\nKKT Swindle count " << swindleIterationCount << ", activeSet size =  " << activeSet.size();
			logger->writeLine(stream);
		}
		
		// Enforce all beta[inactiveSet] = 0
		for (auto& inactive : inactiveSet) {
			if (getBeta(std::get<0>(inactive)) != 0.0) { // Touch only if necessary
				setBeta(std::get<0>(inactive), 0.0);
			}
		}
	
		double updateTime = 0.0;
		lastReturnFlag = SUCCESS;
		if (activeSet.size() > 0) { // find initial mode
			auto start = bsccs::chrono::steady_clock::now();
			  		
			findMode(begin(activeSet), end(activeSet), maxIterations, convergenceType, epsilon);
			
			auto end = bsccs::chrono::steady_clock::now();
			bsccs::chrono::duration<double> elapsed_seconds = end-start;	
			updateTime = elapsed_seconds.count();    			
		}
		
		if (noiseLevel >= QUIET) {
			std::ostringstream stream;
			stream << "update time: " << updateTime << " in " << lastIterationCount << " iterations.";
			logger->writeLine(stream);
		}
		
		if (inactiveSet.size() == 0 || lastReturnFlag != SUCCESS) { // Computed global mode, nothing more to do, or failed
		
			done = true;		
			
		} else { // still inactive covariates
								
			
			if (swindleIterationCount == maxIterations) {
				lastReturnFlag = MAX_ITERATIONS;		
				done = true;
				if (noiseLevel > SILENT) {
					std::ostringstream stream;	
					stream << "Reached maximum swindle iterations";
					logger->writeLine(stream);
				}
			} else {
		
				auto checkConditions = [this] (const ScoreTuple& score) {
					return (std::get<1>(score) <= jointPrior->getKktBoundary(std::get<0>(score)));
				};
				
				auto checkAlmostConditions = [this] (const ScoreTuple& score) {
					return (std::get<1>(score) < 0.9 * jointPrior->getKktBoundary(std::get<0>(score)));
				};				
		
				// Check KKT conditions
									
				computeKktConditions(inactiveSet);				
			
				bool satisfied = std::all_of(begin(inactiveSet), end(inactiveSet), checkConditions);
			
				if (satisfied) {
					done = true;				
				} else {
					auto newActiveSize = initialActiveSize + perPassSize;
					
					auto count1 = std::distance(begin(inactiveSet), end(inactiveSet));
					auto count2 = std::count_if(begin(inactiveSet), end(inactiveSet), checkConditions);
					
					
					// Remove elements from activeSet if less than 90% of boundary
// 					computeKktConditions(activeSet); // TODO Already computed in findMode		
// 					
// // 					for (auto& active : activeSet) {
// // 						std::ostringstream stream;
// // 						stream << "Active: " << std::get<0>(active) << " : " << std::get<1>(active) << " : " << std::get<2>(active);
// // 						logger->writeLine(stream);																						
// // 					}		
// 					
// 					int countActiveViolations = 0;
// 					while(activeSet.size() > 0 &&
// 						checkAlmostConditions(activeSet.back())
// 					) {
// 						auto& back = activeSet.back();
// 						
// // 						std::ostringstream stream;
// // 						stream << "Remove: " << std::get<0>(back) << ":" << std::get<1>(back) 
// // 						       << " cut @ " << jointPrior->getKktBoundary(std::get<0>(back))
// // 						       << " diff = " << (std::get<1>(back) - jointPrior->getKktBoundary(std::get<0>(back)));
// // 						logger->writeLine(stream);						
// 						
// 						inactiveSet.push_back(back);
// 						activeSet.pop_back();		
// 						++countActiveViolations;									
// 					}
					// end
																			
					// Move inactive elements into active if KKT conditions are not met
					while (inactiveSet.size() > 0
							&& !checkConditions(inactiveSet.front())
//  							&& activeSet.size() < newActiveSize
					) {
						auto& front = inactiveSet.front();
						
// 						std::ostringstream stream;
// 						stream << std::get<0>(front) << ":" << std::get<1>(front);
// 						logger->writeLine(stream);
						
						activeSet.push_back(front);
						inactiveSet.pop_front();
					}	
										
					if (noiseLevel >= QUIET) {
						std::ostringstream stream;
// 						stream << "  Active set violations: " << countActiveViolations << std::endl;  
						stream << "Inactive set violations: " << (count1 - count2);
						logger->writeLine(stream);
					}													
				}			
			}									
		}
		++swindleIterationCount;
		perPassSize *= 2;
		
		logger->yield();			// This is not re-entrant safe	
	}
						
	// restore fixBeta
	std::fill(fixBeta.begin(), fixBeta.end(), false);
	for (auto index : excludeSet) {
		fixBeta[index] = true;
	}
}

template <typename Container>
void CyclicCoordinateDescent::computeKktConditions(Container& scoreSet) {

    for (auto& score : scoreSet) {    
        const auto index = std::get<0>(score);     
           
		computeNumeratorForGradient(index);
	
		priors::GradientHessian gh;
		computeGradientAndHessian(index, &gh.first, &gh.second);
		
		std::get<1>(score) = std::abs(gh.first);
    }
    
    scoreSet.sort([] (ScoreTuple& lhs, ScoreTuple& rhs) -> bool {
    	if (std::get<2>(rhs) == std::get<2>(lhs)) {
			return (std::get<1>(rhs) < std::get<1>(lhs));    	    	
    	} else {
    		return(std::get<2>(lhs));
    	}
    });    
}	


void CyclicCoordinateDescent::findMode(
		int maxIterations,
		int convergenceType,
		double epsilon
		) {

	if (convergenceType < GRADIENT || convergenceType > ZHANG_OLES) {
	    std::ostringstream stream;
		stream << "Unknown convergence criterion: " << convergenceType;
		error->throwError(stream);				
	}
	
	if (!validWeights) {    	   	
		computeNEvents();
		computeFixedTermsInLogLikelihood();
		computeFixedTermsInGradientAndHessian();
		validWeights = true;		
	}

	if (!xBetaKnown) {
		computeXBeta();
		xBetaKnown = true;
		sufficientStatisticsKnown = false;
	}

	if (!sufficientStatisticsKnown) {
		computeRemainingStatistics(true, 0); // TODO Check index?
		sufficientStatisticsKnown = true;
	}
	
	resetBounds();

	bool done = false;
	int iteration = 0;
	double lastObjFunc = 0.0;

	if (convergenceType < ZHANG_OLES) {
		lastObjFunc = getObjectiveFunction(convergenceType);
	} else { // ZHANG_OLES
		saveXBeta();
	}
	
	while (!done) {
	
		// Do a complete cycle
		for(int index = 0; index < J; index++) {
		
			if (!fixBeta[index]) {
				double delta = ccdUpdateBeta(index);
				delta = applyBounds(delta, index);
				if (delta != 0.0) {
					sufficientStatisticsKnown = false;
					updateSufficientStatistics(delta, index);
				}
			}
			
			if ( (noiseLevel > QUIET) && ((index+1) % 100 == 0)) {
			    std::ostringstream stream;
			    stream << "Finished variable " << (index+1);
			    logger->writeLine(stream);
			}
			
		}
		
		iteration++;
//		bool checkConvergence = (iteration % J == 0 || iteration == maxIterations);
		bool checkConvergence = true; // Check after each complete cycle

		if (checkConvergence) {

			double conv;
			bool illconditioned = false;
			if (convergenceType < ZHANG_OLES) {
 				double thisObjFunc = getObjectiveFunction(convergenceType);
				if (thisObjFunc != thisObjFunc) {
				    std::ostringstream stream;
					stream << "\nWarning! problem is ill-conditioned for this choice of hyperparameter. Enforcing convergence!";
					logger->writeLine(stream);
					conv = 0.0;
					illconditioned = true;
				} else {
					conv = computeConvergenceCriterion(thisObjFunc, lastObjFunc);
				}
				lastObjFunc = thisObjFunc;
			} else { // ZHANG_OLES
				conv = computeZhangOlesConvergenceCriterion();
				saveXBeta();
			} // Necessary to call getObjFxn or computeZO before getLogLikelihood,
			  // since these copy over XBeta

			double thisLogLikelihood = getLogLikelihood();
			double thisLogPrior = getLogPrior();
			double thisLogPost = thisLogLikelihood + thisLogPrior;

            std::ostringstream stream;
			if (noiseLevel > QUIET) {			    
			    stream << "\n";				
				printVector(&hBeta[0], J, stream);
				stream << "\n";
				stream << "log post: " << thisLogPost
						<< " (" << thisLogLikelihood << " + " << thisLogPrior
						<< ") (iter:" << iteration << ") ";
			}

			if (epsilon > 0 && conv < epsilon) {
				if (illconditioned) {
					lastReturnFlag = ILLCONDITIONED;
				} else {
					if (noiseLevel > SILENT) {
						stream << "Reached convergence criterion";
					}
					lastReturnFlag = SUCCESS;
				}
				done = true;
			} else if (iteration == maxIterations) {
				if (noiseLevel > SILENT) {
					stream << "Reached maximum iterations";
				}
				done = true;
				lastReturnFlag = MAX_ITERATIONS;
			}
			if (noiseLevel > QUIET) {
                logger->writeLine(stream);
			}
						
			logger->yield();			// This is not re-entrant safe
		}						
	}
	lastIterationCount = iteration;
	updateCount += 1;
	
	modelSpecifics.printTiming();

	fisherInformationKnown = false;
	varianceKnown = false;
}

/**
 * Computationally heavy functions
 */

void CyclicCoordinateDescent::computeGradientAndHessian(int index, double *ogradient,
		double *ohessian) {
	modelSpecifics.computeGradientAndHessian(index, ogradient, ohessian, useCrossValidation);
}

void CyclicCoordinateDescent::computeNumeratorForGradient(int index) {
	// Delegate
	modelSpecifics.computeNumeratorForGradient(index);
}

void CyclicCoordinateDescent::computeRatiosForGradientAndHessian(int index) {
    std::ostringstream stream;
	stream << "Error!";
	error->throwError(stream);
}

double CyclicCoordinateDescent::getHessianDiagonal(int index) {

	checkAllLazyFlags();
	double g_d1, g_d2;

	computeNumeratorForGradient(index);
	computeGradientAndHessian(index, &g_d1, &g_d2);

	return g_d2;
}

double CyclicCoordinateDescent::getAsymptoticVariance(int indexOne, int indexTwo) {
	checkAllLazyFlags();
	if (!fisherInformationKnown) {
		computeAsymptoticPrecisionMatrix();
		fisherInformationKnown = true;
	}

	if (!varianceKnown) {
		computeAsymptoticVarianceMatrix();
		varianceKnown = true;
	}

	IndexMap::iterator itOne = hessianIndexMap.find(indexOne);
	IndexMap::iterator itTwo = hessianIndexMap.find(indexTwo);

	if (itOne == hessianIndexMap.end() || itTwo == hessianIndexMap.end()) {
		return NAN;
	} else {
		return varianceMatrix(itOne->second, itTwo->second);
	}
}

double CyclicCoordinateDescent::getAsymptoticPrecision(int indexOne, int indexTwo) {
	checkAllLazyFlags();
	if (!fisherInformationKnown) {
		computeAsymptoticPrecisionMatrix();
		fisherInformationKnown = true;
	}

	IndexMap::iterator itOne = hessianIndexMap.find(indexOne);
	IndexMap::iterator itTwo = hessianIndexMap.find(indexTwo);

	if (itOne == hessianIndexMap.end() || itTwo == hessianIndexMap.end()) {
		return NAN;
	} else {
		return hessianMatrix(itOne->second, itTwo->second);
	}
}

CyclicCoordinateDescent::Matrix CyclicCoordinateDescent::computeFisherInformation(const std::vector<size_t>& indices) const {
    Matrix fisherInformation(indices.size(), indices.size());
    for (size_t ii = 0; ii < indices.size(); ++ii) {
        const int i = indices[ii];
        for (size_t jj = ii; jj < indices.size(); ++jj) {
            const int j = indices[jj];
            double element = 0.0;            
             modelSpecifics.computeFisherInformation(i, j, &element, useCrossValidation);
            fisherInformation(jj, ii) = fisherInformation(ii, jj) = element;
        }
    }
    return fisherInformation;
}

void CyclicCoordinateDescent::computeAsymptoticPrecisionMatrix(void) {

	typedef std::vector<int> int_vec;
	int_vec indices;
	hessianIndexMap.clear();

	int index = 0;
	for (int j = 0; j < J; ++j) {
		if (!fixBeta[j] &&
				(priorType != priors::LAPLACE || getBeta(j) != 0.0)) {
			indices.push_back(j);
			hessianIndexMap[j] = index;
			index++;
		}
	}

	hessianMatrix.resize(indices.size(), indices.size());
	modelSpecifics.makeDirty(); // clear hessian terms

	for (size_t ii = 0; ii < indices.size(); ++ii) {
		for (size_t jj = ii; jj < indices.size(); ++jj) {
			const int i = indices[ii];
			const int j = indices[jj];
//			std::cerr << "(" << i << "," << j << ")" << std::endl;
			double fisherInformation = 0.0;
			modelSpecifics.computeFisherInformation(i, j, &fisherInformation, useCrossValidation);
//			if (fisherInformation != 0.0) {
				// Add tuple to sparse matrix
//				tripletList.push_back(Triplet<double>(ii,jj,fisherInformation));
//			}
			hessianMatrix(jj,ii) = hessianMatrix(ii,jj) = fisherInformation;

//			std::cerr << "Info = " << fisherInformation << std::endl;
//			std::cerr << "Hess = " << getHessianDiagonal(0) << std::endl;
//			exit(-1);
		}
	}

//	sm.setFromTriplets(tripletList.begin(), tripletList.end());
//	cout << sm;
//	auto inv = sm.triangularView<Upper>().solve(dv1);
//	cout << sm.inverse();
//	cout << hessianMatrix << endl;

	// Take inverse
//	hessianMatrix = hessianMatrix.inverse();

//	cout << hessianMatrix << endl;
}

void CyclicCoordinateDescent::computeAsymptoticVarianceMatrix(void) {
	varianceMatrix = hessianMatrix.inverse();
// 	cout << varianceMatrix << endl;
}

double CyclicCoordinateDescent::ccdUpdateBeta(int index) {

	if (!sufficientStatisticsKnown) {
	    std::ostringstream stream;
		stream << "Error in state synchronization.";
		error->throwError(stream);				
	}
		
	computeNumeratorForGradient(index);
	
	priors::GradientHessian gh;
	computeGradientAndHessian(index, &gh.first, &gh.second);
				
	if (gh.second < 0.0) {
	    gh.first = 0.0;	
	    gh.second = 0.0;
	}
	
    return jointPrior->getDelta(gh, hBeta, index);
}

template <class IteratorType>
void CyclicCoordinateDescent::axpy(real* y, const real alpha, const int index) {
	IteratorType it(hXI, index);
	for (; it; ++it) {
		const int k = it.index();
		y[k] += alpha * it.value();
	}
}

void CyclicCoordinateDescent::axpyXBeta(const real beta, const int j) {
	if (beta != static_cast<real>(0.0)) {
		switch (hXI.getFormatType(j)) {
		case INDICATOR:
			axpy < IndicatorIterator > (hXBeta.data(), beta, j);
			break;
		case DENSE:
			axpy < DenseIterator > (hXBeta.data(), beta, j);
			break;
		case SPARSE:
			axpy < SparseIterator > (hXBeta.data(), beta, j);
			break;
		default:
			// throw error
			std::ostringstream stream;
			stream << "Unknown vector type.";
			error->throwError(stream);
		}
	}
}

void CyclicCoordinateDescent::computeXBeta(void) {
	// Note: X is current stored in (sparse) column-major format, which is
	// inefficient for forming X\beta.
	// TODO Make row-major version of X

	if (setBetaList.empty()) { // Update all
		// clear X\beta
		zeroVector(hXBeta.data(), K);
		for (int j = 0; j < J; ++j) {
			axpyXBeta(hBeta[j], j);
		}
	} else {
		while (!setBetaList.empty()) {
			SetBetaEntry entry = setBetaList.front();
			axpyXBeta(hBeta[entry.first] - entry.second, entry.first);
			setBetaList.pop_front();
		}
	}
}

void CyclicCoordinateDescent::updateXBeta(double delta, int index) {
	// Update beta
	real realDelta = static_cast<real>(delta);
	hBeta[index] += delta;

	// Delegate
	modelSpecifics.updateXBeta(realDelta, index, useCrossValidation);
}

void CyclicCoordinateDescent::updateSufficientStatistics(double delta, int index) {
	updateXBeta(delta, index);
	sufficientStatisticsKnown = true;
}

void CyclicCoordinateDescent::computeRemainingStatistics(bool allStats, int index) { // TODO Rename
	// Separate function for benchmarking
	if (allStats) {
		// Delegate
		modelSpecifics.computeRemainingStatistics(useCrossValidation);
	}
}

/**
 * Updating and convergence functions
 */

double CyclicCoordinateDescent::computeConvergenceCriterion(double newObjFxn, double oldObjFxn) {	
	// This is the stopping criterion that Ken Lange generally uses
	return abs(newObjFxn - oldObjFxn) / (abs(newObjFxn) + 1.0);
}

double CyclicCoordinateDescent::applyBounds(double inDelta, int index) {
	double delta = inDelta;
	if (delta < -hDelta[index]) {
		delta = -hDelta[index];
	} else if (delta > hDelta[index]) {
		delta = hDelta[index];
	}

	hDelta[index] = max(2.0 * abs(delta), 0.5 * hDelta[index]);
	return delta;
}

/**
 * Utility functions
 */

void CyclicCoordinateDescent::computeFixedTermsInLogLikelihood(void) {
	modelSpecifics.computeFixedTermsInLogLikelihood(useCrossValidation);
}

void CyclicCoordinateDescent::computeFixedTermsInGradientAndHessian(void) {
	// Delegate
	modelSpecifics.computeFixedTermsInGradientAndHessian(useCrossValidation);
}

template <class T>
void CyclicCoordinateDescent::printVector(T* vector, int length, ostream &os) {
	os << "(" << vector[0];
	for (int i = 1; i < length; i++) {
		os << ", " << vector[i];
	}
	os << ")";			
}

template <class T> 
T* CyclicCoordinateDescent::readVector(
		const char *fileName,
		int *length) {

	ifstream fin(fileName);
	T d;
	std::vector<T> v;
	
	while (fin >> d) {
		v.push_back(d);
	}
	
	T* ptr = (T*) malloc(sizeof(T) * v.size());
	for (int i = 0; i < v.size(); i++) {
		ptr[i] = v[i];
	}
		
	*length = v.size();
	return ptr;
}

void CyclicCoordinateDescent::testDimension(int givenValue, int trueValue, const char *parameterName) {
	if (givenValue != trueValue) {
	    std::ostringstream stream;
		stream << "Wrong dimension in " << parameterName << " vector.";
		error->throwError(stream);		
	}
}	

inline int CyclicCoordinateDescent::sign(double x) {
	if (x == 0) {
		return 0;
	}
	if (x < 0) {
		return -1;
	}
	return 1;
}

} // namespace
