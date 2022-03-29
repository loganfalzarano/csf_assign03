#include <iostream>
#include <string>
#include <cstdlib>
#include <stdbool.h>
#include <sstream>
#include <vector>

using namespace std;

//TODO: should we name this a Block
class Slot {
    public:
        u_int32_t tag; //we want this to be a binary value
        bool is_valid;
        bool is_dirty;
        u_int32_t access_ts;
        u_int32_t load_ts;

        //constructor for creating a slot
        Slot(u_int32_t tag, bool is_dirty, u_int32_t access_ts, u_int32_t load_ts) {
            this->tag = tag;
            this->is_dirty = is_dirty;
            this->access_ts = access_ts;
            this-> load_ts = load_ts;
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
    bool write_allocate;
    bool no_write_allocate;
    bool write_through;
    bool write_back;
    bool FIFO;
    bool LRU;
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
        Cache(u_int32_t sets_in_cache, u_int32_t blocks_in_set, u_int32_t bytes_in_block, bool write_allocate, bool no_write_allocate, bool write_through, bool write_back, bool FIFO, bool LRU) {
            //construct the cache parameters (WE MIGHT NOT NEED THESE HERE)
            this->sets_in_cache = sets_in_cache;
            this->blocks_in_set = blocks_in_set;
            this->bytes_in_block = bytes_in_block;
            this->write_allocate = write_allocate;
            this->no_write_allocate = no_write_allocate;
            this->write_through = write_through;
            this->write_back = write_back;
            this->FIFO = FIFO;
            this->LRU = LRU;

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
                string read_or_write = input_line.substr(0, 1);
                string string_memory_address = input_line.substr(4, 8); //we don't really care about the offset, though
                u_int32_t memory_address = stoul(string_memory_address, 0, 16);
                
                u_int32_t tag = memory_address >> (32 - tag_bits);
                //TODO: Make this more simple
                u_int32_t index = (memory_address & (((1UL << index_bits) - 1) << offset_bits)) >> offset_bits;

                // cout << "tag is: " << tag << endl;
                // cout << "index is: " << index << endl;
                // cout << input_line << endl;
                // cout << endl;

                if(read_or_write.compare("l") == 0) {
                    //cout << "about to load a value" << endl;
                    load_value(index, tag);
                    //cout << "returned";
                } else if (read_or_write.compare("s") == 0) {
                    //cout << "about to load a value" << endl;
                    store_value(index, tag);
                }
                //cout << "____________"<< endl;
            }
        }

        //returns index of the hit in the set if we can find it and -1 otherwise
        int find(const Set & set, u_int32_t tag) {
            //cout << "searching for a hit" << set.slots.size() << endl;
            for (int i=0; i< set.slots.size(); i++) {
                if (set.slots[i].tag == tag) {
                    return i; //we found the given tag
                }
            }
            return -1; //if we can't find the tag we have missed
        }

        int find_index_to_evict(const Set & set) {
            int index_to_evict = 0;
            //cout << "entered eviction function" << endl;
            for (int i=0; i<set.slots.size(); i++) {
                
                if (LRU == true) { // for lru find the least recently accessed
                    //cout << "got here" << endl;
                    //cout << set.slots[i].access_ts << " < " << set.slots[index_to_evict].access_ts << endl;
                    if (set.slots[i].access_ts < set.slots[index_to_evict].access_ts) {
                        index_to_evict = i;
                    }
                } else if (FIFO == true) { // for fifo find lowest load ts
                    if (set.slots[i].load_ts < set.slots[index_to_evict].load_ts) {
                        index_to_evict = i;
                    }
                }
            }
            //cout << "INdec to evict is:" << index_to_evict << endl;
            return index_to_evict;
        }

        void add_to_set(u_int32_t index, Slot new_slot) {
             if (cache[index].slots.size() == blocks_in_set) { //if the set is full, we have to evict
                int index_to_evict = find_index_to_evict(cache[index]);

                // if we are in write-back mode and what we are about to evict is dirty we write that to main memory first
                if ((write_back == true) && cache[index].slots[index_to_evict].is_dirty) { 
                    total_cycles += 100 * bytes_in_block / 4;
                }
                //cout << "Evicting tag:" << cache[index].slots[index_to_evict].tag << " index " << index;
                cache[index].slots[index_to_evict] = new_slot;
            } else { //no eviction needed, add it to the set
                //cout << "About to place a slot at the end" << endl;
                cache[index].slots.push_back(new_slot);
                //cout << cache[index].slots.size() << endl;
            }
            total_cycles++; //adding to a set in the cache takes one cycle
        }

        void load_value(u_int32_t index, u_int32_t tag) {
            total_loads++;
            //cout << "|" << total_loads << "|" << endl;
            Set set_accessed = cache[index];
            int hit = find(set_accessed, tag);
            if (hit != -1) { //load hit
                //cout << "it was a load hit" << endl;
                cache[index].slots[hit].access_ts = total_cycles; //update access ts
                load_hits++;
                total_cycles++;
                //cout << "it was a load miss" << endl;
                
            } else { //load miss
                load_misses++;
                total_cycles += (100 * (bytes_in_block / 4)); //get the cache block from main memory
                Slot new_slot = Slot(tag, false, total_cycles, total_cycles); //slot is not different from memory so dirty_bit is false
                //if have a miss we need to add the new slot to the set in the cache
                add_to_set(index, new_slot);
                //cout << cache[index].slots.size() << "<- Number of things in slot" << endl;
                //load from memory since we missed
                
            }

        }

        /* void store_value(u_int32_t index, u_int32_t tag) {
            total_stores++;
            Set set_accessed = cache[index];
            int hit = find(set_accessed, tag);
            //store miss

            if (hit == -1) {
                cout << "STORE MISS" << endl;
                //if we miss we need to get the block from memory according to the allocate policy
                store_misses++;
                if (allocate_type.compare("no-write-allocate") == 0) {
                    total_cycles += 100; //write straight to memory, no write to cache
                } else if (allocate_type.compare("write-allocate") == 0) {
                    Slot new_slot = Slot(tag, true, total_cycles, total_cycles);
                    add_to_set(index, new_slot);
                    total_cycles += (100 * (bytes_in_block / 4)); //load the block from main into the cache
                }
            } else {
                cout << "STORE HIT" << endl;
                store_hits++;
            }
            // once we have loaded the block accordingly, then we proceed with writing

            // in write-through mode, we write the` block straight to main memory, regardless of whether we just loaded it into the cache
            if (write_type.compare("write-through") == 0) {
                if (hit != -1) {
                    cache[index].slots[hit].access_ts = total_cycles; //don't think we need this
                }
                total_cycles += 100;
            //in write-back mode
            } else if (write_type.compare("write-back") == 0) {
                //write to Cache and mark it as dirty
                Slot new_slot = Slot(tag, true, total_cycles, total_cycles); //these two lines feel repeated from within write-allocate
                add_to_set(index, new_slot); //I think we might be able to get rid of them because write allocate takes care of writing to the cache
                total_cycles++;
                //cache[index].slots.push_back(new_slot);
                //add_to_set(index, new_slot);
            }
        } */

        void store_value(u_int32_t index, u_int32_t tag) {
            total_stores++;
            Set set_accessed = cache[index];
            int hit = find(set_accessed, tag);
            //store hit
            if (hit != -1) {
                store_hits++;
                if (write_through == true) {
                    cache[index].slots[hit].access_ts = total_cycles; //can take out of if
                    total_cycles += 100;
                } else if (write_back == true) {
                    cache[index].slots[hit].is_dirty = true;
                    cache[index].slots[hit].access_ts = total_cycles; //can take out of if
                    //no eviciotn needed since we hit
                    // Slot new_slot = Slot(tag, true, total_cycles, total_cycles); //slot is not different from memory so dirty_bit is false
                    // add_to_set(index, new_slot);
                }
                total_cycles++; //write to cache takes 1 cycle
            } else {
                store_misses++;
                if (write_through == true && no_write_allocate == true) {
                    // nothing gets written to the cache
                    total_cycles += 100;
                } else if (write_through == true && write_allocate == true) {
                    total_cycles += 100 * bytes_in_block / 4;
                    total_cycles += 100;
                    Slot new_slot = Slot(tag, false, total_cycles, total_cycles); //slot is not different from memory so dirty_bit is false
                    add_to_set(index, new_slot);
                } else if (write_back == true && write_allocate == true) {
                    total_cycles += 100 * bytes_in_block / 4;
                    Slot new_slot = Slot(tag, true, total_cycles, total_cycles); //slot is different from memory so dirty_bit is true
                    add_to_set(index, new_slot);
                }
            }
            // once we have loaded the block accordingly, then we proceed with writing

            // in write-through mode, we write the` block straight to main memory, regardless of whether we just loaded it into the cache
            
        }



        void display_stats() {
            cout << "Total loads: " << total_loads << endl;
            cout << "Total stores: " << total_stores << endl;
            cout << "Load hits: " << load_hits << endl;
            cout << "Load misses: " << load_misses << endl;
            cout << "Store hits: " << store_hits << endl;
            cout << "Store misses: " << store_misses << endl;
            cout << "Total cycles: " << total_cycles << endl;
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
        std::cerr << "Too many or too few command line arguments (there must be exactly 7).\n"; // should pru_int32_t to stderr i think
        return false;
    }

    //old code
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
    bool c7 = !((allocate_type->compare("no-write-allocate")==0) && (write_type->compare("write-back")==0));

    if (c1 && c2 && c3 && c4 && c5 && c6 & c7) {
        return true;
    }

    //TODO: give user more feedback on how to give command line args
    std::cerr << "Invalid command line arguments.\n";
    return false;

}

int main(int argc, char** argv) {

    u_int32_t sets_in_cache, blocks_in_set, bytes_in_block;
    string allocate_type, write_type, eviction_type;
    bool write_allocate, no_write_allocate, write_through, write_back, FIFO, LRU;

    if (!check_command_line_args(&sets_in_cache, &blocks_in_set, &bytes_in_block, &allocate_type, &write_type, &eviction_type, argc, argv)) {
        return 1; // Exit with a non-zero exit code
    }

    write_allocate = allocate_type.compare("write-allocate");
    no_write_allocate = allocate_type.compare("no-write-allocate");
    write_through = write_type.compare("write-through");
    write_back = write_type.compare("write-back");
    FIFO = eviction_type.compare("fifo");
    LRU = eviction_type.compare("lru");


    //Once command line args are checked, so we can initialize our cache
    Cache cache(sets_in_cache, blocks_in_set, bytes_in_block, write_allocate, no_write_allocate, write_through, write_back, FIFO, LRU);
    cache.begin_trace();
    cache.display_stats();
  
    return 0;
}