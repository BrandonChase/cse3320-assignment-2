#include <string>
#include <deque>
#include <iostream>
#include <iomanip> 
#include <limits> 
#include <fstream>
#include <chrono>
#include <sys/wait.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>

using namespace std;

//filenames assume running from part directory
const string INPUT_FILENAME = "../earthquakes.csv";
const string TEMP_FILENAME = "latitudes";
const string SORT_PROGRAM_PATH = "./bin/sort";

long timeSort(deque<double> content, int num_lines, int num_processes);
deque<double> loadLatitudes(string filename);
deque<string> splitString(string content, string delimeters);
void saveDataChunk(string filename, deque<double> content, int start_index, int end_index);
void splitAndSave(string filename, deque<double> content, int num_of_lines, int num_of_chunks);
void performParallelSort(int num_processes);

int main(int argc, const char * argv[])
{
	deque<double> data;
	data.push_back(9);
	data.push_back(8);
	data.push_back(7);
	data.push_back(6);
	data.push_back(5);
	data.push_back(4);
	data.push_back(3);
	data.push_back(2);
	data.push_back(1);
	data.push_back(0);
	timeSort(data, data.size(), 2);
	//ask if user wants to do custom parameters for sort and get parameters
	//if user specifies, do their desired operation
	//if not, do for 1, 2, 4, and 10 processes
	//load data as vector of strings
	//call the time to sort and print out results
	//delete temp files
	//save sorted que

	char temp;
	cin >> temp;
	return 0;
}

/*
Desctiption: 	Times how long it takes to use a specified number of processes to do a parallel sort on some data
Parameters: 	num_processes is the number of processes that will handle the sort
Parameters: 	num_lines is how much of the data is to be sorted, by default it is whole file unless another value is specified by the user
Returns: 		the time in seconds it takes to do the sort
*/
long timeSort(deque<double> content, int num_lines, int num_processes)
{
	//start timer
	auto start = chrono::high_resolution_clock::now();

	//split data into chunks and save chunks
	splitAndSave(TEMP_FILENAME, content, num_lines, num_processes);

	performParallelSort(num_processes);

	//read sorted chunks into lists
	//merge chunks using list of lists and doing top of deck of cards process

	//stop timer and return how long it took
	auto end = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(end-start).count();
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

// Saves a chunk of data from a list of data specified from a starting point to an ending point (inclusive) in list
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

// Splits given deque into equal length sections and saves the sections into files
// ex: saves 3 chunks to filename0, filename1, filename2
void splitAndSave(string filename, deque<double> content, int num_of_lines, int num_of_chunks)
{
	assert(num_of_lines >= 0 && num_of_lines <= content.size());
	
	int lines_per_chunk = num_of_lines / num_of_chunks;

	for(int i = 0; i < num_of_chunks; i++)
	{
		string chunk_filename = filename + to_string(i);
		int starting_index = i * lines_per_chunk;

		int ending_index;
		if(i == num_of_chunks - 1) //saving last chunk
		{
			/*
				If content isn't divided evenly, ie num lines % num chunks != 0, include all of last chunk that would otherwide be cut off.	
				Set to content.size() - 1 b/c end_index is inclusive in saveChunk function.
			*/
			ending_index = num_of_lines - 1; 
		}

		else
		{
			ending_index = starting_index + lines_per_chunk - 1;
		}

		saveDataChunk(chunk_filename, content, starting_index, ending_index);
	}
}

void performParallelSort(int num_processes)
{
	//do forks to split into number of processes
	for(int i = 0; i < num_processes; i++)
	{
		pid_t pid = fork();
		if(pid == 0)
		{
			//in child process
			string filename = TEMP_FILENAME+to_string(i);
			//perform sort on ith chunk of data

			char* c_path = new char[SORT_PROGRAM_PATH.length() + 1];
			strcpy(c_path, SORT_PROGRAM_PATH.c_str());
			char* c_filename = new char[filename.length() + 1];
			strcpy(c_filename, filename.c_str());

			char *const args[3] = {c_path, c_filename, NULL};
			execv(args[0], args);
			exit(0); //end child process once sort is complete
		}

		else if(pid < 0) //if fork failed
		{
			cerr << "Failure to fork()." << endl;
		}

		//parent process will continue to create more children processes while child processes do sorts
	}

	//wait for all children to finish sorting
	while(wait(NULL) > 0);
}