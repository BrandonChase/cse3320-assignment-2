#include <string>
#include <deque>
#include <iostream>
#include <iomanip> //used for testing outputting numbers
#include <limits> //used for testing outputting numbers
#include <fstream>

using namespace std;

const string FILENAME = "../earthquakes.csv";

int timeSort(int num_processes, int num_lines);
deque<double> loadLatitudes(string filename);
deque<string> splitString(string content, string delimeters);
void saveDataChunk(string filename, deque<double> content, int start_index, int end_index);

int main(int argc, const char * argv[])
{
	deque<double> latitudes = loadLatitudes(FILENAME);
	for(int i = 0; i < 10; i++)
	{
		cout << setprecision (std::numeric_limits<double>::digits10 + 1) << latitudes.at(i) << endl;
	}

	saveDataChunk("testchunk", latitudes, 0, 99);
	int temp;
	cin >> temp;
	//ask if user wants to do custom parameters for sort and get parameters
	//if user specifies, do their desired operation
	//if not, do for 1, 2, 4, and 10 processes
	//load data as vector of strings
	//call the time to sort and print out results
	return 0;
}

/*
Desctiption: 	Times how long it takes to use a specified number of processes to do a parallel sort on some data
Parameters: 	num_processes is the number of processes that will handle the sort
Parameters: 	num_lines is how much of the data is to be sorted, by default it is whole file unless another value is specified by the user
Returns: 		the time in seconds it takes to do the sort
*/
int timeSort(int num_processes, int num_lines)
{
	//start timer
	//split data into chunks and save chunks
	//do forks to split into number of processes
	//perform the sort on each chunk in each process
	//read sorted chunks into lists
	//merge chunks using list of lists and doing top of deck of cards process
	//stop timer and figure out how long it took
	//return time span
}

// Loads the latitiudes of earthquakes (doubles) from a file into a deque. 
deque<double> loadLatitudes(string filename)
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
		string line;
		//skip first line that contains labels
		getline(file, line);
		while(getline(file, line))
		{
			//split line by commas
			deque<string> words = splitString(line, ",");

			//add second column (latitude) into list after converting to double
			result.push_back(stod(words.at(1)));
		}
	}

	file.close();
	return result;
}

//splits a string by a character delimeter and returns list of strings after being split apart
deque<string> splitString(string content, string delimeters)
{
	deque<string> result;
	string word = ""; 

	for(char c : content)
	{
		if(delimeters.find(c) != string::npos) //if char is a delimeter, add currently build word 
		{
			if(word != "") //dont want to add empty words
			{
				result.push_back(word);
				word = ""; //reset word
			}
		}

		else
		{
			word += c;
		}
	}

	//add last remaining word since content may not end on a delimeter
	if(word != "")
	{
		result.push_back(word);
	}
	return result;
}

// Saves a chunk of data from a list of data specified from a starting point to an ending point in list
void saveDataChunk(string filename, deque<double> content, int start_index, int end_index)
{
	//go from start index to end index and write data from that span into file, one element per line
	ofstream file;
	file.open(filename);
	for(int i = start_index; i <= end_index; i++) //<= end_index because inclusive
	{
		file << setprecision (std::numeric_limits<double>::digits10 + 1) << content.at(i) << endl;
	}
	file.close();
}

