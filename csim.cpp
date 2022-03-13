#include <iostream>
#include <string>
#include <cstdlib>
#include <stdbool.h>
#include <sstream>
#include <vector>

using namespace std;

//TODO: should we name this a Block
class Slot {
    u_int32_t tag; //we want this to be a binary value
    bool is_valid;
    bool is_dirty;
    unsigned access_ts;
    unsigned load_ts;

    public:
        //constructor for creating a slot
        Slot(u_int32_t tag, bool dirty_bit) {
            this->tag = tag;
            this->is_dirty = is_dirty;
        }

        //getter methods
        u_int32_t get_tag() {
            return this->tag;
        }

        bool get_dirty_bit() {
            return this->is_dirty;
        }
};

class Set {
    public:
        vector<Slot> slots;
};

class Cache {
    vector<Set> cache;
    //cache parameters (could declare in global scope)
    u_int32_t sets_in_cache;
    u_int32_t blocks_in_set;
    u_int32_t bytes_in_block;
    string allocate_type;
    string write_type;
    string eviction_type;
    //cache_statistics (all initally 0)
    u_int32_t total_loads = 0;
    u_int32_t total_stores = 0;
    u_int32_t load_hits = 0;
    u_int32_t load_misses = 0;
    u_int32_t store_hits = 0;
    u_int32_t store_misses = 0;
    u_int32_t total_cycles = 0;
    //address information (we are using 32 bit addresses)
    u_int32_t offset_bits = 0;
    u_int32_t index_bits = 0;
    u_int32_t tag_bits = 0;
    
    public:
        Cache(u_int32_t sets_in_cache, u_int32_t blocks_in_set, u_int32_t bytes_in_block, string allocate_type, string write_type, string eviction_type) {
            //construct the cache parameters (WE MIGHT NOT NEED THESE HERE)
            this->sets_in_cache = sets_in_cache;
            this->blocks_in_set = blocks_in_set;
            this->bytes_in_block = bytes_in_block;
            this->allocate_type = allocate_type;
            this->write_type = write_type;
            this->eviction_type = eviction_type;

            //calculate the addressing information
            offset_bits = bitshift_log_base2(bytes_in_block);
            index_bits = bitshift_log_base2(sets_in_cache);
            tag_bits = 32 - index_bits - offset_bits;
            // cout << "Offset bits:" << offset_bits << endl;
            // cout << "Index bits:" << index_bits << endl;
            // cout << "Tag bits:" << tag_bits << endl;

            //TODO: Instantiate vector<Set>
            for(int i = 0; i < sets_in_cache; i++) {
                Set new_set;
                cache.push_back(new_set);
            }
        }

        //TODO: Check if bitwise logbase2 is correct
        int bitshift_log_base2(int n) {
            int res = 0;
            while (n != 1 && n != 0) {
                n = n >> 1;
                res++;
            }
            return res;
        }

        //method to give a trace on a give input file
        void begin_trace() {
            //TODO determine if we need to deal with invalid trace files
            string input_line;
            while(getline(cin, input_line)) {
                cout << input_line << endl;
                string read_or_write = input_line.substr(0, 1);
                string string_memory_address = input_line.substr(4, 8); //we don't really care about the offset, though
                //cout << "memory address is:|" << string_memory_address << "|\n";
                u_int32_t memory_address = stoul(string_memory_address, 0, 16);
                //cout << "real memory address is:|" << memory_address << "|\n";

                //u_int32_t tag = memory_address & (((1UL << tag_bits) - 1) << (32 - tag_bits));
                //u_int32_t index = memory_address & (((1UL << index_bits) - 1) << offset_bits);
                u_int32_t tag = memory_address >> (32 - tag_bits);
                //TODO: Make this more simple
                u_int32_t index = (memory_address & (((1UL << index_bits) - 1) << offset_bits)) >> offset_bits;

                //cout << "tag is: " << tag << endl;
                //cout << "index is: " << index << endl;

                cout << input_line << endl;

                if(read_or_write.compare("l") == 0) {
                    //cout << "about to load a value" << endl;
                    load_value(index, tag);
                    //cout << "returned";
                } else if {
                    
                }
                

                //1110 0000 cout << "r/w is:|" << read_or_write<< "|\n";
                
                
            }
        }

        void load_value(u_int32_t index, u_int32_t tag) {
            total_loads++;
            cout << "|" << total_loads << "|" << endl;
            Set set_accessed = cache[index];
            if (set_accessed.slots.empty()) {
                //cout << "this was a load miss\n\n" << endl;
                load_misses++;
                //cout << "bytes in block is:" << bytes_in_block;
                total_cycles += (100 * (bytes_in_block / 4));
                Slot new_slot = Slot(tag, true);
                cache[index].slots.push_back(new_slot);
            } else {
                //cout << "this was a load hit\n\n";
                load_hits++;
                total_cycles++;
            }
            
            
        }

        void display_stats() {
            cout << total_loads << endl;
            cout << total_stores << endl;
            cout << load_hits << endl;
            cout << load_misses << endl;
            cout << store_hits << endl;
            cout << store_misses << endl;
            cout << total_cycles << endl;
        }
};

int is_power_of_two(int n) {
    /* if (n == 0) {
        return 0;
    }
    while (n != 1) {
        n /= 2;
        if (n%2 != 0 && n != 1) {
            return 0;
        }
    }
    return 1; //if we get out of the loop return 1 */
    return (n != 0) && ((n & (n - 1)) == 0); // pretty sure this works but same thing either way
}

//returns true if there are enough command line arguments and they are all valid
bool check_command_line_args(u_int32_t* sets_in_cache, u_int32_t* blocks_in_set, u_int32_t* bytes_in_block, string* allocate_type, string* write_type, string* eviction_type, u_int32_t argc, char** argv) {
    if (argc != 7) {
        cout << "Too many or too few command line arguments (there must be exactly 7).\n"; // should pru_int32_t to stderr i think
        return false;
    }

    //old cold
    /* if (allocate_type.compare("no-write-allocate") && write_type.compare("write-back")) {
        std::cerr << "combine no-write-allocate with write-back.\n";
        return false;
    }
    if (bytes_in_block < 4) {
        cerr << "cannot have block size is less than 4.\n";
        return false;
    } */

    * sets_in_cache = atoi(argv[1]); //atoi could fail when we have 1234bad_input 
    * blocks_in_set = atoi(argv[2]);
    * bytes_in_block = atoi(argv[3]);
    * allocate_type = argv[4];
    * write_type = argv[5];
    * eviction_type = argv[6];

    bool c1 = is_power_of_two(*sets_in_cache);
    bool c2 = is_power_of_two(*blocks_in_set);
    bool c3 = is_power_of_two(*bytes_in_block) && *bytes_in_block >= 4;
    bool c4 = allocate_type->compare("write-allocate")==0 || allocate_type->compare("no-write-allocate")==0;
    bool c5 = (write_type->compare("write-through")==0 || write_type->compare("write-back")==0);
    bool c6 = (eviction_type->compare("lru")==0 || eviction_type->compare("fifo")==0);
    bool c7 = !(allocate_type->compare("no-write-allocate") && write_type->compare("write-back"));

    if (c1 && c2 && c3 && c4 && c5 && c6 & c7) {
        return true;
    }
    //TODO: give user more feedback on how to give command line args
    cout << "Invalid command line arguments.\n";
    return false;

}

int main(int argc, char** argv) {

    u_int32_t sets_in_cache, blocks_in_set, bytes_in_block;
    string allocate_type, write_type, eviction_type;

    if (!check_command_line_args(&sets_in_cache, &blocks_in_set, &bytes_in_block, &allocate_type, &write_type, &eviction_type, argc, argv)) {
        return 1; // Exit with a non-zero exit code
    }

    //Once command line args are checked, so we can initialize our cache
    Cache cache(sets_in_cache, blocks_in_set, bytes_in_block, allocate_type, write_type, eviction_type);
    cache.begin_trace();
    cache.display_stats();
  
    return 0;
}