#include <iostream>
#include <string>
#include <cstdlib>
#include <stdbool.h>

using namespace std;

//TODO: Could also do with bit shifting
int is_power_of_two(int n) {
    if (n == 0) {
        return 0;
    } 
    while (n != 1) {
        n /= 2;
        if (n%2 != 0 && n != 1) {
            return 0;
        }
    }
    return 1; //if we get out of the loop return 1
}

int main(int argc, char** argv)
{
    int sets_in_cache = atoi(argv[1]); //atoi could fail when we have 1234bad_input 
    int blocks_in_set = atoi(argv[2]);
    int bytes_in_block = atoi(argv[3]);
    string allocate_type = argv[4];
    string write_type = argv[5];
    string eviction_type = argv[6];

    bool sets_in_cache_is_valid = is_power_of_two(sets_in_cache); //atoi could fail when we have 1234bad_input 
    bool blocks_in_set_is_valid = is_power_of_two(blocks_in_set);
    bool bytes_in_block_is_valid = is_power_of_two(bytes_in_block);
    bool allocate_type_is_valid = allocate_type.compare("write-allocate")==0 || allocate_type.compare("no-write-allocate")==0;
    bool write_type_is_valid = (write_type.compare("write-through")==0 || write_type.compare("write-back")==0);
    bool eviction_type_is_valid = (eviction_type.compare("lru")==0 || eviction_type.compare("fifo")==0);

    cout << "The number of sets in the cache is: " << sets_in_cache << " INPUT IS VALID: " << sets_in_cache_is_valid << "\n";
    cout << "The number of blocks in each set is: " << blocks_in_set << " INPUT IS VALID: " << blocks_in_set_is_valid << "\n";
    cout << "The number of bytes in each block is: " << bytes_in_block << " INPUT IS VALID: " << bytes_in_block_is_valid << "\n";
    cout << "The allocation type is: " << allocate_type << " INPUT IS VALID: " << allocate_type_is_valid << "\n";
    cout << "The the write type is: " << write_type << " INPUT IS VALID: " << write_type_is_valid << "\n";
    cout << "The eviction type is: " << eviction_type << " INPUT IS VALID: " << eviction_type_is_valid <<"\n";

    cout << "You have entered " << argc
         << " arguments:" << "\n";
  
    for (int i = 0; i < argc; ++i)
        cout << "|" << argv[i] << "|" << "\n";
  
    return 0;
}