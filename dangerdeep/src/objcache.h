/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

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

	bool ref(const string& objname, T* obj) {
		if (objname.length() == 0) return false;	// no valid name
		typename map<string, pair<unsigned, T*> >::iterator it = cache.find(objname);
		if (it == cache.end()) {
			it = cache.insert(make_pair(objname, make_pair(1, obj))).first;
		} else {
			return false;	// already exists
		}
		return true;
	}

	void unref(const string& objname) {
		if (objname.length() == 0) return;
		typename map<string, pair<unsigned, T*> >::iterator it = cache.find(objname);
		if (it != cache.end()) {
			--(it->second.first);
			if (it->second.first == 0) {
				delete it->second.second;
				cache.erase(it);
			}
		}
	}
	
	void unref(T* obj) {
		for (typename map<string, pair<unsigned, T*> >::iterator it = cache.begin(); it != cache.end(); ++it) {
			if (it->second.second == obj) {
				--(it->second.first);
				if (it->second.first == 0) {
					delete it->second.second;
					cache.erase(it);
				}
				break;
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
