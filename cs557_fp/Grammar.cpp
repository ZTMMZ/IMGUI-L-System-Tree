#include "grammar.h"
#include <iostream>
void Grammar::clear() {
    generations.clear();
}
int Grammar::find(char ch) {
    for(int i=0; i< generations.size(); i++) {
        if(std::get<0>(generations[i]) == ch)
            return i;
    }
    return -1;   //Cannot find, return -1
}
std::string Grammar::search(const char ch) {
    int id = find(ch);
	if (id == -1) {
		std::string str(&ch);
		return str;
	}
    int num = std::get<1>(generations[id]).size();
    int index = rand()%num;
    return std::get<1>(generations[id])[index];
}
void Grammar::addGeneration(const char& ch, const std::string& ref) {

    int id = find(ch);
    if(id == -1){
        std::tuple<char, std::vector<std::string> > temp;
		std::get<0>(temp)=ch;
		std::get<1>(temp).push_back(ref);
        generations.push_back(temp);
        return;
    }
	std::get<1>(generations[id]).push_back(ref);
}
void Grammar::iterateFor(int num) {
    setLevel(num);
	result = start;
    for(int i=0; i<num ;i++){
		std::string tmp = "";
        for(int j=0; j<result.size();j++){
            tmp+=search(result[j]);
        }
		result.assign(tmp);
    }
}
void Grammar::setGrammarName(const std::string& ref)
{
    Gname = ref;
}
void Grammar::setStart(const std::string& ref)
{
    start = ref;
}
void Grammar::setLevel(int num)
{
    level = num;
}
std::string Grammar::getGrammarName()
{
    return Gname;
}
int Grammar::getLevel()
{
    return level;
}
std::string Grammar::getResult()
{
    return result;
}