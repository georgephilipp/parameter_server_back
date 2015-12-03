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

using namespace msii810161816;
// DW: remove
#include <sstream>


namespace mlr {

//constructor
LRSGDSolver::LRSGDSolver(const LRSGDSolverConfig& config) :
  w_table_(config.w_table),
  w_cache_(config.feature_dim),
  w_delta_(config.feature_dim),
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
    return prediction[label] >= 0.5 ? 0 : 1;
  }

//XEntropy Loss: trivial
float LRSGDSolver::CrossEntropyLoss(const std::vector<float>& prediction, int32_t label)
  const {
    CHECK_LE(prediction[label], 1);
    CHECK_GE(prediction[label], 0);
    return -petuum::ml::SafeLog(prediction[label]);
  }

//inc and get: from and to thread cache
void LRSGDSolver::push() {
  // Write delta's to PS table.
  int num_full_rows = feature_dim_ / w_table_num_cols_;
  for (int i = 0; i < num_full_rows; ++i) {
    petuum::DenseUpdateBatch<float> w_update_batch(0, w_table_num_cols_);
    for (int j = 0; j < w_table_num_cols_; ++j) {
      int idx = i * w_table_num_cols_ + j;
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
    }  
    w_table_.DenseBatchInc(i, w_update_batch);
  }

  if (feature_dim_ % w_table_num_cols_ != 0) {
    // last incomplete row.
    int num_cols_last_row = feature_dim_ - num_full_rows * w_table_num_cols_;
    petuum::DenseUpdateBatch<float> w_update_batch(0, num_cols_last_row);
    for (int j = 0; j < num_cols_last_row; ++j) {
      int idx = num_full_rows * w_table_num_cols_ + j;
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
    }
    w_table_.DenseBatchInc(num_full_rows, w_update_batch);
  }

  // Zero delta.
  std::vector<float>& w_delta_vec = w_delta_.GetVector();
  std::fill(w_delta_vec.begin(), w_delta_vec.end(), 0);

  //petuum::PSTableGroup::Clock();
}

void LRSGDSolver::pull() {
  // Read w from the PS.
  int num_full_rows = feature_dim_ / w_table_num_cols_;
  std::vector<float>& w_cache_vec = w_cache_.GetVector();
  std::vector<float> w_cache(w_table_num_cols_);
  for (int i = 0; i < num_full_rows; ++i) {
    petuum::RowAccessor row_acc;
    const auto& r = w_table_.Get<petuum::DenseRow<float>>(i, &row_acc);
    r.CopyToVector(&w_cache);
    std::copy(w_cache.begin(), w_cache.end(),
              w_cache_vec.begin() + i * w_table_num_cols_);
  }
  if (feature_dim_ % w_table_num_cols_ != 0) {
    // last incomplete row.
    int num_cols_last_row = feature_dim_ - num_full_rows * w_table_num_cols_;
    std::vector<float> w_cache(w_table_num_cols_);
    petuum::RowAccessor row_acc;
    const auto& r = w_table_.Get<petuum::DenseRow<float>>(num_full_rows, &row_acc);
    r.CopyToVector(&w_cache);
    std::copy(w_cache.begin(), w_cache.begin() + num_cols_last_row,
        w_cache_vec.begin() + num_full_rows * w_table_num_cols_);
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
