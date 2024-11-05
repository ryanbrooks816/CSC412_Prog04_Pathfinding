# CSC 412 - Assignment 3

Ryan Brooks

Extra Credit Completed:
2, 3 and 4.
(For extra credit 2, the description alongside the implementation can be found on pg. 3 and 4 of the report)

Please install `sudo apt install bc` to allow floating point arithmetic needed for the EC4 bash script!

To compile the script in debug mode use the flag -DDEBUG like so (will take much longer and generates debug text files intended to be used by the Python programs).
`g++ -Wall -DDEBUG -std=c++20 <version_folder>/*.cpp -o prog`

Sources:
How these sources were used are defined in my Report.

For implementing the A* function:
https://en.wikipedia.org/wiki/A*_search_algorithm

For implementing the Caching and Hashing functions:
- https://en.cppreference.com/w/cpp/utility/hash
- https://ianyepan.github.io/posts/cpp-custom-hash/
- https://www.codeproject.com/Articles/1209347/Designing-A-Generic-and-Modular-Cplusplus-Memoizer

For structuring my testing framework
- https://stackoverflow.com/questions/28737776/standard-way-for-writing-a-debug-mode-in-c
