#include <iostream>
#include <string>
#include <cstdlib>
#include <stdbool.h>
#include <sstream>
#include <vector>

using namespace std;

//This class represents a slot in the Cache (a block of memory loaded from main memory)
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

//This class represents a set in the Cache (A collection of Slots loaded from main memory)
class Set {
    public:
        vector<Slot> slots;
};

//This class represents the Cache as whole (what mode the cache is in, as well as the sets)
//and blocks of memory that are currently in the Cache
class Cache {
    vector<Set> cache;
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
    
    //constructor for the Cache block, sets the cache modes and intialializes the correct number of sets
    public:
        Cache(u_int32_t sets_in_cache, u_int32_t blocks_in_set, u_int32_t bytes_in_block, bool write_allocate, bool no_write_allocate, bool write_through, bool write_back, bool FIFO, bool LRU) {
            //construct the cache parameters
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

            for(u_int32_t i = 0; i < sets_in_cache; i++) {
                Set new_set;
                cache.push_back(new_set);
            }
        }

        //this method take the log base 2 of a given integer
        //Parameters:
        //  n - integer to take the log if
        //Returns:
        //  the log base 2 of the integer based in
        int bitshift_log_base2(int n) {
            int res = 0;
            while (n != 1 && n != 0) {
                n = n >> 1;
                res++;
            }
            return res;
        }

        //this method traces through the input file until all the instructions have been completed
        //Parameters:
        //  none
        //Returns:
        //  none
        void begin_trace() {
            string input_line;
            while(getline(cin, input_line)) {
                string read_or_write = input_line.substr(0, 1);
                string string_memory_address = input_line.substr(4, 8); //we don't really care about the offset, though
                u_int32_t memory_address = stoul(string_memory_address, 0, 16);
                
                u_int32_t tag = memory_address >> (32 - tag_bits);
                u_int32_t index = (memory_address & (((1UL << index_bits) - 1) << offset_bits)) >> offset_bits;

                if(read_or_write.compare("l") == 0) {
                    load_value(index, tag);
                } else if (read_or_write.compare("s") == 0) {
                    store_value(index, tag);
                }
            }
        }

        //Returns index of the hit in the set if we can find it and -1 otherwise
        //Parameters:
        //  set - reference to the set we want to search
        //  tag - the tag of the slot to search for
        //Returns:
        //  the index of the slot in the vector of blocks if there is a hit, -1 for a miss
        int find(const Set & set, u_int32_t tag) {
            for (u_int32_t i=0; i< set.slots.size(); i++) {
                if (set.slots[i].tag == tag) {
                    return i; //we found the given tag
                }
            }
            return -1; //if we can't find the tag we have missed
        }

        //Finds the index of the Slot to evict in the vector of Slots when an evivtion is required
        //Parameters:
        //  set - reference to the set we want to search for a slot to evict
        //Returns:
        //  the index of the slot to evict from the set
        int find_index_to_evict(const Set & set) {
            int index_to_evict = 0;
            for (u_int32_t i=0; i<set.slots.size(); i++) {
                
                if (LRU == true) { // for lru find the least recently accessed
                    if (set.slots[i].access_ts < set.slots[index_to_evict].access_ts) {
                        index_to_evict = i;
                    }
                } else if (FIFO == true) { // for fifo find lowest load ts
                    if (set.slots[i].load_ts < set.slots[index_to_evict].load_ts) {
                        index_to_evict = i;
                    }
                }
            }
            return index_to_evict;
        }

        //This method adds a Slot to a Set, performing evictions if needed
        //Parameters:
        //  index - the index of the set (used to access the correct set)
        //  new_slot - the slot to be added to the set
        //Returns:
        //  none
        void add_to_set(u_int32_t index, Slot new_slot) {
             if (cache[index].slots.size() == blocks_in_set) { //if the set is full, we have to evict
                int index_to_evict = find_index_to_evict(cache[index]);

                // if we are in write-back mode and what we are about to evict is dirty we write that to main memory first
                if ((write_back == true) && cache[index].slots[index_to_evict].is_dirty) { 
                    total_cycles += 100 * bytes_in_block / 4;
                }
                cache[index].slots[index_to_evict] = new_slot;
            } else { //no eviction needed, add it to the set
                cache[index].slots.push_back(new_slot);
            }
            total_cycles++; //adding to a set in the cache takes one cycle
        }

        //This method performs a load (executes when "l" is the instruction form the trace)
        //Parameters:
        //  index - the index of the set (used to access the correct set)
        //  tag - the tag of the memory instruction
        //Returns:
        //  none
        void load_value(u_int32_t index, u_int32_t tag) {
            total_loads++;
            Set set_accessed = cache[index];
            int hit = find(set_accessed, tag);
            if (hit != -1) { //load hit
                cache[index].slots[hit].access_ts = total_cycles; //update access ts
                load_hits++;
                total_cycles++;
            } else { //load miss
                load_misses++;
                total_cycles += (100 * (bytes_in_block / 4)); //get the cache block from main memory
                Slot new_slot = Slot(tag, false, total_cycles, total_cycles); //slot is not different from memory so dirty_bit is false
                //if have a miss we need to add the new slot to the set in the cache
                add_to_set(index, new_slot);
                //load from memory since we missed
            }
        }

        //This method performs a store (executes when "s" is the instruction form the trace)
        //Parameters:
        //  index - the index of the set (used to access the correct set)
        //  tag - the tag of the memory instruction
        //Returns:
        //  none
        void store_value(u_int32_t index, u_int32_t tag) {
            total_stores++;
            Set set_accessed = cache[index];
            int hit = find(set_accessed, tag);
            //store hit
            if (hit != -1) {
                store_hits++;
                if (write_through == true) {
                    cache[index].slots[hit].access_ts = total_cycles; 
                    total_cycles += 100;
                } else if (write_back == true) {
                    cache[index].slots[hit].is_dirty = true;
                    cache[index].slots[hit].access_ts = total_cycles; 
                    //no eviciotn needed since we hit
                }
                total_cycles++; //write to cache takes 1 cycle
            } else {
                store_misses++;
                if (write_through == true && no_write_allocate == true) {
                    // nothing gets written to the cache
                    total_cycles += 100;
                } else if (write_through == true && write_allocate == true) {
                    total_cycles += 100 * bytes_in_block / 4; //load the block from main memory
                    total_cycles += 100; //write 4 byte value through to main memory
                    Slot new_slot = Slot(tag, false, total_cycles, total_cycles); //slot is not different from memory so dirty_bit is false
                    add_to_set(index, new_slot);
                } else if (write_back == true && write_allocate == true) {
                    total_cycles += 100 * bytes_in_block / 4; //load the block from main memory
                    Slot new_slot = Slot(tag, true, total_cycles, total_cycles); //slot is different from memory so dirty_bit is true
                    add_to_set(index, new_slot);
                }
            }
        }

        //This methods displays the stats of the Cache after the trace has been performed
        //Parameters:
        //  none
        //Returns:
        //  none
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

//This function determines if a given integer is a power of two.
//Parameters:
//  n - positive integer
//Returns:
//  true if the integer is a power of 2, false otherwise
int is_power_of_two(int n) {
    return (n != 0) && ((n & (n - 1)) == 0);
}

//returns true if there are enough command line arguments and they are all valid
//prints a message to cerr if the command line arguments are not valid
bool check_command_line_args(u_int32_t* sets_in_cache, u_int32_t* blocks_in_set, u_int32_t* bytes_in_block, string* allocate_type, string* write_type, string* eviction_type, u_int32_t argc, char** argv) {
    if (argc != 7) {
        std::cerr << "Too many or too few command line arguments (there must be exactly 7).\n";
        return false;
    }

    //reads command line args
    * sets_in_cache = atoi(argv[1]);
    * blocks_in_set = atoi(argv[2]);
    * bytes_in_block = atoi(argv[3]);
    * allocate_type = argv[4];
    * write_type = argv[5];
    * eviction_type = argv[6];

    //checks command line args are valid
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

    //print to std::cerr
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

    //using tags rather than string comparison for efficiency
    write_allocate = allocate_type.compare("write-allocate") == 0;
    no_write_allocate = allocate_type.compare("no-write-allocate") == 0;
    write_through = write_type.compare("write-through") == 0;
    write_back = write_type.compare("write-back") == 0;
    FIFO = eviction_type.compare("fifo") == 0;
    LRU = eviction_type.compare("lru") == 0;

    //Once command line args are checked, we can initialize our cache
    Cache cache(sets_in_cache, blocks_in_set, bytes_in_block, write_allocate, no_write_allocate, write_through, write_back, FIFO, LRU);
    cache.begin_trace();
    cache.display_stats();
  
    return 0;
}