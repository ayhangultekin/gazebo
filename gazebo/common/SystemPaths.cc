/*
 * Copyright 2011 Nate Koenig
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
/* Desc: Local Gazebo configuration
 * Author: Nate Koenig, Jordi Polo
 * Date: 3 May 2008
 */
#define BOOST_FILESYSTEM_VERSION 2

#include <assert.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

#include "gazebo/sdf/sdf.hh"
#include "gazebo/common/SystemPaths.hh"
#include "gazebo/common/Exception.hh"
#include "gazebo/common/Console.hh"

using namespace gazebo;
using namespace common;


//////////////////////////////////////////////////
SystemPaths::SystemPaths()
{
  this->gazeboPaths.clear();
  this->ogrePaths.clear();
  this->pluginPaths.clear();
  this->modelPaths.clear();

  char *homePath = getenv("HOME");
  std::string home;
  if (!homePath)
    home = "/tmp/gazebo";
  else
    home = homePath;

  this->modelPaths.push_back(home + "/.gazebo/models");

  char *path = getenv("GAZEBO_LOG_PATH");
  std::string fullPath;
  if (!path)
  {
    if (home != "/tmp/gazebo")
      fullPath = home + "/.gazebo";
    else
      fullPath = home;
  }
  else
    fullPath = path;

  DIR *dir = opendir(fullPath.c_str());
  if (!dir)
  {
    mkdir(fullPath.c_str(), S_IRWXU | S_IRGRP | S_IROTH);
  }
  else
    closedir(dir);

  this->logPath = fullPath;

  this->UpdateGazeboPaths();
  this->UpdatePluginPaths();
  this->UpdateOgrePaths();

  // Add some search paths
  // this->suffixPaths.push_back(std::string("/sdf/") + SDF_VERSION + "/");
  this->suffixPaths.push_back("/models/");
  this->suffixPaths.push_back("/media/models/");
  this->suffixPaths.push_back("/Media/models/");

  this->pluginPathsFromEnv = true;
  this->gazeboPathsFromEnv = true;
  this->ogrePathsFromEnv = true;
}

/////////////////////////////////////////////////
std::string SystemPaths::GetLogPath() const
{
  return this->logPath;
}

/////////////////////////////////////////////////
const std::list<std::string> &SystemPaths::GetGazeboPaths()
{
  if (this->gazeboPathsFromEnv)
    this->UpdateGazeboPaths();
  return this->gazeboPaths;
}

/////////////////////////////////////////////////
const std::list<std::string> &SystemPaths::GetPluginPaths()
{
  if (this->pluginPathsFromEnv)
    this->UpdatePluginPaths();
  return this->pluginPaths;
}

/////////////////////////////////////////////////
const std::list<std::string> &SystemPaths::GetModelPaths()
{
  return this->modelPaths;
}

/////////////////////////////////////////////////
const std::list<std::string> &SystemPaths::GetOgrePaths()
{
  if (this->ogrePathsFromEnv)
    this->UpdateOgrePaths();
  return this->ogrePaths;
}

/////////////////////////////////////////////////
void SystemPaths::UpdateGazeboPaths()
{
  std::string delim(":");
  std::string path;

  char *pathCStr = getenv("GAZEBO_RESOURCE_PATH");
  if (!pathCStr || *pathCStr == '\0')
  {
    // gzdbg << "gazeboPaths is empty and GAZEBO_RESOURCE_PATH doesn't exist. "
    //  << "Set GAZEBO_RESOURCE_PATH to gazebo's installation path. "
    //  << "...or are you using SystemPlugins?\n";
    return;
  }
  path = pathCStr;

  size_t pos1 = 0;
  size_t pos2 = path.find(delim);
  while (pos2 != std::string::npos)
  {
    this->InsertUnique(path.substr(pos1, pos2-pos1), this->gazeboPaths);
    pos1 = pos2+1;
    pos2 = path.find(delim, pos2+1);
  }
  this->InsertUnique(path.substr(pos1, path.size()-pos1), this->gazeboPaths);
}

//////////////////////////////////////////////////
void SystemPaths::UpdatePluginPaths()
{
  std::string delim(":");
  std::string path;

  char *pathCStr = getenv("GAZEBO_PLUGIN_PATH");
  if (!pathCStr || *pathCStr == '\0')
  {
    // gzdbg << "pluginPaths and GAZEBO_PLUGIN_PATH doesn't exist."
    //  << "Set GAZEBO_PLUGIN_PATH to Ogre's installation path."
    //  << " ...or are you loading via SystemPlugins?\n";
    return;
  }
  path = pathCStr;

  size_t pos1 = 0;
  size_t pos2 = path.find(delim);
  while (pos2 != std::string::npos)
  {
    this->InsertUnique(path.substr(pos1, pos2-pos1), this->pluginPaths);
    pos1 = pos2+1;
    pos2 = path.find(delim, pos2+1);
  }
  this->InsertUnique(path.substr(pos1, path.size()-pos1), this->pluginPaths);
}

//////////////////////////////////////////////////
void SystemPaths::UpdateOgrePaths()
{
  std::string delim(":");
  std::string path;

  char *pathCStr = getenv("OGRE_RESOURCE_PATH");
  if (!pathCStr || *pathCStr == '\0')
  {
    // gzdbg << "ogrePaths is empty and OGRE_RESOURCE_PATH doesn't exist. "
    //  << "Set OGRE_RESOURCE_PATH to Ogre's installation path. "
    //  << "...or are you using SystemPlugins?\n";
    return;
  }
  path = pathCStr;

  size_t pos1 = 0;
  size_t pos2 = path.find(delim);
  while (pos2 != std::string::npos)
  {
    this->InsertUnique(path.substr(pos1, pos2-pos1), this->ogrePaths);
    pos1 = pos2+1;
    pos2 = path.find(delim, pos2+1);
  }
  this->InsertUnique(path.substr(pos1, path.size()-pos1), this->ogrePaths);
}


//////////////////////////////////////////////////
std::string SystemPaths::GetWorldPathExtension()
{
  return "/worlds";
}

//////////////////////////////////////////////////
std::string SystemPaths::FindFileWithGazeboPaths(const std::string &_filename,
                                                 bool _searchLocalPath)
{
  return this->FindFile(_filename, _searchLocalPath);
}

//////////////////////////////////////////////////
std::string SystemPaths::FindFileURI(const std::string &_uri)
{
  int index = _uri.find("://");
  std::string prefix = _uri.substr(0, index);
  std::string suffix = _uri.substr(index + 3, _uri.size() - index - 3);
  std::string filename;

  // First try to find the file on the current system
  filename = this->FindFile(suffix);

  // If the file was found, then return the full path
  if (!filename.empty())
    return filename;

  // If trying to find a model, return the path to the users home
  // .gazebo/models
  if (prefix == "model")
  {
    std::string path = getenv("HOME");
    path += "/.gazebo/models/";
    filename = path + suffix;
  }
  else if (prefix != "http" && prefix != "https")
    gzerr << "Unknown URI prefix[" << prefix << "]\n";

  return filename;
}

//////////////////////////////////////////////////
std::string SystemPaths::FindFile(const std::string &_filename,
                                  bool _searchLocalPath)
{
  std::string result;

  if (_filename.empty())
    return result;

  if (_filename.find("://") != std::string::npos)
  {
    result = this->FindFileURI(_filename);
  }
  else if (_filename[0] == '/')
  {
    result = _filename;
  }
  else
  {
    struct stat st;
    result = std::string(getenv("PWD")) + "/" + _filename;
    bool found = false;

    std::list<std::string> paths = this->GetGazeboPaths();

    if (_searchLocalPath && stat(result.c_str(), &st) == 0)
    {
      found = true;
    }
    else if ((_filename[0] == '/' || _filename[0] == '.' || _searchLocalPath)
             && stat(_filename.c_str(), &st) == 0)
    {
      result = _filename;
      found = true;
    }
    else
    {
      for (std::list<std::string>::const_iterator iter = paths.begin();
          iter != paths.end() && !found; ++iter)
      {
        result = (*iter) + "/" + _filename;
        if (stat(result.c_str(), &st) == 0)
        {
          found = true;
          break;
        }

        std::list<std::string>::iterator suffixIter;
        for (suffixIter = this->suffixPaths.begin();
            suffixIter != this->suffixPaths.end(); ++suffixIter)
        {
          result = (*iter) + *suffixIter + _filename;
          if (stat(result.c_str(), &st) == 0)
          {
            found = true;
            break;
          }
        }
      }
    }

    if (!found)
    {
      result.clear();
      return std::string();
    }
  }

  boost::filesystem::path dir(result);
  if (!boost::filesystem::exists(dir))
  {
    gzerr << "File or path does not exist[" << result << "]\n";
    result.clear();
  }

  return result;
}

/////////////////////////////////////////////////
void SystemPaths::ClearGazeboPaths()
{
  this->gazeboPaths.clear();
}

/////////////////////////////////////////////////
void SystemPaths::ClearOgrePaths()
{
  this->ogrePaths.clear();
}

/////////////////////////////////////////////////
void SystemPaths::ClearPluginPaths()
{
  this->pluginPaths.clear();
}

/////////////////////////////////////////////////
void SystemPaths::AddGazeboPaths(const std::string &_path)
{
  std::string delim(":");

  size_t pos1 = 0;
  size_t pos2 = _path.find(delim);
  while (pos2 != std::string::npos)
  {
    this->InsertUnique(_path.substr(pos1, pos2-pos1), this->gazeboPaths);
    pos1 = pos2+1;
    pos2 = _path.find(delim, pos2+1);
  }
  this->InsertUnique(_path.substr(pos1, _path.size()-pos1), this->gazeboPaths);
}

/////////////////////////////////////////////////
void SystemPaths::AddOgrePaths(const std::string &_path)
{
  std::string delim(":");
  size_t pos1 = 0;
  size_t pos2 = _path.find(delim);
  while (pos2 != std::string::npos)
  {
    this->InsertUnique(_path.substr(pos1, pos2-pos1), this->ogrePaths);
    pos1 = pos2+1;
    pos2 = _path.find(delim, pos2+1);
  }
  this->InsertUnique(_path.substr(pos1, _path.size()-pos1), this->ogrePaths);
}

/////////////////////////////////////////////////
void SystemPaths::AddPluginPaths(const std::string &_path)
{
  std::string delim(":");
  size_t pos1 = 0;
  size_t pos2 = _path.find(delim);
  while (pos2 != std::string::npos)
  {
    this->InsertUnique(_path.substr(pos1, pos2-pos1), this->pluginPaths);
    pos1 = pos2+1;
    pos2 = _path.find(delim, pos2+1);
  }
  this->InsertUnique(_path.substr(pos1, _path.size()-pos1), this->pluginPaths);
}

/////////////////////////////////////////////////
void SystemPaths::InsertUnique(const std::string &_path,
                               std::list<std::string> &_list)
{
  if (std::find(_list.begin(), _list.end(), _path) == _list.end())
    _list.push_back(_path);
}

/////////////////////////////////////////////////
void SystemPaths::AddSearchPathSuffix(const std::string &_suffix)
{
  std::string s;

  if (_suffix[0] != '/')
    s = std::string("/") + _suffix;
  else
    s = _suffix;

  if (_suffix[_suffix.size()-1] != '/')
    s += "/";

  this->suffixPaths.push_back(s);
}
