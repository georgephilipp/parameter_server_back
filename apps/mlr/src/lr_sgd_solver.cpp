// Author: Dai Wei (wdai@cs.cmu.edu)
// Date: 2014.10.04

#include "lr_sgd_solver.hpp"
#include "common.hpp"
#include <vector>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <petuum_ps_common/include/petuum_ps.hpp>
#include <ml/include/ml.hpp>
#include <ml/util/fastapprox/fastapprox.hpp>
#include "gstd/src/Vector.h"
#include "gstd/src/Printer.h"
#include "gstd/src/ex.h"
#include "updateScheduler.hpp"
#include <petuum_ps_common/include/system_gflags_declare.hpp>

using namespace msii810161816;
// DW: remove
#include <sstream>


namespace mlr {

//constructor
LRSGDSolver::LRSGDSolver(const LRSGDSolverConfig& config) :
  w_table_(config.w_table),
  w_cache_(config.feature_dim),
  w_delta_(config.feature_dim),
  w_delta_other_(config.feature_dim),
  feature_dim_(config.feature_dim),
  w_table_num_cols_(config.w_table_num_cols), lambda_(config.lambda),
  predict_buff_(2) {    // 2 for binary (2 classes).
    if (config.sparse_data) {
      // Sparse feature, dense weight.
      FeatureDotProductFun_ = petuum::ml::SparseDenseFeatureDotProduct;
    } else {
      // Both weight and feature are dense
      FeatureDotProductFun_ = petuum::ml::DenseDenseFeatureDotProduct;
    }
  }

LRSGDSolver::~LRSGDSolver() { }

//calculates a single update: applies it to thread cache and thread delta
void LRSGDSolver::SingleDataSGD(const petuum::ml::AbstractFeature<float>& feature,
    int32_t label, float learning_rate) {
  Predict(feature, &predict_buff_);

  // Apply gradient to w_delta_. This must happen first as w_cache_ would
  // change after gradient application and weight decay uses gradient values.
  petuum::ml::FeatureScaleAndAdd(-learning_rate * (predict_buff_[1] - label), feature,
    &w_delta_);
  if (lambda_ > 0) {
    petuum::ml::FeatureScaleAndAdd(-learning_rate * lambda_, w_cache_,
    &w_delta_);
  }

  // Apply gradient to w_cache_
  if(FLAGS_add_immediately) 
  {
    if (lambda_ > 0) {
      petuum::ml::FeatureScaleAndAdd(-learning_rate * lambda_, w_cache_,
          &w_cache_);
    }
    petuum::ml::FeatureScaleAndAdd(-learning_rate * (predict_buff_[1] - label), feature,
        &w_cache_);
  }

  // If local sync, then we also feed the other cache
  if(FLAGS_is_local_sync)
  {
    petuum::ml::FeatureScaleAndAdd(-learning_rate * (predict_buff_[1] - label), feature,
      &w_delta_other_);
    if (lambda_ > 0) {
      petuum::ml::FeatureScaleAndAdd(-learning_rate * lambda_, w_cache_,
      &w_delta_other_);
    }
  }
}

//predict based on thread cache
void LRSGDSolver::Predict(
    const petuum::ml::AbstractFeature<float>& feature,
    std::vector<float> *result) const {
  std::vector<float> &y_vec = *result;
  // fastsigmoid is numerically unstable for input <-88.
  y_vec[0] = 1 - petuum::ml::Sigmoid(FeatureDotProductFun_(feature, w_cache_));
  y_vec[1] = 1 - y_vec[0];
}

//zerooneloss: trivial
int32_t LRSGDSolver::ZeroOneLoss(const std::vector<float>& prediction, int32_t label)
  const {
    if(label == 0)
        return prediction[0] > 0.5 ? 0 : 1;
    else
        return prediction[1] >= 0.5 ? 0 : 1;
  }

//XEntropy Loss: trivial
float LRSGDSolver::CrossEntropyLoss(const std::vector<float>& prediction, int32_t label)
  const {
    /*CHECK_LE(prediction[label], 1);
    CHECK_GE(prediction[label], 0);*/
    if(!(prediction[label] > 0 && prediction[label] <= 1))
        return 10000000000000;
    else
        return -petuum::ml::SafeLog(prediction[label]);
  }

void LRSGDSolver::pushRow(int index, bool isOffset)
{
  int num_full_rows = feature_dim_ / w_table_num_cols_;
  bool hasPartialRow = (feature_dim_ - w_table_num_cols_ * num_full_rows != 0);
  int numRows = num_full_rows;
  if(hasPartialRow)
    numRows++;
  CHECK(index < numRows) << "index of pulled row too large";
  int numCols = w_table_num_cols_;
  if(index == num_full_rows)
    numCols = feature_dim_ - w_table_num_cols_ * num_full_rows; 
  petuum::DenseUpdateBatch<float> w_update_batch(0, numCols);
  for (int j = 0; j < numCols; ++j) 
  {
    int idx = index * w_table_num_cols_ + j;
    if(!(w_delta_[idx] == w_delta_[idx]))
    {
      if(FLAGS_ignore_nan)
      {
        w_delta_[idx] = 0;
      }
      else
      {
        CHECK_EQ(w_delta_[idx], w_delta_[idx]) << "nan detected.";
      }
    }
    w_update_batch[j] = w_delta_[idx];
    w_delta_[idx] = 0;
  }
  w_table_.DenseBatchInc(index + (isOffset ? numRows : 0), w_update_batch);
}

void LRSGDSolver::pushOther(int index, bool isOffset)
{
  int num_full_rows = feature_dim_ / w_table_num_cols_;
  bool hasPartialRow = (feature_dim_ - w_table_num_cols_ * num_full_rows != 0);
  int numRows = num_full_rows;
  if(hasPartialRow)
    numRows++;
  CHECK(index < numRows) << "index of pulled row too large";
  int numCols = w_table_num_cols_;
  if(index == num_full_rows)
    numCols = feature_dim_ - w_table_num_cols_ * num_full_rows; 
  petuum::DenseUpdateBatch<float> w_update_batch(0, numCols);
  for (int j = 0; j < numCols; ++j) 
  {
    int idx = index * w_table_num_cols_ + j;
    if(!(w_delta_other_[idx] == w_delta_other_[idx]))
    {
      if(FLAGS_ignore_nan)
      {
        w_delta_other_[idx] = 0;
      }
      else
      {
        CHECK_EQ(w_delta_other_[idx], w_delta_other_[idx]) << "nan detected.";
      }
    }
    w_update_batch[j] = w_delta_other_[idx];
    w_delta_other_[idx] = 0;
  }
  w_table_.DenseBatchInc(index + (isOffset ? numRows : 0), w_update_batch);
} 

//inc and get: from and to thread cache
void LRSGDSolver::push(RowUpdateItem item) {
  // Write delta's to PS table.
  int num_full_rows = feature_dim_ / w_table_num_cols_;
  bool hasPartialRow = (feature_dim_ % w_table_num_cols_ != 0);
  int totalRows = num_full_rows;
  if(hasPartialRow)
    totalRows++;
  int currentRow = item.first;

  if(!FLAGS_is_local_sync)
  {
    for (int i = 0; i < item.numRows; ++i) 
    {
      if(currentRow == totalRows)
        currentRow = 0;
      pushRow(currentRow, false);
      currentRow++;
    }

    if(FLAGS_is_bipartite)
    {
      for(int i=0; i<totalRows;i++)
      {
        if((i+FLAGS_client_id) % 2 == 1)
          continue;
        pushRow(i, false);
      }
    }
  }
  else
  {
    for (int i = 0; i < totalRows; i++) 
    {
        pushRow(i, ((FLAGS_client_id % 2 == 1) ? true : false ));
    }
    
    for (int i = 0; i < item.numRows; ++i) 
    {
      if(currentRow == totalRows)
        currentRow = 0;
      pushOther(currentRow, ((FLAGS_client_id % 2 == 0) ? true : false ));
      currentRow++;
    }
  }
}

//inc and get: from and to thread cache
void LRSGDSolver::push() {
  // Write delta's to PS table.
  // Read w from the PS.
  int num_full_rows = feature_dim_ / w_table_num_cols_;
  bool hasPartialRow = (feature_dim_ - w_table_num_cols_ * num_full_rows != 0);
  int totalRows = num_full_rows;
  if(hasPartialRow)
    totalRows++;
  for (int i = 0; i < totalRows; i++) 
  {
    pushRow(i, false);
  }
}


void LRSGDSolver::pullRow(int index, bool isOffset)
{
  int num_full_rows = feature_dim_ / w_table_num_cols_;
  bool hasPartialRow = (feature_dim_ - w_table_num_cols_ * num_full_rows != 0);
  int numRows = num_full_rows;
  if(hasPartialRow)
    numRows++;
  CHECK(index < numRows) << "index of pulled row too large";
  int numCols = w_table_num_cols_;
  if(index == num_full_rows)
    numCols = feature_dim_ - w_table_num_cols_ * num_full_rows;    
  std::vector<float>& w_cache_vec = w_cache_.GetVector();
  std::vector<float> w_cache(w_table_num_cols_);
  petuum::RowAccessor row_acc;
  const auto& r = w_table_.Get<petuum::DenseRow<float>>(index + (isOffset ? numRows : 0), &row_acc);
  r.CopyToVector(&w_cache);
  std::copy(w_cache.begin(), w_cache.begin() + numCols, w_cache_vec.begin() + index * w_table_num_cols_);
}  


void LRSGDSolver::pull(RowUpdateItem item)
{
  int num_full_rows = feature_dim_ / w_table_num_cols_;
  bool hasPartialRow = (feature_dim_ - w_table_num_cols_ * num_full_rows != 0);
  int totalRows = num_full_rows;
  if(hasPartialRow)
    totalRows++;
  int currentRow = item.first;

//////////////
//if(FLAGS_client_id == 0)
//LOG(INFO) << "client " << FLAGS_client_id << " pulling from row " << item.first << " and number of rows is " << item.numRows;
  if(!FLAGS_is_local_sync)
  {
    for(int i=0;i<item.numRows;i++)
    {
      if(currentRow == totalRows)
        currentRow = 0;
      pullRow(currentRow, false);
      currentRow++;
    }
     
    if(FLAGS_is_bipartite)
    {
      for (int i = 0; i < totalRows; i++) 
      {
        if((i+FLAGS_client_id) % 2 == 1)
          continue;
        pullRow(i, false);
      }
    }
  }
  else
  {
    for (int i = 0; i < totalRows; i++) 
    {
        pullRow(i, ((FLAGS_client_id % 2 == 1) ? true : false ));
    }
  }
}


void LRSGDSolver::pull()
{
  // Read w from the PS.
  int num_full_rows = feature_dim_ / w_table_num_cols_;
  bool hasPartialRow = (feature_dim_ - w_table_num_cols_ * num_full_rows != 0);
  int totalRows = num_full_rows;
  if(hasPartialRow)
    totalRows++;
  for (int i = 0; i < totalRows; i++) 
  {
    pullRow(i, false);
  }
}

void LRSGDSolver::SaveWeights(const std::string& filename) const {
  LOG(ERROR) << "SaveWeights is not implemented for binary LR.";
}

float LRSGDSolver::EvaluateL2RegLoss() const {
  double l2_norm = 0.;
  std::vector<float> w = w_cache_.GetVector();
  for (int i = 0; i < feature_dim_; ++i) {
    l2_norm += w[i] * w[i];
  }
  return 0.5 * lambda_ * l2_norm;
}


}  // namespace mlr
