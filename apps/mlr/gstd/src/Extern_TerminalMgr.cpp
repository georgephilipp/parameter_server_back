#include "stdafx.h"

#include "TerminalMgr.h"
#include "Writer.h"
#include "ex.h"
#include "Vector.h"
#include "Timer.h"
#include "Reader.h"
#include "Linalg.h"
#include "File.h"
#include "Dependencies.h"

namespace msii810161816
{
	namespace gstd
	{
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
			gstd::checkMany(gstd::file::remove(removables, false, safe), "could not remove removeables");
		}

		gstd::trial<std::vector<std::string> > TerminalMgr::run()
		{
			//initialize
			int numHeaders = inHeaderFileNames.size();
			int numDatas = inDataFileNames.size();
			std::string stringdelim(1, delimiter);
			gstd::check(numHeaders == (int)inHeaderFiles.size(), "number of header files does not match");
			gstd::check(numDatas == (int)inDataFiles.size(), "number of data files does not match");
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
				std::vector<bool> ex = gstd::file::exists(precleanables);
				int size = ex.size();
				for (int i = 0; i < size; i++)
				{
					gstd::check(!ex[i], "Needs file location that is already used : " + precleanables[i]);
				}
			}
			else
				gstd::checkMany(gstd::file::remove(precleanables, false, safe), "could not remove precleanables");
			
			//write the data
			for (int i = 0; i < numHeaders; i++)
			{
				std::string path = internalPath + inHeaderFileNames[i];
				gstd::Writer::ls(path, inHeaderFiles[i], false, std::ios::trunc);
			}
			for (int i = 0; i < numDatas; i++)
			{
				std::string path = internalPath + inDataFileNames[i];
				gstd::Writer::rs(path, inDataFiles[i], stringdelim, false, std::ios::trunc);
			}

			//write command file		
#ifdef _WIN32
			commandFileNameExternal = commandFileNameExternal + ".bat";
#endif
			if (commandFileNameInternal != "")
				gstd::Writer::w(fullCommandFileNameInternal, command, false, std::ios::trunc);		

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
			gstd::trial<std::vector<std::string> > res = exec(finalCommand, fullOutFileNameInternal, modifiedOutFileNameExternal);

			//set up output
			gstd::vector::empty(outHeaderFiles);
			gstd::vector::empty(outDataFiles);
			outHeaderFiles.resize(numOutHeaders);
			outDataFiles.resize(numOutDatas);
			std::vector<bool> headersFound(numOutHeaders, false);
			std::vector<bool> datasFound(numOutDatas, false);

			//define helper function
			std::function<double(std::string)> caster = [](std::string in){return std::stod(in); };

			//scan for completion
			Timer t;
			while (1)
			{
				if (gstd::file::exists({ completionPath })[0])
					break;
				gstd::check(t.t(false) < maxWaitTime, "waiting for external function call timed out");
				gstd::Timer::sleep(0.1);
			}

			//get output
			for (int i = 0; i < numOutHeaders; i++)
				outHeaderFiles[i] = Reader::ls(internalPath + outHeaderFileNames[i]);
			for (int i = 0; i < numOutDatas; i++)
				outDataFiles[i] = gstd::vector::apply2d(caster, Reader::rs<std::string>(internalPath + outDataFileNames[i], delimiter));

			//cleanup
			cleanup();
                     
			return res;
		}
                
		gstd::trial<std::vector<std::string> > TerminalMgr::exec(std::string command, std::string outFileNameInternal, std::string outFileNameExternal)
        {
			gstd::trial<std::vector<std::string> > res;
			command = command + " >> " + outFileNameExternal;
			int dummy = std::system(command.c_str());
			(void)dummy;
                        if (!gstd::file::exists({ outFileNameInternal })[0])
			{
				res.success = false;
				return res;
			}
			else
			{
				res.success = true;
				res.result = gstd::Reader::ls(outFileNameInternal);
				return res;
			}
        }

		//Base Package
		gstd::TypeName TerminalMgr::getTypeName()
		{
			return TypeNameGetter<TerminalMgr>::get();
		}
		std::string TerminalMgr::toString()
		{
			std::stringstream res;
			res << "This is a gstd::TerminalMgr" << std::endl;
			res << "externalPath is " << externalPath << std::endl;
			res << "internalPath is " << internalPath << std::endl;
			res << "command is " << command << std::endl;
			res << "delimiter is " << delimiter << std::endl;
			res << "inHeaderFileNames are " << gstd::Printer::vp(inHeaderFileNames) << std::endl;
			res << "inDataFileNames are " << gstd::Printer::vp(inDataFileNames) << std::endl;
			res << "outHeaderFileNames are " << gstd::Printer::vp(outHeaderFileNames) << std::endl;
			res << "outDataFileNames are " << gstd::Printer::vp(outDataFileNames) << std::endl;
			res << "outFileNameInternal is " << outFileNameInternal << std::endl;
			res << "outFileNameExternal is " << outFileNameExternal << std::endl;
			res << "commandFileNameInternal is " << commandFileNameInternal << std::endl;
			res << "commandFileNameExternal is " << commandFileNameExternal << std::endl;
			res << "maxWaitTime is " << maxWaitTime << std::endl;			
			return res.str();
		}
		void TerminalMgr::setInputs()
		{
			/*externalPath = dependencies::matlab::getObjRoot() + "SLEP_wrappers/";
			internalPath = externalPath;

			command = "matlab -nojvm -nodisplay -nosplash -r \"Lasso; exit;\"";

			delimiter = ',';
			inHeaderFiles.push_back({ "8,7,6,5,4,3,2,1,0", "1,2,3,4" });
			inDataFiles.push_back({ { 0, 1, 1, 1 }, { 1, 1, 0, 1 }, { 0, 0, 0, 1 }, { 1, 0, 0, 1 }, { 1, 0, 0, 1 }, { 0, 1, 1, 1 }, { 0, 1, 0, 1 }, { 0, 0, 1, 1 }, { 1, 0, 1, 1 } });
			inDataFiles.push_back({ { 4 }, { 4 }, { 4 }, { 4 }, { 4 }, { 5 }, { 4 }, { 4 }, { 4 } });
			inHeaderFileNames.push_back("input_parms.csv");
			inDataFileNames.push_back("input_X.csv");
			inDataFileNames.push_back("input_Y.csv");
			outHeaderFileNames.push_back("output_log.txt");
			outHeaderFileNames.push_back("output_err.txt");
			outDataFileNames.push_back("output_B.csv");

			commandFileNameInternal = gstd::TerminalMgr::defaultCommandFileName;
			commandFileNameExternal = gstd::TerminalMgr::defaultCommandFileName;
#ifndef _WIN32
			commandFileNameExternal = "./" + commandFileNameExternal;
#endif
			outFileNameInternal = gstd::TerminalMgr::defaultOutFileName;
			outFileNameExternal = gstd::TerminalMgr::defaultOutFileName;
			completeFileNameInternal = gstd::TerminalMgr::defaultCompleteFileName;*/
		}
		bool TerminalMgr::test()
		{
			/*TerminalMgr obj;
			obj.safe = true;
			obj.setInputs();
			try
			{
				obj.run();
			}
			catch (std::exception e)
			{
				obj.reportFailure("Failed while running run()");
				return false;
			}
			std::vector<double> target7 = { 0, 0, 0, 3.111 };
			std::vector<double> target8 = { -0.052, 0.210, 0.210, 3.947 };
			if (!gstd::Linalg::mequals(obj.outDataFiles[0][7], target7, 0.03, false) || !gstd::Linalg::mequals(obj.outDataFiles[0][8], target8, 0.03, false))
			{
				obj.reportFailure("Result of run() is incorrect");
				return false;
			}

			//check whether it guards vs file overwriting
			obj.commandFileNameExternal = obj.commandFileNameExternal + "_OTHER";
			obj.commandFileNameInternal = obj.commandFileNameInternal + "_OTHER";
			obj.outFileNameExternal = obj.outFileNameExternal + "_OTHER";
			obj.outFileNameInternal = obj.outFileNameInternal + "_OTHER";
			std::vector<std::string> killpaths = {
				obj.getFullCommandFileNameInternal(),
				obj.getFullOutFileNameInternal(),
				obj.internalPath + obj.inDataFileNames[0],
				obj.internalPath + obj.inHeaderFileNames[0],
				obj.internalPath + obj.outDataFileNames[0]
			};
			int numKillPaths = killpaths.size();
			for (int i = 0; i < numKillPaths; i++)
			{
				gstd::Writer::w(killpaths[i], "", true);
				try
				{
					gstd::Printer::c("expecting error message ...");
					obj.run();
					obj.reportFailure("Failed to guard against overwriting on killpath " + std::to_string(i));
					return false;
				}
				catch (std::exception e) {}
				obj.cleanup();
			}*/
			return true;
		}
	}
}




