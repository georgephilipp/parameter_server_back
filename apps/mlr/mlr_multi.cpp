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
DECLARE_bool(ignore_nan);
DECLARE_string(parm_file);
DECLARE_double(learning_rate);
DECLARE_bool(learning_rate_search);
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
DEFINE_bool(ignore_nan, false, "if this is true, replace nan by 0");
// Model
DEFINE_string(parm_file, "", "Input parameter file");
DEFINE_double(lambda, 0.1, "L2 regularization parameter, only used for binary LR.");
DEFINE_double(learning_rate, 0.1, "Initial step size");
DEFINE_double(decay_rate, 1, "multiplicative decay");
DEFINE_bool(learning_rate_search, false, "if true, then learning rate is optimizes with binary interval search");
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



class IntervalSearchController
{
public:
    IntervalSearchController(
        double startTop, 
        double startBottom, 
        double startTopVal, 
        double startBottomVal, 
        double threshold_
    )
    {
        pointTop = startTop;
        pointBottom = startBottom;
        topVal = startTopVal;
        bottomVal = startBottomVal;
        next = "initMiddle";
        threshold = threshold_;
	searchLog.push_back({"startTop", gstd::Printer::p(pointTop), gstd::Printer::p(topVal)});
	searchLog.push_back({"startBottom", gstd::Printer::p(pointBottom), gstd::Printer::p(bottomVal)});
    }
        
    double get()
    {
        if( next == "initMiddle" )
            suggested = (pointTop + pointBottom) / 2;
        else if( next == "initBottom" )
            suggested = 3*pointCenter - 2*pointTop;
        else if( next == "initTop" )
            suggested = 3*pointCenter - 2*pointBottom;
        else if( next == "top" )
            suggested = (pointTop + pointCenter) / 2;
        else if(next == "bottom")
            suggested = (pointBottom + pointCenter) / 2;
        else
            gstd::error("unknown next value");
        return suggested;
    }
    
    bool consume( double val )
    {
        searchLog.push_back({next, std::to_string(suggested), std::to_string(val)});
        if( next == "initMiddle" )
        {
            if( val <= bottomVal && val <= topVal )
            {
                pointCenter = suggested;
                centerVal = val;
                next = "top";
            }
            else if( topVal >= bottomVal)
            {
                pointCenter = pointBottom;
                centerVal = bottomVal;
                pointTop = suggested;
                topVal = val;
                next = "initBottom";
            }
            else 
            {
                pointCenter = pointTop;
                centerVal = topVal;
                pointBottom = suggested;
                bottomVal = val;
                next = "initTop";
            }
        }
        else if( next == "initBottom" )
        {
            if( val >= centerVal )
            {
                pointBottom = suggested;
                bottomVal = val;
                next = "top";
            }
            else
            {
                pointTop = pointCenter;
                topVal = centerVal;
                pointCenter = suggested;
                centerVal = val;
            }
        }
        else if( next == "initTop" )
        {
            if( val >= centerVal )
            {
                pointTop = suggested;
                topVal = val;
                next = "top";
            }
            else
            {
                pointBottom = pointCenter;
                bottomVal = centerVal;
                pointCenter = suggested;
                centerVal = val;
            }
        }
        else if( next == "top" )
        {
            if(val >= centerVal)
            {
                pointTop = suggested;
                topVal = val;
            }
            else
            {
                pointBottom = pointCenter;
                bottomVal = centerVal;
                pointCenter = suggested;
                centerVal = val;
            }
            next = "bottom";
        }
        else if(next == "bottom")
        {
            if(val >= centerVal)
            {
                pointBottom = suggested;
                bottomVal = val;
            }
            else
            {
                pointTop = pointCenter;
                topVal = centerVal;
                pointCenter = suggested;
                centerVal = val;
            }
            next = "top";
        }
        else 
            gstd::error("unknown next value");
            
        if((next == "top" || next == "bottom") && pointTop - pointCenter <= threshold && pointCenter - pointBottom <= threshold)
            return true;
        else
            return false;
    }
    
    double getPointCenter()
    {
        return pointCenter;
    }
    
    double getCenterVal()
    {
        return centerVal;
    }

    std::vector<std::vector<std::string> > getLog()
    {
	return searchLog;
    }
    
private:
    double pointTop;
    double pointCenter;
    double pointBottom;
    double topVal;
    double centerVal;
    double bottomVal;
    double suggested;
    std::string next;
    double threshold;
    std::vector<std::vector<std::string> > searchLog;
};


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
	int32_t num_batches_per_epoch;
    
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
	num_batches_per_epoch = FLAGS_num_batches_per_epoch;
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
	FLAGS_num_batches_per_epoch = num_batches_per_epoch;

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
	outSuffix << ".BPE" << num_batches_per_epoch;

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
	else if(argname == "num_batches_per_epoch")
	{
		num_batches_per_epoch = std::stoi(argval);
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
		res.push_back("num_batches_per_epoch");
		return res;
	}

	std::vector<std::string> getRow()
	{
		std::vector<std::string> res;
		res.push_back(gstd::Printer::p(num_train_data));
		res.push_back(gstd::Printer::p(client_bandwidth_mbps));
		res.push_back(gstd::Printer::p(server_bandwidth_mbps));
		res.push_back(gstd::Printer::p(table_staleness));
		res.push_back(consistency_model);
		res.push_back(gstd::Printer::p(add_immediately));
		res.push_back(gstd::Printer::p(num_batches_per_clock));
		res.push_back(gstd::Printer::p(num_epochs));
		res.push_back(gstd::Printer::p(lambda));
		res.push_back(gstd::Printer::p(learning_rate));
		res.push_back(gstd::Printer::p(decay_rate));
		res.push_back(gstd::Printer::p(num_batches_per_epoch));
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
	if(!FLAGS_perform_test)
		targetTableHeader = {"Epoch", "Batch", "Train-0-1", "Train-Entropy", "Train-obj", "Num-Train-Used", "Time"};
	int targetHeaderRow = 0;
	for(targetHeaderRow = 0; targetHeaderRow<(int)outFile.size(); targetHeaderRow++)
	{
		if(outFile[targetHeaderRow] == targetTableHeader)
			break;
	}
	gstd::check(targetHeaderRow < (int)outFile.size(), "did not find headerrow in outfile. Target table header was:" + gstd::Printer::vp(targetTableHeader));
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
	gstd::check(stoppageRow.size() == targetTableHeader.size(), "incorrect size of stoppage row. StoppageRowInd was" + gstd::Printer::p(stoppageRowInd) + " Stoppage row was " + gstd::Printer::vp<std::string>(stoppageRow) + " size of stoppage row was " + gstd::Printer::p(stoppageRow.size()) + " size of header was " + gstd::Printer::p(targetTableHeader.size()));
	res["epochs"] = stoppageRow[0];
	res["train01"] = stoppageRow[2];
	res["trainEntropy"] = stoppageRow[3];
	res["trainObj"] = stoppageRow[4];
	if(FLAGS_perform_test)
	{
		res["test01"] = stoppageRow[6];
		res["time"] = stoppageRow[8];
	}
	else
	{
		res["time"] = stoppageRow[6];
		res["test01"] = "-1";
	}

	double waitPercentage = 0;

	for(int i=0; i<FLAGS_num_clients;i++)
	{
		std::vector<std::string> statsFile = gstd::Reader::ls(FLAGS_output_file_prefix + ".stats.yaml." + gstd::Printer::p(i));
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

	res["waitPercentage"] = gstd::Printer::p(waitPercentage);
	return res;
}

std::vector<std::map<std::string,std::string> > readOutFilesComplete()
{
	std::vector<std::map<std::string, std::string> > res;

	std::vector<std::vector<std::string> > outFile = gstd::Reader::rs<std::string>(FLAGS_output_file_prefix + ".loss", ' ');
	std::vector<std::string> targetTableHeader = {"Epoch", "Batch", "Train-0-1", "Train-Entropy", "Train-obj", "Num-Train-Used", "Test-0-1", "Num-Test-Used", "Time"};
	if(!FLAGS_perform_test)
		targetTableHeader = {"Epoch", "Batch", "Train-0-1", "Train-Entropy", "Train-obj", "Num-Train-Used", "Time"};
	int targetHeaderRow = 0;
	for(targetHeaderRow = 0; targetHeaderRow<(int)outFile.size(); targetHeaderRow++)
	{
		if(outFile[targetHeaderRow] == targetTableHeader)
			break;
	}
	gstd::check(targetHeaderRow < (int)outFile.size(), "did not find headerrow in outfile. Target table header was:" + gstd::Printer::vp(targetTableHeader));
	gstd::check(outFile[targetHeaderRow] == targetTableHeader, "outfile reading failed on header row. header row found was " + gstd::Printer::vp(outFile[targetHeaderRow]));
	
	double waitPercentage = 0;

	for(int i=0; i<FLAGS_num_clients;i++)
	{
		std::vector<std::string> statsFile = gstd::Reader::ls(FLAGS_output_file_prefix + ".stats.yaml." + gstd::Printer::p(i));
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

        for(int i=targetHeaderRow+1;i<(int)outFile.size();i++)
	{
		std::vector<std::string> row = outFile[i];
		if((int)outFile[i].size() == 0)
			continue;
		std::map<std::string, std::string> resRow;
		resRow["epochs"] = row[0];
		resRow["train01"] = row[2];
		resRow["trainEntropy"] = row[3];
		resRow["trainObj"] = row[4];
		if(FLAGS_perform_test)
		{
			resRow["test01"] = row[6];
			resRow["time"] = row[8];
		}
		else
		{
			resRow["time"] = row[6];
			resRow["test01"] = "-1";
		}
		resRow["waitPercentage"] = gstd::Printer::p(waitPercentage);
		res.push_back(resRow);
	}	
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
	return gstd::Printer::p(val);
}

std::string printDouble(double val)
{
	return gstd::Printer::p(val);
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
	mgr.maxWaitTime = 1000000;

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
"ignore_nan=" + printBool(FLAGS_ignore_nan) + "\n"
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
"bg_idle_milli=" + gstd::Printer::p(FLAGS_bg_idle_milli) + "\n"
"# Total bandwidth: bandwidth_mbps * num_comm_channels_per_client * 2\n"
"client_bandwidth_mbps=" + gstd::Printer::p(FLAGS_client_bandwidth_mbps) + "\n"
"server_bandwidth_mbps=" + gstd::Printer::p(FLAGS_server_bandwidth_mbps) + "\n"
"# bandwidth / oplog_push_upper_bound should be > miliseconds.\n"
"thread_oplog_batch_size=" + gstd::Printer::p(FLAGS_thread_oplog_batch_size) + "\n"
"server_idle_milli=" + gstd::Printer::p(FLAGS_server_idle_milli) + "\n"
"update_sort_policy=" + FLAGS_update_sort_policy + "\n"
"row_candidate_factor=" + gstd::Printer::p(FLAGS_row_candidate_factor) + "\n"
"append_only_buffer_capacity=" + gstd::Printer::p(FLAGS_append_only_buffer_capacity) + "\n"
"append_only_buffer_pool_size=" + gstd::Printer::p(FLAGS_append_only_buffer_pool_size) + "\n"
"bg_apply_append_oplog_freq=" + gstd::Printer::p(FLAGS_bg_apply_append_oplog_freq) + "\n"
"client_send_oplog_upper_bound=" + gstd::Printer::p(FLAGS_client_send_oplog_upper_bound) + "\n"
"server_push_row_upper_bound=" + gstd::Printer::p(FLAGS_server_push_row_upper_bound) + "\n"
"row_oplog_type=" + gstd::Printer::p(FLAGS_row_oplog_type) + "\n"
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
"if [ \"$num_batches_per_epoch\" -eq \"0\" ]\n"
"then\n"
"  if $add_immediately\n"
"  then\n"
"    echo 'adjusting batches per epoch'\n"
"    let num_batches_per_epoch=num_train_data/num_table_threads\n"
"    let num_batches_per_epoch=num_batches_per_epoch/num_hosts\n"
"    let num_batches_per_epoch=num_batches_per_epoch/25\n"
"  else\n"
"    let num_batches_per_epoch=1\n"
"  fi\n"
"  echo 'Here it comes'\n"
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
"    --ignore_nan=$ignore_nan \\\n"
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

bool errorBasedStoppageCriterion(std::vector<std::string> row, double thresh)
{
    if(std::stod(row[6]) <= thresh)
        return true;
    else
        return false;
}
    
std::map<int32_t, double> topMuMap = {{500, 0.01}, {5000, 0.01}, {50000, 0.01}, {500000, 0.01}};
std::map<int32_t, double> bottomMuMap = {{500, 0.001}, {5000, 0.001}, {50000, 0.001}, {500000, 0.001}};
std::map<int32_t, double> targetErrorMap = {{500, 0.37}, {5000, 0}, {50000, 0}, {500000, 0}};

IntervalSearchController runLearningRateSearch(ParmStruct ps, std::map<double, int32_t>& muEpochMap)
{
    double targetError = targetErrorMap[ps.num_train_data];
    std::function<bool(std::vector<std::string>)> boundErrorBasedStoppageCriterion = [targetError](std::vector<std::string> row)
    {
        return errorBasedStoppageCriterion(row, targetError);
    };

    double topMu = topMuMap[ps.num_train_data];
    muEpochMap[topMu] = ps.num_epochs;
    ps.learning_rate = topMu;
    ps.set();
    run();
    std::map<std::string, std::string> output = readOutFiles(boundErrorBasedStoppageCriterion);
    int32_t numTopEpochsNeeded = std::stoi(output["epochs"]);
    
    double bottomMu = bottomMuMap[ps.num_train_data];
    muEpochMap[bottomMu] = ps.num_epochs;
    ps.learning_rate = bottomMu;
    ps.set();
    run();
    output = readOutFiles(boundErrorBasedStoppageCriterion);
    int32_t numBottomEpochsNeeded  = std::stoi(output["epochs"]);

    gstd::check(numTopEpochsNeeded < ps.num_epochs || numBottomEpochsNeeded < ps.num_epochs, "could not converge on either starting config in mu search");

    int32_t newEpochNumber = numBottomEpochsNeeded + 1;
    if(numTopEpochsNeeded < numBottomEpochsNeeded)
        newEpochNumber = numTopEpochsNeeded + 1;

    ps.num_epochs = newEpochNumber;

    IntervalSearchController controller(log10(topMu), log10(bottomMu), (double)numTopEpochsNeeded, (double)numBottomEpochsNeeded, log10(1.01));
    
    while(1)
    {
        double nextMu = pow(10,controller.get());
        muEpochMap[nextMu] = ps.num_epochs;
        ps.learning_rate = nextMu;
        ps.set();
        run();
        output = readOutFiles(boundErrorBasedStoppageCriterion);
        if(controller.consume(std::stod(output["epochs"])))
        {
            return controller;
        }
        ps.num_epochs = std::stoi(output["epochs"]) + 1;
    }
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
    gstd::check(!gstd::file::exists({FLAGS_output_file_prefix + "_verbose"})[0], "verbose outfile path exists");
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
    gstd::Writer::rs<std::string>(ps.get_output_file_prefix(), {resHeader}, " ", false, std::ios::app);
    for(int i=1;i<(int)parmContent.size();i++)
    {
        gstd::check(parmContentRowSize == (int)parmContent[i].size(), "parms are not a table");
        for(int j=0;j<parmContentRowSize; j++)
            ps.consume(parmContent[0][j], parmContent[i][j]);

        std::function<bool(std::vector<std::string>)> stoppageCriterionUsed = defaultStoppageCriterion;
	if(FLAGS_learning_rate_search)
	{
            std::map<double, int32_t> muEpochMap;
	    IntervalSearchController controller = runLearningRateSearch(ps, muEpochMap);
	    ps.learning_rate = pow(10,controller.getPointCenter());
            ps.num_epochs = muEpochMap[ps.learning_rate];
            ps.set();
            double targetError = targetErrorMap[ps.num_train_data];
            std::function<bool(std::vector<std::string>)> boundErrorBasedStoppageCriterion = [targetError](std::vector<std::string> row)
            {
                return errorBasedStoppageCriterion(row, targetError);
            };
            stoppageCriterionUsed = boundErrorBasedStoppageCriterion;
            std::vector<std::vector<std::string> > searchLog = controller.getLog();
            searchLog.insert(searchLog.begin(), ps.getRow());
	    searchLog.insert(searchLog.begin(), ps.getHeader());
            gstd::Writer::rs<std::string>(ps.get_output_file_prefix() + "_searchLog", searchLog, " ", false, std::ios::app);
	}
	else
	{
            ps.set();
	    run();
	}

	{
		//build concise outfile
		std::map<std::string, std::string> resFiles = readOutFiles(stoppageCriterionUsed);
		std::vector<std::string> resRow = ps.getRow();
		for(int j=0; j<(int)outCols.size(); j++)
		    resRow.push_back(resFiles[outCols[j]]);
		resRow.push_back(gstd::Printer::p(FLAGS_num_test_data));
		resRow.push_back(gstd::Printer::p(FLAGS_perform_test));
		res.push_back(resRow);
		if(FLAGS_client_id == 0)
		    gstd::Writer::rs<std::string>(ps.get_output_file_prefix(), {resRow}, " ", false, std::ios::app);
	}

	{
		//build verbose outfile
		std::vector<std::map<std::string, std::string> > resFiles = readOutFilesComplete();
		std::vector<std::vector<std::string> > verboseRes = {resHeader};
		for(int j=0; j<(int)resFiles.size(); j++)
		{
			std::map<std::string, std::string> row = resFiles[j];
			std::vector<std::string> resRow = ps.getRow();
			for(int j=0; j<(int)outCols.size(); j++)
			    resRow.push_back(row[outCols[j]]);
			resRow.push_back(gstd::Printer::p(FLAGS_num_test_data));
			resRow.push_back(gstd::Printer::p(FLAGS_perform_test));
			verboseRes.push_back(resRow);
		}
		if(FLAGS_client_id == 0)
		    gstd::Writer::rs<std::string>(ps.get_output_file_prefix() + "_verbose", verboseRes, " ", false, std::ios::app);
	}
    }
    
    
    return 0;
}































