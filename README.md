# Find all partions of number
How to build this project on Linux:  
1. $ cd `<dir with your project>`  
2. $ make -s 

You may also:  
1. Rebuild this project, using command:  
   $ make -s rebuild
2. Remove \"bin\" and \"obj\" directories, using command:  
   $ make -s clean
3. Create tar archive with this project,
using command:  
   $ make -s tar  

To run this project after building, use the following commands:
1. $ cd `<Directory with this project>`
2. $ mpirun -n `<number of processes>` ./bin/findpart `<arguments>`

See the manual on how to use this program.  
To do this, open manual.txt or enter a commands:  
1. $ cd `<Directory with this project>`  
2. $ ./bin/findpart -h  
