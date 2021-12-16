#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>

using namespace std;

#define ADDU 1
#define SUBU 3
#define AND 4
#define OR  5
#define NOR 7

string HALT_CODE = "11111111111111111111111111111111";

// Memory size.
// In reality, the memory size should be 2^32, but for this lab and space reasons,
// we keep it as this large number, but the memory is still 32-bit addressable.
#define MemSize 65536


class RF {
public:
    bitset<32> ReadData1, ReadData2;

    RF() {
        Registers.resize(32);   // 32 Register Files
        for (int i = 0; i < 32; i++)
        {
            Registers[i] = bitset<32>(0);
        }
    }

    void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable) {
        ReadData1 = Registers[RdReg1.to_ulong()];   // 转换为10进制，对齐
        ReadData2 = Registers[RdReg2.to_ulong()];   //把RF中的某个寄存器的数据读出来
        if (WrtEnable.to_ulong() == 1) {
            Registers[WrtReg.to_ulong()] = WrtData;    // 把结果写入rd
        }
    }

    void OutputRF() {
        ofstream rfout; //写操作，     ifstream是读操作，fstream同时读写
        rfout.open("RFresult.txt", std::ios_base::app);
        if (rfout.is_open()) {
            rfout << "A state of RF:" << endl;
            for (int j = 0; j < 32; j++) {
                rfout << Registers[j] << endl;
            }

        } else cout << "Unable to open file";
        rfout.close();
    }
private:
    vector<bitset<32>> Registers;
};


class ALU {
public:
    bitset<32> ALUOperation(bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2) {
        unsigned long result;
        if (ALUOP.to_string() == "001") result = oprand1.to_ulong() + oprand2.to_ulong(); // addu
        else if (ALUOP.to_string() == "011") result = oprand1.to_ulong() - oprand2.to_ulong(); // subu
        else if (ALUOP.to_string() == "100") result = oprand1.to_ulong() & oprand2.to_ulong(); // and
        else if (ALUOP.to_string() == "101") result = oprand1.to_ulong() | oprand2.to_ulong(); // or
        else if (ALUOP.to_string() == "111") result = ~(oprand1.to_ulong() | oprand2.to_ulong()); // nor
        bitset<32> ALUresult((int) result); //两个operand运算结果为result
        return ALUresult;
    }
};


class INSMem {
public:
    bitset<32> Instruction;

    INSMem() {
        IMem.resize(MemSize); //Memsize= 2的32次方
        ifstream imem; //读文件，把imem.txt中的二进制读到IMem中，8位一行
        string line;
        int i = 0;
        imem.open("imem.txt");
        if (imem.is_open()) {
            while (getline(imem, line)) {
                IMem[i] = bitset<8>(line);//string类型line转化为二进制
                i++;
            }
        } else cout << "Unable to open file";
        imem.close();
    }

    bitset<32> ReadMemory(bitset<32> ReadAddress) {
        // (Read the byte at the ReadAddress and the following three byte).
        unsigned long count = ReadAddress.to_ulong();
        int index = (int) count;
        string inst;
        for (int i = 0; i < 4; i++) {
            inst += IMem[index].to_string();
            index++;
        }
        bitset<32> Inst(inst);//inst是32长度字符串
        Instruction = Inst; //32位二进制
        return Instruction;
    }

private:
    vector<bitset<8>> IMem;

};

class DataMem {
public:
    bitset<32> readdata;

    DataMem() {
        DMem.resize(MemSize);
        ifstream dmem;
        string line;
        int i = 0;
        dmem.open("dmem.txt");
        if (dmem.is_open()) {
            while (getline(dmem, line)) {
                DMem[i] = bitset<8>(line);
                i++;
            }
        } else cout << "Unable to open file";
        dmem.close();
    }

    bitset<32> MemoryAccess(bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem) {
        unsigned long count = Address.to_ulong();
        int index = (int) count;
        if ((readmem.to_ulong() == 1) && (writemem.to_ulong() == 0)) {
            string data;
            for (int i = 0; i < 4; i++) {
                data += DMem[index].to_string();
                index++;
            }
            bitset<32> Data(data);
            readdata = Data;
        } else if ((readmem.to_ulong() == 0) && (writemem.to_ulong() == 1)) {
            for (int i = 0; i < 32; i = i + 8) {
                string result = WriteData.to_string().substr(i, 8);
                bitset<8> finalResult(result);
                DMem[index] = finalResult;
                index++;
            }
        }
        return readdata;
    }

    void OutputDataMem() {
        ofstream dmemout;
        dmemout.open("dmemresult.txt");
        if (dmemout.is_open()) {
            for (int j = 0; j < 1000; j++) {
                dmemout << DMem[j] << endl;
            }
        } else cout << "Unable to open file";
        dmemout.close();
    }

private:
    vector<bitset<8>> DMem;
};

class Toolbox {
public:
    void PrintMessage(string message) {
        cout << message << endl;
    }

    string InstructionSelector(string opcode) {
        if (opcode == "000000")
            return "R";
        else if (opcode == "000010")
            return "J";
        else if (opcode == "001001")
            return "addiu";
        else if (opcode == "000100")
            return "beq";
        else if (opcode == "100011")
            return "lw";
        else if (opcode == "101011")
            return "sw";
        else if (opcode == "111111")
            return "halt";
        else
            return "Wrong Operation Code!";
    }

    string FunctionalSelector(string ALUOP) {
        if (ALUOP == "001")
            return "addu";
        else if (ALUOP == "011")
            return "subu";
        else if (ALUOP == "100")
            return "and";
        else if (ALUOP == "101")
            return "or";
        else if (ALUOP == "111")
            return "nor";
        else
            return "Wrong ALU Operation Code!";
    }

    string SplitInsturction(bitset<32> insturction, int start, int offset) {
        string ins = insturction.to_string().substr(start, offset);
        return ins;
    }

    bitset<32> ExtendImmediate(bitset<16> immediate_Itype) {
        string temp_imm_str;
        if (immediate_Itype.to_string().at(0) == '0')
            temp_imm_str = "0000000000000000" + immediate_Itype.to_string();//零对齐
        else if (immediate_Itype.to_string().at(0) == '1')
            temp_imm_str = "1111111111111111" + immediate_Itype.to_string(); //符号对齐
        bitset<32> realImmediate(temp_imm_str);
        return realImmediate;
    }
};


int main() {
    RF myRF;
    ALU myALU;
    INSMem myInsMem;
    DataMem myDataMem;

    Toolbox toolbox;

    bitset<32> currentInstruction;
    string instructionType;
    string functionType;

    string operationCode;
    string ALU_operationCode;

    bitset<32> ALUresult;//rs，rt，rd中的数据，32位二进制形式

    string pc_init = "";
    for (int i = 0; i < 32; i++) {
        pc_init += "0";
    }
    bitset<32> programCounter(pc_init);

    while (1) {
        currentInstruction = myInsMem.ReadMemory(programCounter);
        if (currentInstruction.to_string() == HALT_CODE) {
            toolbox.PrintMessage("Process Halt!");
            break;
        }

        operationCode = toolbox.SplitInsturction(currentInstruction, 0, 6);//从第0位开始，拷贝6位（0-5）
        ALU_operationCode = toolbox.SplitInsturction(currentInstruction, 29, 3);//从第29位开始，拷贝3位（29-31）

        instructionType = toolbox.InstructionSelector(operationCode);
        functionType = toolbox.FunctionalSelector(ALU_operationCode);

        if (instructionType == "R") {
            toolbox.PrintMessage("This is R type! Running in progress.");
            bitset<5> rd_Rtype(toolbox.SplitInsturction(currentInstruction,16, 5));// rd是16-20
            bitset<5> rt_Rtype(toolbox.SplitInsturction(currentInstruction, 11, 5));// rt是11-15
            bitset<5> rs_Rtype(toolbox.SplitInsturction(currentInstruction, 6, 5));// rs是6-10

            myRF.ReadWrite(rs_Rtype, rt_Rtype, 0, 0, 0);

            // ALU
            functionType = toolbox.FunctionalSelector(ALU_operationCode);
            if (functionType == "addu") {
                toolbox.PrintMessage("Using addu Instruction");
                bitset<3> ADDU_op(ADDU);
                ALUresult = myALU.ALUOperation(ADDU_op, myRF.ReadData1, myRF.ReadData2);
            }

            else if (functionType == "subu") {
                toolbox.PrintMessage("Using subu Instruction");
                bitset<3> SUBU_op(SUBU);
                ALUresult = myALU.ALUOperation(SUBU_op, myRF.ReadData1, myRF.ReadData2);
            }

            else if (functionType == "and") {
                toolbox.PrintMessage("Using and Instruction");
                bitset<3> AND_op(AND);
                ALUresult = myALU.ALUOperation(AND_op, myRF.ReadData1, myRF.ReadData2);
            }

            else if (functionType == "or") {
                toolbox.PrintMessage("Using or Instruction");
                bitset<3> OR_op(OR);
                ALUresult = myALU.ALUOperation(OR_op, myRF.ReadData1, myRF.ReadData2);
            }

            else if (functionType == "nor") {
                toolbox.PrintMessage("Using nor Instruction");
                bitset<3> NOR_op(NOR);
                ALUresult = myALU.ALUOperation(NOR_op, myRF.ReadData1, myRF.ReadData2);
            }


            myRF.ReadWrite(0, 0, rd_Rtype, ALUresult, 1);

            programCounter = bitset<32>(programCounter.to_ulong() + 4);
        }



            //若是I型指令，有四种操作方式，addiu，beq，lw，sw
        else if (instructionType == "addiu" || instructionType == "beq" || instructionType == "lw" || instructionType == "sw") {

            toolbox.PrintMessage("This is I type! Running in progress.");

            bitset<5> rs_Itype(toolbox.SplitInsturction(currentInstruction, 6, 5));
            bitset<5> rt_Itype(toolbox.SplitInsturction(currentInstruction, 11, 5));
            bitset<16> immediate_Itype(toolbox.SplitInsturction(currentInstruction, 16, 16));

            myRF.ReadWrite(rs_Itype, rt_Itype, 0, 0, 0);

            bitset<32> finalImm = toolbox.ExtendImmediate(immediate_Itype);

            bitset<3> ADDU_op(ADDU);
            ALUresult = myALU.ALUOperation(ADDU_op, myRF.ReadData1, finalImm);

            if (instructionType == "lw") {
                toolbox.PrintMessage("Using loadword Instruction");
                bitset<32> operand1 = myDataMem.MemoryAccess(ALUresult, 0, 1, 0);
                myRF.ReadWrite(0, 0, rt_Itype, operand1, 1); //load value into the register
                programCounter = bitset<32>(programCounter.to_ulong() + 4);
            }


            else if (instructionType == "sw") {
                toolbox.PrintMessage("Using storeword Instruction");
                myRF.ReadWrite(rt_Itype, 0, 0, 0, 0); // get value from register
                bitset<32> justFlag = myDataMem.MemoryAccess(ALUresult, myRF.ReadData1, 0, 1); //store value to data memory
                programCounter = bitset<32>(programCounter.to_ulong() + 4);
            }


            else if (instructionType == "addiu") {
                toolbox.PrintMessage("Using addiu Instruction");
                //ALUresult = myALU.ALUOperation(ADDU_op, myRF.ReadData1, finalImm);
                myRF.ReadWrite(0, 0, rt_Itype, ALUresult, 1);
                programCounter = bitset<32>(programCounter.to_ulong() + 4);
            }


            else if (instructionType == "beq") {
                toolbox.PrintMessage("Using beq Instruction");
                if (myRF.ReadData1.to_ulong() == myRF.ReadData2.to_ulong()) {
                    //cout << "r1=r2" << endl;
                    string beqImm = finalImm.to_string().substr(2, 30) + "00";
                    // cout << "imm:"<< beqImm << endl;
                    bitset<32> signExtImm(beqImm);

                    bitset<32> beqPC(programCounter.to_ulong() + 4 + signExtImm.to_ulong());
                    programCounter = beqPC;
                }
                else {
                    programCounter = bitset<32>(programCounter.to_ulong() + 4);
                }
                myRF.ReadWrite(0, rt_Itype, 0, 0, 0);
            }



        } else if (instructionType == "J") {
            toolbox.PrintMessage("This is J type! Running in progress.");

            bitset<26> Jtype_Address(toolbox.SplitInsturction(currentInstruction, 6, 26));
            programCounter = bitset<32>(programCounter.to_ulong() + 4);
            string pcReconstruction = toolbox.SplitInsturction(programCounter, 0, 4) + Jtype_Address.to_string() + "00";
            bitset<32> nextPC(pcReconstruction);
            programCounter = nextPC;
        }
        myRF.OutputRF();
    }
    myDataMem.OutputDataMem(); // dump data mem
    return 0;
}