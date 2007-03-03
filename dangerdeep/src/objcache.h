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

///\brief Handles and caches instances of globally used objects.
//fixme: to make it useable as *cache* we need to delay destruction. when an object reaches refcount zero, do not
//delete it immediatly. Check periodically or when the next object is deleted, so at least 1-2 objects can be hold
//with refcount zero, like a delete-queue. This avoids permanent reload when e.g. user switches between two menes
//and both use images of the image-cache.
//fixme2: maybe add special handler-class, like an auto_ptr, c'tor ref's an object, d'tor
//unrefs it. Thus objcache usage is easier.

// fixme 2: add "reference" class, that is auto_ptr like reference handler. Do NOT return plain
// pointers from the cache. Because if we generate resources by ref'ing the cache in some
// code and an exception is thrown, the ref'd objects won't get unref'd again, leading to
// memory waste (though NOT memory leaks)
template <class T>
class objcachet
{
	std::map<std::string, std::pair<unsigned, T*> > cache;
	std::string basedir;
	objcachet();
	objcachet<T>& operator= (const objcachet<T>& );
	objcachet(const objcachet<T>& );

public:
	objcachet(const std::string& basedir_) : basedir(basedir_) {}
	~objcachet() {
		clear();
	}

	// call to deinit cache
	void clear() {
		for (typename std::map<std::string, std::pair<unsigned, T*> >::iterator it = cache.begin(); it != cache.end(); ++it)
			delete it->second.second;
		cache.clear();
	}

	T* find(const std::string& objname) {
		if (objname.empty()) return (T*)0;
		typename std::map<std::string, std::pair<unsigned, T*> >::iterator it = cache.find(objname);
		if (it == cache.end())
			return 0;
		return it->second.second;
	}

	T* ref(const std::string& objname) {
		if (objname.empty()) return (T*)0;
		typename std::map<std::string, std::pair<unsigned, T*> >::iterator it = cache.find(objname);
		if (it == cache.end()) {
			it = cache.insert(std::make_pair(objname, std::make_pair(1, new T(basedir + objname)))).first;
		} else {
			++(it->second.first);
		}
		return it->second.second;
	}

	bool ref(const std::string& objname, T* obj) {
		if (objname.empty()) return false;	// no valid name
		typename std::map<std::string, std::pair<unsigned, T*> >::iterator it = cache.find(objname);
		if (it == cache.end()) {
			it = cache.insert(std::make_pair(objname, std::make_pair(1, obj))).first;
		} else {
			return false;	// already exists
		}
		return true;
	}

	void unref(const std::string& objname) {
		if (objname.empty()) return;
		typename std::map<std::string, std::pair<unsigned, T*> >::iterator it = cache.find(objname);
		if (it != cache.end()) {
			if (it->second.first == 0) {
				// error, unref'd too much...
				return;
			}
			--(it->second.first);
			if (it->second.first == 0) {
				delete it->second.second;
				cache.erase(it);
			}
		}
	}
	
	void unref(T* obj) {
		for (typename std::map<std::string, std::pair<unsigned, T*> >::iterator it = cache.begin(); it != cache.end(); ++it) {
			if (it->second.second == obj) {
				if (it->second.first == 0) {
					// error, unref'd too much...
					return;
				}
				--(it->second.first);
				if (it->second.first == 0) {
					delete it->second.second;
					cache.erase(it);
				}
				break;
			}
		}
	}
	
	void print() const {
		std::cout << "objcache: " << cache.size() << " entries.\n";
		for (typename std::map<std::string, std::pair<unsigned, T*> >::const_iterator it = cache.begin(); it != cache.end(); ++it)
			std::cout << "key=\"" << it->first << "\" ref=" << it->second.first << " addr=" << it->second.second << "\n";
	}
};

#endif
