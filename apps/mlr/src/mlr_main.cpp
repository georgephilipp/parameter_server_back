// Author: Dai Wei (wdai@cs.cmu.edu)
// Date: 2014.10.04

#include "mlr_engine.hpp"
#include "common.hpp"
#include <petuum_ps_common/include/petuum_ps.hpp>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <thread>
#include <vector>
#include <cstdint>
#include "gstd/src/Rand.h"
#include "gstd/src/Writer.h"
#include <petuum_ps_common/include/system_gflags_declare.hpp>
#include <petuum_ps_common/include/table_gflags_declare.hpp>
#include <petuum_ps_common/include/init_table_config.hpp>
#include <petuum_ps_common/include/init_table_group_config.hpp>

#include "gstd/src/Printer.h"

using namespace msii810161816;

// Training
DEFINE_string(train_file, "", "The program expects 2 files: train_file, "
    "train_file.meta. If global_data = false, then it looks for train_file.X, "
    "train_file.X.meta, where X is the client_id.");
DEFINE_bool(global_data, false, "If true, all workers read from the same "
    "train_file. If false, append X. See train_file.");
DEFINE_bool(force_global_file_names, false, "If true, when then global_data is false, use global meta file");
DEFINE_int32(num_train_data, 0, "Number of training data. Cannot exceed the "
    "number of data in train_file. 0 to use all train data.");
DEFINE_int32(num_epochs, 1, "Number of data sweeps.");
DEFINE_int32(num_batches_per_epoch, 10, "Since we Clock() at the end of each batch, "
    "num_batches_per_epoch is effectively the number of clocks per epoch (iteration)");
// Model
DEFINE_double(lambda, 0.1, "L2 regularization parameter, only used for binary LR.");
DEFINE_double(learning_rate, 0.1, "Initial step size");
DEFINE_double(decay_rate, 1, "multiplicative decay");
DEFINE_bool(sparse_weight, false, "Use sparse feature for model parameters");
DEFINE_bool(add_immediately, false, "Add computed updates to the feature vector immediately");
//Testing
DEFINE_string(test_file, "", "The program expects 2 files: test_file, "
    "test_file.meta, test_file must have format specified in read_format "
    "flag. All clients read test file if FLAGS_perform_test == true.");
DEFINE_bool(perform_test, false, "Ignore test_file if true.");
DEFINE_int32(num_epochs_per_eval, 10, "Number of batches per evaluation");
DEFINE_int32(num_test_data, 0, "Number of test data. Cannot exceed the "
    "number of data in test_file. 0 to use all test data.");
DEFINE_int32(num_train_eval, 20, "Use the next num_train_eval train data "
    "(per thread) for intermediate eval.");
DEFINE_int32(num_test_eval, 20, "Use the first num_test_eval test data for "
    "intermediate eval. 0 for using all. The final eval will always use all "
    "test data.");
//checkpoint/restart
DEFINE_bool(use_weight_file, false, "True to use init_weight_file as init");
DEFINE_string(weight_file, "", "Use this file to initialize weight. "
  "Format of the file is libsvm (see SaveWeight in MLRSGDSolver).");
DEFINE_int32(num_secs_per_checkpoint, 600, "# of seconds between each saving to disk");
// table
DEFINE_int32(w_table_num_cols, 100, "# of columns in w_table. Only used for binary LR.");
// output
DEFINE_string(output_file_prefix, "", "Results go here.");
DEFINE_string(signal_file_path, "", "Path to the file used to signal completion");


const int32_t kDenseRowFloatTypeID = 0;
//const int32_t kSparseFeatureRowFloatTypeID = 1;

int main(int argc, char *argv[]) {
  //initialize parms and data
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  CHECK(!FLAGS_sparse_weight) << "Not yet supported!";
  mlr::MLREngine mlr_engine;
  //STATS_APP_LOAD_DATA_BEGIN();
  mlr_engine.ReadData();
  //STATS_APP_LOAD_DATA_END();
  int32_t feature_dim = mlr_engine.GetFeatureDim();
  int32_t num_labels = mlr_engine.GetNumLabels();

  //print parms
  if(false)
  {
	  LOG(INFO) << "train_file: " << FLAGS_train_file << "\n";
	  LOG(INFO) << "global_data: " << (FLAGS_global_data? "true" : "false") << "\n";
	  LOG(INFO) << "num_train_data: " << FLAGS_num_train_data << "\n";
	  LOG(INFO) << "num_epochs: " << FLAGS_num_epochs << "\n";
	  LOG(INFO) << "num_batches_per_epoch: " << FLAGS_num_batches_per_epoch << "\n";
	  LOG(INFO) << "lambda: " << FLAGS_lambda << "\n";
	  LOG(INFO) << "learning_rate: " << FLAGS_learning_rate << "\n";  
	  LOG(INFO) << "decay_rate: " << FLAGS_decay_rate << "\n";
	  LOG(INFO) << "sparse_weight: " << (FLAGS_sparse_weight? "true" : "false") << "\n";
	  LOG(INFO) << "test_file: " << FLAGS_test_file << "\n";
	  LOG(INFO) << "perform_test: " << (FLAGS_perform_test? "true" : "false") << "\n";
	  LOG(INFO) << "num_epochs_per_eval: " << FLAGS_num_epochs_per_eval << "\n";
	  LOG(INFO) << "num_train_eval: " << FLAGS_num_train_eval << "\n";
	  LOG(INFO) << "num_test_eval: " << FLAGS_num_test_eval << "\n";  
	  LOG(INFO) << "use_weight_file: " << (FLAGS_use_weight_file? "true" : "false") << "\n";
	  LOG(INFO) << "weight_file: " << FLAGS_weight_file << "\n";
	  LOG(INFO) << "num_secs_per_checkpoint: " << FLAGS_num_secs_per_checkpoint << "\n";
	  LOG(INFO) << "w_table_num_cols: " << FLAGS_w_table_num_cols << "\n";
	  LOG(INFO) << "consistency_model: " << FLAGS_consistency_model << "\n";
	  LOG(INFO) << "num_table_threads: " << FLAGS_num_table_threads << "\n";  
	  LOG(INFO) << "num_comm_channels_per_client: " << FLAGS_num_comm_channels_per_client << "\n";
	  LOG(INFO) << "table_staleness: " << FLAGS_table_staleness << "\n";
	  LOG(INFO) << "client_bandwidth_mbps: " << FLAGS_client_bandwidth_mbps << "\n";
	  LOG(INFO) << "server_bandwidth_mbps: " << FLAGS_server_bandwidth_mbps << "\n";
  	  LOG(INFO) << "stats_path: " << FLAGS_stats_path << "\n";
    	  LOG(INFO) << "num_clients: " << FLAGS_num_clients << "\n";
  	  LOG(INFO) << "client_id: " << FLAGS_client_id << "\n";
	  LOG(INFO) << "hostfile: " << FLAGS_hostfile << "\n";
          LOG(INFO) << "feature_dim: " << feature_dim << "\n";
	  LOG(INFO) << "num_labels: " << num_labels << "\n";
          return 0;
  }

  //initialize petuum
  //table group
  petuum::PSTableGroup::RegisterRow<petuum::DenseRow<float> >
    (kDenseRowFloatTypeID);
  petuum::TableGroupConfig table_group_config;
  petuum::InitTableGroupConfig(&table_group_config, 2);
  petuum::PSTableGroup::Init(table_group_config, false);
  // weight table : Treat binary LR as special case.
  petuum::ClientTableConfig table_config;
  petuum::InitTableConfig(&table_config);
  table_config.table_info.row_type = kDenseRowFloatTypeID;
  table_config.table_info.row_capacity = (num_labels == 2) ? FLAGS_w_table_num_cols : feature_dim;
  table_config.table_info.dense_row_oplog_capacity = (num_labels == 2) ? FLAGS_w_table_num_cols : feature_dim;
  table_config.process_cache_capacity = (num_labels == 2) ? std::ceil(static_cast<float>(feature_dim) / FLAGS_w_table_num_cols) : num_labels;
  LOG(INFO) << "feature dim = " << feature_dim;
  LOG(INFO) << "process cache capacity = " << table_config.process_cache_capacity;
  LOG(INFO) << "num_batches_per_epoch = " << FLAGS_num_batches_per_epoch;
  table_config.oplog_capacity = table_config.process_cache_capacity;
  petuum::PSTableGroup::CreateTable(kWTableID, table_config);
  // loss table.
  table_config.process_storage_type = petuum::BoundedSparse;
  table_config.table_info.row_type = kDenseRowFloatTypeID;
  table_config.table_info.row_capacity = mlr::kNumColumnsLossTable;
  table_config.table_info.dense_row_oplog_capacity = mlr::kNumColumnsLossTable;
  table_config.process_cache_capacity = 1000;
  table_config.oplog_capacity = table_config.process_cache_capacity;
  petuum::PSTableGroup::CreateTable(kLossTableID, table_config);
  //done
  petuum::PSTableGroup::CreateTableDone();

  //execute mlr
  LOG(INFO) << "Starting MLR with " << FLAGS_num_table_threads << " threads "
    << "on client " << FLAGS_client_id;
  std::vector<std::thread> threads(FLAGS_num_table_threads);
  for (auto& thr : threads) {
    thr = std::thread(&mlr::MLREngine::Start, std::ref(mlr_engine));
  }
  for (auto& thr : threads) {
    thr.join();
  }
  petuum::PSTableGroup::ShutDown();
  if(FLAGS_signal_file_path != "" && FLAGS_client_id == 0)
    gstd::Writer::w(FLAGS_signal_file_path, "done", true);
  LOG(INFO) << "MLR finished and shut down!";


  return 0;
}
