#include <petuum_ps_common/include/petuum_ps.hpp>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <thread>
#include <vector>
#include <map>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <petuum_ps_common/include/system_gflags_declare.hpp>
#include <petuum_ps_common/include/table_gflags_declare.hpp>
#include <petuum_ps_common/include/init_table_config.hpp>
#include <petuum_ps_common/include/init_table_group_config.hpp>
#include <cstdio>

//LDA parms
//data
DEFINE_string(doc_file, "", "File containing document in LibSVM format. Each document is a line.");
DEFINE_string(doc_file_suffix, "", "File containing document in LibSVM format. Each document is a line.");
DEFINE_int32(num_vocabs, -1, "Number of vocabs.");
DEFINE_int32(max_vocab_id, -1, "Maximum word index, which could be different from num_vocabs if there are unused vocab indices.");
DEFINE_int32(num_topics, 100, "Number of topics.");
//model
DEFINE_double(alpha, 1, "Dirichlet prior on document-topic vectors.");
DEFINE_double(beta, 0.1, "Dirichlet prior on vocab-topic vectors.");
//network
DEFINE_int32(communication_factor, -1, "The factor by which communication is artificially delayed");
DEFINE_int32(virtual_staleness, -1, "Artificial staleness");
DEFINE_bool(is_bipartite, false, "bipartite topology in virtual staleness");
DEFINE_bool(is_local_sync, false, "local styncing in bipartite topology in virtual staleness");
//training
DEFINE_int32(num_work_units, 1, "Number of work units");
DEFINE_int32(compute_ll_interval, 1, "Copmute log likelihood over local dataset on every N iterations");
DEFINE_int32(num_iters_per_work_unit, 1, "number of iterations per work unit");
DEFINE_int32(num_clocks_per_work_unit, 1, "number of clocks per work unit");
DEFINE_string(error_threshold, "0", "error_threshold");
DEFINE_int32(seed, 0, "random seed for sampling topics. It is not used for initialization");

// System parms
DEFINE_uint64(word_topic_table_process_cache_capacity, 100000, "Word topic table process cache capacity");

// paths
DEFINE_string(output_path, "", "LDA results.");
DEFINE_string(parm_file, "", "parm_file");
DEFINE_string(system_path, "", "parm_file");

// No need to change the following.
DEFINE_int32(word_topic_table_id, 1, "ID within Petuum-PS");
DEFINE_int32(summary_table_id, 2, "ID within Petuum-PS");
DEFINE_int32(llh_table_id, 3, "ID within Petuum-PS");


std::string printBool(bool val)
{
	if(val)
		return "true";
	else
		return "false";
}


std::string printInt(int val)
{
	std::stringstream stream;
	stream << val;
	return stream.str();
}

std::string printDouble(double val)
{
	std::stringstream stream;
	stream << val;
	return stream.str();
}

std::vector<std::string> split(std::string input, char delim)
{
    std::vector<std::string> res;
    std::stringstream stream(input);
    std::string item;
    while (std::getline(stream, item, delim)) 
    {
        res.push_back(item);
    }
    return res;
}

std::vector<std::string> rls(std::string path)
{
	std::vector<std::string> result;
	std::ifstream stream;
	stream.open(path);
	CHECK(!stream.fail()) << "output file not found";	
	while(!stream.eof())
	{
		std::string line;
		std::getline(stream, line);
		result.push_back(line);
	}
	stream.close();
	return result;	
}

std::vector<std::vector<std::string> > rrs(std::string path, char delim)
{
	std::vector<std::vector<std::string> > result;
	std::ifstream stream;
	stream.open(path);
	CHECK(!stream.fail()) << "output file not found at path " << path;	
	while(!stream.eof())
	{
		std::string line;
		std::getline(stream, line);
		if(stream.eof())
			break;
		std::stringstream lineStream(line);
		std::vector<std::string> fields;
		while(!lineStream.eof())
		{
			std::string field;
			std::getline(lineStream, field, delim);
			fields.push_back(field);
		}
		result.push_back(fields);
	}
	stream.close();
	return result;	
}

std::vector<std::vector<int> > rrsInt(std::string path, char delim)
{
	std::vector<std::vector<int> > result;
	std::ifstream stream;
	stream.open(path);
	CHECK(!stream.fail()) << "output file not found";	
	while(!stream.eof())
	{
		std::string line;
		std::getline(stream, line);
		if(stream.eof())
			break;
		std::stringstream lineStream(line);
		std::vector<int> fields;
		while(!lineStream.eof())
		{
			std::string field;
			std::getline(lineStream, field, delim);
			fields.push_back(std::stoi(field));
		}
		result.push_back(fields);
	}
	stream.close();
	return result;	
}

std::vector<std::vector<double> > rrsDouble(std::string path, char delim)
{
	std::vector<std::vector<double> > result;
	std::ifstream stream;
	stream.open(path);
	CHECK(!stream.fail()) << "output file not found";
	while(!stream.eof())
	{
		std::string line;
		std::getline(stream, line);
		if(stream.eof())
			break;
		std::stringstream lineStream(line);
		std::vector<double> fields;
		while(!lineStream.eof())
		{
			std::string field;
			std::getline(lineStream, field, delim);
			fields.push_back(std::stod(field));
		}
		result.push_back(fields);
	}
	stream.close();
	return result;	
}

template<typename type>
void wrs(std::string path, std::vector<std::vector<type> > input, std::string delim, std::ios_base::openmode mode)
{
	std::ofstream stream;
	stream.open(path, mode);
	CHECK(!stream.fail()) << "output file not found";
	int size = (int)input.size();
	for(int i=0;i<size;i++)
	{
		std::vector<type> row = input[i];
		int rowSize = (int)row.size();
		if(i!=0)
			stream << '\n';
		for(int j=0;j<rowSize;j++)
		{
			if(j!=0)
				stream << delim;
			stream << row[j];
		}
	}
	stream.close();
}

template<typename type>
void wls(std::string path, std::vector<type> input, std::ios_base::openmode mode)
{
	std::ofstream stream;
	stream.open(path, mode);
	CHECK(!stream.fail()) << "output file not found";
	int size = (int)input.size();
	for(int i=0;i<size;i++)
	{
		type row = input[i];
		if(i!=0)
			stream << '\n';
		stream << row;
	}
	stream.close();
}

template<typename intype, typename outtype>
std::vector<std::vector<outtype> > apply2d(std::function<outtype(intype)> fun, std::vector<std::vector<intype> > input)
{
	int size1 = input.size();
	std::vector<std::vector<outtype> > res(size1);
	for (int i = 0; i < size1; i++)
	{
		int size2 = input[i].size();
		res[i].resize(size2);
		for (int j = 0; j < size2; j++)
		{
			res[i][j] = fun(input[i][j]);
		}
	}
	return res;
}

std::vector<bool> exists(std::vector<std::string> locations)
{
	std::vector<bool> res;
	int numLocations = (int)locations.size();
	for(int i=0;i<numLocations;i++)
	{
		std::ifstream ifstream;			
		ifstream.open(locations[i]);
		if(ifstream.fail())
			res.push_back(false);
		else
			res.push_back(true);
	}
	return res;
}

void removeFile(std::string location)
{
	::remove(location.c_str());
	std::ifstream ifstream;
	ifstream.open(location);
	CHECK(ifstream.fail()) << "cannot remove removeable at location " << location;
}

class TerminalMgr
{
public:
	TerminalMgr();
	
	//inputs
	std::string externalPath;
	std::string internalPath;
	std::string command;
	char delimiter;
	std::vector<std::vector<std::string> > inHeaderFiles; //specified relative to internalPath
	std::vector<std::vector<std::vector<double> > > inDataFiles; //specified relative to internalPath
	std::vector<std::string> inHeaderFileNames; //specified relative to internalPath
	std::vector<std::string> inDataFileNames; //specified relative to internalPath
	std::vector<std::string> outHeaderFileNames;
	std::vector<std::string> outDataFileNames;
	std::string outFileNameInternal; //specified relative to internalPath
	std::string outFileNameExternal; //this must match the above after moving to externalPath
        std::string completeFileNameInternal; //this file signifies completion of the command
        
	static const std::string defaultOutFileName;
	static const std::string defaultCommandFileName;
        static const std::string defaultCompleteFileName;

	//options
	std::string commandFileNameInternal; //use this if the command is triggered via a file; this is specified relative to internalPath
	std::string commandFileNameExternal; ///this must match the above after moving to externalPath !!!NOTE!!! This must be specified in a way that it can execute. IE in Unix, write ./X, not X
	double maxWaitTime;
	/*//inherited:*/ bool safe; //THIS IS SET TO TRUE BY DEFAULT because this class has the power to change external files.

	//outputs
	std::vector<std::vector<std::string> > outHeaderFiles;
	std::vector<std::vector<std::vector<double> > > outDataFiles;
	
	//actions
	bool run();
	static bool exec(std::string command, std::string outFileNameInternal = defaultOutFileName, std::string outFileNameExternal = defaultOutFileName);

private:
	std::string getFullCommandFileNameInternal();
	std::string getFullOutFileNameInternal();
	void cleanup();
};

const std::string TerminalMgr::defaultOutFileName = "msii810161816_TMP_OUT";
const std::string TerminalMgr::defaultCommandFileName = "msii810161816_TMP_CMD";
const std::string TerminalMgr::defaultCompleteFileName = "msii810161816_TMP_COMPLETE";

TerminalMgr::TerminalMgr()
{
	commandFileNameInternal = "";
	commandFileNameExternal = "";
	maxWaitTime = 3600;
	safe = true;
}	

std::string TerminalMgr::getFullCommandFileNameInternal()
{
	std::string res = internalPath + commandFileNameInternal;
#ifdef _WIN32
	res = res + ".bat";
#endif
	return res;
}

std::string TerminalMgr::getFullOutFileNameInternal()
{
	return internalPath + outFileNameInternal;
}

void TerminalMgr::cleanup()
{
	std::string fullCommandFileNameInternal = getFullCommandFileNameInternal();
	std::string fullOutFileNameInternal = getFullOutFileNameInternal();
	std::vector<std::string> removables;
	if (commandFileNameInternal != "")
		removables.push_back(fullCommandFileNameInternal);
	removables.push_back(fullOutFileNameInternal);
	removables.push_back(internalPath + completeFileNameInternal);
	int numHeaders = inHeaderFileNames.size();
	int numDatas = inDataFileNames.size();
	int numOutHeaders = outHeaderFileNames.size();
	int numOutDatas = outDataFileNames.size();
	for (int i = 0; i < numHeaders; i++)
		removables.push_back(internalPath + inHeaderFileNames[i]);
	for (int i = 0; i < numDatas; i++)
		removables.push_back(internalPath + inDataFileNames[i]);
	for (int i = 0; i < numOutHeaders; i++)
		removables.push_back(internalPath + outHeaderFileNames[i]);
	for (int i = 0; i < numOutDatas; i++)
		removables.push_back(internalPath + outDataFileNames[i]);
	for (int i = 0; i < (int)removables.size(); i++)
		removeFile(removables[i]);
}

bool TerminalMgr::run()
{
	//initialize
	int numHeaders = inHeaderFileNames.size();
	int numDatas = inDataFileNames.size();
	std::string stringdelim(1, delimiter);
	CHECK(numHeaders == (int)inHeaderFiles.size()) << "number of header files does not match";
	CHECK(numDatas == (int)inDataFiles.size()) << "number of data files does not match";
	int numOutHeaders = outHeaderFileNames.size();
	int numOutDatas = outDataFileNames.size();
	std::string fullCommandFileNameInternal = getFullCommandFileNameInternal();
	std::string fullOutFileNameInternal = getFullOutFileNameInternal();
	std::string completionPath = internalPath + completeFileNameInternal;

	//collect precleanables
	std::vector<std::string> precleanables;
	for (int i = 0; i < numHeaders; i++)
		precleanables.push_back(internalPath + inHeaderFileNames[i]);
	for (int i = 0; i < numDatas; i++)
		precleanables.push_back(internalPath + inDataFileNames[i]);
	for (int i = 0; i < numOutHeaders; i++)
		precleanables.push_back(internalPath + outHeaderFileNames[i]);
	for (int i = 0; i < numOutDatas; i++)
		precleanables.push_back(internalPath + outDataFileNames[i]);
	precleanables.push_back(fullOutFileNameInternal);
	if (commandFileNameInternal != "")
	    precleanables.push_back(fullCommandFileNameInternal);
	precleanables.push_back(completionPath);

	//do the precleaning
	if (safe)
	{
		std::vector<bool> ex = exists(precleanables);
		int size = ex.size();
		for (int i = 0; i < size; i++)
		{
			CHECK(!ex[i]) << "Needs file location that is already used : " << precleanables[i];
		}
	}
	else
	{
		for(int i=0;i<(int)precleanables.size(); i++)
		    removeFile(precleanables[i]);
	}
	
	//write the data
	for (int i = 0; i < numHeaders; i++)
	{
		std::string path = internalPath + inHeaderFileNames[i];
		wls<std::string>(path, inHeaderFiles[i], std::ios::trunc);
	}
	for (int i = 0; i < numDatas; i++)
	{
		std::string path = internalPath + inDataFileNames[i];
		wrs<double>(path, inDataFiles[i], stringdelim, std::ios::trunc);
	}

	//write command file		
#ifdef _WIN32
	commandFileNameExternal = commandFileNameExternal + ".bat";
#endif
	if (commandFileNameInternal != "")
		wls<std::string>(fullCommandFileNameInternal, {command}, std::ios::trunc);		

	//build the final command
	std::string finalCommand = "";
	if (externalPath != "")
	{
		//first, go to externalPath
		finalCommand = finalCommand + "cd " + externalPath;
		//separator
#ifdef _WIN32
		finalCommand = finalCommand + " & ";
#else
		finalCommand = finalCommand + "; ";
#endif
	}
	//then, the original command
	//if file was used, go for that
	if (commandFileNameInternal != "")
	{
		//give permission to the file
#ifndef _WIN32
		finalCommand = finalCommand + "chmod 777 " + commandFileNameExternal + "; ";
#endif
		finalCommand = finalCommand + commandFileNameExternal;
	}
	//otherwise, use the originally specified command
	else
		finalCommand = finalCommand + command;
	//if linux, we need the file name to terminate the command
        std::string modifiedOutFileNameExternal = outFileNameExternal;
#ifndef _WIN32
	modifiedOutFileNameExternal = modifiedOutFileNameExternal + ";";
#endif

	//execute command
	bool res = exec(finalCommand, fullOutFileNameInternal, modifiedOutFileNameExternal);

	//set up output
	outHeaderFiles.clear();
	outDataFiles.clear();
	outHeaderFiles.resize(numOutHeaders);
	outDataFiles.resize(numOutDatas);
	std::vector<bool> headersFound(numOutHeaders, false);
	std::vector<bool> datasFound(numOutDatas, false);

	//define helper function
	std::function<double(std::string)> caster = [](std::string in){return std::stod(in); };

	//scan for completion
	while (1)
	{
		if (exists({ completionPath })[0])
			break;
		struct timespec tim;
		tim.tv_sec = 0;
		tim.tv_nsec = 100000000;
		nanosleep(&tim, 0);
	}

	//get output
	for (int i = 0; i < numOutHeaders; i++)
		outHeaderFiles[i] = rls(internalPath + outHeaderFileNames[i]);
	for (int i = 0; i < numOutDatas; i++)
		outDataFiles[i] = apply2d(caster, rrs(internalPath + outDataFileNames[i], delimiter));

	//cleanup
	cleanup();
     
	return res;
}

bool TerminalMgr::exec(std::string command, std::string outFileNameInternal, std::string outFileNameExternal)
{
	command = command + " >> " + outFileNameExternal;
	int dummy = std::system(command.c_str());
	(void)dummy;
	if (!exists({ outFileNameInternal })[0])
	{
		return false;
	}
	else
	{
		return true;
	}
}

class ParmStruct
{
public:
	std::string doc_file_suffix;
	int32_t num_topics;
	double alpha;
	double beta;	
	int32_t communication_factor;
	int32_t virtual_staleness;
	bool is_bipartite;
        bool is_local_sync;
	int32_t num_work_units;
	int32_t compute_ll_interval;
	std::string error_threshold;
	int32_t seed;

    ParmStruct()
    {
	doc_file_suffix = FLAGS_doc_file_suffix;
	num_topics = FLAGS_num_topics;
	alpha = FLAGS_alpha;
	beta = FLAGS_beta;
        communication_factor = FLAGS_communication_factor;
        virtual_staleness = FLAGS_virtual_staleness;
	is_bipartite = FLAGS_is_bipartite;
        is_local_sync = FLAGS_is_local_sync;
        num_work_units = FLAGS_num_work_units;
        compute_ll_interval = FLAGS_compute_ll_interval;
	output_path = FLAGS_output_path;
	error_threshold = FLAGS_error_threshold;
	seed = FLAGS_seed;
    }      
        
    void set()
    {
	FLAGS_doc_file_suffix = doc_file_suffix;
	FLAGS_num_topics = num_topics;
	FLAGS_alpha = alpha;
	FLAGS_beta = beta;
        FLAGS_communication_factor = communication_factor;
        FLAGS_virtual_staleness = virtual_staleness;
	FLAGS_is_bipartite = is_bipartite;
        FLAGS_is_local_sync = is_local_sync;
        FLAGS_num_work_units = num_work_units;
        FLAGS_compute_ll_interval = compute_ll_interval;
	FLAGS_error_threshold = error_threshold;
	FLAGS_seed = seed;

        std::stringstream outSuffix;
        
	outSuffix << ".SUF" << doc_file_suffix;
	outSuffix << ".TOP" << num_topics;
	outSuffix << ".A" << alpha;
	outSuffix << ".B" << beta;
	outSuffix << ".CF" << communication_factor;
	outSuffix << ".VS" << virtual_staleness;
	outSuffix << ".BI" << is_bipartite;
        outSuffix << ".LS" << is_local_sync;
        outSuffix << ".WU" << num_work_units;
        outSuffix << ".LL" << compute_ll_interval;
	outSuffix << ".ET" << error_threshold;
	outSuffix << ".SD" << seed;

	FLAGS_output_path = output_path + outSuffix.str();
    }
    
    void consume(std::string argname, std::string argval)
    {
	if(argname == "doc_file_suffix")
	{
		doc_file_suffix = argval;
	}
	else if(argname == "num_topics")
	{
		num_topics = std::stoi(argval);
	}
	else if(argname == "alpha")
	{
		alpha = std::stod(argval);
	}
	else if(argname == "beta")
	{
		beta = std::stod(argval);
	}
	else if(argname == "communication_factor")
	{
		communication_factor = std::stoi(argval);
	}
	else if(argname == "virtual_staleness")
	{
		virtual_staleness = std::stoi(argval);
	}
	else if(argname == "is_bipartite")
	{
		is_bipartite = (argval == "1");
	}
        else if(argname == "is_local_sync")
        {
                is_local_sync = (argval == "1");
        }
	else if(argname == "num_work_units")
	{
		num_work_units = std::stoi(argval);
	}
	else if(argname == "compute_ll_interval")
	{
		compute_ll_interval = std::stoi(argval);
	}
	else if(argname == "error_threshold")
	{
		error_threshold = argval;
	}
	else if(argname == "seed")
	{
		seed = std::stoi(argval);
	}
	else
	{
		CHECK(false) << "unknown parm " << argname;
	}
	
    }

	std::string get_output_path()
	{
		return output_path;
	}

	std::vector<std::string> getHeader()
	{
		std::vector<std::string> res;
		res.push_back("doc_file_suffix");
		res.push_back("num_topics");
		res.push_back("alpha");
		res.push_back("beta");
		res.push_back("communication_factor");
		res.push_back("virtual_staleness");
		res.push_back("is_bipartite");
                res.push_back("is_local_sync");
		res.push_back("num_work_units");
		res.push_back("compute_ll_interval");
		res.push_back("error_threshold");
		res.push_back("seed");
		return res;
	}

	std::vector<std::string> getRow()
	{
		std::vector<std::string> res;
		res.push_back(doc_file_suffix);
		res.push_back(printInt(num_topics));
		res.push_back(printDouble(alpha));
		res.push_back(printDouble(beta));
		res.push_back(printInt(communication_factor));
		res.push_back(printInt(virtual_staleness));
		res.push_back(printBool(is_bipartite));
                res.push_back(printBool(is_local_sync));
		res.push_back(printInt(num_work_units));
		res.push_back(printInt(compute_ll_interval));
		res.push_back(error_threshold);
		res.push_back(printInt(seed));
		return res;
	}

private:
	std::string output_path;

};


void run()
{
	TerminalMgr mgr;
	mgr.internalPath = FLAGS_system_path;
	mgr.externalPath = FLAGS_system_path;
	mgr.outFileNameInternal = TerminalMgr::defaultOutFileName;
	mgr.outFileNameExternal = TerminalMgr::defaultOutFileName;
	mgr.completeFileNameInternal = TerminalMgr::defaultCompleteFileName;
	mgr.commandFileNameInternal = TerminalMgr::defaultCommandFileName;
	mgr.commandFileNameExternal = "./" + TerminalMgr::defaultCommandFileName;
	mgr.maxWaitTime = 1000000;

	mgr.command = 
"#!/bin/bash -u\n"
"\n"
"# Find other Petuum paths by using the script's path\n"
"script_path=`readlink -f $0`\n"
"script_dir=`dirname $script_path`\n"
"app_root=`dirname $script_dir`\n"
"\n"
"# input parms\n"
"doc_file=\"" + FLAGS_doc_file + FLAGS_doc_file_suffix + "." + printInt(FLAGS_num_clients) + "\"\n"
"num_vocabs=" + printInt(FLAGS_num_vocabs) + "\n"
"max_vocab_id=" + printInt(FLAGS_max_vocab_id) + "\n"
"num_topics=" + printInt(FLAGS_num_topics) + "\n"
"alpha=" + printDouble(FLAGS_alpha) + "\n"
"beta=" + printDouble(FLAGS_beta) + "\n"
"communication_factor=" + printInt(FLAGS_communication_factor) + "\n"
"virtual_staleness=" + printInt(FLAGS_virtual_staleness) + "\n"
"is_bipartite=" + printBool(FLAGS_is_bipartite) + "\n"
"is_local_sync=" + printBool(FLAGS_is_local_sync) + "\n"
"num_work_units=" + printInt(FLAGS_num_work_units) + "\n"
"compute_ll_interval=" + printInt(FLAGS_compute_ll_interval) + "\n"
"num_clocks_per_work_unit=" + printInt(FLAGS_num_clocks_per_work_unit) + "\n"
"num_iters_per_work_unit=" + printInt(FLAGS_num_iters_per_work_unit) + "\n"
"seed=" + printInt(FLAGS_seed) + "\n"
"\n"
"# changing system parms\n"
"table_staleness=" + printInt(FLAGS_table_staleness) + "\n"
"consistency_model=" + FLAGS_consistency_model + "\n"
"client_bandwidth_mbps=" + printInt(FLAGS_client_bandwidth_mbps) + "\n"
"server_bandwidth_mbps=" + printInt(FLAGS_server_bandwidth_mbps) + "\n"
"num_comm_channels_per_client=" + printInt(FLAGS_num_comm_channels_per_client) + "\n"
"num_table_threads=" + printInt(FLAGS_num_table_threads) + "\n"
"hostfile=\"" + FLAGS_hostfile + "\"\n"
"\n"
"# constant system parms\n"
"word_topic_table_process_cache_capacity=" + (FLAGS_is_local_sync ? printInt(2*(FLAGS_max_vocab_id+1)) : printInt(FLAGS_max_vocab_id+1))+ "\n"
"bg_idle_milli=2\n"
"server_idle_milli=2\n"
"thread_oplog_batch_size=14000\n"
"row_candidate_factor=100\n"
"update_sort_policy=RelativeMagnitude\n"
"numa_opt=false\n"
"numa_policy=Even\n"
"row_oplog_type=0\n"
"server_push_row_upper_bound=500\n"
"client_send_oplog_upper_bound=5000\n"
"append_only_buffer_capacity=$((1024*512))\n"
"append_only_buffer_pool_size=3\n"
"bg_apply_append_oplog_freq=-1\n"
"process_storage_type=BoundedDense\n"
"oplog_type=Dense\n"
"no_oplog_replay=true\n"
"naive_table_oplog_meta=false\n"
"suppression_on=false\n"
"use_approx_sort=false\n"
"\n"
"# paths\n"
"output_file_prefix=" + FLAGS_output_path + "/output\n"
"stats_path=" + FLAGS_output_path + "/lda_stats.yaml\n"
"log_dir=" + FLAGS_output_path + "/log\n"
"signal_file_path=" + FLAGS_system_path + TerminalMgr::defaultCompleteFileName + "\n"
"progname=lda_main\n"
"prog_path=${app_root}/lda/bin/${progname}\n"
"rm -r " + FLAGS_output_path + "\n"
"mkdir " + FLAGS_output_path + "\n"
"\n"
//"doc_file=$(readlink -f $doc_filename)\n"
//"hostfile=$(readlink -f $host_filename)\n"
"\n"
"ssh_options=\"-oStrictHostKeyChecking=no -oUserKnownHostsFile=/dev/null -oLogLevel=quiet\"\n"
"\n"
"# Parse hostfile\n"
"host_list=`cat $hostfile | awk '{ print $2 }'`\n"
"unique_host_list=`cat $hostfile | awk '{ print $2 }' | uniq`\n"
"num_unique_hosts=`cat $hostfile | awk '{ print $2 }' | uniq | wc -l`\n"
"host_list=`cat $hostfile | awk '{ print $2 }'`\n"
"num_hosts=`cat $hostfile | awk '{ print $2 }' | wc -l`\n"
"\n"
"echo ${num_unique_hosts}\n"
"\n"
"# Kill previous instances of this program\n"
"echo \"Killing previous instances of '$progname' on servers, please wait...\"\n"
"for ip in $unique_host_list; do\n"
"  ssh $ssh_options $ip \\\n"
"    killall -q $progname\n"
"done\n"
"echo \"All done!\"\n"
"# exit\n"
"\n"
"# Spawn program instances\n"
"client_id=0\n"
"for ip in $host_list; do\n"
"    client_id_mod=$(( client_id%10 ))\n"
"    if [ $client_id_mod -eq 0 ]\n"
"    then\n"
"	echo \"wait for 1 sec\"\n"
"	sleep 1\n"
"    fi\n"
"\n"
"  echo Running client $client_id on $ip\n"
"  log_path=${log_dir}.${client_id}\n"
"  numa_index=$(( client_id%num_unique_hosts ))\n"
"\n"
//ls /l0;\n"
"  echo $signal_file_path \\\n"
"\n"
"  cmd=\"rm -rf ${log_path}; mkdir -p ${log_path};\n"
"GLOG_logtostderr=true \\\n"
"      GLOG_log_dir=${log_path} \\\n"
"      GLOG_v=-1 \\\n"
"      GLOG_minloglevel=0 \\\n"
"      GLOG_vmodule=\"\" \\\n"
"      $prog_path \\\n"
"    --stats_path ${stats_path}\\\n"
"    --num_clients $num_hosts \\\n"
"    --num_comm_channels_per_client $num_comm_channels_per_client \\\n"
"    --init_thread_access_table=false \\\n"
"    --num_table_threads ${num_table_threads} \\\n"
"    --client_id $client_id \\\n"
"    --hostfile ${hostfile} \\\n"
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
"    --nooplog_dense_serialized \\\n"
"    --oplog_type ${oplog_type} \\\n"
"    --append_only_oplog_type DenseBatchInc \\\n"
"    --append_only_buffer_capacity ${append_only_buffer_capacity} \\\n"
"    --append_only_buffer_pool_size ${append_only_buffer_pool_size} \\\n"
"    --bg_apply_append_oplog_freq ${bg_apply_append_oplog_freq} \\\n"
"    --process_storage_type ${process_storage_type} \\\n"
"    --no_oplog_replay=${no_oplog_replay} \\\n"
"    --server_push_row_upper_bound ${server_push_row_upper_bound} \\\n"
"    --client_send_oplog_upper_bound ${client_send_oplog_upper_bound} \\\n"
"    --alpha $alpha \\\n"
"    --beta $beta \\\n"
"    --num_topics $num_topics \\\n"
"    --num_vocabs $num_vocabs \\\n"
"    --max_vocab_id $max_vocab_id \\\n"
"    --num_clocks_per_work_unit $num_clocks_per_work_unit \\\n"
"    --num_iters_per_work_unit $num_iters_per_work_unit \\\n"
"    --seed $seed \\\n"
"    --word_topic_table_process_cache_capacity $word_topic_table_process_cache_capacity \\\n"
"    --num_work_units $num_work_units \\\n"
"    --compute_ll_interval=$compute_ll_interval \\\n"
"    --doc_file=${doc_file}.$client_id \\\n"
"    --output_file_prefix ${output_file_prefix} \\\n"
"    --communication_factor=$communication_factor \\\n"
"    --virtual_staleness=$virtual_staleness \\\n"
"    --is_bipartite=$is_bipartite \\\n"
"    --is_local_sync=$is_local_sync \\\n"
"    --signal_file_path=$signal_file_path\"\n"
"\n"
"#  echo $cmd\n"
"  ssh $ssh_options $ip $cmd &\n"
"\n"
"# Wait a few seconds for the name node (client 0) to set up\n"
"  if [ $client_id -eq 0 ]; then\n"
"    echo \"Waiting for name node to set up...\"\n"
"    sleep 3\n"
"  fi\n"
"  client_id=$(( client_id+1 ))\n"
"done";
	CHECK(mgr.run()) << "Terminal run failed";
}


bool fexists(std::string path)
{
	std::ifstream stream;
	stream.open(path);
	if(stream.fail())
		return false;
	else
		return true;
}

int getKeyLine(std::vector<std::vector<std::string> > results, double errorThreshold)
{
	int resultSize = (int)results.size();
	int i=0;
	int resultCounter = 0;
	for(i=0;i<resultSize;i++)
	{
		if(std::stod(results[i][1]) >= errorThreshold)
			resultCounter++;
		else
			resultCounter = 0;
		if(resultCounter == 3)
			break;
	}
	return i;
}

int main(int argc, char *argv[]) 
{
	google::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	CHECK(!FLAGS_is_local_sync) << "Not yet supported!";
	std::vector<std::vector<std::string> > parms = rrs(FLAGS_parm_file, ' ');
	int numParmRows = (int)parms.size();
	std::vector<std::string> header = parms[0];
	int parmRowSize = (int)header.size();
	ParmStruct ps;
	std::vector<std::string> outHeader = ps.getHeader();
	outHeader.push_back("epoch");
	outHeader.push_back("error");
	outHeader.push_back("time");
	CHECK(!fexists(ps.get_output_path())) << "outpath file exists";
	wrs<std::string>(ps.get_output_path(), {outHeader}, " ", std::ios::trunc);
	wrs<std::string>(ps.get_output_path() + "_verbose", {}, " ", std::ios::trunc);
	for(int i=1;i<numParmRows;i++)
	{
		std::vector<std::string> row = parms[i];
		CHECK(parmRowSize == (int)row.size()) << "parms are not a table";
		for(int j=0;j<parmRowSize; j++)
			ps.consume(header[j], row[j]);
		ps.set();
		run();
		std::vector<std::vector<std::string> > resultRaw = rrs(FLAGS_output_path + "/output.llh", ' ');
		int resultSize = (int)resultRaw.size();
		std::vector<std::vector<std::string> > result = {{}};
		std::vector<std::string> errorThresholds = split(FLAGS_error_threshold, ':');
		int numThresholds = (int)errorThresholds.size();
		for(int j=0;j<numThresholds;j++)
		{
			int keyLineIndex = getKeyLine(resultRaw, std::stod(errorThresholds[j]));
			if(keyLineIndex == resultSize)
				keyLineIndex = resultSize - 1;
			std::vector<std::string> resRow = ps.getRow();
			for(int j=0;j<3;j++)
				resRow.push_back(resultRaw[keyLineIndex][j]);
			result.push_back(resRow);
		}		
		wrs<std::string>(ps.get_output_path(), result, " ", std::ios::app);
		std::vector<std::vector<std::string> > resultLong = {{}, outHeader};
		for(int k=0;k<(int)resultRaw.size();k++)
		{
			std::vector<std::string> resRow = ps.getRow();
			for(int j=0;j<3;j++)
				resRow.push_back(resultRaw[k][j]);
			resultLong.push_back(resRow);
		}
		wrs<std::string>(ps.get_output_path() + "_verbose", resultLong, " ", std::ios::app);
	}
	LOG(INFO) << "DONE";
	return 0;
}





















