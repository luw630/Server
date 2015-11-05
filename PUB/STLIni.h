#ifndef STLINI_FILE
#define STLINI_FILE

#include <map>
#include <fstream>
#include <strstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

typedef map<string, string, less<string> > strMap;
typedef strMap::iterator strMapIt;

const char*const MIDDLESTRING = "-_-";
struct analyzeini{
    string strsect;
    strMap *pmap;
    analyzeini(strMap & strmap):pmap(&strmap),strsect("MAIN"){}
    void operator()( const string & strini)
    {
        size_t first =strini.find('[');
        size_t last = strini.rfind(']');
        if( first != string::npos && last != string::npos && first != last+1)
        {
            strsect = strini.substr(first+1,last-first-1);
            return ;
        }
        if(strsect.empty())
            return ;
        if((first=strini.find('='))== string::npos)
            return ;
        string strtmp1= strini.substr(0,first);
        string strtmp2=strini.substr(first+1, string::npos);
        first= strtmp1.find_first_not_of(" \t");
        last = strtmp1.find_last_not_of(" \t");
        if(first == string::npos || last == string::npos)
            return ;
        string strkey = strtmp1.substr(first, last-first+1);
        first = strtmp2.find_first_not_of(" \t");
        if(((last = strtmp2.find("\t#", first )) != string::npos) ||
            ((last = strtmp2.find(" #", first )) != string::npos) ||
            ((last = strtmp2.find("\t//", first )) != string::npos)||
            ((last = strtmp2.find(" //", first )) != string::npos))
        {
            strtmp2 = strtmp2.substr(0, last-first);
        }
        last = strtmp2.find_last_not_of(" \t");
        if(first == string::npos || last == string::npos)
            return ;
        string value = strtmp2.substr(first, last-first+1);
        string mapkey = strsect + MIDDLESTRING;
        mapkey += strkey;
        (*pmap)[mapkey]=value;
        return ;
    }
};


class IniFiles
{
public:
    IniFiles( ){};
    ~IniFiles( ){};
    bool open(const char* pinipath)
    {
        return do_open(pinipath);
    }
    bool open(char* inidata, size_t size)
    {
        return do_open(inidata, size);
    }
    string read(const char*psect, const char*pkey, const char *pdefault = NULL)
    {
        string mapkey( "MAIN" );
        if ( psect ) mapkey = psect;
        mapkey += MIDDLESTRING;
        mapkey += pkey;
        strMapIt it = c_inimap.find(mapkey);
        if(it == c_inimap.end())
            return pdefault ? pdefault : "";
        else
		{
			string first = it->first;
            string second = it->second; 
			return second;
		}
    }
	int read_int(const char*psect, const char* pkey,int defval = 0)
	{
		string strval = read(psect,pkey,"");
		if  (!strval.empty()) 
		{
			return atoi(strval.c_str());
		} else
			return defval;  
	}
	double read_float(const char*psect, const char* pkey,double defval = 0.0)
	{
		string strval = read(psect,pkey,"");
		if  (!strval.empty()) 
		{
			return atof(strval.c_str());
		} else
			return defval; 

	}
protected:
    bool do_open(const char* pinipath)
    {
        ifstream fin(pinipath);
        if(!fin.is_open())
            return false;
        vector<string> strvect;
        while(!fin.eof())
        {
            string inbuf;
            getline(fin, inbuf,'\n');
            strvect.push_back(inbuf);
        }
        if(strvect.empty())
            return false;
        for_each(strvect.begin(), strvect.end(), analyzeini(c_inimap));
        return !c_inimap.empty();
    }
    bool do_open(char* inidata, size_t size)
    {
		strstream fin(inidata, (std::streamsize)size);
        vector<string> strvect;
        while(!fin.eof())
        {
            string inbuf;
            getline(fin, inbuf,'\n');
            strvect.push_back(inbuf);
        }
        if(strvect.empty())
            return false;
        for_each(strvect.begin(), strvect.end(), analyzeini(c_inimap));
        return !c_inimap.empty();
    }
    strMap c_inimap;
};

#endif