// a generic object cache
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef OBJCACHE_H
#define OBJCACHE_H

#include <string>
#include <map>
#include <iostream>
using namespace std;

template <class T>
class objcachet
{
	map<string, pair<unsigned, T*> > cache;
	string basedir;
	objcachet<T>();
	objcachet<T>& operator= (const objcachet<T>& );
	objcachet<T>(const objcachet<T>& );

public:
	objcachet<T>(const string& basedir_) : basedir(basedir_) {}
	~objcachet<T>() {
		for (typename map<string, pair<unsigned, T*> >::iterator it = cache.begin(); it != cache.end(); ++it)
			delete it->second.second;
	}

	T* find(const string& objname) {
		if (objname.length() == 0) return (T*)0;
		typename map<string, pair<unsigned, T*> >::iterator it = cache.find(objname);
		if (it == cache.end())
			return 0;
		return it->second.second;
	}

	T* ref(const string& objname) {
		if (objname.length() == 0) return (T*)0;
		typename map<string, pair<unsigned, T*> >::iterator it = cache.find(objname);
		if (it == cache.end()) {
			it = cache.insert(make_pair(objname, make_pair(1, new T(basedir + objname)))).first;
		} else {
			++(it->second.first);
		}
		return it->second.second;
	}

	void unref(const string& objname) {
		if (objname.length() == 0) return;
		typename map<string, pair<unsigned, T*> >::iterator it = cache.find(objname);
		if (it != cache.end()) {
			--(it->second.first);
			if (it->second.first == 0) {
				cache.erase(it);
			}
		}
	}
	
	void print(void) const {
		cout << "objcache: " << cache.size() << " entries.\n";
		for (typename map<string, pair<unsigned, T*> >::const_iterator it = cache.begin(); it != cache.end(); ++it)
			cout << "key=\"" << it->first << "\" ref=" << it->second.first << " addr=" << it->second.second << "\n";
	}
};

#endif
