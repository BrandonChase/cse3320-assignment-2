#include <string>
#include <deque>
#include <iostream>
#include <iomanip> //used for testing outputting numbers
#include <limits> //used for testing outputting numbers
#include <fstream>
#include <chrono>

#include <assert.h>
#include <unistd.h>

using namespace std;

deque<double> loadContent(string filename);
void saveContent(string filename, deque<double> data);
deque<double> performInsertionSortOn(deque<double> data);
void insertInto(deque<double>& data, double item);

int main(int argc, const char * argv[])
{
	string filename;
	if(argc > 0) //ensure filepath was passed as argument
	{
		filename = argv[1]; //to get execv working, for some reason path of executable had to be passed as argument so second argument is filename
	}
	deque<double> data = loadContent(filename);
	deque<double> sorted_data = performInsertionSortOn(data);
	saveContent(filename, sorted_data);
	return 0;
}

deque<double> loadContent(string filename)
{
	deque<double> result;
	//open file to read
	ifstream file;
	file.open(filename);

	if(!file) //check for errors opening file
	{
		cerr << "Unable to open file " << filename << endl;
    	exit(1);   // call system to stop
	}

	else 
	{
		double num;
		while(file >> num)
		{
			result.push_back(num);
		}
	}

	file.close();
	return result;
}

void saveContent(string filename, deque<double> data)
{
	ofstream file;
	file.open(filename);
	for(int i = 0; i < data.size(); i++)
	{
		file << setprecision (std::numeric_limits<double>::digits10 + 1) << data.at(i) << endl;
	}
	file.close();
}

deque<double> performInsertionSortOn(deque<double> data)
{
	deque<double> sorted_result;
	for(double num : data)
	{
		insertInto(sorted_result, num);
	}

	return sorted_result;
}

void insertInto(deque<double>& data, double item)
{
	if(data.size() == 0)
	{
		data.push_back(item);
	}

	else
	{
		for(int i = 0; i < data.size(); i++)
		{
			//if found place in deque where item is no longer bigger than element, insert in deque before that element to keep deque sorted and exit loop
			if(item <= data.at(i))
			{
				data.insert(data.begin() + i, item);
				break;
			}

			//if reached end without inserting, item is bigger than everything and insert at end
			else if(i == data.size() - 1) 
			{
				data.push_back(item);
				break; //break out of loop here b/c if insert at end, then list size grows by one and loop 
						//actually goes again and then does first if insert without this break
			}
		}
	}
}