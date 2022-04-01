Logan Falzarano
Tal Magdish

We split up the work mostly evenly with Logan taking a large chunk of the code's foundation and Tal 
tying up lose ends with test trace files and efficiency/runtime solutions. In the end we both worked 
together on debugging and improving readbility.


In our trials we tested all 6 legal permutations of allocate type, write type, and eviction type which includes:
	write allocate, write back, and least recently used
	write allocate, write through, and least recently used
	no write allocate, write back, and least recently used
	write allocate, write back, and first in first out
	write allocate, write through, and first in first out
	no write allocate, write back, and first in first out
And we tested each of these on
	8192 sets, 1 block each, 16 bytes of memory (aka a 1-way set-associative cache) (direct mapped)
	2048 sets, 4 block each, 16 bytes of memory (aka a 4-way set-associative cache)
	1024 sets, 8 block each, 16 bytes of memory (aka a 8-way set-associative cache)
	512 sets, 16 block each, 16 bytes of memory (aka a 8-way set-associative cache)
	128 sets, 64 block each, 16 bytes of memory (aka a 64-way set-associative cache)
	1 sets, 8192 block each, 16 bytes of memory (aka a 64-way set-associative cache) (fully associative)
All of which have a cache capacity of 8192 bytes
Each test was conducted on the trace file gcc.trace
In determining best overall effectiveness we focused on store misses and total cycles. Our statistics show that
write-allocate consistently has fewer misses than no-write-allocate, and when write allocate is paired with 
write-back it also consistently has the lowest number of clock cycles. From this we can safely say that write-allocate
and write-back together have the best overall effectiveness. LRU also appears to result in fewer misses and clock cycles.
Once we chose the write-allocate, write-back, LRU cache, we looked at associativity factor. Clock cycles and misses 
steadily decereased as our associativity increased, with the direct mapped cache producing the highest number of 
misses and clock cycles, while the fully associative cache performed best. This information comes together to show that
the most efficient cache would be a fully associative, write-allocate, write-back, LRU cache. We do understand that it 
is expensive and often infeasible to maximize associativity factor, so we investigated at which point the benefit of increased
associativity factor seemed to diminish. Going from the 4-way set-associative cache to the direct mapped 1-way set-associative 
cache, our total clock cycles decreased by 4% when averaging across all permutations of allocate type, write type, and eviction 
type, going from the 4-way set-associative cache to the 8-way set-associative cache, our total clock cycles decreased by less 
than 1%, and then less and less thereafter. A similar pattern was identified with number of misses. In an effort to find the 
most effective associativity factor within reasonablee affordability, it seems as though the 4-way set-associative cache provides 
the most efficient solution, as any higher associativity factor does not result in a significant decrease in clock cycles.
Therefore keeping in mind minimizing cost while maximixing effectiveness, we chose the 4-way set-associative write-allocate, 
write-back, LRU cache.

1-way-set-associative	write-allocate write-back lru	no-write-allocate write-through lru	    write-allocate write-through lru	write-allocate write-back fifo	no-write-allocate write-through fifo	write-allocate write-through fifo
Total loads	            318197	                        318197	                                318197	                            318197	                        318197	                                318197
Total stores            97486	                        197486	                                197486	                            197486	                        197486	                                197486
Load hits	            315204	                        311693	                                315204	                            315204	                        311693	                                315204
Load misses	            2993	                        6504	                                2993	                            2993	                        6504	                                2993
Store hits	            188486	                        164946	                                188486	                            188486	                        164946	                                188486
Store misses	        9000	                        32540	                                9000	                            9000	                        32540	                                9000
Total cycles	        6626883	                        22833343	                            25061483	                        6626883	                        22833343	                            25061483			
						
						
4-way-set-associative	write-allocate write-back lru	no-write-allocate write-through lru	    write-allocate write-through lru	write-allocate write-back fifo	no-write-allocate write-through fifo	write-allocate write-through fifo
Total loads	            318197	                        318197	                                318197	                            318197	                        318197	                                318197
Total stores	        197486	                        197486	                                197486	                            197486	                        197486	                                197486
Load hits	            315715	                        312192	                                315715	                            315594	                        312165                              	315594
Load misses	            2482	                        6005	                                2482	                            2603	                        6032	                                2603
Store hits	            188595	                        165213	                                188595	                            188567	                        165190	                                188567
Store misses	        8891	                        32273	                                8891	                            8919	                        32296	                                8919
Total cycles	        6028083	                        22634010	                            24813483	                        6123283	                        22644787	                            24873083					
						
						
8-way-set-associative	write-allocate write-back lru	no-write-allocate write-through lru	    write-allocate write-through lru	write-allocate write-back fifo	no-write-allocate write-through fifo	write-allocate write-through fifo
Total loads	            318197	                        318197	                                318197	                            318197	                        318197	                                318197
Total stores	        197486	                        197486	                                197486	                            197486	                        197486	                                197486
Load hits	            315755	                        312202	                                315755	                            315627	                        312179	                                315627
Load misses	            2442	                        5995	                                2442	                            2570	                        6018	                                2570
Store hits	            188603	                        165214	                                188603	                            188566	                        165197	                                188566
Store misses	        8883	                        32272	                                8883	                            8920	                        32289	                                8920
Total cycles	        5959283	                        22630011	                            24794283	                        6065683	                        22639194	                            24860283				
						
						
64-way-set-associative	write-allocate write-back lru	no-write-allocate write-through lru	    write-allocate write-through lru	write-allocate write-back fifo	no-write-allocate write-through fifo	write-allocate write-through fifo
Total loads	            318197	                        318197	                                318197	                            318197	                        318197	                                318197
Total stores	        197486	                        197486	                                197486	                            197486	                        197486	                                197486
Load hits	            315779	                        312206	                                315779	                            315779	                        312204	                                315656
Load misses	            2418	                        5991	                                2418	                            2418	                        5993	                                2541
Store hits	            188608	                        165216	                                188608	                            188608	                        165213	                                188576
Store misses	        8878	                        32270	                                8878	                            8878	                        32273	                                8910
Total cycles	        5896083	                        22628413	                            24782683	                        5896083	                        22629210	                            24844683				
						
						
512-way-set-associative	write-allocate write-back lru	no-write-allocate write-through lru 	write-allocate write-through lru	write-allocate write-back fifo	no-write-allocate write-through fifo	write-allocate write-through fifo
Total loads	            318197	                        318197	                                318197	                            318197	                        318197	                                318197
Total stores	        197486	                        197486	                                197486	                            197486	                        197486	                                197486
Load hits	            315785	                        312207	                                315785	                            315672	                        312206	                                315672
Load misses	            2412	                        5990	                                2412	                            2525	                        5991	                                2525
Store hits	            188606	                        165216	                                188606	                            188570	                        165216	                                188570
Store misses	        8880	                        32270	                                8880	                            8916	                        32270	                                8916
Total cycles	        5893683	                        22628013	                            24781083	                        5978483	                        22628413	                            24840683				

8192-way-associative  	write-allocate write-back lru	no-write-allocate write-through lru		write-allocate write-through lru	write-allocate write-back fifo	no-write-allocate write-through fifo	write-allocate write-through fifo
Total loads				318197							318197									318197								318197							318197									318197
Total stores			197486							197486									197486								197486							197486									197486
Load hits				315790							312207									315790								315666							312207									315666
Load misses				2407							5990									2407								2531							5990									2531
Store hits				188612							165216									188612								188574							165216									188574
Store misses			8874							32270									8874								8912							32270									8912
Total cycles			5880483							22628013								24776683							5967683							22628013								24841483