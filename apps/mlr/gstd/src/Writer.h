#include "stdafx.h"

#pragma once

#include"standard.h"
#include"Base.h"
#include <iostream>
#include <fstream>

namespace msii810161816
{
	namespace gstd
	{
		class MSII810161816_GSTD_API Writer : virtual public Base
		{
		public:
			Writer();
			~Writer();

			//inputs
			std::string location;

			//options
			std::string delimiter;
			std::ios_base::openmode mode;

			//actions
			void open();
			void close();
			bool isOpen();

			template<typename type>
			void write(type input)
			{
				gstd::check(fileIsOpen, "must open file before writing");
				file << input;
			}

			template<typename type>
			void line(type input)
			{
				gstd::check(fileIsOpen, "must open file before writing");
				file << input << "\n";
			}

			template<typename type>
			void row(std::vector<type> input, bool newline = true)
			{
				gstd::check(fileIsOpen, "must open file before writing");
				int size = input.size();
				for (int i = 0; i < size; i++)
				{
					file << input[i];
					if (i + 1 != size)
						file << delimiter;
				}
				if (newline)
    				file << "\n";
			}

			template<typename type>
			static void w(std::string location, type input, bool safe, std::ios_base::openmode mode = std::ios::trunc)
			{
				Writer writer;
				writer.location = location;
				writer.safe = safe;
				writer.mode = mode;
				writer.open();
				writer.write<type>(input);
				writer.close();
			}

			template<typename type>
			static void l(std::string location, type input, bool safe, std::ios_base::openmode mode = std::ios::trunc)
			{
				Writer writer;
				writer.location = location;
				writer.safe = safe;
				writer.mode = mode;
				writer.open();
				writer.line<type>(input);
				writer.close();
			}

			template<typename type>
			static void ls(std::string location, std::vector<type> input, bool safe, std::ios_base::openmode mode = std::ios::trunc)
			{
				Writer writer;
				writer.location = location;
				writer.safe = safe;
				writer.mode = mode;
				writer.open();
				int size = input.size();
				for (int i = 0; i < size; i++)
			        writer.line<type>(input[i]);
				writer.close();
			}

			template<typename type>
			static void rs(std::string location, std::vector<std::vector<type> > input, std::string delimiter, bool safe, std::ios_base::openmode mode = std::ios::trunc)
			{
				Writer writer;
				writer.location = location;
				writer.delimiter = delimiter;
				writer.safe = safe;
				writer.mode = mode;
				writer.open();
				int size = input.size();
				for (int i = 0; i < size; i++)
			    	writer.row<type>(input[i], true);
				writer.close();
			}

		protected:
			std::ofstream file;
			bool fileIsOpen;
	    private:
			bool openInner();

			//Base Package    
		public:
			virtual gstd::TypeName getTypeName();
			virtual void setInputs();
			virtual bool test();
			virtual std::string toString();
        private:
			std::string getTestFileName(); //to run the test
			std::vector<std::string> readTestFile(); //to run the test
		};

	}
}

namespace msii810161816
{
	namespace gstd
	{
		template<>
		struct TypeNameGetter<Writer>
		{
			static TypeName get()
			{
				TypeName t;
				t.name = "gstd::Writer";
				return t;
			}
		};
	}
}