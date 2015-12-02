#include "stdafx.h"

#pragma once
#ifndef TERMINALMGR_H
#define TERMINALMGR_H
#include"standard.h"
#include"Base.h"


namespace msii810161816
{
	namespace gstd
	{
		

		class MSII810161816_GSTD_API TerminalMgr : virtual public Base
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
			gstd::trial<std::vector<std::string> > run();
			static gstd::trial<std::vector<std::string> > exec(std::string command, std::string outFileNameInternal = defaultOutFileName, std::string outFileNameExternal = defaultOutFileName);

		private:
			std::string getFullCommandFileNameInternal();
			std::string getFullOutFileNameInternal();
			void cleanup();

			//Base package
		public:
			virtual void setInputs(); //sets inputs for inputs for quick testing
			virtual bool test(); //test the algorithm
			virtual gstd::TypeName getTypeName(); //get string name of the type of this object
			virtual std::string toString();
		};	

		template<>
		struct TypeNameGetter<TerminalMgr>
		{
			static TypeName get()
			{
				TypeName t;
				t.name = "gstd::TerminalMgr";
				return t;
			}
		};
	}
}
#endif
