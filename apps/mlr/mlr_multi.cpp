// Author: Dai Wei (wdai@cs.cmu.edu)
// Date: 2014.10.04

#include <petuum_ps_common/include/petuum_ps.hpp>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <thread>
#include <vector>
#include <map>
#include <cstdint>
#include "gstd/src/Rand.h"
#include "gstd/src/Reader.h"
#include "gstd/src/File.h"
#include "gstd/src/Writer.h"
#include "gstd/src/Printer.h"
#include "gstd/src/TerminalMgr.h"
#include "gstd/src/Map.h"
#include <petuum_ps_common/include/system_gflags_declare.hpp>
#include <petuum_ps_common/include/table_gflags_declare.hpp>
#include <petuum_ps_common/include/init_table_config.hpp>
#include <petuum_ps_common/include/init_table_group_config.hpp>



using namespace msii810161816;

bool tableIsCreated = false;


DECLARE_int32(num_train_data);
DECLARE_string(train_file);
DECLARE_bool(global_data);
DECLARE_bool(force_global_file_names);
DECLARE_string(test_file);
DECLARE_int32(num_test_data);
DECLARE_string(out_cols);
DECLARE_int32(num_train_eval);
DECLARE_int32(num_test_eval);
DECLARE_bool(perform_test);
DECLARE_bool(use_weight_file);
DECLARE_string(weight_file);
DECLARE_int32(num_epochs);
DECLARE_int32(num_batches_per_epoch);
DECLARE_int32(num_batches_per_clock);
DECLARE_string(parm_file);
DECLARE_double(learning_rate);
DECLARE_double(decay_rate);
DECLARE_int32(num_epochs_per_eval);
DECLARE_bool(sparse_weight);
DECLARE_double(lambda);
DECLARE_bool(add_immediately);
DECLARE_string(output_file_prefix);
DECLARE_int32(num_secs_per_checkpoint);
DECLARE_int32(w_table_num_cols);
DECLARE_string(prog_path);
DECLARE_string(system_path);

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
DEFINE_int32(num_batches_per_clock, 1000000000, "doesn't do anything yet");
// Model
DEFINE_string(parm_file, "", "Input parameter file");
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
DEFINE_string(out_cols, "", "List of outpug values to write to the output table");
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
// program
DEFINE_string(prog_path, "", "path to the program executable");
DEFINE_string(system_path, "", "path to the directory where to hold shenanigans");

const int32_t kDenseRowFloatTypeID = 0;
//const int32_t kSparseFeatureRowFloatTypeID = 1;

class ParmStruct
{
public:
	int32_t num_train_data;
	uint64_t client_bandwidth_mbps;
	uint64_t server_bandwidth_mbps;
	int32_t table_staleness;
	std::string consistency_model;
	bool add_immediately;
	int32_t num_batches_per_clock;
	int32_t num_epochs;
	double lambda;
	double learning_rate;
	double decay_rate;
    
    ParmStruct()
    {
        num_train_data = FLAGS_num_train_data;
        client_bandwidth_mbps = FLAGS_client_bandwidth_mbps;
        server_bandwidth_mbps = FLAGS_server_bandwidth_mbps;
        table_staleness = FLAGS_table_staleness;
        consistency_model = FLAGS_consistency_model;
	add_immediately = FLAGS_add_immediately;
        num_batches_per_clock = FLAGS_num_batches_per_clock;
	num_epochs = FLAGS_num_epochs;
        lambda = FLAGS_lambda;
        learning_rate = FLAGS_learning_rate;
        decay_rate = FLAGS_decay_rate;
	//cached for later
	output_file_prefix = FLAGS_output_file_prefix;
    }      
        
    void set()
    {
        FLAGS_num_train_data = num_train_data;
        FLAGS_client_bandwidth_mbps = client_bandwidth_mbps;
        FLAGS_server_bandwidth_mbps = server_bandwidth_mbps;
        FLAGS_table_staleness = table_staleness;
        FLAGS_consistency_model = consistency_model;
	FLAGS_add_immediately = add_immediately;
        FLAGS_num_batches_per_clock = num_batches_per_clock;
	FLAGS_num_epochs = num_epochs;
        FLAGS_lambda = lambda;
        FLAGS_learning_rate = learning_rate;
        FLAGS_decay_rate = decay_rate;

        std::stringstream outSuffix;
        
        outSuffix << ".NTD" << num_train_data;
        outSuffix << ".CB" << client_bandwidth_mbps;
        outSuffix << ".SB" << server_bandwidth_mbps;
        outSuffix << ".TS" << table_staleness;
        outSuffix << ".CM" << consistency_model;
        outSuffix << ".AI" << add_immediately;
        outSuffix << ".BPC" << num_batches_per_clock;
        outSuffix << ".NE" << num_epochs;
        outSuffix << ".L" << lambda;
        outSuffix << ".MU" << learning_rate;
        outSuffix << ".MUD" << decay_rate;

	FLAGS_output_file_prefix = output_file_prefix + outSuffix.str();
	FLAGS_stats_path = FLAGS_output_file_prefix + ".stats.yaml";
    }
    
    void consume(std::string argname, std::string argval)
    {
	if(argname == "num_train_data")
	{
		num_train_data = std::stoi(argval);
	}
	else if(argname == "client_bandwidth_mbps")
	{
		client_bandwidth_mbps = (uint64_t)std::stoi(argval);
	}
	else if(argname == "server_bandwidth_mbps")
	{
		server_bandwidth_mbps = (uint64_t)std::stoi(argval);
	}
	else if(argname == "table_staleness")
	{
		table_staleness = std::stoi(argval);
	}
	else if(argname == "consistency_model")
	{
		consistency_model = argval;
	}
	else if(argname == "add_immediately")
	{
		add_immediately = (argval == "1");
	}
	else if(argname == "num_batches_per_clock")
	{
		num_batches_per_clock = std::stoi(argval);
	}
	else if(argname == "num_epochs")
	{
		num_epochs = std::stoi(argval);
	}
	else if(argname == "lambda")
	{
		lambda = std::stod(argval);
	}
	else if(argname == "learning_rate")
	{
		learning_rate = std::stod(argval);
	}
	else if(argname == "decay_rate")
	{
		decay_rate = std::stod(argval);
	}
	else
	{
		gstd::error("unknown parm");
	}
    }

	std::string get_output_file_prefix()
	{
		return output_file_prefix;
	}

	std::vector<std::string> getHeader()
	{
		std::vector<std::string> res;
		res.push_back("num_train_data");
		res.push_back("client_bandwidth_mbps");
		res.push_back("server_bandwidth_mbps");
		res.push_back("table_staleness");
		res.push_back("consistency_model");
		res.push_back("add_immediately");
		res.push_back("num_batches_per_clock");
		res.push_back("num_epochs");
		res.push_back("lambda");
		res.push_back("learning_rate");
		res.push_back("decay_rate");
		return res;
	}

	std::vector<std::string> getRow()
	{
		std::vector<std::string> res;
		res.push_back(std::to_string(num_train_data));
		res.push_back(std::to_string(client_bandwidth_mbps));
		res.push_back(std::to_string(server_bandwidth_mbps));
		res.push_back(std::to_string(table_staleness));
		res.push_back(consistency_model);
		res.push_back(std::to_string(add_immediately));
		res.push_back(std::to_string(num_batches_per_clock));
		res.push_back(std::to_string(num_epochs));
		res.push_back(std::to_string(lambda));
		res.push_back(std::to_string(learning_rate));
		res.push_back(std::to_string(decay_rate));
		return res;
	}

private:
	std::string output_file_prefix;

};


std::map<std::string,std::string> readOutFiles(std::function<bool(std::vector<std::string>)> stoppageCriterion)
{
	std::map<std::string, std::string> res;

	std::vector<std::vector<std::string> > outFile = gstd::Reader::rs<std::string>(FLAGS_output_file_prefix + ".loss", ' ');
	std::vector<std::string> targetTableHeader = {"Epoch", "Batch", "Train-0-1", "Train-Entropy", "Train-obj", "Num-Train-Used", "Test-0-1", "Num-Test-Used", "Time"};
	int targetHeaderRow = (int)outFile.size() - 2 - FLAGS_num_epochs;
	gstd::check(outFile[targetHeaderRow] == targetTableHeader, "outfile reading failed on header row. header row found was " + gstd::Printer::vp(outFile[targetHeaderRow]));
	int stoppageRowInd = 0;	
	for(stoppageRowInd=targetHeaderRow+1;stoppageRowInd<(int)outFile.size() - 1;stoppageRowInd++)
	{
		if(stoppageCriterion(outFile[stoppageRowInd]))
		{
			break;
		}
	}
	std::vector<std::string> stoppageRow = outFile[stoppageRowInd];
	gstd::check(stoppageRow.size() == targetTableHeader.size(), "incorrect size of stoppage row. StoppageRowInd was" + std::to_string(stoppageRowInd) + " Stoppage row was " + gstd::Printer::vp<std::string>(stoppageRow));
	res["epochs"] = stoppageRow[0];
	res["train01"] = stoppageRow[2];
	res["trainEntropy"] = stoppageRow[3];
	res["trainObj"] = stoppageRow[4];
	res["test01"] = stoppageRow[6];
	res["time"] = stoppageRow[8];

	double waitPercentage = 0;

	for(int i=0; i<FLAGS_num_clients;i++)
	{
		std::vector<std::string> statsFile = gstd::Reader::ls(FLAGS_output_file_prefix + ".stats.yaml." + std::to_string(i));
		for(int j=0;j<(int)statsFile.size();j++)
		{
			std::string argname = "app_sum_accum_comm_block_sec_percent";
			if(statsFile[j].substr(0, argname.size()) == argname)
			{
				std::vector<std::string> temp = gstd::Parser::vector<std::string>(statsFile[j], ' ');
				waitPercentage += std::stod(temp[1]) / ((double)FLAGS_num_clients);
			}
		}
	}

	res["waitPercentage"] = std::to_string(waitPercentage);
	return res;
}

std::string printBool(bool val)
{
	if(val)
		return "true";
	else
		return "false";
}

std::string printInt(int val)
{
	return std::to_string(val);
}

std::string printDouble(double val)
{
	return std::to_string(val);
}

void run()
{
	gstd::TerminalMgr mgr;
	mgr.internalPath = FLAGS_system_path;
	mgr.externalPath = FLAGS_system_path;
	mgr.outFileNameInternal = gstd::TerminalMgr::defaultOutFileName;
	mgr.outFileNameExternal = gstd::TerminalMgr::defaultOutFileName;
	mgr.completeFileNameInternal = gstd::TerminalMgr::defaultCompleteFileName;
	mgr.commandFileNameInternal = gstd::TerminalMgr::defaultCommandFileName;
	mgr.commandFileNameExternal = "./" + gstd::TerminalMgr::defaultCommandFileName;

	mgr.command = 
"#!/bin/bash -u\n"
"\n"
"\n"
"#bash file inputs\n"
"ssh_options=\"-oStrictHostKeyChecking=no -oUserKnownHostsFile=/dev/null -oLogLevel=quiet\"\n"
"\n"
"#app-specific inputs\n"
"#training\n"
"train_file=" + FLAGS_train_file + "\n"
"global_data=" + printBool(FLAGS_global_data) + "\n"
"force_global_file_names=" + printBool(FLAGS_force_global_file_names) + "\n"
"num_train_data=" + printInt(FLAGS_num_train_data) + "\n"
"num_epochs=" + printInt(FLAGS_num_epochs) + "\n"
"num_batches_per_epoch=" + printInt(FLAGS_num_batches_per_epoch) + "\n"
"#model\n"
"lambda=" + printDouble(FLAGS_lambda) + "\n"
"learning_rate=" + printDouble(FLAGS_learning_rate) + "\n"
"decay_rate=" + printDouble(FLAGS_decay_rate) + "\n"
"sparse_weight=" + printBool(FLAGS_sparse_weight) + "\n"
"add_immediately=" + printBool(FLAGS_add_immediately) + "\n"
"#testing\n"
"test_file=" + FLAGS_test_file + "\n"
"perform_test=" + printBool(FLAGS_perform_test) + "\n"
"num_epochs_per_eval=" + printInt(FLAGS_num_epochs_per_eval) + "\n"
"num_test_data=" + printInt(FLAGS_num_test_data) + "\n"
"num_train_eval=" + printInt(FLAGS_num_train_eval) + "\n"
"num_test_eval=" + printInt(FLAGS_num_test_eval) + "\n"
"#checkpoint/restart\n"
"use_weight_file=" + printBool(FLAGS_use_weight_file) + "\n"
"weight_file=" + FLAGS_weight_file + "\n"
"num_secs_per_checkpoint=" + printInt(FLAGS_num_secs_per_checkpoint) + "\n"
"#table\n"
"w_table_num_cols=" + printInt(FLAGS_w_table_num_cols) + "\n"
"\n"
"#Extra files\n"
"output_file_prefix=\"" + FLAGS_output_file_prefix + "\"\n"
"log_dir=\"" + FLAGS_log_dir + "\"\n"
"stats_path=\"" + FLAGS_stats_path + "\"\n"
"signal_file_path=\"" + FLAGS_system_path + gstd::TerminalMgr::defaultCompleteFileName + "\"\n"
"\n"
"# System parameters:\n"
"host_file=\"" + FLAGS_hostfile + "\"\n"
"consistency_model=\"" + FLAGS_consistency_model + "\"\n"
"num_table_threads=" + printInt(FLAGS_num_table_threads) + "\n"
"num_comm_channels_per_client=" + printInt(FLAGS_num_comm_channels_per_client) + "\n"
"table_staleness=" + printInt(FLAGS_table_staleness) + "\n"
"\n"
"#Obscure System Parms:\n"
"bg_idle_milli=" + std::to_string(FLAGS_bg_idle_milli) + "\n"
"# Total bandwidth: bandwidth_mbps * num_comm_channels_per_client * 2\n"
"client_bandwidth_mbps=" + std::to_string(FLAGS_client_bandwidth_mbps) + "\n"
"server_bandwidth_mbps=" + std::to_string(FLAGS_server_bandwidth_mbps) + "\n"
"# bandwidth / oplog_push_upper_bound should be > miliseconds.\n"
"thread_oplog_batch_size=" + std::to_string(FLAGS_thread_oplog_batch_size) + "\n"
"server_idle_milli=" + std::to_string(FLAGS_server_idle_milli) + "\n"
"update_sort_policy=" + FLAGS_update_sort_policy + "\n"
"row_candidate_factor=" + std::to_string(FLAGS_row_candidate_factor) + "\n"
"append_only_buffer_capacity=" + std::to_string(FLAGS_append_only_buffer_capacity) + "\n"
"append_only_buffer_pool_size=" + std::to_string(FLAGS_append_only_buffer_pool_size) + "\n"
"bg_apply_append_oplog_freq=" + std::to_string(FLAGS_bg_apply_append_oplog_freq) + "\n"
"client_send_oplog_upper_bound=" + std::to_string(FLAGS_client_send_oplog_upper_bound) + "\n"
"server_push_row_upper_bound=" + std::to_string(FLAGS_server_push_row_upper_bound) + "\n"
"row_oplog_type=" + std::to_string(FLAGS_row_oplog_type) + "\n"
"oplog_type=" + FLAGS_oplog_type + "\n"
"process_storage_type=" + FLAGS_process_storage_type + "\n"
"no_oplog_replay=" + printBool(FLAGS_no_oplog_replay) + "\n"
"numa_opt=" + printBool(FLAGS_numa_opt) + "\n"
"numa_policy=" + FLAGS_numa_policy + "\n"
"naive_table_oplog_meta=" + printBool(FLAGS_naive_table_oplog_meta) + "\n"
"suppression_on=" + printBool(FLAGS_suppression_on) + "\n"
"use_approx_sort=" + printBool(FLAGS_use_approx_sort) + "\n"
"\n"
"# Figure out the paths.\n"
"script_path=`readlink -f $0`\n"
"script_dir=`dirname $script_path`\n"
"app_dir=`dirname $script_dir`\n"
"progname=mlr_main\n"
"prog_path=$app_dir/mlr/${progname}\n"
"\n"
"# Parse hostfile\n"
"host_list=`cat $host_file | awk '{ print $2 }'`\n"
"unique_host_list=`cat $host_file | awk '{ print $2 }' | uniq`\n"
"num_unique_hosts=`cat $host_file | awk '{ print $2 }' | uniq | wc -l`\n"
"host_list=`cat $host_file | awk '{ print $2 }'`\n"
"num_hosts=`cat $host_file | awk '{ print $2 }' | wc -l`\n"
"\n"
"#derived\n"
"if $add_immediately\n"
"then\n"
"  echo 'adjusting batches per epoch'\n"
"  let num_batches_per_epoch=num_train_data/num_table_threads\n"
"  let num_batches_per_epoch=num_batches_per_epoch/num_hosts\n"
"  let num_batches_per_epoch=num_batches_per_epoch/25\n"
"  echo 'Here is comes'\n"
"  echo $num_batches_per_epoch\n"
"fi\n"
"\n"
"# Kill previous instances of this program\n"
"echo \"Killing previous instances of '$progname' on servers, please wait...\"\n"
"for ip in $unique_host_list; do\n"
"    echo \"killing \".$ip\n"
"  ssh $ssh_options $ip \\\n"
"    killall -q $progname\n"
"done\n"
"echo \"All done!\"\n"
"# exit\n"
"\n"
"# Spawn program instances\n"
"client_id=0\n"
"for ip in $host_list; do\n"
"  echo Running client $client_id on $ip\n"
"  log_path=${log_dir}.${client_id}\n"
"\n"
"  numa_index=$(( client_id%num_unique_hosts ))\n"
"\n"
"  cmd=\"rm -rf ${log_path}; mkdir -p ${log_path}; \\\n"
"GLOG_logtostderr=true \\\n"
"    GLOG_log_dir=$log_path \\\n"
"      GLOG_v=-1 \\\n"
"      GLOG_minloglevel=0 \\\n"
"      GLOG_vmodule=\"\" \\\n"
"      $prog_path \\\n"
"    --stats_path ${stats_path}\\\n"
"    --signal_file_path ${signal_file_path}\\\n"
"    --num_clients $num_hosts \\\n"
"    --num_comm_channels_per_client $num_comm_channels_per_client \\\n"
"    --init_thread_access_table=false \\\n"
"    --num_table_threads ${num_table_threads} \\\n"
"    --client_id $client_id \\\n"
"    --hostfile ${host_file} \\\n"
"    --consistency_model $consistency_model \\\n"
"    --client_bandwidth_mbps $client_bandwidth_mbps \\\n"
"    --server_bandwidth_mbps $server_bandwidth_mbps \\\n"
"    --bg_idle_milli $bg_idle_milli \\\n"
"    --thread_oplog_batch_size $thread_oplog_batch_size \\\n"
"    --row_candidate_factor ${row_candidate_factor} \\\n"
"    --server_idle_milli $server_idle_milli \\\n"
"    --update_sort_policy $update_sort_policy \\\n"
"    --numa_opt=${numa_opt} \\\n"
"    --numa_index ${numa_index} \\\n"
"    --numa_policy ${numa_policy} \\\n"
"    --naive_table_oplog_meta=${naive_table_oplog_meta} \\\n"
"    --suppression_on=${suppression_on} \\\n"
"    --use_approx_sort=${use_approx_sort} \\\n"
"    --table_staleness $table_staleness \\\n"
"    --row_type 0 \\\n"
"    --row_oplog_type ${row_oplog_type} \\\n"
"    --oplog_dense_serialized \\\n"
"    --oplog_type ${oplog_type} \\\n"
"    --append_only_oplog_type DenseBatchInc \\\n"
"    --append_only_buffer_capacity ${append_only_buffer_capacity} \\\n"
"    --append_only_buffer_pool_size ${append_only_buffer_pool_size} \\\n"
"    --bg_apply_append_oplog_freq ${bg_apply_append_oplog_freq} \\\n"
"    --process_storage_type ${process_storage_type} \\\n"
"    --no_oplog_replay=${no_oplog_replay} \\\n"
"    --client_send_oplog_upper_bound ${client_send_oplog_upper_bound} \\\n"
"    --server_push_row_upper_bound ${server_push_row_upper_bound} \\\n"
"    --train_file=$train_file \\\n"
"    --global_data=$global_data \\\n"
"    --force_global_file_names=$force_global_file_names \\\n"
"    --num_train_data=$num_train_data \\\n"
"    --num_epochs=$num_epochs \\\n"
"    --num_batches_per_epoch=$num_batches_per_epoch \\\n"
"    --lambda=$lambda \\\n"
"    --learning_rate=$learning_rate \\\n"
"    --decay_rate=$decay_rate \\\n"
"    --sparse_weight=${sparse_weight}\\\n"
"    --add_immediately=${add_immediately} \\\n"
"    --test_file=$test_file \\\n"
"    --perform_test=$perform_test \\\n"
"    --num_epochs_per_eval=$num_epochs_per_eval \\\n"
"    --num_test_data=$num_test_data \\\n"
"    --num_train_eval=$num_train_eval \\\n"
"    --num_test_eval=$num_test_eval \\\n"
"    --use_weight_file=$use_weight_file \\\n"
"    --weight_file=$weight_file \\\n"
"    --num_secs_per_checkpoint=${num_secs_per_checkpoint} \\\n"
"    --w_table_num_cols=$w_table_num_cols \\\n"
"    --output_file_prefix=$output_file_prefix\"\n"
"\n"
"  ssh $ssh_options $ip $cmd &\n"
"  #eval $cmd  # Use this to run locally (on one machine).\n"
"  #echo $cmd   # echo the cmd for just the first machine.\n"
"  #exit\n"
"\n"
"  # Wait a few seconds for the name node (client 0) to set up\n"
"  if [ $client_id -eq 0 ]; then\n"
"    #echo $cmd   # echo the cmd for just the first machine.\n"
"    echo \"Waiting for name node to set up...\"\n"
"    sleep 3\n"
"  fi\n"
"  client_id=$(( client_id+1 ))\n"
"done";
	gstd::check(mgr.run().success, "Terminal run failed");
}


int main(int argc, char *argv[]) 
{
	std::function<bool(std::vector<std::string>)> defaultStoppageCriterion = [](std::vector<std::string> inRow)
	{
		return false;
	};

    //initialize parms and data
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    CHECK(!FLAGS_sparse_weight) << "Not yet supported!";    
    gstd::check(!gstd::file::exists({FLAGS_output_file_prefix})[0], "outfile path exists");
    std::vector<std::string> outFields;
    std::vector<std::vector<std::string> > parmContent;
	if(FLAGS_parm_file != "")
		parmContent = gstd::Reader::rs<std::string>(FLAGS_parm_file, ' ');
    else
        parmContent.push_back(std::vector<std::string>());
    int parmContentRowSize = (int)parmContent[0].size();
    ParmStruct ps;
    std::vector<std::string> outCols = gstd::Parser::vector<std::string>(FLAGS_out_cols, ':');
    std::vector<std::vector<std::string> > res;
    std::vector<std::string> resHeader = ps.getHeader();
    resHeader.insert(resHeader.end(), outCols.begin(), outCols.end());
    resHeader.push_back("num_test_data");
    resHeader.push_back("perform_test");
    res.push_back(resHeader);
    for(int i=1;i<(int)parmContent.size();i++)
    {
        gstd::check(parmContentRowSize == (int)parmContent[i].size(), "parms are not a table");
        for(int j=0;j<parmContentRowSize; j++)
            ps.consume(parmContent[0][j], parmContent[i][j]);
        ps.set();
	run();
        std::map<std::string, std::string> resFiles = readOutFiles(defaultStoppageCriterion);
        std::vector<std::string> resRow = ps.getRow();
        for(int j=0; j<(int)outCols.size(); j++)
            resRow.push_back(resFiles[outCols[j]]);
	resRow.push_back(std::to_string(FLAGS_num_test_data));
	resRow.push_back(std::to_string(FLAGS_perform_test));
        res.push_back(resRow);
    }
    if(FLAGS_client_id == 0)
        gstd::Writer::rs<std::string>(ps.get_output_file_prefix(), res, " ", true);
    
    return 0;
}































