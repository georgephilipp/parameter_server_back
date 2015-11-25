#include "mlr_dataParsers.hpp"
#include "gstd/src/Reader.h"
#include "gstd/src/Rand.h"
#include "gstd/src/Timer.h"
#include "gstd/src/File.h"
#include "gstd/src/Writer.h"
#include <petuum_ps_common/include/system_gflags_declare.hpp>

using namespace msii810161816;

namespace mlr
{
	namespace mlrDataParsers
	{

		std::vector<std::vector<float> >readDelimitedFileFast(std::string path, char delim)
		{
			std::ifstream file;
			file.open(path);
			std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			file.close();

			std::vector<std::vector<float> > res;
			int sizeContents = (int)fileContents.size();
			std::vector<float> row;
			int counter = 0;
	
			while (1)
			{
				if (counter >= sizeContents)
					break;

				float val = 0.0F;


				switch(fileContents[counter])
				{
				case '1':
					val += 1;
					counter++;
					break;
				case '2':
					val += 2;
					counter++;
					break;
				}					

				counter++;

				switch (fileContents[counter])
				{
				case '0':
					break;
				case '1':
					val += 0.1F;
					break;
				case '2':
					val += 0.2F;
					break;
				case '3':
					val += 0.3F;
					break;
				case '4':
					val += 0.4F;
					break;
				case '5':
					val += 0.5F;
					break;
				case '6':
					val += 0.6F;
					break;
				case '7':
					val += 0.7F;
					break;
				case '8':
					val += 0.8F;
					break;
				case '9':
					val += 0.9F;
					break;
				default:
					CHECK(false) << "Faulty input file at counter value " << counter << " and file value " << fileContents[counter];
					break;
				}

				counter++;

				switch (fileContents[counter])
				{
				case '0':
					break;
				case '1':
					val += 0.01F;
					break;
				case '2':
					val += 0.02F;
					break;
				case '3':
					val += 0.03F;
					break;
				case '4':
					val += 0.04F;
					break;
				case '5':
					val += 0.05F;
					break;
				case '6':
					val += 0.06F;
					break;
				case '7':
					val += 0.07F;
					break;
				case '8':
					val += 0.08F;
					break;
				case '9':
					val += 0.09F;
					break;
				default:
					CHECK(false) << "Faulty input file at counter value " << counter << " and file value " << fileContents[counter];
					break;
				}

				counter++;

				switch (fileContents[counter])
				{
				case '0':
					break;
				case '1':
					val += 0.001F;
					break;
				case '2':
					val += 0.002F;
					break;
				case '3':
					val += 0.003F;
					break;
				case '4':
					val += 0.004F;
					break;
				case '5':
					val += 0.005F;
					break;
				case '6':
					val += 0.006F;
					break;
				case '7':
					val += 0.007F;
					break;
				case '8':
					val += 0.008F;
					break;
				case '9':
					val += 0.009F;
					break;
				default:
					CHECK(false) << "Faulty input file at counter value " << counter << " and file value " << fileContents[counter];
					break;
				}

				row.push_back(val);

				counter++;

				if (fileContents[counter] != delim)
				{
					res.push_back(row);
					row = std::vector<float>();
				}

				counter ++;
			}

			return res;
		}
		

		std::vector<std::vector<float> > readDelimitedFile(std::string path, char delim)
		{
			std::vector<std::vector<float> > res;
			std::ifstream file;
			gstd::check(gstd::file::open(&file, path), "Failed to open file " + path);
			std::string line = "";
			LOG(INFO) << "starting read loop\n";
			while(std::getline(file, line))
			{
				/*if (file.eof() && line == "")
				{
					file.clear();
					break;
				}*/
				std::vector<float> resLine;
				std::string field;
				std::stringstream lineStream(line);			
				while(std::getline(lineStream, field, delim))
				{
					resLine.push_back(std::stof(field));
				}
				res.push_back(resLine);
			}
			file.clear();
			LOG(INFO) << "finished read loop with " << res.size() << " results and a random data point has " << res[gstd::Rand::i(250)].size() << " elements.\n";
			gstd::check(gstd::file::close(&file), "Failed to close file " + path);
			return res;
		}		

		void custom_imnet
		(
			std::string filePath, 
			int32_t fileSize, 
			int32_t numData,
			int32_t offset,
			std::vector<petuum::ml::AbstractFeature<float>*>* features,
			std::vector<int32_t>* labels
		)
		{
			int numClients = FLAGS_num_clients;
			int clientId = FLAGS_client_id;
			CHECK(numData % (2*numClients) == 0);
			CHECK(offset % 2 == 0);
			int firstDataPoint = (numData / (2*numClients)) * clientId + offset / 2;
			int lastDataPoint = (numData / (2*numClients)) * (clientId + 1) - 1 + offset / 2;
			int firstDataPointFileId = firstDataPoint / fileSize;
			int firstDataPointWithinFileId = firstDataPoint - firstDataPointFileId * fileSize;
			int lastDataPointFileId = lastDataPoint / fileSize;
			int lastDataPointWithinFileId = lastDataPoint - lastDataPointFileId * fileSize;
			for(int i=firstDataPointFileId; i<=lastDataPointFileId; i++)
			{
				int firstPoint = 0;
				int lastPoint = fileSize - 1;
				if(i==firstDataPointFileId)
					firstPoint = firstDataPointWithinFileId;
				if(i==lastDataPointFileId)
					lastPoint = lastDataPointWithinFileId;
				LOG(INFO) << "clientID " << clientId << " starting data point file " << i << " at point " << firstPoint << " and finishing at point " << lastPoint << "\n";
				char* buffer = new char[20];
				sprintf(buffer, "%07d", fileSize);
				std::string fileSizeString(buffer);
				char* buffer2 = new char[20];
				sprintf(buffer2, "%05d", i);
				std::string fileIdString(buffer2);
				std::string personPath = filePath + "/person/imnet." + fileSizeString + "." + fileIdString;
				std::string nonPersonPath = filePath + "/nonPerson/imnet." + fileSizeString + "." + fileIdString;
				gstd::Timer::inst.reset();
				std::vector<std::vector<float> > personDataRaw = readDelimitedFileFast(personPath, ' ');
				std::vector<std::vector<float> > nonPersonDataRaw = readDelimitedFileFast(nonPersonPath, ' ');
				/////////////////////////
				//gstd::Rand::randomize();
				//personDataRaw = gstd::vector::shuffle(personDataRaw);
				//nonPersonDataRaw = gstd::vector::shuffle(nonPersonDataRaw);
				/////////////////////////
				LOG(INFO) << "Part 1: " << gstd::Timer::inst.t(false) << "\n";
				//int_32t dataSize = (int_32t)personDataRaw.size();
				for(int j=firstPoint; j <=lastPoint;j++)
				{
					labels->push_back(1);
					features->push_back(new petuum::ml::DenseFeature<float>(personDataRaw[j]));
					labels->push_back(0);
					features->push_back(new petuum::ml::DenseFeature<float>(nonPersonDataRaw[j]));
					/*petuumFeatVec = new petuum::ml::AbstractFeature<float>(dataSize);
					for(int_32t k = 0;k < dataSize; k++)
						petuumFeatVec.SetFeatureVal(k, nonPersonDataRaw[j][k]);
					features->push_back(petuumFeatVec);*/
				}
				LOG(INFO) << "Finished files in time " << gstd::Timer::inst.t(false) << "\n";
			}
			CHECK((int)features->size() == numData / numClients);
		}

		void printDataSet(std::string path, std::vector<petuum::ml::AbstractFeature<float>*> data, int32_t featureDim)
		{
			std::vector<std::vector<float> > printable;
			for(int i=0;i<(int)data.size(); i++)
			{
				std::vector<float> printableRow;
				for(int j=0;j<featureDim;j++)
					printableRow.push_back(data[i]->GetFeatureVal(j));
				printable.push_back(printableRow);
			}
			gstd::Writer::rs<float>(path, printable, " ", true);
		}
	}
}




























			


