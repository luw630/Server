#pragma once

// template
//#include <string>
//typedef std::string ustring;

#include <set>

template<class T>
class CTSet
{
    typedef std::set<T> SST;
    typedef typename std::set<T>::iterator SIT;

    SST _set;

public:
    CTSet(void);
    ~CTSet(void);

    bool Add(T type);
    void Del(T typeKey);
    size_t Size(void) const { return _set.size();}
    bool Locate(T typeKey);
    void GetItBE(SIT &it_b, SIT &it_e);
    void Clear(void) { _set.clear(); }
};



template<class T>
CTSet<T>::CTSet(void)
{
}

template<class T>
CTSet<T>::~CTSet(void)
{
}

template<class T>
bool CTSet<T>::Add(T type)
{
    if(Locate(type))
        return false;

    _set.insert(type);
    return true;
}

template<class T>
void CTSet<T>::Del(T typeKey)
{
    if(!Locate(typeKey))
        return;

    SIT it = _set.find(typeKey);
    if(it != _set.end())
    {
        _set.erase(it);
    }
}

template<class T>
bool CTSet<T>::Locate(T typeKey) 
{ 
    return _set.count(typeKey) > 0 ? true : false;
}

template<class T>
void CTSet<T>::GetItBE(SIT &it_b, SIT &it_e)
{
    it_b = _set.begin(); 
    it_e = _set.end(); ;
}