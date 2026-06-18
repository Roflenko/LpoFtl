#include <iostream>
#include <fstream>
#include <string>

using namespace std;

string alf = "aiueoxwympfntslrkhcqbvdzgj";

bool gud_glas_para(int id1, int id2){
	if (id1>8 || id2>8){   //хоть одна не гласная
		return true;
	} else if (id1<5 && id2<5 && id1==id2){
		return false;   //двойная обычная гласная
	} else if ( (id1==5 && id2==0) || (id1==6 && id2==1) || (id1==7 && id2==1) ){
		return false;   //xa, wu, yo
	}
	return true;
}

int main() {
    ofstream out_file("1.txt"); // Файл для вывода

    // 1-буквенные комбинации (все 26 букв)
    for (int i = 0; i < 26; i++) {
        out_file << alf[i] << endl;
    }

    // 2-буквенные комбинации
    string word2 = "  ";
    for (int i = 0; i < 26; i++) {
        word2[0] = alf[i];
        for (int j = 0; j < 26; j++) {
            word2[1] = alf[j];
			out_file << word2 << endl;
        }
    }

    // === 3-буквенные комбинации ===
    string word3 = "   ";
    for (int i = 0; i < 26; i++) {
        word3[0] = alf[i];
        for (int j = 0; j < 26; j++) {
            word3[1] = alf[j];
            for (int k = 0; k < 26; k++) {
                word3[2] = alf[k];
				out_file << word3 << endl;
            }
        }
    }

    out_file.close();
    cout << "ol okay, result in 1.txt" << endl;
}
