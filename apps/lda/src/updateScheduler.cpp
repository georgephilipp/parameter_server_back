#include "updateScheduler.hpp"
#include <petuum_ps_common/include/table_gflags_declare.hpp>
#include "common.hpp"
#include <gflags/gflags.h>
#include <glog/logging.h>

namespace lda
{
	UpdateScheduler::UpdateScheduler(int communicationFactor, int virtualStaleness, int numRows)
	{
		R = numRows;
		N = virtualStaleness + 1;
		k = communicationFactor;
		numerator = 0;//-N;
////////////
LOG(INFO) << "initializing UpdateScheduler with R, N, k, numerator: " << R << " " << N << " " << k << " " << numerator << " ";
	}

	RowUpdateItem UpdateScheduler::next()
	{
		CHECK(N >= 2 && k >= 1 && FLAGS_num_clocks_per_work_unit == 1 && FLAGS_table_staleness == 0) << "cannot use the UpdateScheduler with these parms";
		RowUpdateItem res;
		int nextNumerator = numerator + 2*(k+1);
		int numPastRows = (numerator * R) / (2*k*N);
		if(numerator < 0)
			numPastRows = 0;
		int numNextRows = (nextNumerator * R) / (2*k*N);
		res.first = numPastRows % R;
		res.numRows = numNextRows - numPastRows;
		numerator = nextNumerator;
		return res;
	}

}

