#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <climits>
#include <cstdlib>
using namespace std;
unordered_map<string,string> registers={{"zero",  "00000"},
    {"ra",  "00001"},{"sp",  "00010"},{"gp",  "00011"},{"tp",  "00100"},
    {"t0",  "00101"},{"t1",  "00110"},
    {"t2",  "00111"},
    {"s0",  "01000"},
    {"fp",  "01000"},
    {"s1",  "01001"},
    {"a0", "01010"},
    {"a1", "01011"},
    {"a2", "01100"},
    {"a3", "01101"},
    {"a4", "01110"},
    {"a5", "01111"},
    {"a6", "10000"},
    {"a7", "10001"},
    {"s2", "10010"},
    {"s3", "10011"},
    {"s4", "10100"},
    {"s5", "10101"},
    {"s6", "10110"},
    {"s7", "10111"},
    {"s8", "11000"},
    {"s9", "11001"},
    {"s10", "11010"},
    {"s11", "11011"},
    {"t3", "11100"},
    {"t4", "11101"},
    {"t5", "11110"},
    {"t6", "11111"}
};
struct inst{
    string type;
    string opcode;
    string func3;
    string func7;
};
unordered_map<string,inst> inst_set={
    //R-type
    {"add",  {"R", "0110011", "000", "0000000"}},
    {"sub",  {"R", "0110011", "000", "0100000"}},
    {"sll",  {"R", "0110011", "001", "0000000"}},
    {"slt",  {"R", "0110011", "010", "0000000"}},
    {"sltu", {"R", "0110011", "011", "0000000"}},
    {"xor",  {"R", "0110011", "100", "0000000"}},
    {"srl",  {"R", "0110011", "101", "0000000"}},
    {"sra",  {"R", "0110011", "101", "0100000"}},
    {"or",   {"R", "0110011", "110", "0000000"}},
    {"and",  {"R", "0110011", "111", "0000000"}},
    {"mul",  {"R", "0110011", "000", "0000001"}},
    {"rem",  {"R", "0110011", "110", "0000001"}},
    //I-type
    {"addi", {"I", "0010011", "000", ""}},
    {"slti", {"I", "0010011", "010", ""}},
    {"sltiu",{"I", "0010011", "011", ""}},
    {"xori", {"I", "0010011", "100", ""}},
    {"ori",  {"I", "0010011", "110", ""}},
    {"andi", {"I", "0010011", "111", ""}},
    //I-shift-type
    {"slli", {"I", "0010011", "001", "0000000"}},
    {"srli", {"I", "0010011", "101", "0000000"}},
    {"srai", {"I", "0010011", "101", "0100000"}},
     //load instructions
    {"lb",   {"I", "0000011", "000", ""}},
    {"lh",   {"I", "0000011", "001", ""}},
    {"lw",   {"I", "0000011", "010", ""}},
    {"lbu",  {"I", "0000011", "100", ""}},
    {"lhu",  {"I", "0000011", "101", ""}},
    //store instructions
    {"sb",   {"S", "0100011", "000", ""}},
    {"sh",   {"S", "0100011", "001", ""}},
    {"sw",   {"S", "0100011", "010", ""}},
    //branch type
    {"beq",  {"B", "1100011", "000", ""}},
    {"bne",  {"B", "1100011", "001", ""}},
    {"blt",  {"B", "1100011", "100", ""}},
    {"bge",  {"B", "1100011", "101", ""}},
    {"bltu", {"B", "1100011", "110", ""}},
    {"bgeu", {"B", "1100011", "111", ""}},
     //jump type
    {"jal",  {"J", "1101111", "", ""}},
    {"jalr", {"I", "1100111", "000", ""}}
};
string convert_binary(int y,int x)
{
    string res="";
    int y1=abs(y);
    if(y1==0){
        res="0";
    }
    else{
    while(y1)
    {
        res+=(y1%2+'0');
        y1=y1/2;
    }
    }
    reverse(res.begin(),res.end());
    int p=res.size();
    if(p<x){
     res=string(x-p,'0')+res;
    }
    if(y<0)
    {
        int c=1;
        for(int i=0;i<res.size();i++)
           res[i]=(res[i]=='0')?'1':'0';
        for(int i=res.size()-1;i>=0;i--)
        {
            if(res[i]=='1' && c==1)
             res[i]='0';
             else if(c==1)
             {
                 res[i]='1';
                 c=0;
             }
        }
         
    }
    return res;
   
}
string R_type(string line)
{
    string f3="",reg1="",reg2="",reg3="";
    int i=0;
    while(line[i]!=' ')
    {
         f3+=line[i];
         i++;
    }
    i++;
    if(reg1=="")
        {
            while(line[i]!=','){
                reg1+=line[i];
                i++;
            }
             i++;
        }
        if(reg1!="" && reg2=="")
        {
             while(line[i]!=','){
                reg2+=line[i];
                i++;
            }
             i++;
        }
        while(i<line.size()){
                reg3+=line[i];
                i++;
   
        }
    return inst_set[f3].func7+registers[reg3]+registers[reg2]+inst_set[f3].func3+registers[reg1]+inst_set[f3].opcode;
}
string B_type(string line)
{
    string f3="",reg1="",reg2="",reg3="";
    int i=0;
    while(line[i]!=' ')
    {
         f3+=line[i];
         i++;
    }
    i++;
    if(reg1=="")
        {
            while(line[i]!=','){
                reg1+=line[i];
                i++;
            }
             i++;
        }
         if(reg1!="" && reg2=="")
        {
             while(line[i]!=','){
                reg2+=line[i];
                i++;
            }
             i++;
        }
        while(i<line.size()){
                reg3+=line[i];
                i++;
            }
    // B type
    int y=stoi(reg3);
    string r=convert_binary(y,12);
    return r.substr(0,7)+registers[reg2]+registers[reg1]+inst_set[f3].func3+r.substr(7,5)+inst_set[f3].opcode;
}
string S_type(string line)
{
    string f3="",reg1="",reg2="",reg3="";
    int i=0;
    while(line[i]!=' ')
    {
         f3+=line[i];
         i++;
    }
    i++;
    if(reg1=="")
        {
            while(line[i]!=','){
                reg1+=line[i];
                i++;
            }
             i++;
        }
        if(reg1!="" && reg2=="")
        {
             while(line[i]!='('){
                reg2+=line[i];
                i++;
            }
             i++;
        }
             while(line[i]!=')'){
                reg3+=line[i];
                i++;
            }
    int y=stoi(reg2);
    string r=convert_binary(y,12);
    // store type
    if(inst_set[f3].opcode=="0100011"){
        return r.substr(0,7)+registers[reg1]+registers[reg3]+inst_set[f3].func3+r.substr(7,5)+inst_set[f3].opcode;
    }
    // load  and jalr type
    return r+registers[reg3]+inst_set[f3].func3+registers[reg1]+inst_set[f3].opcode;
}
string J_type(string line)
{
    string f3="",reg1="",reg3="";
    int i=0;
    while(line[i]!=' ')
    {
         f3+=line[i];
         i++;
    }
    i++;
    if(reg1=="")
        {
            while(line[i]!=','){
                reg1+=line[i];
                i++;
            }
             i++;
        }
             while(i<line.size()){
                reg3+=line[i];
                i++;
            }
    int y=stoi(reg3);
    string r=convert_binary(y,20);
    return r+registers[reg1]+inst_set[f3].opcode;
}
string I_type(string line)
{  

    string f3="",reg1="",reg2="",reg3="";
    int i=0;
    while(line[i]!=' ')
    {
         f3+=line[i];
         i++;
    }
    i++;
    // load instruction
    if(inst_set[f3].opcode=="0000011")
     return S_type(line);
     if(inst_set[f3].opcode=="1100111")
      return S_type(line);
    if(reg1=="")
        {
            while(line[i]!=','){
                reg1+=line[i];
                i++;
            }
             i++;
        }
        if(reg1!="" && reg2=="")
        {
             while(line[i]!=','){
                reg2+=line[i];
                i++;
            }
             i++;
        }
             while(i<line.size()){
                reg3+=line[i];
                i++;
            }
    // I -Type
    if(inst_set[f3].func7==""){
         int y=stoi(reg3);
    string r=convert_binary(y,12);
    return r+registers[reg2]+inst_set[f3].func3+registers[reg1]+inst_set[f3].opcode;
    }
    int y=stoi(reg3);
    // I-shift type
    return inst_set[f3].func7+convert_binary(y,5)+registers[reg2]+inst_set[f3].func3+registers[reg1]+inst_set[f3].opcode;
}
int main()
{
    ifstream infile("input_factorial.txt");
    ofstream outfile("output_factorial.txt");
    if(!infile)
    {
        cerr<<"Failed to open input file!"<<endl;
        return 1;
    }
    if(!outfile)
    {
        cerr<<"Failed to open output file!"<<endl;
        return 1;
    }
    string line;
    while(getline(infile, line))
    {
        if(line.empty()) continue;
         string p="";
         int i=0;
         while(line[i]!=' ')
         {
            p+=line[i];
            i++;
         }
          string result;
         if(inst_set[p].type=="I")
          result=I_type(line);
          else if(inst_set[p].type=="R")
          result=R_type(line);
           else if(inst_set[p].type=="B")
          result=B_type(line);
           else if(inst_set[p].type=="S")
          result=S_type(line);
           else if(inst_set[p].type=="J")
          result=J_type(line);
          else{
            cout<<"Invalid Instruction ";
            continue;
          }
         outfile<<result<<endl;
    }
    outfile.flush();
    outfile.close();
    infile.close();
    return 0;
}