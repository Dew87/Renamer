#ifndef RENAMER_HPP
#define RENAMER_HPP

#include <stack>
#include <string>
#include <vector>

class Renamer
{
public:
	Renamer(void);
	~Renamer(void);
	void Run(void);

private:
	// Type definition
	typedef std::string string;
	typedef std::pair<string, string> Entry;
	typedef std::vector<Entry> EntrySet;
	typedef std::stack<EntrySet> EntrySetStack;

	// Private copy constructor
	Renamer(const Renamer& renamer);
	Renamer& operator = (const Renamer& renamer);

	// Deallocation functions
	void ClearRedo(void);
	void ClearUndo(void);

	// Functions
	void AddEntry(EntrySet &entrySet, string old_name, string new_name);
	void AddEntrySet(EntrySet &entrySet);
	void Auto(void);
	string BoolToString(const bool boolean);
	unsigned int Digits(const unsigned int number);
	void Info(void);
	void Option(void);
	void Redo(void);
	void Rename(void);
	void Replace(void);
	void Undo(void);

	// Function Data
	EntrySetStack mRedo;
	EntrySetStack mUndo;
};

#endif