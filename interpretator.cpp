#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>

using namespace std;

std::ifstream ins;

enum lex_type {L_NULL,
        VAR, WHILE, WRITE,                                                //reserved_words
        L_DIGIT, L_IDENT,
        L_PLUS, L_MINUS, L_DIV, L_MULT, SEMICOLUMN, EQ, L_MORE,           //tokens
        OPEN_BRACE, CLOSE_BRACE, OPEN_FIGURE_BRACE, CLOSE_FIGURE_BRACE,   //tokens
        L_END};
enum state {S, DIGIT, IDENT};

const std::vector<string> reserved_words = {"var", "while", "write"};
const std::vector<string> tokens = {"+", "-", "/", "*", ";", "=", ">", "(", ")", "{", "}"};

class Lex{
  lex_type l_type;
  int value;
  string str;
 public:
  Lex(lex_type l = L_NULL, int v=0, string s = "") : l_type(l), value(v), str(s) {}
  void print(){
    cout << "lexem with type " << l_type << " with value " ;
    if(str.empty()) cout << value << endl;
    else cout << str << endl;
  }
  lex_type get_type() const {return l_type; }
};

class Ident{
  lex_type l_type;
  int value;
  string name;
 public:
  Ident() {}
  Ident(string& n) : name(n) {}
  bool operator==(const string& s){ return name == s; }

  void print(){
    cout << "lexem with type " << l_type << " with value " ;
    if(name == "") cout << value << endl;
    else cout << name << endl;
  }
};

std::vector<Ident> table_of_ident;

int add_to_table(string &new_id){
  std::vector<Ident>::iterator i;
  if((i = find(table_of_ident.begin(), table_of_ident.end(), new_id)) != table_of_ident.end())
    return i - table_of_ident.begin();
  else {
    Ident I(new_id);
    table_of_ident.push_back(I);
    return table_of_ident.size() - 1;
  }
}

class Scanner{
  char c;
  bool should_read;
  void gc() {cin.read(&c, 1);}
 public:
  Scanner(): should_read(true) {}
  Scanner(const Scanner &s) : c(s.c), should_read(s.should_read) {}
  Lex get_Lex();
};

int find_in_buff(const std::vector<string> &buff, string &str){
  int res;
  for(int i=0; i < buff.size(); ++i) {
    if(str == buff[i]) return i;
  }
  return -1;
}

Lex Scanner::get_Lex(){
  string new_lex = "";
  int new_dig = 0;
  state cur_state = S;

  while(1) {
    if (should_read) gc();
    else should_read = true;

    switch (cur_state) {
      case S:
        if (cin.eof()) return Lex(L_END);
        if(!isspace(c)) {
          if (isalpha(c)) {
            cur_state = IDENT;
            new_lex.push_back(c);
          } else if (isdigit(c)) {
            new_dig = c - '0';
            cur_state = DIGIT;
          } else {
            /*if (c == '=') return Lex(EQ);
            else if (c == ';') return Lex(SEMICOLUMN, 0, "");
            else if (c == '>') return Lex(L_MORE);
            else if (c == '+') return Lex(L_PLUS);
            else if (c == '-') return Lex(L_MINUS);
            else if (c == '*') return Lex(L_MULT);
            else if (c == '/') return Lex(L_DIV);
            else if (c == '(') return Lex(OPEN_BRACE);
            else if (c == ')') return Lex(CLOSE_BRACE);
            else if (c == '{') return Lex(OPEN_FIGURE_BRACE);
            else if (c == '}') return Lex(CLOSE_FIGURE_BRACE);*/
            new_lex.push_back(c);
            int pos = find_in_buff(tokens, new_lex);
            if(pos == -1) throw c;
            else return Lex((lex_type)(pos+6), 0, new_lex);
          }
        }
        break;
      case DIGIT:
        if(isdigit(c)){
          new_dig = new_dig*10 + c - '0';
        } else if(isalpha(c)) throw c;
        else {
          should_read = false;
          return Lex(L_DIGIT, new_dig);
        }
        break;
      case IDENT:
        if(isalpha(c) || isdigit(c) || (c == '_')){
          new_lex += c;
        } else {
          should_read = false;
          int is_res = find_in_buff(reserved_words, new_lex);
          if (is_res == -1) {
            is_res = add_to_table(new_lex);
            return Lex(L_IDENT, is_res, new_lex);
          }
          else return Lex((lex_type)(is_res + 1), is_res, new_lex);
        }
        break;
    }
  }
}

int main(int argc, char** argv){
  if(argc > 1) {
    ins.open(argv[1]);
    if(!ins) {
      throw "input file not opening\n";
    }
    cin.rdbuf(ins.rdbuf());
  } else cout << "standart input" << endl;


  Scanner sc;
  Lex l;

  while(1){
    try{
      l = sc.get_Lex();
      l.print();
      if(l.get_type() == L_END) break;
    }
    catch(char ch) {cout << "error at " << ch << endl;}
    catch(string s) {cout << "error: " << s << endl;}
  }
}