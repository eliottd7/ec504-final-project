#include <string>
#include <algorithm>
#include <vector>
//Levenshtein algorithm used to count how many difference two strings have. Can be used to detect total differences between string. 
using namespace std;
//#ifdef LEVI
//#define LEVI
int levenshtein(const string& x, const string& y)
{
    int m = x.length();
    int n = y.length();
    vector<int> previous(n + 1, 0);
    vector<int> current(n + 1, 0);
    for (int j = 0; j <= n; j++) {
        previous[j] = j;
    }
    for (int i = 1; i <= m; i++) {
        current[0] = i;
        for (int j = 1; j <= n; j++) {
            if (x[i - 1] == y[j - 1]) {
                current[j] = previous[j - 1];
            }
            else {
                current[j] = 1 + min((current[j - 1]), min((previous[j]), previous[j - 1]));
            }
        }
        previous = current;
    }
    return current[n];
}

void filescan(string& x, string& y) {
    int diff = levenshtein(x, y);
    if (diff <= 5) {
        // keep file associtaed to y in
    }
    else {
        // remove file accociated to y
    }
}
//#endif
