long convert(char* i, int x) //x is size of string
{ //code to convert the char string into a long array. Might be WIP since I havent tested in within a situation regarding the rest of the project but it works outside of it.
//placed in a seperated file cause Im not certain where it should be placed.
    long newline[x];
    char novachar[4];
    int totalcount = x;
    for (int j = 0; j < x; j += 4) {
        if ((totalcount - 4) < 0) {
            for (int k = 0; j < totalcount; k++) {
                novachar[k] = i[x - totalcount - k];
            }
            for (int m = 4 - totalcount; m < totalcount; m++)
                novachar[m] = 0;
            newline[j] = novachar;
            j = x;
        }
        else {
            novachar[j] = i[j];
            novachar[j + 1] = i[j + 1];
            novachar[j + 2] = i[j + 2];
            novachar[j + 3] = i[j + 3];
            newline[j] = novachar;
            totalcount = totalcount - 4;
        }
        return newline;
    }
