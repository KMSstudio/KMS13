#include <iostream>
#include <fstream>
#include <sstream>

#include <algorithm>
#include <vector>

using namespace std;

/*
* [file construction]
* contents file is constructed by [size of key file, [key file], [contents file]]
* size of key file means the number of element in key file, which can expressed in two pair of caracter
* it also means the number of Huffman code
*/

/*
* [key file construction]
*
* key file is constructed by loop of [character, code length]
* 
* specially, key file include not code but code length
* since In this program, Huffman code will be formed, sorted regularly, 
* so code can be rebuilt by code length 
* 
* character and code length is just one char valueable, so it can read and write easily
* 
* (It's definitely not that code length is bigger than 127, 
* since this program apply 128 character, so Huffman code cannot exceed 127)
*/

/*
* [data file construction]
*
* data file is constructed by one number and successive code list which is converted from original file
* file's first number is "length of successive code %(mod) 8", this number help program to read file correctly
* and the others are part for save code list.
* (code will stored in reverse state due to convenient of reading/writing)
*
* for instance, if code of 'a', 'b', 'c', is "0110", "1011" and "110", "accbac" will be converted like:
*           01101101 10101101 10110, and expressed like:
*           00000101 10110110 10110101 00001101
*/

namespace KmsStd {
    typedef long long intll;

    enum { FILE_CON_ERR = 0x11, FILE_EMPTY_ERR = 0x12, SUCCESS = 0x00 };

    template<typename T>
    std::ostream& write_typed_data(std::ostream& stream, const T& value) {
        return stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }
    template<typename T>
    std::istream& read_typed_data(std::istream& stream, T& value) {
        return stream.read(reinterpret_cast<char*>(&value), sizeof(T));
    }

    /* ////////////////////////////// pressing ////////////////////////////// */
    namespace prs {
        typedef pair < pair < intll, char >, string > TYP1;

        bool CLC1(TYP1 a, TYP1 b) { return a.second < b.second; }
        void CLC2(string& BPF, ofstream& fout, int cycle, unsigned char ODF = 0) {
            for (int j = 0; j < cycle; j++) { ODF |= (unsigned char)(BPF[j] - '0') << j; }
            write_typed_data(fout, ODF);
        }

        //input file name, key file name, data file name
        int press(string IFN = "input.txt", string DFN = "data") {
            ifstream fin;

            string IFC; //input file contents
            stringstream ss;
            int IFS; //input file size

            bool registed[128] = { 0, };
            int IIV[128] = { 0, }; //index in vector
            int SEM; //semi valuable for calculation

            int vector_size;
            pair<int, intll> CM; //calculation supporter about constructing minimun value
            intll CS; //calculation support valuable

            ofstream fout;

            intll PFS; //predicted pressed file size (just in range of data file)
            intll PWS; //predicted whole file size

            vector < TYP1 > code; //use_count, char, code
            string BPF; //buffer for writing file

            /* read file */
            fin.open(IFN);
            if (!fin) { return FILE_CON_ERR; }
            ss << fin.rdbuf();
            IFC = ss.str();
            IFS = IFC.size();
            if (!IFS) { return FILE_EMPTY_ERR; }
            fin.close();

            /* caclulate use count */
            SEM = 0;
            for (int i = 0; i < IFS; i++) {
                if ((IFC[i] > 127) || (IFC[i] < 0)) { IFC[i] = '?'; }

                if (registed[IFC[i]]) {
                    code[IIV[IFC[i]]].first.first++;
                }
                else {
                    registed[IFC[i]] = 1;
                    IIV[IFC[i]] = SEM++;

                    code.push_back(make_pair(make_pair(1, IFC[i]), ""));
                }
            }
            cout << "get file & organize file successfully ( " << IFC.size() * 8 << " bit )" << endl;

            sort(code.rbegin(), code.rend());

            /* construct huffman code */
            code[0].second = "0";
            if (code.size() > 1) { code[1].second = "1"; }

            vector_size = code.size();
            for (int i = 2; i < vector_size; i++) {
                CM = make_pair(-1, LLONG_MAX);
                for (int j = 0; j < i; j++) {
                    CS = code[j].first.first + (code[i].first.first * (code[j].second.size() + 1));
                    if (CS < CM.second) { CM.first = j; CM.second = CS; }
                }
                code[i].second = code[CM.first].second + "0";
                code[CM.first].second += "1";
            }
            sort(code.begin(), code.end(), CLC1);

            /* construct key file & calculate pressed file size*/
            fout.open(DFN, std::ios::binary);
            if (!fout) { return FILE_CON_ERR; }
            PFS = 0;
            write_typed_data(fout, (char)vector_size);//
            for (int i = 0; i < vector_size; i++) {
                write_typed_data(fout, code[i].first.second);
                write_typed_data(fout, (char)code[i].second.size());

                PFS += code[i].first.first * code[i].second.size();
                IIV[code[i].first.second] = i;
            }
            PWS = PFS + 8 + (16 * vector_size);
            cout << "construct huffman code & key file successfully ( " << PWS << " bit )" << endl;
            cout << "  { " << (double) PWS * 12.5 / IFS << " % }" << endl;

            /* construct data file */
            BPF = "";
            write_typed_data(fout, (unsigned char)((PFS - 1) % 8 + 1));
            for (int i = 0; i < IFS; i++) {
                BPF += code[IIV[IFC[i]]].second;
                while (BPF.size() >= 8) {
                    CLC2(BPF, fout, 8);
                    BPF.erase(0, 8);
                }
            }
            if (BPF.size()) { CLC2(BPF, fout, BPF.size()); }
            fout.close();

            return SUCCESS;
        }
    }


    /* ////////////////////////////// extending ////////////////////////////// */
    namespace ext {
        enum { lft, rht };
        //this calss may be unsafe 
        class BTnode {
        public:
            char data;
            BTnode* left;
            BTnode* right;
            BTnode* top;

            BTnode(BTnode* left = nullptr, BTnode* right = nullptr, BTnode* top = nullptr) {
                this->data = 0x0;
                this->left = left;
                this->right = right;
                this->top = top;
            }
            BTnode(char data, BTnode* left = nullptr, BTnode* right = nullptr, BTnode* top = nullptr) {
                this->data = data;
                this->left = left;
                this->right = right;
                this->top = top;
            }
            ~BTnode() {
                if (this->left != nullptr) { delete(this->left); }
                if (this->left != nullptr) { delete(this->right); }
            }

            void BTnode_con_unsafe(int dir, char V = 0x0) {
                BTnode* lt = new BTnode;
                if (dir == lft) {
                    this->left = lt;
                    this->left->data = V; //left code = this code + "0"
                }
                else {
                    this->right = lt;
                    this->right->data = V; //right code = this code + "1"
                }
                lt->top = this;
            }

            //are node's left and right filled?
            bool isF(void) {
                if ((this->left != nullptr) && (this->right != nullptr)) { return true; }
                else { return false; }
            }
        };

        //calculation 2 (cur and BTtree, fout must be initalized, opened)
        void CLC1(const unsigned char IPI, BTnode*& cur, BTnode*& BTR, ofstream& fout, int cycle = 8) {
            for (int i = 0; i < cycle; i++) {
                if (IPI & (1 << i)) { cur = cur->right; }
                else { cur = cur->left; }

                if (cur->data) {
                    fout.put(cur->data);
                    cur = BTR;
                }
            }
        }

        //result file name, key file name, data file name
        int extend(string RFN = "extended_file.txt", string DFN = "data") {
            ifstream fin;
            ofstream fout;

            unsigned char IHS; //input Huffman code size
            unsigned char IPC; //input character
            unsigned char SKF; //size of key file
            bool LVO; //logic valuable for optimization

            BTnode* BTR = new BTnode; //binary tree root node
            BTnode* cur = BTR; //
            int DPT; //depth of tree

            unsigned char LFS; // last file size
            unsigned char IPI; //input string data
            unsigned char BIPI; //before input string data

            /* read key file & construct BT tree */
            fin.open(DFN, std::ios::binary);
            if (!fin) { return FILE_CON_ERR; }
            if (fin.eof()) { return FILE_EMPTY_ERR; }

            read_typed_data(fin, SKF);
            DPT = 0;
            for (int k = 0; k < SKF; k++) {
                read_typed_data(fin, IPC);
                read_typed_data(fin, IHS);

                for (LVO = cur->isF(); (DPT < (IHS - 1)) || LVO; LVO = cur->isF()) {
                    if (LVO) { cur = cur->top; DPT--; }
                    else {
                        if (cur->left == nullptr) {
                            cur->BTnode_con_unsafe(lft);
                            cur = cur->left;
                        }
                        else {
                            cur->BTnode_con_unsafe(rht);
                            cur = cur->right;
                        }
                        DPT++;
                    }
                    LVO = cur->isF();
                }

                if (cur->left == nullptr) { cur->BTnode_con_unsafe(lft, IPC); }
                else { cur->BTnode_con_unsafe(rht, IPC); }
            }
            cout << "read / construct Huffman code tree sucessfully" << endl;

            /* read data file & make extended file */
            fout.open(RFN);
            if (!fout) { return FILE_CON_ERR; }

            cur = BTR;
            read_typed_data(fin, LFS);
            read_typed_data(fin, IPI);
            for (;;) {
                BIPI = IPI;
                read_typed_data(fin, IPI);
                if (fin.eof()) { break; }
                CLC1(BIPI, cur, BTR, fout);
            }
            CLC1(BIPI, cur, BTR, fout, LFS);
            delete BTR;

            fin.close();
            fout.close();
            cout << "write extended file successfully" << endl;

            return SUCCESS;
        }
    }
}

int FNC1(int RST) {
    switch (RST) {
    case KmsStd::FILE_CON_ERR:
        cout << "file connect error\n";
        return 1;
    case KmsStd::FILE_EMPTY_ERR:
        cout << "file empty error\n";
        return 1;
    case KmsStd::SUCCESS:
        cout << "success\n";
        return 0;
    default:
        cout << "unknown error\n";
        return 1;
    }
}

int main() {
    int INP, RST;
    string FNM;

    for (;;) {
        cout << "\n     [  compress : 1  |  extend : 2  |  auto compress & extend : 3  |  exit : 4  ]\n";
        do {
            cout << "->";
            cin >> INP;
        } while ((INP < 1) || (INP > 4));

        if (INP == 1) {
            cout << "     input file name\n->";
            cin >> FNM;
            if (FNM == "auto") { FNM = "input.txt"; }
            RST = KmsStd::prs::press(FNM);
            FNC1(RST);
        }
        else if (INP == 2) {
            cout << "     result file name\n->";
            cin >> FNM;
            if (FNM == "auto") { FNM = "output.txt"; }
            RST = KmsStd::ext::extend(FNM);
            FNC1(RST);
        }
        else if (INP == 3) {
            RST = KmsStd::prs::press();
            if (!FNC1(RST)) { RST = KmsStd::ext::extend(); }
            FNC1(RST);
        }
        else { break; }
    }
    return 0;
}