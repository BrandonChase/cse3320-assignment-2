#include <string>
#include <deque>
#include <iostream>
#include <iomanip> 
#include <limits> 
#include <fstream>
#include <chrono>
#include <thread>
#include <functional> //ref()

#include <sys/wait.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

using namespace std;

//filenames assume running from part2 directory
const string INPUT_FILENAME = "../earthquakes.csv";
const string TEMP_FILENAME = "latitudes";
const string SORT_PROGRAM_PATH = "./bin/sort";
const int nums_of_processes[] = {1, 2, 4, 8};

long timeSort(deque<double> content, int num_lines, int num_processes);
deque<double> loadLatitudes(string filename);
deque<string> splitString(string content, string delimeters);
deque<deque<double>> performParallelSort(deque<double>& content, int num_lines, int num_processes);
deque<double> mergeDataChunks(deque<deque<double>> chunks);
void saveContent(string filename, deque<double> data);
void performInsertionSortOn(deque<double>& data, int start_index, int end_index);
void insertInto(deque<double>& data, double item);

int main(int argc, const char * argv[])
{
	
	deque<double> latitudes = loadLatitudes(INPUT_FILENAME);
	bool wants_to_go_again;
	do
	{
		//ask if user wants to do custom parameters for sort and get parameters
		cout << "Do you want to enter custom parameters? [y] or [n]: ";
		char input;
		cin >> input;
		if(input == 'y') //custom behaivor
		{
			int lines, processes;
			cout << "How many lines of latitude? [9736 max]: ";
			cin >> lines;
			cout << "How many threads to run sort on?: ";
			cin >> processes;
			assert(lines > 0 && processes > 0);
			cout << "Customized sort took " << timeSort(latitudes, lines, processes) / 1000.0 << "s using " << processes << " thread(s)." << endl;
		}
		else if(input == 's')//special mode
		{
			deque<double> data;
			int lines, processes;
			cout << "How many numbers to sort?: ";
			cin >> lines;
			cout << "How many threads to run sort on?: ";
			cin >> processes;
			for(int i = 0; i < lines / 2; i++) { data.push_front(i); data.push_front(-i);}
			assert(lines > 0 && processes > 0);
			cout << "Customized sort took " << timeSort(data, lines, processes) / 1000.0 << "s using " << processes << " thread(s)." << endl;

			for(int num_of_processes : nums_of_processes)
			{
				cout << "Sort took " << timeSort(data, data.size(), num_of_processes) / 1000.0 << "s using " << num_of_processes << " thread(s)." << endl;
			}
		}
		else //predefined behaivor
		{
			for(int num_of_processes : nums_of_processes)
			{
				cout << "Sort took " << timeSort(latitudes, latitudes.size(), num_of_processes) / 1000.0 << "s using " << num_of_processes << " thread(s)." << endl;
			}
		}

		cout << "Go again? [y] or [n]: ";
		cin >> input;
		if(input == 'y') { wants_to_go_again = true; }
		else { wants_to_go_again = false; }

	}while(wants_to_go_again);	
	//delete temp files
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
	
	deque<deque<double>> chunks = performParallelSort(content, num_lines, num_processes);

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

//creates child processes that each read their own file, perform an insertion on the data, and saves sorted data back to same file.
//then waits for all children to finish 
deque<deque<double>> performParallelSort(deque<double>& content, int num_lines, int num_processes)
{
	assert(num_lines >= 0 && num_lines <= content.size());
	int lines_per_chunk = num_lines / num_processes;

	deque<thread> threads;
	//pass split up chunks to threads
	for(int i = 0; i < num_processes; i++) 
	{
		int starting_index = i * lines_per_chunk;

		int ending_index;
		if(i == num_processes - 1) //saving last chunk
		{
			/*
				If content isn't divided evenly, ie num lines % num chunks != 0, include all of last chunk that would otherwide be cut off.	
				Set to content.size() - 1 b/c end_index is inclusive in saveChunk function.
			*/
			ending_index = num_lines - 1; 
		}

		else
		{
			ending_index = starting_index + lines_per_chunk - 1;
		}

		//create thread and insertion sort
		threads.push_back(thread(performInsertionSortOn, ref(content), starting_index, ending_index));
	}

	//wait for all threads to finish sorting
	for(int i = 0; i < threads.size(); i++)
	{
		threads.at(i).join();
	}

	//copy array into result
	deque<deque<double>> sorted_chunks;
	for(int i = 0; i < num_processes; i++) 
	{
		int starting_index = i * lines_per_chunk;

		int ending_index;
		if(i == num_processes - 1) //saving last chunk
		{
			/*
				If content isn't divided evenly, ie num lines % num chunks != 0, include all of last chunk that would otherwide be cut off.	
				Set to content.size() - 1 b/c end_index is inclusive in saveChunk function.
			*/
			ending_index = num_lines - 1; 
		}

		else
		{
			ending_index = starting_index + lines_per_chunk - 1;
		}

		deque<double> current_chunk;
		for(int i = starting_index; i <= ending_index; i++)
		{
			current_chunk.push_back(content.at(i));
		}

		sorted_chunks.push_back(current_chunk);
	}

	return sorted_chunks;
}

//inclusive end index
void performInsertionSortOn(deque<double>& data, int start_index, int end_index)
{
	deque<double> sorted_result;
	for(int i = start_index; i <= end_index; i++)
	{
		insertInto(sorted_result, data.at(i));
	}

	//copy sorted_result into original. TODO: mutex?
	for(int i = 0; i < sorted_result.size(); i++)
	{
		data.at(start_index+i) = sorted_result.at(i);
	}
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