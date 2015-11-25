#include "stdafx.h"

#include "Text.h"
#include <set>
#include <sstream>
#include "Printer.h"
#include "Map.h"

namespace msii810161816
{
	namespace gstd
	{
		namespace text
		{
			const std::vector<char> defaultBlanks = { '(', ')', ' ', ',', ';', ':', '.', '?', '!' };
			const std::map<char, char> upperLower = { { 'A', 'a' }, { 'B', 'b' }, { 'C', 'c' }, { 'D', 'd' }, { 'E', 'e' }, { 'F', 'f' }, { 'G', 'g' }, { 'H', 'h' }, { 'I', 'i' }, { 'J', 'j' }, { 'K', 'k' }, { 'L', 'l' }, { 'M', 'm' }, { 'N', 'n' }, { 'O', 'o' }, { 'P', 'p' }, { 'Q', 'q' }, { 'R', 'r' }, { 'S', 's' }, { 'T', 't' }, { 'U', 'u' }, { 'V', 'v' }, { 'W', 'w' }, { 'X', 'x' }, { 'Y', 'y' }, { 'Z', 'z' }, };
			const std::map<char, char> lowerUpper = { { 'a', 'A' }, { 'b', 'B' }, { 'c', 'C' }, { 'd', 'D' }, { 'e', 'E' }, { 'f', 'F' }, { 'g', 'G' }, { 'h', 'H' }, { 'i', 'I' }, { 'j', 'J' }, { 'k', 'K' }, { 'l', 'L' }, { 'm', 'M' }, { 'n', 'N' }, { 'o', 'O' }, { 'p', 'P' }, { 'q', 'Q' }, { 'r', 'R' }, { 's', 'S' }, { 't', 'T' }, { 'u', 'U' }, { 'v', 'V' }, { 'w', 'W' }, { 'x', 'X' }, { 'y', 'Y' }, { 'z', 'Z' } };
			const std::map<char, bool> letterMap = { { 'A', true }, { 'B', true }, { 'C', true }, { 'D', true }, { 'E', true }, { 'F', true }, { 'G', true }, { 'H', true }, { 'I', true }, { 'J', true }, { 'K', true }, { 'L', true }, { 'M', true }, { 'N', true }, { 'O', true }, { 'P', true }, { 'Q', true }, { 'R', true }, { 'S', true }, { 'T', true }, { 'U', true }, { 'V', true }, { 'W', true }, { 'X', true }, { 'Y', true }, { 'Z', true }, { 'a', true }, { 'b', true }, { 'c', true }, { 'd', true }, { 'e', true }, { 'f', true }, { 'g', true }, { 'h', true }, { 'i', true }, { 'j', true }, { 'k', true }, { 'l', true }, { 'm', true }, { 'n', true }, { 'o', true }, { 'p', true }, { 'q', true }, { 'r', true }, { 's', true }, { 't', true }, { 'u', true }, { 'v', true }, { 'w', true }, { 'x', true }, { 'y', true }, { 'z', true } };
			
			std::string makeUpper(std::string input)
			{
				for (int i = 0; i < (int)input.length(); i++)
				{
					if (lowerUpper.count(input[i]) == 1)
						input[i] = lowerUpper.at(input[i]);
				}
				return input;
			}
			
			std::string makeLower(std::string input)
			{
				for (int i = 0; i < (int)input.length(); i++)
				{
					if (upperLower.count(input[i]) == 1)
						input[i] = upperLower.at(input[i]);
				}
				return input;
			}
			std::map<std::string, std::vector<int> > invert(std::vector<std::string> input, bool killAllCapitals, std::vector<char> blanks)
			{
				std::map<std::string, std::vector<int> > res;
				std::set<char> blanksSet(blanks.begin(), blanks.end());
				for (int i = 0; i < (int)input.size(); i++)
				{
					std::string currentInput = input[i];
					std::stringstream current;
					bool isActive = false;
					std::vector<std::string> words;
					for (int j = 0; j <= (int)currentInput.size(); j++)
					{
						if (j == (int)currentInput.size() || blanksSet.count(currentInput[j]))
						{
							if (isActive)
							{
								std::string word = current.str();
								if (killAllCapitals)
								{
									for (int k = 0; k < (int)word.length(); k++)
									{
										if (upperLower.count(word[k]) == 1)
											word[k] = upperLower.at(word[k]);
									}
								}
								words.push_back(word);
								current.str("");
								isActive = false;
							}
						}
						else
						{
							current << currentInput[j];
							isActive = true;
						}
					}
					for (int k = 0; k < (int)words.size(); k++)
					{
						std::string word = words[k];
						if (res.count(word) == 0)
							res[word] = { i };
						else
							res[word].push_back(i);
					}
				}
				return res;
			}

			bool test()
			{
			    //lowerUpper
				if ((int)lowerUpper.size() != 26)
				{
					gstd::reportFailure("gstd::text::lowerUpper");
					return false;
				}

				//upperLower
				if ((int)upperLower.size() != 26)
				{
					gstd::reportFailure("gstd::text::upperLower");
					return false;
				}

				//letterMap
				if ((int)letterMap.size() != 52)
				{
					gstd::reportFailure("gstd::text::letterMap");
					return false;
				}

				//makeUpper
				{
					std::string input = "asdHWjsWU239 A;@skkK;; aJio";
					std::string target = "ASDHWJSWU239 A;@SKKK;; AJIO";
					if (target != makeUpper(input))
					{
						gstd::reportFailure("gstd::text::makeUpper");
						return false;
					}
				}

				//makeLower
				{
					std::string input = "asdHWjsWU239 A;@skkK;; aJio";
					std::string target = "asdhwjswu239 a;@skkk;; ajio";
					if (target != makeLower(input))
					{
						gstd::reportFailure("gstd::text::makeLower");
						return false;
					}
				}

				//invert
				{
					std::vector<std::string> input = { "I like you.", "You    like: me", "I'm, liked BY you.", "You are.Liked.by.me" };
					std::map<std::string, std::vector<int> > res1;
					res1["I"] = { 0 };
					res1["like"] = { 0, 1 };
					res1["you"] = { 0, 2 };
					res1["You"] = { 1, 3 };
					res1["me"] = { 1, 3 };
					res1["I'm"] = { 2 };
					res1["liked"] = { 2 };
					res1["Liked"] = { 3 };
					res1["BY"] = { 2 };
					res1["are"] = { 3 };
					res1["by"] = { 3 };
					std::map<std::string, std::vector<int> > out1 = invert(input, false);
					if (res1 != out1)
					{
						gstd::reportFailure("gstd::text::invert, test 1");
						return false;
					}
					std::map<std::string, std::vector<int> > res2;
					res2["i"] = { 0 };
					res2["like"] = { 0, 1 };
					res2["you"] = { 0, 1, 2, 3 };
					res2["me"] = { 1, 3 };
					res2["i'm"] = { 2 };
					res2["liked"] = { 2, 3 };
					res2["are"] = { 3 };
					res2["by"] = { 2, 3 };
					std::map<std::string, std::vector<int> > out2 = invert(input, true);
					if (res2 != out2)
					{
						gstd::reportFailure("gstd::text::invert, test 2");
						return false;
					}
					std::map<std::string, std::vector<int> > res3;
					res3["I"] = { 0, 2 };
					res3["like"] = { 0 };
					res3["like:"] = { 1 };
					res3["you"] = { 0, 2 };
					res3["You"] = { 1, 3 };
					res3["me"] = { 1, 3 };
					res3["m"] = { 2 };
					res3["liked"] = { 2 };
					res3["Liked"] = { 3 };
					res3["BY"] = { 2 };
					res3["are"] = { 3 };
					res3["by"] = { 3 };
					std::map<std::string, std::vector<int> > out3 = invert(input, false, { ' ', '\'', '.', ',' });
					if (res3 != out3)
					{
						gstd::reportFailure("gstd::text::invert, test 3");
						return false;
					}
				}	

				return true;
			}
		}
	}
}
