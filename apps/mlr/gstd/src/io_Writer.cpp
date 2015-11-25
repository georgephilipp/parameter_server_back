#include "stdafx.h"

#include "Writer.h"
#include "ex.h"
#include "Printer.h"
#include "Reader.h"
#include "Timer.h"
#include "File.h"
#include "Dependencies.h"

namespace msii810161816
{
	namespace gstd
	{
		Writer::Writer()
		{
			delimiter = "";
			fileIsOpen = false;
			mode = std::ios::trunc;
			
			//safe is enabled by default
			safe = true;
		}

		Writer::~Writer()
		{
			if (fileIsOpen)
				close();
		}

		void Writer::open()
		{
			gstd::check(!fileIsOpen, "cannot open file that is already open");
			if (safe)
				gstd::check(!gstd::file::exists({ location })[0], "cannot write to location with existing file in safe mode");
			gstd::check(gstd::file::writeopen(&file, location, mode), "opening failed at location " + location);
			fileIsOpen = true;
		}

		void Writer::close()
		{
			gstd::check(fileIsOpen, "cannot close file that is not open");
			gstd::check(gstd::file::writeclose(&file), "closing failed at location " + location);
			fileIsOpen = false;
		}

		bool Writer::isOpen()
		{
			return fileIsOpen;
		}

		//Base Package
		TypeName Writer::getTypeName()
		{
			return gstd::TypeNameGetter<Writer>::get();
		}
		std::string Writer::getTestFileName()
		{
			return dependencies::cpp::getHome() + "libs/gstd/src/WriterTestFile.txt";
		}
		void Writer::setInputs()
		{
			location = getTestFileName();
			delimiter = ",";
		}
		bool Writer::test()
		{
			Writer w;
			w.setInputs();
			std::string testFileName = w.location;
			w.safe = false;
			try
			{
				w.open();
				w.close();
				w.open();
			}
			catch (std::exception e) 
			{
				w.reportFailure("Failed test 1");
				return false;
			}
			try
			{
				gstd::Printer::c("Expecting error message");
				w.open();
				w.reportFailure("Failed test 2");
				return false;
			}
			catch (std::exception e){}
			w.close();
			try
			{
				gstd::Printer::c("Expecting error message");
				w.close();
				w.reportFailure("Failed test 3");
				return false;
			}
			catch (std::exception e){}
			w.safe = true;
			try
			{
				gstd::Printer::c("Expecting error message");
				w.open();
				w.reportFailure("Failed test 4");
				return false;
			}
			catch (std::exception e){}
			w.safe = false;	
			try
			{
				gstd::Printer::c("Expecting error message");
				w.write("");
				w.reportFailure("Failed test 5");
				return false;
			}
			catch (std::exception e){}
			try
			{
				gstd::Printer::c("Expecting error message");
				w.line("");
				w.reportFailure("Failed test 6");
				return false;
			}
			catch (std::exception e){}
			try
			{
				gstd::Printer::c("Expecting error message");
				w.row<std::string>({ "" });
				w.reportFailure("Failed test 7");
				return false;
			}
			catch (std::exception e){}
			w.open();
			w.write("1");
			w.line("2");
			w.row<int>({ 3, 4 });
			w.close();
			std::vector<std::string> contents = gstd::Reader::ls(testFileName);
			if (contents != std::vector<std::string>({ "12", "3,4" }))
			{
				w.reportFailure("Failed test 8");
				return false;
			}
			gstd::file::remove({ testFileName }, true, true);
			Writer::w(testFileName, "bling", true);
			contents = gstd::Reader::ls(testFileName);
			if (contents != std::vector<std::string>({ "bling" }))
			{
				w.reportFailure("Failed test 9");
				return false;
			}
			try
			{
				gstd::Printer::c("Expecting error message");
				Writer::w(testFileName, "bling", true);
				w.reportFailure("Failed test 10");
				return false;
			}
			catch (std::exception e) {}
			Writer::l(testFileName, "bling", false);
			if (gstd::Reader::ls(testFileName) != std::vector<std::string>({ "bling" }))
			{
				w.reportFailure("Failed test 11");
				return false;
			}
			Writer::ls<int>(testFileName, { 1, 2 }, false, std::ios::ate|std::ios::in);
			if (gstd::Reader::ls(testFileName) != std::vector<std::string>({ "bling", "1", "2" }))
			{
				w.reportFailure("Failed test 12");
				return false;
			}
			Writer::rs<double>(testFileName, { { 1, 1.1 }, { 2.123 } }, ",", false, std::ios::app);
			if (gstd::Reader::ls(testFileName) != std::vector<std::string>({ "bling", "1", "2", "1,1.1", "2.123" }))
			{
				w.reportFailure("Failed test 13");
				return false;
			}
			return true;
		}
		std::string Writer::toString()
		{
			std::stringstream res;
			res << "This is gstd::writer" << std::endl;
			res << "location: " << location << std::endl;
			res << "delimiter: " << delimiter << std::endl;
			return res.str();
		}
	}
}
