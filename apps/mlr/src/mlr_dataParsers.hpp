#include "gstd/src/standard.h"
#include "abstract_mlr_sgd_solver.hpp"

namespace mlr
{
	namespace mlrDataParsers
	{
		void custom_imnet
		(
			std::string filePath, 
			int32_t fileSize, 
			int32_t numData,
			int32_t offset,
			std::vector<petuum::ml::AbstractFeature<float>*>* features,
			std::vector<int32_t>* labels
		);

		void printDataSet(std::string path, std::vector<petuum::ml::AbstractFeature<float>*> data, int32_t featureDim);
	}
}
