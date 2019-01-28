#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
using namespace std;

#define NIL -9999

enum type
{
   INT,
   BOOL,
   EPSILON
};

struct lexer
{
   string token;
   string lexeme;
};
struct table_elem
{
   string type;
   string id;
   int address;
};

struct instruction
{
   string _operator;
   int _operand;
};

vector<lexer> v;
vector<table_elem> symbol_table;
vector<instruction> instruction_table;

stack<int> jumpstack;
int curr_address = 2000;
string curr_type;
int counter = 0;
bool is_in_declaration = false;
bool is_negative;

void Rat18S();
void Opt_Function_Definitions();
void Opt_Declaration_List();
void Statement_List();
void Function_Definitions();
void Opt_Parameter_List();
void Body();
void Parameter_List();
void Parameter();
void Parameter_List_Prime();
void IDS();
void Qualifier();
void IDS_Prime();
void Declaration_List();
void Declaration();
void Declaration_List_Prime();
void Function();
void Function_Definitions_Prime();
void Statement();
void Statement_List();
void Statement_List_Prime();
void Compound();
void Assign();
void IF();
void Return();
void Print();
void Scan();
void While();
void Expression();
void Condition();
void IF_Prime();
void Term();
void Return_Prime();
void Expression_Prime();
void Factor();
void Term_Prime();
void Primary();
string Relop();
bool Empty(int);
void Empty();

void back_patch(int addr) {
   int line_num=jumpstack.top();
   jumpstack.pop();
   instruction_table[line_num]._operand=addr;
}

bool is_on_table(string id)
{
   for (auto elem : symbol_table)
   {
      if (elem.id == id)
      {
         return true;
      }
   }
   return false;
}

int get_symbol_address(string id)
{
   for (auto elem : symbol_table)
   {
      if (elem.id == id)
      {
         return elem.address;
      }
   }
   return 0;
}

string get_symbol_type(string id)
{
   for (auto elem : symbol_table)
   {
      if (elem.id == id)
      {
         return elem.type;
      }
   }
   return "";
}
bool push_onto_symbol_table(string id)
{
   if (!is_on_table(id))
   {
      table_elem insert;
      insert.id = id;
      insert.type = curr_type;
      insert.address = curr_address;
      curr_address++;
      symbol_table.push_back(insert);
      return true;
   }
   else
   {
      return false;
   }
}

void gen_instr(string op, int operand)
{

   instruction i;
   i._operator = op;
   i._operand = operand;
   instruction_table.push_back(i);
}

int main()
{
   fstream file;
   file.open("output.txt", ios::in);
   string t, l, temp;
   file >> temp;
   file >> temp;
   file >> temp;
   while (file >> t >> l)
   {
      lexer lex;
      lex.token = t;
      lex.lexeme = l;
      v.push_back(lex);
   }
   while (counter + 1 != v.size())
   {
      Rat18S();
   }
   cout << "Symbol Table:\n";
   for (auto elem : symbol_table)
   {
      cout << elem.id << " " << elem.type << " " << elem.address << "\n";
   }
   int increment=1;
   cout << endl;
   cout << "Assembly Instruction:\n";
   for(auto elem : instruction_table)
   {
      cout << increment << ". " << elem._operator << " " << elem._operand<<endl;
      increment++;
   }
   return 0;
}

//<Rat18S> → <Opt Function Definitions>   %%  <Opt Declaration List>  <Statement List>
void Rat18S()
{
   try
   {
      Opt_Function_Definitions();
   }
   catch (const char *msg)
   {
      cerr << msg << endl;
   }
   if (v[counter].token == "Separator" && v[counter].lexeme == "%%")
   {
      try
      {
         Opt_Declaration_List();
         Statement_List();
      }
      catch (const char *msg)
      {
         cerr << msg << endl;
         exit(1);
      }
   }
}
//<Opt Function Definitions> → <Function Definitions>  |  ε
void Opt_Function_Definitions()
{
   if (v[counter].token == "Keyword" && v[counter].lexeme == "function") // epsilon
   {
      Function_Definitions();
      counter++;
   }
}

//<Function Definitions> → <Function> <Function Definitions_Prime>
void Function_Definitions()
{
   Function();
   Function_Definitions_Prime();
}

//<Function> →  function  <Identifier>  [ <Opt Parameter List> ]  <Opt Declaration List>  <Body>
void Function()
{
   if (v[counter].token == "Keyword" && v[counter].lexeme == "function")
   {
      counter++;
      if (v[counter].token == "Identifier")
      {
         counter++;
         if (v[counter].token == "Separator" && v[counter].lexeme == "[")
         {
            Opt_Parameter_List();
            counter++;
            if (v[counter].token == "Separator" && v[counter].lexeme == "]")
            {
               Opt_Declaration_List();
               Body();
            }
         }
         else
         {
            throw "Error at Function()";
         }
      }
      else
      {
         throw "Error at Function()";
      }
   }
   else
   {
      throw "Error at Function()";
   }
}

//<Function Definitions_Prime> → ε | <Function Definitions>

void Function_Definitions_Prime()
{
   if (v[counter + 1].token == "Keyword" && v[counter + 1].lexeme == "function")
   {
      Function_Definitions();
   }
}

//<Opt Parameter List> →   <Parameter List> | ε
void Opt_Parameter_List()
{
   if (v[counter + 1].token == "Identifier") //epsilon
   {
      Parameter_List();
   }
}

//<Parameter List> → <Parameter> <Parameter List_Prime>
void Parameter_List()
{
   Parameter();
   Parameter_List_Prime();
}

//<Parameter> →  <IDs > : <Qualifier>
void Parameter()
{
   counter++;
   IDS();
   counter++;
   if (v[counter].token == "Separator" && v[counter].lexeme == ":")
   {
      Qualifier();
   }
}

//<Parameter List_Prime> → ε | , <Parameter List>
void Parameter_List_Prime()
{
   if (v[counter + 1].token == "Separator" && v[counter + 1].lexeme == ",")
   {
      counter++;
      Parameter_List();
   }
}

//<IDs> → <Identifier> <IDs_Prime>
void IDS()
{
   if (v[counter].token == "Identifier")
   {
      if (is_in_declaration)
      {
         if (!push_onto_symbol_table(v[counter].lexeme))
         {
            throw "Error-Redeclaration of variable\n";
         }
      }
      else {
         gen_instr("STDIN", NIL);
         gen_instr("POPM", get_symbol_address(v[counter].lexeme));
      }
      IDS_Prime();
   }
   else
   {
      throw "Error at IDS()";
   }
}

//<IDs_Prime> → , <IDs>  |  ε
void IDS_Prime()
{

   if (v[counter + 1].token == "Separator" && v[counter + 1].lexeme == ",")
   {
      counter += 2;
      IDS();
   }
}

//<Qualifier> → int  |  boolean    |  real
void Qualifier()
{
   counter++;
   if (v[counter].token == "Keyword" && (v[counter].lexeme == "int" || v[counter].lexeme == "boolean" || v[counter].lexeme == "real"))
   {
      curr_type = v[counter].lexeme;
      return;
   }
   throw "Error at Qualifier()";
}

//<Opt Declaration List> → <Declaration List>   |   ε
void Opt_Declaration_List() //Hard coded checking
{
   if (v[counter + 1].token == "Keyword" && (v[counter + 1].lexeme == "int" || v[counter + 1].lexeme == "boolean" || v[counter + 1].lexeme == "real")) //Epsilon from qualifier
   {
      Declaration_List();
   }
}

//<Declaration List> → <Declaration> ; <Declaration List_Prime>
void Declaration_List()
{
   Declaration();
   counter++;
   if (v[counter].token == "Separator" && v[counter].lexeme == ";")
   {
      Declaration_List_Prime();
   }
   else
   {
      throw "Error at Declaration_List()";
   }
}

//<Declaration List_Prime> → ε | <Declaration List>
void Declaration_List_Prime()
{
   if (v[counter + 1].token == "Keyword" && (v[counter + 1].lexeme == "int" || v[counter + 1].lexeme == "boolean" || v[counter + 1].lexeme == "real")) //Not a declaration list, Epsilon, HARD CODED THE "{"
   {
      Declaration_List();
   }
}

//<Declaration> →  <Qualifier> <IDs>
void Declaration()
{
   is_in_declaration = true;
   Qualifier();
   counter++;
   IDS();
   is_in_declaration = false;
}

//<Body> → {  < Statement List>  }
void Body()
{
   counter++;
   if (v[counter].token == "Separator" && v[counter].lexeme == "{")
   {
      Statement_List();
   }
   else
   {
      throw "Error at Body()";
      return;
   }
   counter++;
   if (!(v[counter].token == "Separator" && v[counter].lexeme == "}"))
   {
      throw "Error at Body()";
   }
}

//<Statement List> → <Statement> <Statement List_Prime>
void Statement_List()
{
   Statement();
   if (Empty(counter))
   {
      return;
   }
   Statement_List_Prime();
}

//<Statement> ->   <Compound>  |  <Assign>  |   <If>  |  <Return>   | <Print>   |   <Scan>   |  <While>
void Statement()
{
   if (v[counter + 1].token == "Separator" && v[counter + 1].lexeme == "{")
   {
      Compound();
   }
   else if (v[counter + 1].token == "Identifier")
   {
      Assign();
   }
   else if (v[counter + 1].token == "Keyword" && v[counter + 1].lexeme == "if")
   {
      IF();
   }
   else if (v[counter + 1].token == "Keyword" && v[counter + 1].lexeme == "return")
   {
      Return();
   }
   else if (v[counter + 1].token == "Keyword" && v[counter + 1].lexeme == "put")
   {
      Print();
   }
   else if (v[counter + 1].token == "Keyword" && v[counter + 1].lexeme == "get")
   {
      Scan();
   }
   else if (v[counter + 1].token == "Keyword" && v[counter + 1].lexeme == "while")
   {
      While();
   }
   else
   {
      throw "Error at Statement";
   }
}

//<Compound> ->   {  <Statement List>  }
void Compound()
{
   counter++;
   if (v[counter].token == "Separator" && v[counter].lexeme == "{")
   {
      Statement_List();
   }
   else
   {
      throw "Error at Compound()";
      return;
   }
   counter++;
   if (!(v[counter].token == "Separator" && v[counter].lexeme == "}"))
   {
      throw "Error at Compound()";
   }
}

//<Assign> -> 	<Identifier> = <Expression> ;
void Assign()
{
   counter++;
   if (v[counter].token == "Identifier")
   {
      string save = v[counter].lexeme;
      counter++;
      if (v[counter].lexeme == "=")
      {
         counter++;
         try{
            Expression();
         }
         catch(type & e){
            string type_of_lhs = get_symbol_type(save);
            if((e == BOOL && type_of_lhs == "int")||(e == INT && type_of_lhs == "boolean")) {
               throw "Type mismatch at assignment";
            }
         }
         gen_instr("POPM", get_symbol_address(save));
         counter++;
         if (!(v[counter].token == "Separator" && v[counter].lexeme == ";"))
         {
            throw "Error at Assign()";
         }
      }
      else
      {
         throw "Error at Assign()";
      }
   }
   else
   {
      throw "Error at Assign()";
   }
}

//<If> → if (<Condition>) <Statement> <If_Prime>
void IF()
{
   counter++;
   if (v[counter].token == "Keyword" && v[counter].lexeme == "if")
   {
      counter++;
      if (v[counter].token == "Separator" && v[counter].lexeme == "(")
      {
         Condition();
         counter++;
         if (v[counter].token == "Separator" && v[counter].lexeme == ")")
         {
            Statement();
            IF_Prime();
         }
         else
         {
            throw "Error at IF()";
         }
      }
      else
      {
         throw "Error at IF()";
      }
   }
   else
   {
      throw "Error at IF()";
   }
}

//<If_Prime> → endif | else <Statement> endif
void IF_Prime()
{
   counter++;
   if (v[counter].token == "Keyword" && v[counter].lexeme == "endif")
   {
      back_patch(instruction_table.size() + 1);
      gen_instr("LABEL", NIL);
      return;
   }
   else if (v[counter].token == "Keyword" && v[counter].lexeme == "else")
   {
      gen_instr("JUMP", NIL);
      back_patch(instruction_table.size() + 1);
      jumpstack.push(instruction_table.size() - 1);
      gen_instr("LABEL", NIL);
      Statement();
      back_patch(instruction_table.size() + 1);
      gen_instr("LABEL", NIL);
      counter++;
      if (v[counter].token != "Keyword" && v[counter].lexeme != "endif")
      {
         throw "Error at IF_Prime()";
      }
   }
   else
   {
      throw "Error at IF_Prime()";
   }
}

//<Return> → return <Return_Prime>
void Return()
{
   counter++;
   if (v[counter].token == "Keyword" && v[counter].lexeme == "return")
   {
      Return_Prime();
   }
   else
   {
      throw "Error at Return()";
   }
}

//<Return_Prime> → ; | <Expression> ;
void Return_Prime()
{
   counter++;
   if (v[counter].token == "Separator" && v[counter].lexeme == ";")
   {
      return;
   }
   else if (v[counter].lexeme == "-" || v[counter].token == "Identifier" || v[counter].token == "Integer" || v[counter].lexeme == "(" || v[counter].token == "Real" ||
   (v[counter].token == "Keyword" && (v[counter].lexeme == "true" || v[counter].lexeme == "false")))
   {
      Expression();
      counter++;
      if (v[counter].token != "Separator" && v[counter].lexeme != ";")
      {
         throw "Error at Return_Prime()";
      }
   }
   else
   {
      throw "Error at Return_Prime()";
   }
}

//<Print> ->	put ( <Expression>);
void Print()
{
   counter++;
   if (v[counter].token == "Keyword" && v[counter].lexeme == "put")
   {
      counter++;
      if (v[counter].token == "Separator" && v[counter].lexeme == "(")
      {
         counter++;
         try {
            Expression();
         }
         catch(type &t) {}
         gen_instr("STDOUT", NIL);
         counter++;
         if (v[counter].token == "Separator" && v[counter].lexeme == ")")
         {
            counter++;
            if (v[counter].token != "Separator" && v[counter].lexeme != ";")
            {
               throw "Error at Print()";
            }
         }
         else
         {
            throw "Error at Print()";
         }
      }
      else
      {
         throw "Error at Print()";
      }
   }
   else
   {
      throw "Error at Print()";
   }
}

//<Scan> ::=	get ( <IDs> );
void Scan()
{
   counter++;
   if (v[counter].token == "Keyword" && v[counter].lexeme == "get")
   {
      counter++;
      if (v[counter].token == "Separator" && v[counter].lexeme == "(")
      {
         counter++;
         IDS();
         counter++;
         if (v[counter].token == "Separator" && v[counter].lexeme == ")")
         {
            counter++;
            if (v[counter].token != "Separator" && v[counter].lexeme != ";")
            {
               throw "Error at Print()";
            }
         }
         else
         {
            throw "Error at Print()";
         }
      }
      else
      {
         throw "Error at Print()";
      }
   }
   else
   {
      throw "Error at Print()";
   }
}

//<While> ->  while ( <Condition>  )  <Statement>
void While()
{
   counter++;
   if (v[counter].token == "Keyword" && v[counter].lexeme == "while")
   {
      int address=instruction_table.size() + 1;
      gen_instr("LABEL", NIL);
      counter++;
      if (v[counter].token == "Separator" && v[counter].lexeme == "(")
      {
         counter++;
         Condition();
         counter++;
         if (v[counter].token == "Separator" && v[counter].lexeme == ")")
         {
            Statement();
            gen_instr("JUMP", address);
            back_patch(instruction_table.size() + 1);
         }
         else
         {
            throw "Error at Print()";
         }
      }
      else
      {
         throw "Error at Print()";
      }
   }
   else
   {
      throw "Error at Print()";
   }
}

// <Expression> → <Term> <Expression_Prime>
void Expression()
{
   bool Term_is_bool = false;
   try
   {
      Term();
   }
   catch (type &t1)
   {
      if (t1 == BOOL)
      {
         Term_is_bool = true;
      }
   }
   try
   {
      Expression_Prime();
   }
   catch (type &t2)
   {
      if (t2 == EPSILON && Term_is_bool)
      {
         throw BOOL;
      }
      else if ((t2 == EPSILON || t2 == INT) && !Term_is_bool)
      {
         throw INT;
      }
      else
      {
         throw "Error";
      }
   }
}

//<Term> → <Factor><Term_Prime>
void Term()
{
   Factor();
   try
   {
      Term_Prime();
   }
   catch (type &t)
   {
      if (t == INT || t == EPSILON)
      {
         return;
      }
      else
      {
         throw t;
      }
   }
}

//<Factor> →   	-  <Primary> | <Primary>
void Factor()
{
   if (v[counter + 1].lexeme == "-")
   {
      is_negative = true;
      counter += 2;
      try
      {
         Primary();
      }
      catch (type &t)
      {
         throw "Error at negative boolean";
      }
      is_negative = false;
   }
   else
   {
      Primary();
   }
}

//<Primary> →     <Identifier>  |  <Integer>  |   <Identifier>  ( <IDs> )   |   ( <Expression> )   |  <Real>  |   true   |  false
void Primary()
{
   if (v[counter].token == "Identifier" && v[counter + 1].lexeme == "(")
   {
      counter += 2;
      IDS();
      counter++;
      if (v[counter].lexeme != ")")
      {
         throw "Error at Primary()";
         return;
      }
   }
   else if (v[counter].token == "Identifier")
   {
      if (is_on_table(v[counter].lexeme))
      {
         int address = get_symbol_address(v[counter].lexeme);
         gen_instr("PUSHM", address);
         if (get_symbol_type(v[counter].lexeme) == "boolean")
         {
            throw BOOL;
         }
         else
         {
            return;
         }
      }
      return;
   }
   else if (v[counter].token == "Integer")
   {
      int temp = stoi(v[counter].lexeme);
      if (is_negative)
      {
         temp = temp * -1;
      }
      gen_instr("PUSHI", temp);
      return;
   }
   else if (v[counter].lexeme == "(")
   {
      counter++;
      Expression();
      counter++;
      if (v[counter].lexeme != ")")
      {
         throw "Error at Primary()";
         return;
      }
   }
   else if (v[counter].token == "Real")
   {
      return;
   }
   else if (v[counter].token == "Keyword" && (v[counter].lexeme == "true" || v[counter].lexeme == "false"))
   {
      if (v[counter].lexeme == "true")
      {
         gen_instr("PUSHI", 1);
      }
      else
      {
         gen_instr("PUSHI", 0);
      }
      throw BOOL;
      return;
   }
   else
   {
      throw "Error at Primary()";
   }
}

//<Term_Prime> → * <Factor> <Term_Prime> | / <Factor> <Term_Prime> | ε
void Term_Prime()
{

   if (v[counter + 1].lexeme == "*")
   {
      counter += 2;
      Factor();
      gen_instr("MUL", NIL);
      Term_Prime();
   }
   else if (v[counter + 1].lexeme == "/")
   {
      counter += 2;
      Factor();
      gen_instr("DIV", NIL);
      Term_Prime();
   }
   else{
      Empty();
   }
}

//<Expression_Prime>→ + <Term><Expression_Prime> | - <Term><Expression_Prime>|ε
void Expression_Prime()
{

   if (v[counter + 1].lexeme == "+")
   {
      counter += 2;
      Term();
      gen_instr("ADD", NIL);
      Expression_Prime();
   }
   else if (v[counter + 1].lexeme == "-")
   {
      counter += 2;
      Term();
      gen_instr("SUB", NIL);
      Expression_Prime();
   }
   else{
      Empty();
   }
}

//<Statement List_Prime> → ε | <Statement List>
void Statement_List_Prime()
{

   if ((v[counter + 1].token == "Separator" && v[counter + 1].lexeme == "{") ||
   (v[counter + 1].token == "Identifier") ||
   (v[counter + 1].token == "Keyword" && v[counter + 1].lexeme == "if") ||
   (v[counter + 1].token == "Keyword" && v[counter + 1].lexeme == "return") ||
   (v[counter + 1].token == "Keyword" && v[counter + 1].lexeme == "put") ||
   (v[counter + 1].token == "Keyword" && v[counter + 1].lexeme == "get") ||
   (v[counter + 1].token == "Keyword" && v[counter + 1].lexeme == "while"))
   {
      Statement_List();
   }
}

//<Condition> → <Expression>  <Relop>   <Expression>
void Condition()
{
   bool expression1_is_bool=false;
   try{
      Expression();
   }
   catch(type &t) {
      if(t==BOOL) {
         expression1_is_bool=true;
      }
   }
   string comparison_ops = Relop();
   try{
      Expression();
   }
   catch(type &t) {
      if(!((t==BOOL&&expression1_is_bool) || (t==INT&&!expression1_is_bool))) {
         throw "Type mismatch in comparison.\n";
      }
   }
   if (comparison_ops == "==") {
      gen_instr("EQU", NIL);
      jumpstack.push(instruction_table.size());
      gen_instr("JUMPZ", NIL);
   }
   else if(comparison_ops == "^=") {
      gen_instr("NEQ", NIL);
      jumpstack.push(instruction_table.size());
      gen_instr("JUMPZ", NIL);
   }
   else if(comparison_ops == ">") {
      gen_instr("GRT", NIL);
      jumpstack.push(instruction_table.size());
      gen_instr("JUMPZ", NIL);
   }
   else if(comparison_ops == "<") {
      gen_instr("LES", NIL);
      jumpstack.push(instruction_table.size());
      gen_instr("JUMPZ", NIL);
   }
   else if(comparison_ops == "=>") {
      gen_instr("GEQ", NIL);
      jumpstack.push(instruction_table.size());
      gen_instr("JUMPZ", NIL);
   }
   else if(comparison_ops == "=<") {
      gen_instr("LEQ", NIL);
      jumpstack.push(instruction_table.size());
      gen_instr("JUMPZ", NIL);
   }
}

//<Relop> →    	==   |   ^=	|   >     |   <	|   =>	|   =<
string Relop()
{
   counter++;
   if (v[counter].lexeme == "==")
   {
      counter++;
      return "==";
   }
   else if(v[counter].lexeme == "^=") {
      counter++;
      return "^=";
   }
   else if(v[counter].lexeme == ">") {
      counter++;
      return ">";
   }
   else if(v[counter].lexeme == "<") {
      counter++;
      return "<";
   }
   else if(v[counter].lexeme == "=>") {
      counter++;
      return "=>";
   }
   else if(v[counter].lexeme == "=<") {
      counter++;
      return "=<";
   }
   else
   {
      throw "Error at Relop()";
   }
}

bool Empty(int counter)
{
   int _counter = v.size();
   if (counter + 1 == _counter)
   {
      return true;
   }
   return false;
}
void Empty(){
   throw EPSILON;
}
