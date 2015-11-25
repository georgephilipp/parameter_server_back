#include "stdafx.h"

#include "File.h"
#include "Printer.h"
#include "Reader.h"
#include "Timer.h"
#include "Writer.h"
#include "Dependencies.h"

namespace msii810161816
{
	namespace gstd
	{
		namespace file
		{
			int fileOpenBufferReps = 1;
			double fileOpenBufferDelay = 0.1;

			std::string joinpaths(std::string path1, std::string path2)
			{
				if (path2.length() > 0)
					gstd::check(path2.at(0) != '/' && path2.at(0) != '\\', "path2 does not seem to be a relative path. Please check!");
				if (path1.at(path1.length() - 1) == '/' || path1.at(path1.length() - 1) == '\\')
				{
					return path1 + path2;
				}
				else
					return path1 + "/" + path2;
			}
			bool open(std::ifstream *stream, std::string location, int reps /*= fileOpenBufferReps*/, double delay /*= fileOpenBufferDelay*/)
			{
				for (int i = 0; i < reps; i++)
				{
					stream->open(location);
					if (stream->fail())
					{
						stream->clear();
						if (i + 1 == reps)
							return false;
						else
							gstd::Timer::sleep(delay);
					}
					else
						break;
				}
				return true;
			}

			bool writeopen(std::ofstream *stream, std::string location, std::ios_base::openmode mode, int reps /*= fileOpenBufferReps*/, double delay /*= fileOpenBufferDelay*/)
			{
				for (int i = 0; i < reps; i++)
				{
					stream->open(location, mode);
					if (stream->fail())
					{
						stream->clear();
						if (i + 1 == reps)
							return false;
						else
							gstd::Timer::sleep(delay);
					}
					else
						break;
				}
				return true;
			}
			bool close(std::ifstream *stream, int reps /*= fileOpenBufferReps*/, double delay /*= fileOpenBufferDelay*/)
			{
				for (int i = 0; i < reps; i++)
				{
					stream->close();
					if (stream->fail())
					{
						stream->clear();
						if (i + 1 == reps)
							return false;
						else
							gstd::Timer::sleep(delay);
					}
					else
						break;
				}
				return true;
			}
			bool writeclose(std::ofstream *stream, int reps /*= fileOpenBufferReps*/, double delay /*= fileOpenBufferDelay*/)
			{
				for (int i = 0; i < reps; i++)
				{
					stream->close();
					if (stream->fail())
					{
						stream->clear();
						if (i + 1 == reps)
							return false;
						else
							gstd::Timer::sleep(delay);
					}
					else
						break;
				}
				return true;
			}
			std::vector<double> existsProbabilities(std::vector<std::string> locations, int reps /*= fileOpenBufferReps*/, double delay /*= fileOpenBufferDelay*/)
			{
				int size = (int)locations.size();
				double increment = 1 / (double)reps;
				std::vector<double> probabilities(size, 0);
				

				for (int i = 0; i < reps; i++)
				{
					//we regenerate the streams instead of closing them
					std::vector<std::ifstream> streams(size);
					std::vector<bool> success(size);
					for (int j = 0; j < size; j++)
					{
						success[j] = open(&streams[j], locations[j], 1, 0);
						if (success[j])
						{
							probabilities[j] += increment;
						}
					}
					//internal closing loop
					/*{
						std::vector<bool> closingSuccess = success;
						for (int j = 0; j < size; j++)
							closingSuccess[j] = !closingSuccess[j];
						for (int k = 0; k < reps; k++)
						{
							bool failsRemain = false;
							for (int j = 0; j < size; j++)
							{
								if (!closingSuccess[j])
								{
									streams[j].close();
									if (streams[j].fail())
									{
										streams[j].clear();
										failsRemain = true;
									}
									else
										closingSuccess[j] = true;
								}
							}
							if (failsRemain)
							{
								if (k + 1 < reps)
									gstd::Timer::sleep(delay);
								else
									gstd::error("cannot procede with file existence check due to closing error");
							}	
						}			
					}*/
					//end closing loop
					if (i + 1 < reps)
					    gstd::Timer::sleep(delay);
				}
				return probabilities;
			}
			std::vector<bool> exists(std::vector<std::string> locations, int reps /*= fileOpenBufferReps*/, double delay /*= fileOpenBufferDelay*/)
			{
				int size = (int)locations.size();
				std::vector<bool> res(size, false);
				std::vector<double> probabilities = existsProbabilities(locations, reps, delay);
				for (int i = 0; i < size; i++)
				{
					if (probabilities[i] < 0.5)
						res[i] = false;
					else
						res[i] = true;
				}
				return res;
			}

			std::vector<bool> remove(std::vector<std::string> paths, bool enforceBefore, bool enforceAfter, int reps /*= fileOpenBufferReps*/, double delay /*= fileOpenBufferDelay*/)
			{
				int size = paths.size();
				if (enforceBefore)
				{
					std::vector<bool> existsBefore = exists(paths, reps, delay);
					for (int i = 0; i < size; i++)
						gstd::check(existsBefore[i], "cannot remove file that does not exist : filname is " + paths[i]);
				}
				for (int i = 0; i < size; i++)
					::remove(paths[i].c_str());
				if (!enforceAfter)
 					return std::vector<bool>(size, true);
				std::vector<bool> remains(size, true);
				for (int j = 0; j < reps; j++)
				{
					if (j > 0)
					{
						for (int i = 0; i < size; i++)
						{
							if (remains[i])
								::remove(paths[i].c_str());
						}
					}
					std::vector<std::string> remainingPaths;
					for (int i = 0; i < size; i++)
					{
						if (remains[i])
							remainingPaths.push_back(paths[i]);
					}
					std::vector<bool> remainingPathRemains = gstd::file::exists(remainingPaths, 1, 0);
					bool somethingRemains = false;
					for (int i = 0; i < size; i++)
					{
						int index = 0;
						if (remains[i])
						{
							remains[i] = remainingPathRemains[index];
							if (remains[i])
								somethingRemains = true;
							index++;
						}
					}
					if (somethingRemains && j + 1 < reps)
						gstd::Timer::sleep(delay);
					else
						return remains;
				}
				return std::vector<bool>(size, false);
			}

			std::string getTestFileName()
			{
				return gstd::dependencies::cpp::getHome() + "libs/gstd/src/FileTestFile.txt";
			}

			bool isAcceptableProb(std::vector<double> probs, std::vector<bool> results)
			{
				double tolerance = 0.2;

				if (probs.size() != results.size())
					return false;
				for (int i = 0; i < (int)probs.size(); i++)
				{
					if (results[i])
					{
						if (probs[i] > 1.000001 || probs[i] < 1 - tolerance)
							return false;
					}
					else
					{
						if (probs[i] < -0.000001 || probs[i] > tolerance)
							return false;
					}
				}
				return true;
			}

			bool test()
			{
				gstd::Printer::c("This is a test for group of functions gstd::file");

				//joinpaths
				{
					std::string start1 = "/bla/bla";
					std::string start2 = "bling";
					std::string target = "/bla/bla/bling";
					if (joinpaths(start1, start2) != target)
					{
						gstd::reportFailure("gstd::file::joinpaths, test 1");
						return false;
					}
					start1 = "/bla/bla/";
					target = "/bla/bla/bling";
					if (joinpaths(start1, start2) != target)
					{
						gstd::reportFailure("gstd::file::joinpaths, test 2");
						return false;
					}
					start1 = "/bla/bla\\";
					target = "/bla/bla\\bling";
					if (joinpaths(start1, start2) != target)
					{
						gstd::reportFailure("gstd::file::joinpaths, test 3");
						return false;
					}
					start2 = "/bling";
					try
					{
						gstd::Printer::c("Expecting error message ...");
						joinpaths(start1, start2);
						gstd::reportFailure("gstd::file::joinpaths, test 4");
						return false;
					}
					catch (std::exception e) {}
					start2 = "\\bling";
					try
					{
						gstd::Printer::c("Expecting error message ...");
						joinpaths(start1, start2);
						gstd::reportFailure("gstd::file::joinpaths, test 5");
						return false;
					}
					catch (std::exception e) {}
				}

				//opening, closing and file existence
				std::ifstream stream;
				std::ofstream writestream;
				std::string testFileName = getTestFileName();
				if (open(&stream, testFileName))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 1");
					return false;
				}
				if (close(&stream))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 2");
					return false;
				}
				if (writeclose(&writestream))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 4");
					return false;
				}
				if (exists({ testFileName, "This is not a valid location" }) != std::vector<bool>({ false, false }))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 5");
					return false;
				}
				if (exists({ testFileName, "This is not a valid location" }, 7, 0.01) != std::vector<bool>({ false, false }))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 5");
					return false;
				}
				if (!isAcceptableProb(existsProbabilities({ testFileName, "This is not a valid location" }),std::vector<bool>({ false, false })))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 5");
					return false;
				}
				if (!isAcceptableProb(existsProbabilities({ testFileName, "This is not a valid location" }, 50, 0.01), std::vector<bool>({ false, false })))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 5");
					return false;
				}
				if (!writeopen(&writestream, testFileName, std::ios_base::trunc))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 5");
					return false;
				}
				if (writeopen(&writestream, testFileName, std::ios_base::trunc))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 5");
					return false;
				}
				writeclose(&writestream);
				if (exists({ testFileName, "This is not a valid location", testFileName }) != std::vector<bool>({ true, false, true }))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 12");
					return false;
				}
				if (exists({ testFileName, "This is not a valid location", testFileName }, 5, 0.1) != std::vector<bool>({ true, false, true }))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 12");
					return false;
				}
				if (!isAcceptableProb(existsProbabilities({ testFileName, "This is not a valid location", testFileName }), std::vector<bool>({ true, false, true })))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 12");
					return false;
				}
				if (!isAcceptableProb(existsProbabilities({ testFileName, "This is not a valid location", testFileName }, 50, 0.01), std::vector<bool>({ true, false, true })))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 12");
					return false;
				}
				//gstd::Writer::w(testFileName, "bling", false, std::ios::trunc);
				if (!open(&stream, testFileName))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 6");
					return false;
				}
				if (open(&stream, testFileName))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 7");
					return false;
				}
				if (!close(&stream))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 8");
					return false;
				}
				if (!writeopen(&writestream, testFileName, std::ios_base::trunc))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 9");
					return false;
				}
				if (writeopen(&writestream, testFileName, std::ios_base::trunc))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 10");
					return false;
				}
				if (!writeclose(&writestream))
				{
					gstd::reportFailure("gstd::file::open/close/exists, test 11");
					return false;
				}
				

				//remove
				{
					std::string testFileName = getTestFileName();
					if (!remove({ testFileName }, false, false)[0])
					{
						gstd::reportFailure("gstd::file::remove, test 1");
						return false;
					}
					if (exists({ testFileName })[0])
					{
						gstd::reportFailure("gstd::file::remove, test 2");
						return false;
					}
					try
					{
						gstd::Printer::c("Expecting error message ...");
						remove({ testFileName }, true, false);
						gstd::reportFailure("gstd::file::remove, test 3");
						return false;
					}
					catch (std::exception e) {}
					if (!remove({ testFileName }, false, false)[0])
					{
						gstd::reportFailure("gstd::file::remove, test 4");
						return false;
					}
					gstd::Writer::w(testFileName, "bling", false, std::ios::trunc);
					try
					{
						remove({ testFileName, testFileName, testFileName }, true, true);
						gstd::check(!gstd::file::exists({ testFileName })[0], "");
					}
					catch (std::exception e)
					{
						gstd::reportFailure("gstd::file::remove, test 5");
						return false;
					}
					if (exists({ testFileName })[0])
					{
						gstd::reportFailure("gstd::file::remove, test 6");
						return false;
					}
				}
				return true;
			}


		}
	}
}
