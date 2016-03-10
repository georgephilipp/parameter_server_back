// Author: Dai Wei (wdai@cs.cmu.edu)
// Date: 2014.10.04

#pragma once

#include <petuum_ps_common/include/petuum_ps.hpp>
#include <ml/include/ml.hpp>
#include <cstdint>
#include <vector>
#include <functional>
#include "abstract_mlr_sgd_solver.hpp"

namespace mlr {

struct MLRSGDSolverConfig {
  int32_t feature_dim;
  int32_t num_labels;
  bool sparse_data;
  bool sparse_weight;
  petuum::Table<float> w_table;
};

// The caller thread must be registered with PS.
class MLRSGDSolver : public AbstractMLRSGDSolver {
public:
  MLRSGDSolver(const MLRSGDSolverConfig& config);
  ~MLRSGDSolver();

  // Compute gradient using feature and label and apply to w_table_.
  void SingleDataSGD(const petuum::ml::AbstractFeature<float>& feature,
      int32_t label, float step_size);

  void SingleDataSGD(const petuum::ml::DenseFeature<float>& feature,
                     int32_t label, float step_size);

  // Predict the probability of each label.
  void Predict(const petuum::ml::AbstractFeature<float>& feature,
      std::vector<float> *result) const;

  // Return 0 if a prediction (of length num_labels_) correctly gives the
  // ground truth label 'label'; 0 otherwise.
  int32_t ZeroOneLoss(const std::vector<float>& prediction, int32_t label)
    const;

  // Compute cross entropy loss of a prediction (of length num_labels_) and the
  // ground truth label 'label'.
  float CrossEntropyLoss(const std::vector<float>& prediction, int32_t label)
    const;

  //Write pending updates to PS for a specified set of rows
  void push(RowUpdateItem item, int epoch);

  // Write pending updates to PS
  void push();

  //Read fresh values from PS for a specified set of rows
  void pull(RowUpdateItem item, int epoch);

  //Read fresh values from PS
  void pull();

  // Save the current weight in cache in libsvm format.
  void SaveWeights(const std::string& filename) const;

  //Evaluate the regularization penalty of the parameter vector
  float EvaluateL2RegLoss() const;

private:    // private functions
  void RefreshParamsDense();
  void RefreshParamsSparse();

private:
  // ======== PS Tables ==========
  // The weight of each class (stored as single feature-major row).
  petuum::Table<float> w_table_;

  // Thread-cache.
  std::vector<petuum::ml::AbstractFeature<float>*> w_cache_;
  std::vector<petuum::ml::AbstractFeature<float>*> w_delta_;

  int32_t feature_dim_; // feature dimension
  int32_t num_labels_; // number of classes/labels
  int32_t w_dim_;       // dimension of w_table_ = feature_dim_ * num_labels_.
  std::vector<float> predict_buff_;

  // Specialization Functions
  std::function<float(const petuum::ml::AbstractFeature<float>&,
      const petuum::ml::AbstractFeature<float>&)> FeatureDotProductFun_;
  std::function<void(MLRSGDSolver&)> RefreshParamFun_;
};

}  // namespace mlr
