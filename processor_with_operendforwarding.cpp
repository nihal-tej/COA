#include<iostream>
#include<vector>
#include<unordered_map>
#include<fstream>
#include<string>
#include<stdexcept>
using namespace std;
class RegisterFile{
  vector<int> GPR;
  public:
  RegisterFile():GPR(32,0){
      GPR[2]=512;
      GPR[8]=512;
  }
  int read(int idx){
      if(idx<0||idx>=(int)GPR.size()){
          cerr<<"Register read index is out of bound"<<idx<<endl;
          return 0;
      }
      return GPR[idx];
  }
  void write(int idx,int value){
      if(idx==0) return;//x0 is always 0
      if(idx<0||idx>=(int)GPR.size()) throw out_of_range("Invalid register index");
      GPR[idx]=value;
  }
  void print(void){
      for(auto x:GPR) cout<<x<<" ";
      cout<<endl;
  }
};
class Memory{
    public:
    vector<int> mem;
    //initialize memory
    Memory(int size):mem(size,0){};
    int loadWord(int addr){
        if(addr<0||addr>=(int)mem.size()){
            throw out_of_range("Memory read out of range");
        }
        return mem[addr];
    }
    void storeWord(int addr,int value){
        if(addr<0||addr>=(int)mem.size()) cerr<<"Memory write out of range";
        //now store input integer into 8 byte string
        mem[addr]=value;
    }
};
struct CW{
    int regRead=0,regWrite=0;
    int ALUsrc=0;
    int memRead=0,memWrite=0,mem2Reg=0,branch=0,jump=0;
    string ALUOp="xx";
    string ALUSelect;
};
struct IFID{
    int dpc=0,npc=0;
    bool valid=false;
    string inst;
};
struct IDEX{
    int dpc=0,imm=0,jpc=0,bpc=0,npc=0;
    int rs1=0,rs2=0,rdl=0,rsl2=0,rsl1=0;
    string func3,func7;
    CW ctrl;
    bool valid=false;
    string inst;
};
struct EXMO{
  int dpc=0,aluout=0,rsl2=0,rdl=0,imm=0,bpc=0,jpc=0,npc=0;
  bool aluzeroflag=false;
  CW ctrl;
  bool valid=false;
}; 
struct MOWB{
    int dpc=0,ldout=0,aluout=0,rdl=0,bpc=0,jpc=0,npc=0;
    CW ctrl;
    bool valid=false;
};
int doALU(string h,string func7,string func3,int rs1,int rs2){
    int out=0;
    if(h=="00") out=rs1+rs2;
    else if(h=="01"){
        if(func3=="000") out=rs1-rs2;//BEQ compare
        else if(func3=="001") out=(rs1==rs2);//BNE compare
        else if(func3=="101") out=(rs1>=rs2)?1:0;//BGE
        else if(func3=="100") out=(rs1<rs2)?1:0;//BLT
    }
    else if(h=="10"){
        if(func7[1]=='1'){
            if(func3=="000") out=rs1-rs2;//SUB
        }
        else if(func7[6]=='1'){
            if(func3=="000") out=rs1*rs2;//MUL
            else if(func3=="110") out=rs1%rs2;//REM
        }
        else{
            if(func3=="000") out=rs1+rs2;//ADD
            else if(func3=="111") out=rs1&rs2;//AND
            else if(func3=="110") out=rs1|rs2;//OR
            else if(func3=="010") out=(rs1<rs2)?1:0;//SLT
        }
    }
    return out;
}
CW controller(string s){
    struct CW a;
    a.regRead=0;
    a.regWrite=0;
    a.ALUsrc=0;
    a.memRead=0;
    a.memWrite=0;
    a.mem2Reg=0;
    a.branch=0;
    a.jump=0;
    a.ALUOp="xx";
    //R-type
    if(s=="0110011"){
        a.regRead=1;
        a.regWrite=1;
        a.ALUOp="10";
    }
    //I-type
    if(s=="0010011"){
        a.regWrite=1;
        a.regRead=1;
        a.ALUsrc=1;
        a.ALUOp="10";
    }
    // load
    if(s=="0000011"){
        a.regRead=1;
        a.regWrite=1;
        a.ALUsrc=1;
        a.memRead=1;
        a.mem2Reg=1;
        a.ALUOp="00";
    }
    //store
    if(s=="0100011"){
        a.regRead=1;
        a.ALUsrc=1;
        a.memWrite=1;
        a.ALUOp="00";
    }
    //branch
    if(s=="1100011"){
        a.regRead=1;
        a.branch=1;
        a.ALUOp="01";
    }
    // jal
    if(s=="1101111"){
        a.regWrite=1;
        a.jump=1;
    }
    //jalr
    if(s=="1100111"){
        a.regWrite=1;
        a.regRead=1;
        a.ALUsrc=1;
        a.jump=1;
    }
    return a;
}
unordered_map<string,int> ops={
    // I-type
    {"0010011_000",1},//ADDI
    {"0010011_010",1},// SLTI
    {"0010011_011",1},// SLTIU
    {"0010011_100",1},// XORI
    {"0010011_110",1},// ORI
    {"0010011_111",1},// ANDI
    {"0010011_001",4},// SLLI
    {"0010011_101",4},// SRLI
    // Jalr
    {"1100111_000",1},
    // Jal
     {"1101111_xxx",2},
    //  B type
    {"1100011_000",3},//BEQ
    {"1100011_001",3},//BNE
    {"1100011_100",3},//BLT
    {"1100011_101",3},//BGE
    // S type
    {"0100011_010",3},
    // L tpye
    {"0000011_010",1}
};
int signextend(string f)
{   
    // as we are making the immediate value into respective integer
    int n=(int)f.size(),ans=0;
    for(int i=0;i<n;i++)
    {
        ans=(ans<<1)|(f[i]-'0');
    }
    // if 1st bit is 1 then it is an -ve int
    if(f[0]=='1') return ans-=(1<<n);
     return ans;
}
int binToInt(string s){
    return stoi(s,nullptr,2);
}
int genimm(string s)
{
    // corresponding immediate value
    string opcode=s.substr(25,7);
    string func3=s.substr(17,3);
    string f=opcode+'_'+func3;
    if(opcode=="0110011") return 0;
    auto it=ops.find(f);
    if(it==ops.end()){
        cerr<<"Undefined opcode:"<<f<<endl;
        return 0;
    }
    int f1=it->second;
    //I,jalr,L
    if(f1==1) return signextend(s.substr(0,12));
    //jal
    else if(f1==2) return signextend(s.substr(0,20));
    //B,S
    else if(f1==3) return signextend(s.substr(0,7)+s.substr(20,5));
    //I shift
    else if(f1==4) return signextend(s.substr(7,5));
    else{
        cerr<<"Undefined opcode"<<endl;
        return 0;
    }
}
IDEX Nop(){
    IDEX temp;
    temp.valid=false;
    return temp;
}
bool pipelineEmpty(IFID &ifid,IDEX &idex,EXMO &exmo,MOWB &mowb,int pc,int tot){
    bool se=!ifid.valid&&!idex.valid&&!exmo.valid&&!mowb.valid;
    bool nf=(pc>=tot);
    return se&nf;
}
int main(){
    RegisterFile reg;
    Memory mem(1024);
    ifstream infile("output_sum_of_N_numbers.txt");
    string line;
    vector<string> arr;
    int cycle=0;
    while(getline(infile,line)){
        if(!line.empty()) arr.push_back(line);
    }
    int pc=0,n=arr.size();
    IFID x1;
    IDEX x2;
    EXMO x3;
    MOWB x4;
    while(!pipelineEmpty(x1,x2,x3,x4,pc,n)){
        cycle++;
        if(x4.valid&&x4.ctrl.regWrite&&x4.rdl!=0){
            int writev=(x4.ctrl.mem2Reg)?x4.ldout:x4.aluout;
            reg.write(x4.rdl,writev);
        }
        MOWB tx4;
        tx4.valid=false;
        if(x3.valid){
            tx4.valid=true;
            tx4.ctrl=x3.ctrl;
            tx4.rdl=x3.rdl;
            tx4.dpc=x3.dpc;
            tx4.npc=x3.npc;
            tx4.bpc=x3.bpc;
            tx4.jpc=x3.jpc;
            tx4.aluout=x3.aluout;
            if(x3.ctrl.memRead) tx4.ldout=mem.loadWord(x3.aluout);
            if(x3.ctrl.memWrite) mem.storeWord(x3.aluout,reg.read(x3.rsl2));
        }
        EXMO tx3;
        tx3.valid=false;
        if(x2.valid){
            tx3.valid=true;
            tx3.ctrl=x2.ctrl;
            tx3.dpc=x2.dpc;
            tx3.imm=x2.imm;
            tx3.rdl=x2.rdl;
            tx3.npc=x2.npc;
            tx3.bpc=x2.bpc;
            tx3.jpc=x2.jpc;
            int rs1=x2.rs1;
            int rs2=x2.ctrl.ALUsrc?x2.imm:x2.rs2;
            if(x4.valid&&x4.ctrl.regWrite&&x4.rdl!=0){
                int wb=(x4.ctrl.mem2Reg?x4.ldout:x4.aluout);
                if(x4.rdl==x2.rsl1) rs1=wb;
                if(!x2.ctrl.ALUsrc&&x4.rdl==x2.rsl2) rs2=wb;
            }
            if(x3.valid&&x3.ctrl.regWrite&&x3.rdl!=0){
                if(x3.rdl==x2.rsl1) rs1=x3.aluout;
                if(!x2.ctrl.ALUsrc&&x3.rdl==x2.rsl2) rs2=x3.aluout;
            }
            tx3.aluout=doALU(x2.ctrl.ALUOp,x2.func7,x2.func3,rs1,rs2);
            tx3.rsl2=x2.rsl2;
            tx3.aluzeroflag=(tx3.aluout==0);
        }
        IDEX tx2;
        tx2.valid=false;
        bool stall=false;
        if(x1.valid){
            string s=x1.inst;
            int imm=genimm(s);
            int rsl1=binToInt(s.substr(12,5));
            int rsl2=binToInt(s.substr(7,5));
            int rdl=binToInt(s.substr(20,5));
            string func3=s.substr(17,3);
            string func7=s.substr(0,7);
            string opcode=s.substr(25,7);
            CW ctrl=controller(opcode);
            if(opcode=="0010011") func7="1011110";
            if(x2.valid&&x2.ctrl.memRead&&x2.rdl!=0&&(x2.rdl==rsl1||x2.rdl==rsl2)) stall=true;
            if(!stall){
                tx2.valid=true;
                tx2.ctrl=ctrl;
                tx2.dpc=x1.dpc;
                tx2.npc=x1.npc;
                tx2.bpc=imm-1;
                tx2.jpc=imm-1;
                tx2.inst=x1.inst;
                tx2.imm=imm;
                tx2.rsl1=rsl1;
                tx2.rs1=reg.read(rsl1);
                tx2.rsl2=rsl2;
                tx2.rs2=reg.read(rsl2);
                tx2.rdl=rdl;
                tx2.func3=func3;
                tx2.func7=func7;
            }
        }
        IFID tx1;
        tx1.valid=false;
        if(!stall&&pc<n){
            tx1.valid=true;
            tx1.inst=arr[pc];
            tx1.dpc=pc;
            tx1.npc=tx1.dpc+1;
            pc++;
        }
        int pc_after_branch=-1;
        if(tx3.valid&&tx3.ctrl.branch&&tx3.aluzeroflag) pc_after_branch=tx3.bpc;
        if(tx3.valid&&tx3.ctrl.jump&&tx3.aluzeroflag) pc_after_branch=tx3.jpc;
        if(pc_after_branch!=-1){
            pc=pc_after_branch;
            tx1.valid=false;
            tx2=Nop();
        }
        if(stall){
            tx2=Nop();
            tx1=x1;
        }
        x4=tx4;
        x3=tx3;
        x2=tx2;
        x1=tx1;
    }
    cout<<"Number of clock cycles:\t"<<cycle<<endl;
    reg.print();
    return 0;
}