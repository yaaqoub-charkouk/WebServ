#include "FileUtils.hpp"
#include <dirent.h>
#include <algorithm>

FileInfo FileUtils::getInfo(const std::string &path)
{
    FileInfo info;
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
        return info;
    info.exist      = true;
    info.isDir      = S_ISDIR(st.st_mode);
    info.isFile     = S_ISREG(st.st_mode);
    info.isReadable = (access(path.c_str(), R_OK) == 0);
    info.size       = static_cast<long>(st.st_size);
    return info;
}

std::string FileUtils::getContent(const std::string &path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    std::stringstream buff;

    if (!file.is_open())
        return ("");

    buff << file.rdbuf();
    return buff.str();
}

static bool entryCompare(const DirEntry& a, const DirEntry& b)
{
    if (a.isDir != b.isDir)
        return a.isDir > b.isDir; // directories first
    return a.name < b.name;
}

std::vector<DirEntry> FileUtils::listDir(const std::string &path)
{
    std::vector<DirEntry> entries;
    DIR* dir = opendir(path.c_str());
    if (!dir)
        return entries;

    struct dirent* ent;
    while ((ent = readdir(dir)) != NULL)
    {
        std::string name = ent->d_name;
        if (name == "." || name == "..")
            continue;

        DirEntry entry;
        entry.name  = name;
        entry.isDir = (ent->d_type == DT_DIR);

        // Fallback for filesystems that don't report d_type
        if (ent->d_type == DT_UNKNOWN)
        {
            struct stat st;
            std::string fullPath = path + "/" + name;
            if (stat(fullPath.c_str(), &st) == 0)
                entry.isDir = S_ISDIR(st.st_mode);
        }

        entries.push_back(entry);
    }
    closedir(dir);

    std::sort(entries.begin(), entries.end(), entryCompare);
    return entries;
}
