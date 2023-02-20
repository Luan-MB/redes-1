#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <bitset>

int main() {

    FILE *arq = fopen("cu.txt", "wb");

    char *data = (char *) malloc (63);
    for (int i=0; i<63; ++i)
        data[i] = -127;
    
    fwrite(data, 1, 63, arq);
    fclose(arq);

}