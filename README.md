# Round Robbin Scheduler

We have to make a scheduler Round Robbin with lists of high and low priorities. Simultaneously, we are going to schedule the shell(is given some code that simulate the shell that there is in that repository). Also we can give dynamically instructions to the shell that communicates with scheduler for executing them.Instructions:
"e <program>":insert a proccess
"k <id>":kill a process with an id 
"q":quit shell
"p":print task list
"h <id>": set task identified by id to high priority
"l <id>": set task identified by id to low priority
  
 The scheduler has to schedule the high priority tasks till the high priority list is empty and then the low priority tasks.
 If "q" instruction is given the schel die. The shell's id has been chosen as 0.
 For example:
 Type "make" for executing the appropriate compilations and then 
 Type: "scheduler-priority <name_of_executable> <name_of_executable> <name_of_executable> ..."
 and then there will be made a list of tasks that will be inserted in low task list. 
