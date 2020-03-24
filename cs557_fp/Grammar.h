#include <string>
#include <vector>
#include <tuple>
#include <time.h>
class Grammar {
private:
    std::string Gname; 
    int level;  
	std::string start;  
	std::string result;  
	std::vector<std::tuple<char, std::vector<std::string>>> generations; 
public:
    Grammar(){}
    void clear();
    int find(char ch);
	std::string search(const char ch);
    void addGeneration(const char& ch,  const std::string& ref);
    void iterateFor(int num);
    void setGrammarName(const std::string& ref);
    void setStart(const std::string& ref);
    void setLevel(int num);
	std::string getGrammarName();
    int getLevel();
	std::string getResult();
};