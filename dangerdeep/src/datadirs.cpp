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

// global directory data
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "datadirs.h"
#include "error.h"
#include "filehelper.h"
#include "log.h"

// Note! this is a global variable and is inited before main,
// any data depening on that variable, like objcache, textures etc. etc.
// MUST be inited after main starts, NOT as global static variables themselves,
// or the static initiliazation order fiasco will result.
std::string global_datadir = DATADIR;

const std::string& get_data_dir()
{
	return global_datadir;
}



// Note! call this only once, and very early in main()!
void set_data_dir(const std::string& datadir)
{
	global_datadir = datadir;
}



data_file_handler::data_file_handler()
{
	// scan data dir for all .data files
	std::string dir = "objects/";
	parse_for_data_files(dir + "airplanes/", airplane_ids);
	parse_for_data_files(dir + "ships/", ship_ids);
	parse_for_data_files(dir + "submarines/", submarine_ids);
	parse_for_data_files(dir + "torpedoes/", torpedo_ids);
	parse_for_data_files(dir + "props/", prop_ids);
}



static const std::string data_file_ext = ".data";
void data_file_handler::parse_for_data_files(std::string dir, std::list<std::string>& idlist)
{
	directory d(get_data_dir() + dir);
	for (std::string f = d.read(); !f.empty(); f = d.read()) {
		if (f[0] == '.' || f == "CVS") {
			// avoid . and .. entries, as well as hidden files, and CVS directories as well
			continue;
		} else if (is_directory(get_data_dir() + dir + f)) {
			parse_for_data_files(dir + f + "/", idlist);
		} else if (f.length() > data_file_ext.length() && f.substr(f.length() - data_file_ext.length()) == data_file_ext) {
			std::string id = f.substr(0, f.length() - data_file_ext.length());
// 			log_info("found file " << dir << " for id " << id);
			data_files[id] = dir;
			idlist.push_back(id);
		}
	}
}



data_file_handler* data_file_handler::my_instance = 0;



const data_file_handler& data_file_handler::instance()
{
	if (!my_instance)
		my_instance = new data_file_handler();
	return *my_instance;
}



const std::string& data_file_handler::get_rel_path(const std::string& objectid) const
{
	static std::string emptystr;
	std::map<std::string, std::string>::const_iterator it = data_files.find(objectid);
	if (it == data_files.end()) {
		throw error(std::string("can't find path for object '") + objectid + std::string("'"));
	}
	return it->second;
}



std::string data_file_handler::get_path(const std::string& objectid) const
{
	return get_data_dir() + get_rel_path(objectid);
}



std::string data_file_handler::get_rel_filename(const std::string& objectid) const
{
	return get_rel_path(objectid) + objectid + data_file_ext;
}



std::string data_file_handler::get_filename(const std::string& objectid) const
{
	return get_data_dir() + get_rel_filename(objectid);
}
