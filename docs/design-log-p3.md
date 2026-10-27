# Project 3 Design Log
**Author:** <Hector Perez> · **P2 code used:** <my own | instructor reference>
**Tools used:** While completing this projects I used tools such as VS Code to complete and edit the project, as well as 
for formatting for ease of understanding the code. Furthermore, ChatGPT/Codex was utilized to explain certain C++ concepts
along with formatting along with assisting me with understanding code and potential errors. 


## 1. Hash table

For the hash table implementation, I chose to use chaining rather than open adressing. In doing so, an outer vector of buckets and an inner vector of owning tool and pointers in each bucket was used. Their tools share a bucket when names collide, and lookup searches that bucket. Because collision handling is simple and doesn't need looking through nearby slots, I preffered this method above linear probing. Furthermore, a 64 bit FNV 1a was used as it processes every character while also considering the order of the characters. There are 16 buckets in the registration at first. It determines whether adding a new tool would cause the load factor to rise above 1.0. In that case, the bucket count is doubled and the current tools are rehashed with the new count.

The weak hase performed poorly as every tested name started with t. As a result, all 9,000 names from tool_1000 to tool_9999 collided with names of equal length. These names were significantly more evenly dispersed by FNV-1a. Insertion is amortized O(1) with a decent hash distribution as doubling allows for numerous cheap insertions prior to another resize, but a sporadic resize necessitates redistributing all entries.

## 2. Pointer stability
The registry owns each tool through a std::unique_ptr<Tool>. Furthermore, the tool itself is seperatly allocated, while the owning pointer is stored in a bucket. These references are moved into new buckets during rehashing without the tool objects being moved. As a result, even after resizing, a raw pointer that find() returns still refers to the same object. That pointer is borrowed by the caller and cannot be deleted. After inserting the remaining tools and saving the address of the first tool, my scale test verified that the address remained the same after 10,000 insertions.

## 3. Calculator trace
Example: TOKEN | WHAT HAPPENS | NUMBERS | OPERATORS

2| push 2| 2| empty
2| push *| 2| *
(| push (| 2| *(
3| push 3| 2 , 3| *(
+ | push + | 2,3 | *(+
4 | push 4 | 2,3,4 | *( +
) | apply 3 + 4, push 7, remove ( | 2, 7 | *
- | apply * resulting in 14, then push - | 14 | -
5 | push 5 | 14, 5 | -
END | apply 14 -5 | 9 | EMPTY

The right operand is popped first in each operation, followed by the left operand. This maintains the proper division and subtraction order. Because the precedence comparison use >=, left-to-right evaluation results from applying waiting operators with equal precedence first. Nine is the only number that remains after all operators have been applied.

## 4. Errors as values

During a normal operator, tool failures or erros are expected. For example, a path might escape teh workspace, a requested file may potentially not exist, and a calculation may attempt to divide by 0. Returni9ng an error value allows the harness to send the failure back to the model in order for it to explain teh issue or attempt another request. My resolve() and calc_eval() functiuons return false and fill in an error message whereas tools use ToolResult::failure(). In addition, exceptions are appropriate for programming errors, such as calling as_string() on a JSON number. Lastly, the dispatches catches unexpected exceptions as a final safegaurd so a failed tool call does not creash the conversation.

## 5. What I would design differently

Approaching this program differetly I would improve the order in which I developed and tested the registry. Firstly, instead of waiting for the 10,000 tool test to check resizing. I would test the first resize immediately after implementing it. Inserting 17 tools into the initial 16 buckets would allow a small example that is potentiall easier to debug. Furthermore, I would verify teh tool count, look up every name, and compare saved pointers before and after resizing. This could have helped me catch the misplaced bucket assignment earlier and provided me with increased secureity before running a larger test. 