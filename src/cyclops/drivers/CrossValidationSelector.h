/*
 * CrossValidation.h
 *
 *  Created on: Sep 9, 2010
 *      Author: msuchard
 */

#ifndef CROSSVALIDATION_H_
#define CROSSVALIDATION_H_

#include <vector>

#include "AbstractSelector.h"

namespace bsccs {

class CrossValidationSelector : public AbstractSelector {
public:
	CrossValidationSelector(
			int inFold,
			std::vector<int> inIds,
			SelectorType inType,
			long inSeed,
    	    loggers::ProgressLoggerPtr _logger,
	    	loggers::ErrorHandlerPtr _error,
			std::vector<double> const* base_weights);

	virtual ~CrossValidationSelector();

	void permute();
	
 	void reseed();

	void getWeights(int batch, std::vector<real>& weights);

	void getComplement(std::vector<real>& weights);
	
	AbstractSelector* clone() const;

private:
	int fold;
	std::vector<int> permutation;
	std::vector<int> intervalStart;
	std::vector<double> const* base_weights;
	std::vector<int> weight_map;
	int num_base_weights;
};

} // namespace

#endif /* CROSSVALIDATION_H_ */
