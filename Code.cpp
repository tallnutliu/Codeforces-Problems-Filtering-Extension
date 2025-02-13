#include <iostream>
#include <Windows.h>
#include <cstdlib>
#include <string>
#include <cstdio>
#include <vector>
#include <algorithm>
#include "rapidjson/document.h"
#pragma comment(lib,"Urlmon.lib")
using namespace std;
using namespace rapidjson;
string command;
char* problemset;
Document json;
vector<string> must_have_tags,must_not_have_tags,must_have_one_tags;
int minrating,maxrating,sortorder;
int tmprating;
Value filtered_problems(kArrayType);
void end() { Sleep(INFINITE); }
void show()
{
    puts("Welcome to CodeForces Problems Filtering Extension!");
    puts("Please input \"-help\" to show help, or \"-update\" to update the problemset saved.");
    puts("Or else, input the file name of your rule.");
}
void help()
{
    puts("You need to put a rules file in the same directory as this program.");
    puts("The format of the rules file:");
    puts("  The first line: the tags that a problem must contain all of them, separated by commas. Can be leaved blank, which means you do not care about this aspect.");
    puts("  The second line: the tags that a problem must not contain any of them, separated by commas. Can be leaved blank, which means you do not care about this aspect.");
    puts("  The third line: the tags that a problem must contain at least one of them, separated by commas. Can be leaved blank, which means you do not care about this aspect.");
    puts("  The fourth line: the minimum rating of a problem. Can be \"-1\", which means you do not care about this aspect.");
    puts("  The fifth line: the maximum rating of a problem. Can be \"-1\", which means you do not care about this aspect.");
    puts("  The sixth line: how to sort the problems.");
    puts("    \'0\' means you do not want to sort.");
    puts("    \'1\' means you want to sort the problems by their rating, in ascending order.");
    puts("    \'2\' means you want to sort the problems by their rating, in descending order.");
    puts("Especially, please use the tag \"*special\" instead of \"*special problem\".");
    puts("The result will be wrote in an HTML file.");
}
void readproblemset()
{
    FILE* file;
    puts("Reading from problemset file...");
    fopen_s(&file,"problemset.json", "rb");
    if (file != 0)
    {
        fseek(file,0,SEEK_END);
        int len = ftell(file);
        fseek(file,0,SEEK_SET);
        problemset = (char*)calloc(static_cast<size_t>(len) + 1,1);
        if (problemset != nullptr)
        {
            size_t result = fread(problemset,1,len,file);
            if (result != len)
            {
                puts("Oops! Cannot read from problemset file.");
                end();
            }
        }
        else
        {
            puts("Oops! Cannot read from problemset file.");
            end();
        }
        fclose(file);
    }
    else
    {
        puts("Oops! Cannot read from problemset file.");
        end();
    }
    puts("Parsing problemset...");
    json.Parse(const_cast<const char*>(problemset));
    if (!json.IsObject())
    {
        puts("Oops! Format error in problemset file.");
        end();
    }
    if (json["status"].GetString() == "FAILED")
    {
        puts("Oops! CodeForces returns \"FAILED\" status while updating problemset.");
        end();
    }
    puts("Read from problemset file successfully!");
}
void update()
{
    string url = "https://codeforces.com/api/problemset.problems";
    puts("Reading from CodeForces...");
    HRESULT result = URLDownloadToFileA(NULL,url.c_str(),"problemset.json",0,NULL);
    if (result != S_OK)
    {
        puts("Oops! Cannot read from CodeForces.");
        end();
    }
    readproblemset();
    puts("Updated successfully!");
}
void readrules()
{
    FILE* file;
    freopen_s(&file,command.c_str(),"r",stdin);
    if (file == nullptr)
    {
        puts("Oops! Cannot read from rules file.");
        end();
    }
    string s,tmps;
    getline(cin,s);
    for (int i = 0;i < s.size();i++)
    {
        if (s[i] == ',')
        {
            if (tmps != "") must_have_tags.push_back(tmps);
            tmps = "";
        }
        else tmps += s[i];
    }
    if (tmps != "") must_have_tags.push_back(tmps);
    tmps = "";
    getline(cin,s);
    for (int i = 0;i < s.size();i++)
    {
        if (s[i] == ',')
        {
            if (tmps != "") must_not_have_tags.push_back(tmps);
            tmps = "";
        }
        else tmps += s[i];
    }
    if (tmps != "") must_not_have_tags.push_back(tmps);
    tmps = "";
    getline(cin,s);
    for (int i = 0;i < s.size();i++)
    {
        if (s[i] == ',')
        {
            if (tmps != "") must_have_one_tags.push_back(tmps);
            tmps = "";
        }
        else tmps += s[i];
    }
    if (tmps != "") must_have_one_tags.push_back(tmps);
    tmps = "";
    cin >> minrating;
    if (cin.fail())
    {
        puts("Oops! Format error in the rules file.");
        end();
    }
    if (minrating == -1) minrating = 0;
    cin >> maxrating;
    if (cin.fail())
    {
        puts("Oops! Format error in the rules file.");
        end();
    }
    if (maxrating == -1) maxrating = 10000;
    cin >> sortorder;
    if (cin.fail())
    {
        puts("Oops! Format error in the rules file.");
        end();
    }
    if (sortorder != 0 && sortorder != 1 && sortorder != 2)
    {
        puts("Oops! Sorting order error in the rules file.");
        end();
    }
    puts("Read from rules file successfully!");
}
void filter()
{
    const Value& problems = json["result"]["problems"].GetArray();
    if (!problems.IsArray())
    {
        puts("Oops! Format error in the rules file.");
        end();
    }
    for (SizeType i = 0;i < problems.Size();i++)
    {
        vector<string> tmptags;
        const Value& cur = problems[i];
        if (cur.HasMember("rating"))
        {
            const Value& rating = cur["rating"];
            if (!rating.IsNumber())
            {
                puts("Oops! Format error in the rules file.");
                end();
            }
            tmprating = cur["rating"].GetInt();
        }
        else tmprating = 0;
        if (tmprating < minrating || tmprating > maxrating) continue;
        if (cur.HasMember("tags"))
        {
            const Value& tags = cur["tags"];
            if (!tags.IsArray())
            {
                puts("Oops! Format error in the rules file.");
                end();
            }
            for (SizeType j = 0;j < tags.Size();j++)
            {
                const Value& curtag = tags[j];
                if (!curtag.IsString())
                {
                    puts("Oops! Format error in the rules file.");
                    end();
                }
                tmptags.push_back(curtag.GetString());
            }
        }
        else
        {
            puts("Oops! Format error in the rules file.");
            end();
        }
        bool flag_hasall = true;
        bool flag_hasnot = true;
        bool flag_hasone = false;
        for (int i = 0;i < must_have_tags.size();i++)
        {
            bool have_this_tag = false;
            for (int j = 0;j < tmptags.size();j++)
                if (must_have_tags[i] == tmptags[j])
                {
                    have_this_tag = true;
                    break;
                }
            if (!have_this_tag)
            {
                flag_hasall = false;
                break;
            }
        }
        if (!flag_hasall) continue;
        for (int i = 0;i < must_not_have_tags.size();i++)
        {
            bool have_not_this_tag = true;
            for (int j = 0;j < tmptags.size();j++)
                if (must_not_have_tags[i] == tmptags[j])
                {
                    have_not_this_tag = false;
                    break;
                }
            if (!have_not_this_tag)
            {
                flag_hasnot = false;
                break;
            }
        }
        if (!flag_hasnot) continue;
        for (int i = 0;i < must_have_one_tags.size();i++)
        {
            bool have_this_tag = false;
            for (int j = 0;j < tmptags.size();j++)
                if (must_have_one_tags[i] == tmptags[j])
                {
                    have_this_tag = true;
                    break;
                }
            if (have_this_tag)
            {
                flag_hasone = true;
                break;
            }
        }
        if (must_have_one_tags.size() == 0) flag_hasone = true;
        if (!flag_hasone) continue;
        Value tmpvalue(kObjectType);
        tmpvalue.CopyFrom(cur,json.GetAllocator());
        filtered_problems.PushBack(tmpvalue,json.GetAllocator());
    }
    puts("Problems filtered successfully!");
}
void sortproblems_1(int l,int r)
{
    Value& fil = filtered_problems.GetArray();
    int pivot = fil[r]["rating"].GetInt();
    int k = l - 1;
    for (SizeType j = l;j < r;j++)
        if (fil[j]["rating"].GetInt() < pivot) swap(fil[j],fil[++k]);
    swap(fil[r],fil[++k]);
    if (l < k - 1) sortproblems_1(l,k - 1);
    if (k + 1 < r) sortproblems_1(k + 1,r);
}
void sortproblems_2(int l,int r)
{
    Value& fil = filtered_problems.GetArray();
    int pivot = fil[r]["rating"].GetInt();
    int k = l - 1;
    for (SizeType j = l;j < r; j++)
        if (fil[j]["rating"].GetInt() > pivot) swap(fil[j],fil[++k]);
    swap(fil[r],fil[++k]);
    if (l < k - 1) sortproblems_2(l,k - 1);
    if (k + 1 < r) sortproblems_2(k + 1,r);
}
void sortproblems()
{
    puts("Sorting filtered problems...");
    if (sortorder == 1) sortproblems_1(0,filtered_problems.Size() - 1);
    if (sortorder == 2) sortproblems_2(0,filtered_problems.Size() - 1);
    puts("Sorted successfully!");
}
void output()
{
    puts("Writing the result to \"filtered_result.html\"...");
    FILE* file;
    freopen_s(&file,"filtered_result.html","w",stdout);
    if (file != 0)
    {
        puts("<!DOCTYPE html>");
        puts("<html>");
        puts("<head>");
        puts("  <title>Filtered Result</title>");
        puts("</head>");
        puts("<body>");
        Value& array = filtered_problems.GetArray();
        for (SizeType i = 0; i < array.Size(); i++)
        {
            cout << "   <a href=\"https://codeforces.com/problemset/problem/";
            cout << array[i]["contestId"].GetInt() << '/';
            cout << array[i]["index"].GetString();
            cout << "\"target=\"_blank\">" << array[i]["name"].GetString();
            cout << "</a>\n<br>\n";
        }
        puts("</body>");
        puts("</html>");
        fclose(file);
        cerr << "Wrote successfully!";
    }
    else
    {
        puts("Oops! Can't output the result into \"filtered_result.html\".");
        end();
    }
}
int main()
{
    show();
    cin >> command;
    if (command == "-help") help();
    else if (command == "-update") update();
    else
    {
        readproblemset();
        readrules();
        filter();
        sortproblems();
        output();
    }
    end();
}
