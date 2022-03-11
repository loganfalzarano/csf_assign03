#include <iostream>
#include <string>
#include <cstdlib>
#include <stdbool.h>
#include <sstream>
#include <vector>

using namespace std;

//TODO: should we name this a Block
class Slot {
    string tag;
    bool dirty_bit;

    public:
        //constructor for creating a slot
        Slot(string tag, bool dirty_bit) {
            this->tag = tag;
            this->dirty_bit = dirty_bit;
        }

        //getter methods
        string get_tag() {
            return this->tag;
        }

        bool get_dirty_bit() {
            return this->dirty_bit;
        }
};

class Set {
    vector<Slot> slots;
};

class Cache {
    vector<Set> cache;
    //cache parameters (could declare in global scope)
    int sets_in_cache;
    int blocks_in_set;
    int bytes_in_block;
    string allocate_type;
    string write_type;
    string eviction_type;
    //cache_statistics (all initally 0)
    int total_loads = 0;
    int total_stores = 0;
    int load_hits = 0;
    int load_misses = 0;
    int store_hits = 0;
    int store_misses = 0;
    int total_cycles = 0;
    
    public:
        Cache(int sets_in_cache, int blocks_in_set, int bytes_in_block, string allocate_type, string write_type, string eviction_type) {
            //construct the cache parameters (WE MIGHT NOT NEED THESE HERE)
            this->sets_in_cache = sets_in_cache;
            this->blocks_in_set = blocks_in_set;
            this->bytes_in_block = bytes_in_block;
            this->allocate_type = allocate_type;
            this->write_type = write_type;
            this->eviction_type = eviction_type;
            //TODO: Instantiate vector<Set>
        }

        //method to give a trace on a give input file
        void begin_trace() {
            //TODO determine if we need to deal with invalid trace files
            string input_line;
            while(getline(cin, input_line)) {
                string read_or_write = input_line.substr(0, 1);
                string memory_address = input_line.substr(4, 10); //we don't really care about the offset, though
                cout << "r/w is:|" << read_or_write<< "|\n";
                cout << "memory address is is:|" << memory_address << "|\n";
                cout << input_line << endl;
            }
        }          
};

//TODO: Could also do with bit shifting
int is_power_of_two(int n) {
    // if (n == 0) {
    //     return 0;
    // }
    // while (n != 1) {
    //     n /= 2;
    //     if (n%2 != 0 && n != 1) {
    //         return 0;
    //     }
    // }
    // return 1; //if we get out of the loop return 1
    return (n != 0) && ((n & (n - 1)) == 0); // pretty sure this works but same thing either way
}

//returns true if there are enough command line arguments and they are all valid
bool check_command_line_args(int sets_in_cache, int blocks_in_set, int bytes_in_block, string allocate_type, string write_type, string eviction_type, int argc, char** argv) {
    if (argc != 7) {
        cout << "Too many or too few command line arguments (there must be exactly 7).\n"; // should print to stderr i think
        return false;
    }
    // if (allocate_type.compare("no-write-allocate") && write_type.compare("write-back")) {
    //     std::cerr << "combine no-write-allocate with write-back.\n";
    //     return false;
    // }
    if (bytes_in_block < 4) {
        std::cerr << "cannot have block size is less than 4.\n";
        return false;
    }

    sets_in_cache = atoi(argv[1]); //atoi could fail when we have 1234bad_input 
    blocks_in_set = atoi(argv[2]);
    bytes_in_block = atoi(argv[3]);
    allocate_type = argv[4];
    write_type = argv[5];
    eviction_type = argv[6];

    bool c1 = is_power_of_two(sets_in_cache); //atoi could fail when we have 1234bad_input 
    bool c2 = is_power_of_two(blocks_in_set);
    bool c3 = is_power_of_two(bytes_in_block) && bytes_in_block >= 4;
    bool c4 = allocate_type.compare("write-allocate")==0 || allocate_type.compare("no-write-allocate")==0;
    bool c5 = (write_type.compare("write-through")==0 || write_type.compare("write-back")==0);
    bool c6 = (eviction_type.compare("lru")==0 || eviction_type.compare("fifo")==0);

    if (c1 && c2 && c3 && c4 && c5 && c6) {
        return true;
    }
    //TODO: give user more feedback on how to give command line args
    cout << "Invalid command line arguments.\n";
    return false;

}

//probably good to make this member function inside the Cache class eventually


int main(int argc, char** argv) {
    
    //TODO: Check if there are a correct number of inputs

    int sets_in_cache, blocks_in_set, bytes_in_block;
    string allocate_type, write_type, eviction_type;

    if (!check_command_line_args(sets_in_cache, blocks_in_set, bytes_in_block, allocate_type, write_type, eviction_type, argc, argv)) {
        return 1; // Exit with a non-zero exit code
    }

    //Once command line args are checked, so we can initialize our cache
    Cache cache(sets_in_cache, blocks_in_set, bytes_in_block, allocate_type, write_type, eviction_type);
    cache.begin_trace();
  
    return 0;
}