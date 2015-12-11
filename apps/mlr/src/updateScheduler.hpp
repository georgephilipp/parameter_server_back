#include "gstd/src/standard.h"

#pragma once

namespace mlr
{
	struct RowUpdateItem
	{
		int first;
		int numRows;
	};

	class UpdateScheduler
	{
	public:		
		UpdateScheduler(int communicationFactor, int virtualStaleness, int numRows);
		RowUpdateItem next();

	private:
		int N;
		int k;
		int R;
		int numerator;
	};
}
