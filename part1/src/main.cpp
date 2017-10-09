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
#include <stdio.h>

using namespace std;

//filenames assume running from part1 directory
const string INPUT_FILENAME = "../earthquakes.csv";
const string TEMP_FILENAME = "latitudes";
const string SORT_PROGRAM_PATH = "./bin/sort";
const int nums_of_processes[] = {1, 2, 4, 10};

long timeSort(deque<double> content, int num_lines, int num_processes);
deque<double> loadLatitudes(string filename);
deque<string> splitString(string content, string delimeters);
void saveDataChunk(string filename, deque<double> content, int start_index, int end_index);
void splitAndSave(string filename, deque<double> content, int num_of_lines, int num_of_chunks);
void performParallelSort(int num_processes);
deque<double> mergeDataChunks(deque<deque<double>> chunks);
deque<double> loadContent(string filename);
void saveContent(string filename, deque<double> data);
void removeTempFiles(int num_processes, string filename);

int main(int argc, const char * argv[])
{
	
	deque<double> latitudes = loadLatitudes(INPUT_FILENAME);
	bool wants_to_go_again;
	do
	{
		//ask if user wants to do custom parameters for sort and get parameters
		cout << "Do you want to enter custom parameters? [y]/[n], [s] for special mode: ";
		char input;
		cin >> input;
		if(input == 'y') //custom behaivor
		{
			int lines, processes;
			do
			{
				cin.clear();
        		cin.ignore(numeric_limits<streamsize>::max(), '\n');
        		
				cout << "How many lines of latitude? [9736 max]: ";
				cin >> lines;
			} while(cin.fail());

			do
			{
				cin.clear();
        		cin.ignore(numeric_limits<streamsize>::max(), '\n');
        		
				cout << "How many processes to run sort on?: ";
				cin >> processes;
			} while(cin.fail());

			assert(lines > 0 && processes > 0);
			if(lines > latitudes.size()) { lines = 9736; }
			cout << "Performing Sort...\n" << endl;
			cout << "Customized sort took " << timeSort(latitudes, lines, processes) / 1000.0 << "s using " << processes << " process(es)." << endl;
			removeTempFiles(processes, TEMP_FILENAME);
		}
		else if(input == 's')//special mode
		{
			deque<double> data;
			int lines, processes;
			do
			{
				cin.clear();
        		cin.ignore(numeric_limits<streamsize>::max(), '\n');
        		
				cout << "How many numbers to sort?: ";
				cin >> lines;
			} while(cin.fail());

			do
			{
				cin.clear();
        		cin.ignore(numeric_limits<streamsize>::max(), '\n');
        		
				cout << "How many processes to run sort on?: ";
				cin >> processes;
			} while(cin.fail());

			for(int i = 0; i < lines / 2; i++) { data.push_front(i); data.push_front(-i);}
			assert(lines > 0 && processes > 0);
			cout << "Performing Sort...\n" << endl;
			cout << "Customized sort took " << timeSort(data, lines, processes) / 1000.0 << "s using " << processes << " process(es)." << endl;
			removeTempFiles(processes, TEMP_FILENAME);
		}
		else //predefined behaivor
		{
			cout << "Performing Sort...\n" << endl;
			for(int num_of_processes : nums_of_processes)
			{
				cout << "Sort took " << timeSort(latitudes, latitudes.size(), num_of_processes) / 1000.0 << "s using " << num_of_processes << " process(es)." << endl;
				removeTempFiles(num_of_processes, TEMP_FILENAME);
			}
		}

		cout << "Go again? [y]/[n]: ";
		cin >> input;
		if(input == 'y') { wants_to_go_again = true; }
		else { wants_to_go_again = false; }

	}while(wants_to_go_again);	
	//delete temp files
	cout << endl << "Result of sort saved to sorted.txt" << endl;
	return 0;
}

void removeTempFiles(int num_processes, string filename)
{
	for(int i = 0; i < num_processes; i++)
	{
		string temp_filename = filename + to_string(i);
		if(remove(temp_filename.c_str( )) !=0)
		{
           cerr << "Failed to remove file: " << temp_filename << endl;
		}
	}
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
	deque<deque<double>> chunks;
	for(int i = 0; i < num_processes; i++)
	{
		string chunk_filename = TEMP_FILENAME + to_string(i);
		chunks.push_back(loadContent(chunk_filename));
	}

	//merge chunks using list of lists and doing top of deck of cards process
	deque<double> sorted_latitudes = mergeDataChunks(chunks);
	//save sorted list to file?
	saveContent("sorted.txt", sorted_latitudes);
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

// Loads the list of doubles in file and returns as deque of doubles. Used in this project for loading sorted chunks
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

//creates child processes that each read their own file, perform an insertion on the data, and saves sorted data back to same file.
//then waits for all children to finish 
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

//takes a list of chunks and performs a merge sort on them to return one chunk that is sorted
//uses technique of comparing top card of each sorted deck and taking smallest card out of all of them
//and adding it to final deck. perform until all decks empty
deque<double> mergeDataChunks(deque<deque<double>> chunks)
{
	assert(chunks.size() > 0);
	deque<double> sorted_result;
		
	bool sort_complete = true;
	double arbitrary_large = numeric_limits<double>::max();
	do
	{
		//RESET VALUES 
		double min_value = arbitrary_large;
		int min_value_index; //chunk number that contains smallest value
		//all_chunks_empty = true; remove
		sort_complete = true;

		//Get smallest value of all chunks
		for(int i = 0; i < chunks.size(); i++)
		{
			//deque<double> current_chunk = chunks.at(i); //remove bc might be faster
			if(!chunks.at(i).empty())
			{
				double current_chunk_smallest = chunks.at(i).front(); //chunk sorted ascending order so smallest on top
				if(current_chunk_smallest <= min_value)
				{
					min_value = current_chunk_smallest; 
					min_value_index = i;
				}
			}
		}

		if(min_value != arbitrary_large) //no new smallest was set
		{
			sort_complete = false;
			sorted_result.push_back(min_value); //add smallest to sorted list
			chunks.at(min_value_index).pop_front(); //remove smallest from chunks
		}

		} while (!sort_complete);
	return sorted_result;//todo
}

void saveContent(string filename, deque<double> data)
{
	ofstream file;
	file.open(filename);
	for(int i = 0; i < data.size(); i++)
	{
		file << setprecision (numeric_limits<double>::digits10 + 1) << data.at(i) << endl;
	}
	file.close();
}