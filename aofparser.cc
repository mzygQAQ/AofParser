/**
 * @file aofparser.cc
 * @author guoxiang (mzygdeaq@163.com)
 * @brief a parser of redis aof files.
 * @version 0.1
 * @date 2021-11-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#ifdef __GNUC__
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

void FATAL(std::string msg)
{
    std::cout << "FATAL: " << msg << std::endl;
    std::exit(1);
}

/**
 * @brief 
 * 
 * @param str 
 * @param begin contains
 * @param end  not contains
 * @return int 
 */
int ToInt(const std::string &str, std::size_t begin, std::size_t end)
{
    std::stringstream ss;
    while (begin < end)
    {
        ss.put(str[begin]);
        begin++;
    }
    int ret = 0;
    ss >> ret;
    return ret;
}
void GetLine(std::ifstream &aof, std::string &line /* OUT */)
{
    std::getline(aof, line, '\n');
    if (!line.empty())
    {
        assert(line[line.length() - 1] == '\r');
        line.pop_back();
    }
}

int ParseArrayLen(const std::string &line)
{
    assert(line[0] == '*');
    return ToInt(line, 1, line.length());
}

int ParseBulkLen(const std::string &line)
{
    assert(line[0] == '$');
    return ToInt(line, 1, line.length());
}

void ParseBuck(std::ifstream &aof, std::string &line /* OUT */, size_t N)
{
    // Cannot readLine because of userdata may have \r\n .
    line.resize(N + strlen("\r\n"));
    aof.read(&line[0], line.size());
    assert(line[line.size() - 1] == '\n');
    assert(line[line.size() - 2] == '\r');
    line.pop_back(); // pop \n
    line.pop_back(); // pop \r
}

std::string ParseSingleline(const std::string &line)
{
    assert(line[0] == '+');
    return line.substr(1);
}

std::string ParseError(const std::string &line)
{
    assert(line[0] == '-');
    return line.substr(1);
}

std::string ParseInteger(const std::string &line)
{
    assert(line[0] == ':');
    return line.substr(1);
}

std::string ParseOneMsg(std::ifstream &aof)
{
    std::string retval;
    std::string line;
    GetLine(aof, line);
    if (aof.eof() || line.empty())
    {
        return retval;
    }

    switch (line[0])
    {
    case '*':
    {
        /* array */
        int arraylen = ParseArrayLen(line);
        assert(arraylen > 0);
        for (int i = 0; i < arraylen; i++)
        {
            std::string tmp = ParseOneMsg(aof);
            retval += tmp;
            retval += " ";
        }
        break;
    }
    case '$':
    {
        /* bulk strings */
        int bulklen = ParseBulkLen(line);
        ParseBuck(aof, line, bulklen);
        assert(bulklen == line.size());
        retval += line;
        break;
    }
    case '+':
        return ParseSingleline(line);
    case '-':
        return ParseError(line);
    case ':':
        return ParseInteger(line);
    default:
        FATAL("illegal resp start symbols");
        break;
    }
    return retval;
}

// aofparser myfile.aof
int main(int argc, char **argv)
{
    if (argc != 2 || !strcmp("--help", argv[1]))
    {
        std::cout << "aofparser.app path/to/you/aof/file.aof" << std::endl;
        return 0;
    }

    std::ifstream aof_file;
    aof_file.open(argv[1], std::ifstream::in);
    if (!aof_file.good())
    {
        FATAL("Failed to open aof file givend.");
    }

    // Check
#ifdef __GNUC__
    struct stat st;
    stat(argv[1], &st);
    if (st.st_mode & S_IFDIR)
    {
        FATAL("Expected a aof file, but a directory was given!");
    }

    // 普通aof要么是空的要么不会低于5字节
    // aof混合版至少不会低于5直接 "REDIS"
    if (st.st_size > 0 && st.st_size < 5)
    {
        FATAL("Invalid aof file format!");
    }
#endif

    // check aof_use_rdb_preamble
    char sig[5] = {0};
    aof_file.read(sig, 5);
    assert(aof_file.good());

    if (!std::strncmp(sig, "REDIS", 5)) {
        FATAL("aof_use_rdb_preamble not supported!!!");
    } else {
        // 不是AOF混合版则seek到开头.
        aof_file.seekg(0, std::ios_base::beg);
    }

    // Parse loop
    while (aof_file.good())
        std::cout << ParseOneMsg(aof_file) << std::endl;

    return 0;
}
