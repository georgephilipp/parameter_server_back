// Author: Dai Wei (wdai@cs.cmu.edu)
// Date: 2015.02.04

#pragma once

#include <petuum_ps_common/include/petuum_ps.hpp>
#include <ml/include/ml.hpp>
#include <cstdint>
#include <vector>
#include "abstract_mlr_sgd_solver.hpp"
#include "updateScheduler.hpp"

namespace mlr {

struct LRSGDSolverConfig {
  int32_t feature_dim;

  petuum::Table<float> w_table;
  int32_t w_table_num_cols;
  bool sparse_data = false;

  float lambda = 0;   // l2 regularization parameter
};

// Binary solver. Does not support sparse LR parameters. Labels y \ in {0, 1}.
// NOT {-1, 1}
class LRSGDSolver : public AbstractMLRSGDSolver {
public:
  LRSGDSolver(const LRSGDSolverConfig& config);
  ~LRSGDSolver();

  // Compute gradient using feature and label and store internally.
  void SingleDataSGD(const petuum::ml::AbstractFeature<float>& feature,
      int32_t label, float learning_rate);

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

  // Write pending updates to PS for a specified set of rows
  void push(RowUpdateItem item);

  // Write pending updates to PS
  void push();

  //Read fresh values from PS for a specified set of rows
  void pull(RowUpdateItem item);

  //Read fresh values from PS
  void pull();

  // Save the current weight in cache in libsvm format.
  void SaveWeights(const std::string& filename) const;

  //Evaluate the regularization penalty of the parameter vector
  float EvaluateL2RegLoss() const;

private:
  // ======== PS Tables ==========
  // The weight of each class (stored as single feature-major row).
  petuum::Table<float> w_table_;

  // Thread-cache.
  petuum::ml::DenseFeature<float> w_cache_;
  petuum::ml::DenseFeature<float> w_delta_;
  petuum::ml::DenseFeature<float> w_delta_other_;

  int32_t feature_dim_; // feature dimension
  // feature_dim % w_table_num_cols might not be 0
  int32_t w_table_num_cols_;  // # of cols in w_table.
  float lambda_;   // l2 regularization parameter
  std::vector<float> predict_buff_;

  // Specialization Functions
  std::function<float(const petuum::ml::AbstractFeature<float>&,
      const petuum::ml::AbstractFeature<float>&)> FeatureDotProductFun_;

  void pullRow(int index, bool isOffset);

  void pushRow(int index, bool isOffset);

  void pushOther(int index, bool isOffset);
};

}  // namespace mlr
