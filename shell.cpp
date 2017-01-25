//shell.cpp
//A linux command line interpreter to issue commands

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ARGS 50
#define IS_SPACE(x) ( (x) > 0 && (x) < 127 && isspace((x)) )

using namespace std;

class myShCommands
{
private:
    string prompt;
    bool doesFileExist(string filePath);

public:
    myShCommands();
    ~myShCommands();	
    
    void echo(vector<string> &n);        //prints string out to terminal
    void echoNoCR(vector<string> &n);    //if -n option is used with echo, this function runs
    void PS1(vector<string> &h);         //changes user prompt
    void cat(vector <string> &l);        //displays content of a file
    void cp(vector <string> & cpVec);    //copies from one file to the next, if more than two files copies into directory
    void rm(vector <string> & rmVec);    //removes file from directory
    bool exit();                         //exits mysh
    void userError(vector <string> &m);  //prints error if user entered something wrong
    void printPrompt();			             //prints the current prompt, initialized by constructor to default prompt "$ "
    int isEmptyLine(const char* s);      //check if there is an empty line
    int do_exec(vector <string> x, char * p);      //search for external commands and

    //test functions
    void printVector(vector<string> &n);
 };

bool myShCommands::doesFileExist(string filePath)
{
    ifstream openFile;

    openFile.open(filePath.c_str());
    if (!openFile.is_open()){
        //file did not open
        //cout << "File not found" << endl;
        return false;
    }
    else {
        //cout << "File found" << endl;
        openFile.close(); //close file
        return true;
    }
}

int myShCommands::do_exec(vector <string> xa, char * userInput)
{
    char path[256];
    char* p;
    char* pch;
    char* args[MAX_ARGS];

    int status = 0;
    int argCount;
    int child_status = 0;
    
    string hello;
    bool found = false;
    DIR *dir;

    //read arguments into args	
    pch = strtok(userInput, " \t");
    for (argCount = 0; pch && status == 0; ++argCount) {
      if (argCount < (MAX_ARGS - 1)) {
        args[argCount] = pch;
        pch = strtok(0, " \t");
      }
      else {
        cout << "ERROR: shell argument array overflow!" << endl;
        return -1;
      }
    }
    
    args[argCount] = 0;
    
    // test args input
    //for (int i = 0; i < argCount; i++) {
    //    cout << args[i] << endl;
    //} 

    hello = xa.at(0); //copy file name to var
    
    //does not support changing directories, return -1 if cd is an argument
    if (xa.at(0) == "cd" || xa.at(0) == "CD") {
        return -1;
    }

    // if command has "/", test for file existence
    if (strchr(hello.c_str(), '/')) {
        // test for file existence
        if (doesFileExist(hello)) { //hello as argument
            //file exists
            strcpy(path, hello.c_str()); // copy name of file to path
            found = true;
        }
    }
    else {
        
        //file does not exist in path, search path for file
        p = getenv("PATH");
        //cout << "P IS: " << p << endl;

        //copy path to local string
        strcpy(path, p); 

        //get each path part and test for existence
        pch = strtok(path, ":");
	//cout << "FIRST pch: " << pch << endl;
        
        //add command to end of path taken
        string y;
        string x = pch;
        y = x + '/' + hello.c_str();
	

	while (pch != NULL) {
            //OPEN PATH CHECK IF FILE IS THERE
            //check if directory is open
            if ((dir = opendir(pch)) == NULL) {
                cerr << "Cannot open directory.";
                return -1;
            }
            //search for file there
            if (doesFileExist(y)) {
                found = true;
                strcpy(path, y.c_str());
                break;
            }
	    closedir(dir);
            //continue searching through PATH
            pch = strtok(NULL, ":");    
	    if (pch == NULL) {
	    	break;
	    }           
	    x = pch;
            y = x + '/' + hello.c_str(); //set new search location
            //cout << "pch: " << pch << endl;
	    //cout << "y: " << y << endl;
	}
    }

    // closedir(dir);
   /* catch (...) {
	cout << xa.at(0) << " command not found" << endl;
	return -1;
    } */

    if (found == true) {
        if (fork() == 0) {
            execvp(path, args);
        }
        
	else {
            waitpid(-1, &child_status, 0);
            if (child_status != 0)
            {
                cerr << strerror(child_status);
                return -1;
            }
        }
    }
    else {
        cerr << hello << " command not found" << endl;
        return -1;
    }
    return 1;
}

int myShCommands::isEmptyLine(const char* s)
{
    while(IS_SPACE(*s)) ++s;
    return *s == '\0' ? 1 : 0; 
}

void myShCommands::printVector(vector<string> &n)
{
    cout << "The vector elements are: " << endl;
    for (int i =0; i < n.size(); i++) {
        cout << n.at(i) << endl;
    }
}

myShCommands::myShCommands()
{
    prompt = "$ ";
}

myShCommands::~myShCommands()
{}

void myShCommands::echo(vector<string> &n)
{
    for (int i = 1; i < n.size(); i++) {
        cout << n.at(i) << ' ';
    }
    cout << "\r"; //assumes and outputs a carriage return at end of output
}

void myShCommands::echoNoCR(vector<string> &n)
//no carriage return as string parsed in main does not read carriage return
{
    string x;

    //make sure to delete carriage return from n here
    for (int i = 2; i < n.size(); i++) {
        if ( i == n.size()) {
            x = n.at(i); // copy last element to string 
            x.erase(remove(x.begin(), x.end(), '\n'), x.end()); // erase carriage return
            n.at(i) = x; // copy value back to last element of vector
            return;
        }
        cout << n.at(i) << ' ';
    }
}


void myShCommands::PS1(vector <string> &h)
{
    //check to see if command is space delimited
    if (h.at(0) != "PS1") {
        //cout << "PS1 command needs to be space delimited. i.e PS1 = \"your prompt name\"" << endl;
        //cout << "No spaces allowed in prompt name. Feature to be implemented soon.";
        return;
    }
    else {
        string x;
        x = h.at(2);
        char nochars[]="\"";

        for (int i = 0; i < strlen(nochars); i++) {
            x.erase (remove(x.begin(), x.end(), nochars[i]), x.end());
        }
        prompt = x.append(" ");
    }
}

//cat only works with txt files at the moment
void myShCommands::cat(vector <string> &l) 
{
	ifstream out; //file we're opening to read
	string ins;   //variable to store lines read
	string he;    //variable to store name of file

	for (int i = 1; i < l.size(); i++) 
	{
		he = l.at(i);
		out.open(he.c_str(), ios::in); //c_str() is a 0 terminated string
		
		//check to see if file is open
		if (!out.is_open()) {
			cout << "Error: could not open file.\n";
			return;
		}

		while (!out.eof())
		{
			getline(out, ins);
			cout << ins << endl;
		}
		out.close();
	}
}

void myShCommands::cp(vector <string> & cpVec)
{
  fstream filein;
  ofstream filecp; //file to copy

  string fin;
  string fcp;     //copy arguments to string

  if ((cpVec.size() - 1) >= 3) 
  {  //-1 because we have the cp command in the vector there
       //deal with more than three arguments.
       //last argument must be a directory
       
       //get directory name at the end of the vector
       string dirname;
       dirname = cpVec.back();
       
       //create a directory
       mkdir(dirname.c_str(), 0777);
       //mkdir(dirname.c_str());

       //Iterate through the vector and copy each file found into the directory
       //cpVec.size() - 1 is there so we don't include name of directory at the end
       //i = 1 so we don't iterate cp command
       for (int i = 1; i < cpVec.size() - 1; i++) 
       {

           //get name of file at vector position in i
           fin = cpVec.at(i);
           
           //open the file to be copied and check to see if file opened
           filein.open(fin.c_str());

           if (!filein.is_open()) {
               cout << "Could not open file " << fin << endl;
               return;
           } 

           //create a fullpath variable
           string fullpath;
           fullpath = dirname+"/"+fin;
           
           //open the file at the path specified in fullpath
           filecp.open(fullpath.c_str());
           
           //check to see if file opened
           if (!filecp.is_open()) {
               cout << "Could not open directory file " << fullpath << endl;
               return;
           }

           //copy contents of file into the file in the path specified, this case our directory
           while(!filein.eof()) {
                string x;
                getline(filein, x);
                filecp << x << "\n";
           }
           //close files so we can open a new file
           filein.close();
           filecp.close();
        }
        
    //close files just in case
    filein.close();
    filecp.close();
   }

   else {
       //deal with two files
       fin = cpVec.at(1);
       fcp = cpVec.at(2);

       filein.open(fin.c_str());
       filecp.open(fcp.c_str());

       if (!filein.is_open()) {
           cout << "Error: could not open file " << fin << " " << "\n";
           return;
        }
    
       if (!filecp.is_open()) {
           cout << "Error: could not open file " << fcp << " " << "\n";
           return;
        }

       while (!filein.eof()) 
       {
           string x;
           getline(filein, x);         //get lines from first file
           filecp << x << "\n";        //copy into second file name
       }
    }
    //close files
    filein.close();
    filecp.close();
}

void myShCommands::rm(vector <string> & rmVec)
{
  string pathname;
 
  for (int i = 1; i < rmVec.size(); i++) {
    pathname = rmVec.at(i);
    if(doesFileExist(pathname)) {
        remove(pathname.c_str());
    }
    else {
        cout << "File does not exist";
    }
  }

}

bool myShCommands::exit()
{
  return false;
}

void myShCommands::userError(vector <string> &m)
{
  cout << m.at(0) << " command not found.\n";
}

void myShCommands::printPrompt()
{
  cout << prompt;
}

int main(int argc, char *argv[])
{
  bool run = true;
  string userInput;
  string userCommand;
  string userArgs;
   
  string ps1var;
  char input[1024];
  char *p;
  vector <string> args;
    
  myShCommands cmd;
   	
  while (run) 
  {
    cmd.printPrompt(); 
      getline(cin, userCommand); //gets the whole line including whitespace, carriage return and newline

      //copy to input variable 
      strcpy(input, userCommand.c_str());
      
      //If line is empty, continue to next while loop iteration
      if (cmd.isEmptyLine(input)) {
          continue;
      }

      if (userCommand == "\n" || userCommand == " " || userCommand == "\r") {
        continue;
      }

      //remove whitespace and input each individual command and arguments into vector seperately
      istringstream a1(userCommand);	//sets the string as a stream for input
      string temp;			//temp variable to store our keywords without whitespace or newline/carriage return into vector

      while (a1 >> temp) {
        args.push_back(temp);
      }
      //test vector input
      // cmd.printVector(args);

      // test array input

      //Check command line argument
      if (args.at(0) == "echo") {
        if (args.at(1) == "-n") { cmd.echoNoCR(args); } else { cmd.echo(args); cout << endl; } 
      }

      else if (args.at(0) == "PS1") {
        cmd.PS1(args);
	cout << endl;
      }

      else if (args.at(0) == "cat") {
        cmd.cat(args);
	cout << endl;
      }

      else if (args.at(0) == "cp") {
        cmd.cp(args);
	cout << endl;
      }

      else if (args.at(0) == "rm") {
        cmd.rm(args);
	cout << endl;
      }

      else if (args.at(0) == "exit" || args.at(0) == "quit") {
        run = cmd.exit();
      }
      
      else { // search external commands
        if (cmd.do_exec(args, input) < 0 ) {
        // do nothing, do_exec will output it's own error messages
            cout << endl;
        } else {
	    cout << endl; //newline for prompt
	}
      }
      //clear the vector for next set of arguments and keep memory free
      args.clear();
      //cout << endl; //set a newline for prompt
    } //end while

    return 0;
}