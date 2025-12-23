#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <string>
#include <stdexcept>
using namespace std;
int clockcycle=0;
// memory layout
class Memory
{
public:
    vector<int> mem;
    // initialize memory
    Memory(int size) : mem(size, 0) {};
    unsigned int loadWord(int addr)
    {
        if (addr < 0 || addr >= (int)mem.size())
        {
            throw out_of_range("Memory read out of range");
        }
        return mem[addr];
    }
    void storeWord(int addr, int value)
    {
        if (addr < 0 || addr >= (int)mem.size())
            cerr << "Memory write out of range";
        // now store input integer into 8 byte string
        mem[addr] = value;
    }
};
// register file
class RegisterFile
{
    vector<int> GPR;

public:
    RegisterFile() : GPR(32, 0)
    {
        GPR[2] = 512;
        GPR[8] = 512;
    }
    int read(int idx)
    {
        if (idx < 0 || idx >= (int)GPR.size())
        {
            cerr << "Register read index is out of bound" << idx << endl;
            return 0;
        }
        return GPR[idx];
    }
    void write(int idx, int value)
    {
        if (idx == 0)
            return; // x0 is always 0
        if (idx < 0 || idx >= 32)
            throw out_of_range("Invalid register index");
        // cout<<value<<endl;
        GPR[idx] = value;
    }
    void printregisters(void)
    {
        for (auto x : GPR)
        {
            cout << x << " ";
        }
        cout << endl;
    }
};
// structure for control unit
struct CW
{
    int regRead, regWrite;
    int ALUsrc;
    int memRead, memWrite, mem2Reg, branch, jump;
    string ALUOp;
    string ALUSelect;
};
unordered_map<string, int> ops = {
    {"0010011_000", 1}, // ADDI
    {"0010011_010", 1}, // SLTI
    {"0010011_011", 1}, // SLTIU
    {"0010011_100", 1}, // XORI
    {"0010011_110", 1}, // ORI
    {"0010011_111", 1}, // ANDI
    {"0010011_001", 4}, // SLLI (I-shift)
    {"0010011_101", 4}, // SRLI/SRAI (I-shift)
    {"0000011_010", 1}, // Load
    {"0100011_010", 3}, // Store
    {"1100011_000", 3}, // BEQ
    {"1100011_001", 3}, // BNE
    {"1100011_100", 3}, // BLT
    {"1100011_101", 3}, // BGE
    {"1100111_000", 1}, // JALR
    {"1101111_xxx", 2}  // JAL (func3 not used)
};
struct CW controller(string s)
{
    struct CW a;
    a.regRead = 0;
    a.regWrite = 0;
    a.ALUsrc = 0;
    a.memRead = 0;
    a.memWrite = 0;
    a.mem2Reg = 0;
    a.branch = 0;
    a.jump = 0;
    a.ALUOp = "xx";
    // R-type
    if (s == "0110011")
    {
        a.regRead = 1;
        a.regWrite = 1;
        a.ALUOp = "10";
    }
    // I-type
    else if (s == "0010011")
    {
        a.regWrite = 1;
        a.regRead = 1;
        a.ALUsrc = 1;
        a.ALUOp = "10";
    }
    // load
    else if (s == "0000011")
    {
        a.regRead = 1;
        a.regWrite = 1;
        a.ALUsrc = 1;
        a.memRead = 1;
        a.mem2Reg = 1;
        a.ALUOp = "00";
    }
    // store
    else if (s == "0100011")
    {
        a.regRead = 1;
        a.ALUsrc = 1;
        a.memWrite = 1;
        a.ALUOp = "00";
    }
    // branch
    else if (s == "1100011")
    {
        a.regRead = 1;
        a.branch = 1;
        a.ALUOp = "01";
    }
    else if (s == "1101111")
    { // JAL
        a.regWrite = 1;
        a.jump = 1;
    }
    else if (s == "1100111")
    { // JALR
        a.regWrite = 1;
        a.regRead = 1;
        a.ALUsrc = 1;
        a.jump = 1;
    }

    return a;
}
int signextend(string f)
{
    // as we are making the immediate value into respective integer
    int n = f.size(), ans = 0;
    for (int i = 0; i < n; i++)
    {
        ans += (f[i] - '0') * (1 << (n - 1 - i));
    }
    // if 1st bit is 1 then it is an -ve int

    if (f[0] == '1') // negative
        ans -= (1 << n);

    return ans;
}
int binToInt(string s)
{
    return stoi(s, nullptr, 2);
}
int outputselect(int mem2Reg, int jump, int aluresult, int ldresult, int npc)
{
    if (mem2Reg)
        return ldresult;
    else if (jump)
        return npc;
    else
        return aluresult;
}
int genimm(string s)
{
    // corresponding immediate value
    string opcode = s.substr(25, 7);
    string func3 = s.substr(17, 3);
    string f = opcode + '_' + func3;

    if (opcode == "0110011")
        return 0;
    auto it = ops.find(f);
    if (it == ops.end())
    {
        cerr << "Undefined opcode: " << f << endl;
        return 0;
    }
    int f1 = it->second;
    // I,jalr,L
    if (f1 == 1)
        return signextend(s.substr(0, 12));
    //  jal
    else if (f1 == 2)
        return signextend(s.substr(0, 20));
    //  B,S
    else if (f1 == 3)
        return signextend(s.substr(0, 7) + s.substr(20, 5));
    //  I shift
    else if (f1 == 4)
        return signextend(s.substr(7, 5));
    else
    {
        cerr << "Undefined opcode" << endl;
        return 0;
    }
}
string ALUControl(string h, string func7, string func3)
{
    string r = "xxxx";
    if (h == "00")
        r = "0010";
    else if (h == "01")
    {
        if (func3 == "000" || func3 == "001")
            r = "0110";
        else if (func3 == "101")
            r = "1001";
        else if (func3 == "100")
            r = "1111";
    }
    else if (h == "10")
    {
        if (func7[1] == '1')
        {
            if (func3 == "000")
                r = "0110"; // sub
        }
        else if (func7[6] == '1')
        {
            if (func3 == "000")
                r = "0111"; // mul
            else if (func3 == "110")
                r = "1010";
        }
        else
        {
            if (func3 == "000")
                r = "0010"; // add
            else if (func3 == "111")
                r = "0000"; // AND
            else if (func3 == "110")
                r = "0001"; // OR
            else if (func3 == "010")
                r = "1111"; // slt
        }
    }
    return r;
}
int ALU(string ALUSelect, int rs1, int rs2)
{
    if (ALUSelect == "0010")
    {
        // ADD
        return rs1 + rs2;
    }
    else if (ALUSelect == "0111")
    {
        // MUL
        return rs1 * rs2;
    }
    else if (ALUSelect == "0110")
    {
        // SUB
        return rs1 - rs2;
    }
    else if (ALUSelect == "0000")
    {
        // AND
        return rs1 & rs2;
    }
    else if (ALUSelect == "0001")
    {
        // OR
        return rs1 | rs2;
    }
    else if (ALUSelect == "0011")
    {
        // XOR
        return rs1 ^ rs2;
    }
    else if (ALUSelect == "0100")
    {
        // SLL
        return rs1 << rs2;
    }
    else if (ALUSelect == "0101")
    {
        // SRL
        return ((unsigned int)rs1) >> rs2;
    }
    else if (ALUSelect == "1010")
    {
        // rem
        return rs1 % rs2;
    }
    else if (ALUSelect == "1111") // SLT
        return (rs1 < rs2) ? 1 : 0;
    else if (ALUSelect == "1001")
        return (rs1 >= rs2) ? 1 : 0;
    else
    {
        cerr << "Undefined ALU operation" << endl;
        return 0;
    }
}
void make_comp(const vector<string> &arr, RegisterFile &reg, Memory &memory)
{
    int pc = 0;
    int n = arr.size();
    while (pc < n)
    {
        clockcycle++;
        // cout<<pc<<endl;
        string s = arr[pc];
        int npc = pc + 1;
        int y = genimm(arr[pc]);
        int jpc = npc + y;
        int rs1 = binToInt(s.substr(12, 5));
        int rs2 = binToInt(s.substr(7, 5));
        int rdl = binToInt(s.substr(20, 5));
        string opcode = s.substr(25, 7);
        string func3 = s.substr(17, 3);
        string func7 = s.substr(0, 7);
        int imm = y;
        struct CW temp = controller(opcode);
        int SDvalue = rs2;
        if (temp.regRead)
        {
            rs1 = reg.read(rs1);
        }
        if (temp.regRead)
        {
            rs2 = reg.read(rs2);
        }
        if (temp.ALUsrc)
            rs2 = imm;
        if (opcode == "0010011")
            func7 = "1011110";
        temp.ALUSelect = ALUControl(temp.ALUOp, func7, func3);
        int aluresult = ALU(temp.ALUSelect, rs1, rs2);
        // cout<<aluresult<<endl;
        int aluzeroflag = 0;
        if (aluresult == 0)
        {
            aluzeroflag = 1;
        }

        int bpc = imm - 1;
        int tpc;
        if (temp.branch)
        {
            if ((func3 == "000" && aluzeroflag) || // BEQ
                (func3 == "001" && !aluzeroflag) || (func3 == "101" && aluzeroflag) || (func3 == "100" && aluzeroflag))
            { // BNE
                tpc = bpc;
            }
            else
                tpc = npc;
        }
        else if (temp.jump)
            tpc = jpc;
        else
            tpc = npc;
        unsigned int ldresult = 0;
        if (temp.memRead)
            ldresult = memory.loadWord(aluresult);

        if (temp.memWrite)
            memory.storeWord(aluresult, reg.read(SDvalue));

        if (temp.regWrite)
            reg.write(rdl, outputselect(temp.mem2Reg, temp.jump, aluresult, ldresult, npc));
        // cout<<pc<<" "<<npc<<" "<<bpc<<" "<<tpc<<endl;
        // // reg.printregisters();
        pc = tpc;
    }
}
int main()
{
    Memory memory(1024);
    RegisterFile reg;
    vector<string> arr;
    ifstream infile("output_factorial.txt");
    string line;

    while (getline(infile, line))
    {
        if (!line.empty())
        {

            arr.push_back(line);
        }
    }
    // arr.push_back("11111101000000010000000100010011");
    make_comp(arr, reg, memory);
    reg.printregisters();
    cout<<clockcycle<<endl;
    return 0;
}