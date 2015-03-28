// Author: Dai Wei (wdai@cs.cmu.edu)
// Date: 2014.07.14

#pragma once

#include <gflags/gflags.h>
#include <cstdint>

// Globally accessible gflags.
DECLARE_int32(num_train_data);
DECLARE_string(train_file);
DECLARE_bool(global_data);
DECLARE_string(test_file);
DECLARE_int32(num_train_eval);
DECLARE_int32(num_test_eval);
DECLARE_bool(perform_test);
DECLARE_bool(use_weight_file);
DECLARE_string(weight_file);

DECLARE_int32(num_epochs);
DECLARE_int32(num_batches_per_epoch);
DECLARE_double(learning_rate);
DECLARE_bool(use_minibatch_lr);
DECLARE_double(decay_rate);
DECLARE_int32(num_epochs_per_eval);
DECLARE_bool(sparse_weight);
DECLARE_double(lambda);
DECLARE_bool(use_minibatch_lambda);

DECLARE_string(output_file_prefix);
DECLARE_int32(num_secs_per_checkpoint);
DECLARE_int32(w_table_num_cols);

const int32_t kWTableID = 0;
const int32_t kLossTableID = 1;

namespace mlr {

const int32_t kColIdxLossTableEpoch = 0;
const int32_t kColIdxLossTableBatch = 1;
const int32_t kColIdxLossTableZeroOneLoss = 2;    // training set
const int32_t kColIdxLossTableEntropyLoss = 3;    // training set
const int32_t kColIdxLossTableNumEvalTrain = 4;    // # train point eval
const int32_t kColIdxLossTableTestZeroOneLoss = 5;
const int32_t kColIdxLossTableNumEvalTest = 6;  // # test point eval.
const int32_t kColIdxLossTableTime = 7;
const int32_t kColIdxLossTableRegLoss = 8;    // reg loss (with lambda)

const int32_t kNumColumnsLossTable = 9;

}  // namespace mlr
