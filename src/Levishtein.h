#include "binfmt.h"
#include <algorithm>
#include <string>
#include <vector>
// Levenshtein algorithm used to count how many difference two strings have. Can be used to detect total differences between string.
using namespace std;
//#ifdef LEVI
//#define LEVI
/*Current Issues: - In order to properly implement the current funciton, there needs to be an array of edit_ts;
 * that can be created upon each edit detected. as of now there is no way to do so.
 * - Unsure if reading is consistent in every case.
 * - Unsure how to account for "at most 5 edits accepted"
 * - Needs more testing. (obviouslY)
 */
string levenshtein(const string& x, const string& y, edit_t* z, int size) {

    int m = x.length();
    int n = y.length();
    vector<int> previous(m + 1, 0);
    vector<int> current(n + 1, 0);
    string newy;
    int newpoint = 0;
    long offsize = 0;
    int counter = 0;
    int editcounter = 0;
    for ( int j = 0; j <= n; j++ ) {
        previous[j] = j;
    }
    for ( int i = 1; i <= m; i++ ) {
        current[0] = i;
        for ( int j = 1; j <= n; j++ ) {
            counter++;
            if ( x[i - 1] == y[j - 1] ) {
                current[j] = previous[j - 1];
            } else {
                if ( y[j - 1] == x[j] ) { // these structs instruct how to replicate the string upon call. Maybe?
                    z->type = EDIT_INSERT;
                    z->offset = counter;
                    z->c = y[j - 1];
                } else if ( y[j] == x[j - 1] ) {
                    z->type = EDIT_DELETE;
                    z->offset = counter;
                    z->c = x[j - 1];
                } else {
                    z->type = EDIT_MODIFY;
                    z->offset = counter;
                    z->c = y[j - 1];
                }
                current[j] = 1 + min((current[j - 1]), min((previous[j]), previous[j - 1]));
            }
        }

        previous = current;
    }
    newy = reconstruct(y, z, m);

    return newy;
}

string reconstruct(const string& og, edit_t* z, int size) {
    string newprod;
    for ( int i = 0; i < size; i++ ) {
        if ( i == z->offset ) {
            if ( z->type == EDIT_MODIFY ) {
                newprod[i] = z->c;
            }
            if ( z->type == EDIT_INSERT ) {
                newprod[i] = z->c;
            }
            if ( z->type == EDIT_DELETE ) {
                newprod[i] = z->c;
            }
        } else {
            newprod[i] = og[i];
        }
    }
    return newprod;
}
// endif
