#include "Renamer.hpp"
#include "dirent.h"
#include <conio.h>
#include <iostream>
#include <sstream>
#include <vector>

#define OLD_NAME first
#define NEW_NAME second

using namespace std;

// Global option variables
static bool EXTENSION_DIGITS_NEW = true;
static bool EXTENSION_DIGITS_OLD = true;
static bool MANUALLY_INPUT_DIGITS = false;
static bool NAME = true;
static bool NAME_EXTENSION = false;
static bool RENUMBER = false;
static bool SEARCH_PATH = false;

Renamer::Renamer(void)
{}

Renamer::~Renamer(void)
{
	// Deallocation
	ClearRedo();
	ClearUndo();
}

void Renamer::AddEntry(EntrySet &entrySet, string old_name, string new_name)
{
	// Add new entry to a set
	Entry entry;
	entry.OLD_NAME = old_name;
	entry.NEW_NAME = new_name;
	entrySet.push_back(entry);
}

void Renamer::AddEntrySet(EntrySet &entrySet)
{
	// Save set and clear redo stack
	ClearRedo();
	mUndo.push(entrySet);
}

void Renamer::Auto(void)
{
	// Renaming variables
	bool new_data = false;
	EntrySet files;
	EntrySet data;
	string extra_digits_new;
	string file_extension;
	string new_name;
	string old_name;
	string search_path;
	string temp_name;
	unsigned int digits_new;
	unsigned int index;
	DIR* directory;
	dirent* entry;

	// Input search path
	if (SEARCH_PATH)
	{
		cout << "Enter search path for files: ";
		getline(cin, search_path);
	}
	else
	{
		search_path = ".\\";
	}

	// Open directory to read files from
	directory = opendir(search_path.c_str());
	if (directory != NULL)
	{
		// Save all names of files that is due to be renamed and close directory
		while ((entry = readdir(directory)) != NULL)
		{
			// Extract old name of files and file extension
			old_name = entry->d_name;

			index = old_name.find_last_of('.');
			if (index != string::npos)
			{
				temp_name = old_name.substr(0, index);
				file_extension = old_name.substr(index);

				// Clear new file name
				new_name.clear();

				// Only save the characters that are numbers for new name
				for (string::iterator i = temp_name.begin(); i != temp_name.end(); i++)
				{
					int integear = (int)(*i);
					if (47 < integear && integear < 58)
					{
						new_name += (*i);
					}
				}

				// Add file if new name contains any characters
				if (!new_name.empty())
				{
					new_name += file_extension;
					files.push_back(Entry(old_name, new_name));
				}
			}
		}
		closedir(directory);

		// Input extension digits for new file name
		if (EXTENSION_DIGITS_NEW && MANUALLY_INPUT_DIGITS)
		{
			cout << "Enter new number of digits: ";
			cin >> digits_new;
			unsigned int min_digits = Digits(files.size());
			if (digits_new < min_digits)
			{
				digits_new = min_digits;
			}
		}
		else
		{
			digits_new = Digits(files.size());
		}

		// Automatic renaming loop
		if (RENUMBER)
		{
			// Rename file into numbers depending on their index
			for (unsigned int i = 0; i < files.size(); i++)
			{
				stringstream ss_new_name;

				// Old file name and file extension
				old_name = files[i].OLD_NAME;
				index = old_name.find_last_of('.');
				if (index != string::npos)
				{
					file_extension = old_name.substr(index);

					// Renaming start at 1 instead of 0
					index = i + 1;

					// Extra extension digits for new name
					extra_digits_new.clear();
					if (EXTENSION_DIGITS_NEW)
					{
						extra_digits_new.append(digits_new - Digits(index), '0');
					}

					// Finale new name
					ss_new_name << extra_digits_new << index << file_extension;
					new_name = ss_new_name.str();

					// Renaming function
					if (!rename(old_name.c_str(), new_name.c_str()))
					{
						new_data = true;
						AddEntry(data, old_name, new_name);
						cout << old_name << " renamed to " << new_name << endl;
					}
					else
					{
						cout << old_name << " renaming failed" << endl;
					}
				}
			}
		}
		else
		{
			// Rename file to only numbers
			for (EntrySet::iterator i = files.begin(); i != files.end(); i++)
			{
				old_name = (*i).OLD_NAME;
				new_name = (*i).NEW_NAME;

				// Add or remove extension digits if manual input
				if (MANUALLY_INPUT_DIGITS)
				{
					index = new_name.find_last_of('.');
					if (index != string::npos)
					{
						temp_name = new_name.substr(0, index);
						file_extension = new_name.substr(index);

						if (temp_name.size() < digits_new)
						{
							// Add extension digits
							extra_digits_new.clear();
							extra_digits_new.append(digits_new - temp_name.size(), '0');
							new_name = extra_digits_new + temp_name + file_extension;
						}
						else if (temp_name.size() > digits_new)
						{
							// Remove extension digits
							new_name = new_name.substr(temp_name.size() - digits_new);
						}
					}
				}

				// Renaming function
				if (!rename(old_name.c_str(), new_name.c_str()))
				{
					new_data = true;
					AddEntry(data, old_name, new_name);
					cout << old_name << " renamed to " << new_name << endl;
				}
				else
				{
					cout << old_name << " renaming failed" << endl;
				}
			}
			// Extra line break to get input and output nice and tidy
			if (MANUALLY_INPUT_DIGITS)
			{
				cin.get();
			}
		}
	}
	else
	{
		cout << "Could not open directory " << search_path << endl;
	}

	// Save data for renamed files
	if (new_data)
	{
		AddEntrySet(data);
	}

	// Extra line break to get input and output nice and tidy
	cout << endl;
}

string Renamer::BoolToString(const bool boolean)
{
	return boolean ? "On" : "Off";
}

void Renamer::ClearRedo(void)
{
	// Remove elements and deallocate redo stack
	while (!mRedo.empty())
	{
		mRedo.pop();
	}
}

void Renamer::ClearUndo(void)
{
	// Remove elements and deallocate undo stack
	while (!mUndo.empty())
	{
		mUndo.pop();
	}
}

unsigned int Renamer::Digits(const unsigned int number)
{
	// Return number of digits of a number
	stringstream ss;
	ss << number;

	return ss.str().size();
}

void Renamer::Info(void)
{
	// Info output
	cout << "File Renamer made by David Erikssen" << endl;
	cout << "Redo stack size: " << mRedo.size() << endl;
	cout << "Undo stack size: " << mUndo.size() << endl;
	cout << endl;
}

void Renamer::Option(void)
{
	// Option menu
	while (true)
	{
		cout << "Type corresponding number to change each setting" << endl;
		cout << "1: EXTENSION_DIGITS_OLD  " << BoolToString(EXTENSION_DIGITS_OLD) << endl;
		cout << "2: EXTENSION_DIGITS_NEW  " << BoolToString(EXTENSION_DIGITS_NEW) << endl;
		cout << "3: MANUALLY_INPUT_DIGITS " << BoolToString(MANUALLY_INPUT_DIGITS) << endl;
		cout << "4: NAME                  " << BoolToString(NAME) << endl;
		cout << "5: NAME_EXTENSION        " << BoolToString(NAME_EXTENSION) << endl;
		cout << "6: RENUMBER              " << BoolToString(RENUMBER) << endl;
		cout << "7: SEARCH_PATH           " << BoolToString(SEARCH_PATH) << endl;
		cout << "Any other character to return" << endl;

		char answer = _getch();
		cout << endl;

		switch (answer)
		{
		case '1': EXTENSION_DIGITS_OLD = !EXTENSION_DIGITS_OLD; break;
		case '2': EXTENSION_DIGITS_NEW = !EXTENSION_DIGITS_NEW; break;
		case '3': MANUALLY_INPUT_DIGITS = !MANUALLY_INPUT_DIGITS; break;
		case '4': NAME = !NAME; break;
		case '5': NAME_EXTENSION = !NAME_EXTENSION; break;
		case '6': RENUMBER = !RENUMBER; break;
		case '7': SEARCH_PATH = !SEARCH_PATH; break;
		default: return;
		}
	}
}

void Renamer::Redo(void)
{
	// Redo to last undone state
	if (mRedo.empty())
	{
		cout << "Nothing to redo" << endl;
	}
	else
	{
		// Get state and move from redo to undo stack
		EntrySet data = mRedo.top();
		mRedo.pop();
		mUndo.push(data);

		// Iterate through all entries and rename
		for (EntrySet::iterator i = data.begin(); i != data.end(); i++)
		{
			if (!rename(i->OLD_NAME.c_str(), i->NEW_NAME.c_str()))
			{
				cout << i->OLD_NAME << " renamed to " << i->NEW_NAME << endl;
			}
			else
			{
				cout << i->OLD_NAME << " renaming failed" << endl;
			}
		}
	}

	// Extra line break to get input and output nice and tidy
	cout << endl;
}

void Renamer::Rename(void)
{
	// Renaming Variables
	bool new_data = false;
	EntrySet data;
	string extra_digits_new;
	string extra_digits_old;
	string file_extension;
	string new_name;
	string new_name_extension;
	string old_name;
	string old_name_extension;
	string search_path;
	unsigned int min;
	unsigned int max;
	unsigned int renumber;
	unsigned int digits_old;
	unsigned int digits_new;

	// Input search path
	if (SEARCH_PATH)
	{
		cout << "Enter search path for files: ";
		getline(cin, search_path);
	}

	// Input file name
	if (NAME)
	{
		cout << "Enter old names of files: ";
		getline(cin, old_name);
		cout << "Enter new names of files: ";
		getline(cin, new_name);
	}

	// Input file name extension (part after the number)
	if (NAME_EXTENSION)
	{
		cout << "Enter old name extension of files: ";
		getline(cin, old_name_extension);
		cout << "Enter new name extension of files: ";
		getline(cin, new_name_extension);
	}

	// Input file extension
	cout << "Enter file extension: ";
	getline(cin, file_extension);

	// Input number range
	cout << "Enter number range: ";
	cin >> min >> max;
	if (RENUMBER)
	{
		cout << "Enter renumbering start: ";
		cin >> renumber;
	}
	else
	{
		renumber = min;
	}

	// Input extension digits for old file name
	if (EXTENSION_DIGITS_OLD && MANUALLY_INPUT_DIGITS)
	{
		cout << "Enter old number of digits: ";
		cin >> digits_old;
		unsigned int min_digits = Digits(max);
		if (digits_old < min_digits)
		{
			digits_old = min_digits;
		}
	}
	else
	{
		digits_old = Digits(max);
	}

	// Input extension digits for new file name
	if (EXTENSION_DIGITS_NEW && MANUALLY_INPUT_DIGITS)
	{
		cout << "Enter new number of digits: ";
		cin >> digits_new;
		unsigned int min_digits = Digits(max + renumber - min);
		if (digits_new < min_digits)
		{
			digits_new = min_digits;
		}
	}
	else
	{
		digits_new = Digits(max + renumber - min);
	}

	// Renaming loop
	for (unsigned int i = min, j = renumber; i <= max; ++i, ++j)
	{
		// Clear variables
		extra_digits_old.clear();
		extra_digits_new.clear();
		stringstream ss_old_name;
		stringstream ss_new_name;

		// Extra extension digits for old name
		if (EXTENSION_DIGITS_OLD)
		{
			extra_digits_old.append(digits_old - Digits(i), '0');
		}

		// Extra extension digits for new name
		if (EXTENSION_DIGITS_NEW)
		{
			extra_digits_new.append(digits_new - Digits(j), '0');
		}

		// Finale old name
		ss_old_name << search_path << old_name << extra_digits_old << i << old_name_extension << '.' << file_extension;

		// Finale new name
		ss_new_name << search_path << new_name << extra_digits_new << j << new_name_extension << '.' << file_extension;

		// Renaming function
		if (!rename(ss_old_name.str().c_str(), ss_new_name.str().c_str()))
		{
			new_data = true;
			AddEntry(data, ss_old_name.str(), ss_new_name.str());
			cout << ss_old_name.str() << " renamed to " << ss_new_name.str() << endl;
		}
		else
		{
			cout << ss_old_name.str() << " renaming failed" << endl;
		}
	}

	// Save data for renamed files
	if (new_data)
	{
		AddEntrySet(data);
	}

	// Extra line break to get input and output nice and tidy
	cout << endl;
	cin.get();
}

void Renamer::Run()
{
	// Main console menu
	Info();

	while (true)
	{
		cout << "1: Rename" << endl;
		cout << "2: Option" << endl;
		cout << "3: Redo" << endl;
		cout << "4: Undo" << endl;
		cout << "5: Auto" << endl;
		cout << "6: Replace" << endl;
		cout << "7: Info" << endl;
		cout << "Any other character to quit" << endl;

		char answer = _getch();
		cout << endl;

		switch (answer)
		{
		case '1': Rename(); break;
		case '2': Option(); break;
		case '3': Redo(); break;
		case '4': Undo(); break;
		case '5': Auto(); break;
		case '6': Replace(); break;
		case '7': Info(); break;
		default: return;
		}
	}
}

void Renamer::Replace(void)
{
	// Renaming variables
	bool rename_file;
	bool new_data = false;
	EntrySet files;
	EntrySet data;
	string new_name;
	string new_input;
	string old_name;
	string old_input;
	string search_path;
	string temp_name;
	unsigned int index;
	DIR* directory;
	dirent* entry;

	// Input search path
	if (SEARCH_PATH)
	{
		cout << "Enter search path for files: ";
		getline(cin, search_path);
	}
	else
	{
		search_path = ".\\";
	}

	// Input replace data
	cout << "Enter text to replace: ";
	getline(cin, old_input);
	cout << "Enter text to replace with: ";
	getline(cin, new_input);

	if (old_input.size() != 0)
	{
		// Open directory to read files from
		directory = opendir(search_path.c_str());
		if (directory != NULL)
		{
			// Save all names of files
			while ((entry = readdir(directory)) != NULL)
			{
				old_name = entry->d_name;
				new_name = old_name;
				files.push_back(Entry(old_name, new_name));
			}
			closedir(directory);

			// Rename file by replacing substrings
			for (EntrySet::iterator i = files.begin(); i != files.end(); i++)
			{
				rename_file = false;
				old_name = (*i).OLD_NAME;
				temp_name = old_name;
				new_name.clear();

				// Replace input substrings of old with new 
				for (index = temp_name.find(old_input); index != string::npos; index = temp_name.find(old_input))
				{
					rename_file = true;
					new_name += temp_name.substr(0, index) + new_input;
					temp_name = temp_name.substr(index + old_input.size());
				}

				// Don't rename unaltered files
				if (rename_file)
				{
					new_name += temp_name;

					// Renaming function
					if (!rename(old_name.c_str(), new_name.c_str()))
					{
						new_data = true;
						AddEntry(data, old_name, new_name);
						cout << old_name << " renamed to " << new_name << endl;
					}
					else
					{
						cout << old_name << " renaming failed" << endl;
					}
				}
			}
		}
		else
		{
			cout << "Could not open directory " << search_path << endl;
		}
	}
	else
	{
		cout << "Must have some text to replace" << endl;
	}

	// Save data for renamed files
	if (new_data)
	{
		AddEntrySet(data);
	}

	// Extra line break to get input and output nice and tidy
	cout << endl;
}

void Renamer::Undo(void)
{
	// Undo to last renaming state
	if (mUndo.empty())
	{
		cout << "Nothing to undo" << endl;
	}
	else
	{
		// Get state and move from undo to redo stack
		EntrySet data = mUndo.top();
		mUndo.pop();
		mRedo.push(data);

		// Iterate backwards through all entries and rename
		for (EntrySet::reverse_iterator i = data.rbegin(); i != data.rend(); i++)
		{
			if (!rename(i->NEW_NAME.c_str(), i->OLD_NAME.c_str()))
			{
				cout << i->NEW_NAME << " renamed to " << i->OLD_NAME << endl;
			}
			else
			{
				cout << i->NEW_NAME << " renaming failed" << endl;
			}
		}
	}

	// Extra line break to get input and output nice and tidy
	cout << endl;
}
