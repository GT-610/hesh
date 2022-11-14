# WARNING 
* The program has many critical bugs. 
* And this documentation is a joke.
* The program does not have the functions described.

***
# hesh (highly easily shell)

***

### Info

​		**hesh** provides limited function. hesh has a difference with real linux shell. For example, when user send SIGINT to foreground process, kernal will send the signal to the process in real linux shell, but the function is implemented by parent process forward to child process in hesh.

​		Roughly speacking, hesh pushes back function implementing thought real linux shell.

***

### How to work
​		You will see a simple command line when you start-up the program. It's the program interface. It's waitting input a command then to process. 

​		hesh has a particular function called 'process'. The function every character one by one, and handle special symbol. It will replace all space to '\0'. Then, it will check every string and handle special character or string.  hesh allocate indexed array to every flag. Every location of flag will be storaged its own indexed array. When it's done, hesh will use a specific way to execute them.

​		It has logic on own. '&' and "&&" have highest level in the program. 

Because of '&' and '&&' have highest level, so you may will encounter such a situation:

> grep "love" text.txt | prog & echo "hesh is garbage" >> tmp.txt & sleep 3 && jobs

For this command, hesh treat `grep "love" text.txt`  and `echo "hesh is grabage" >> tmp.txt` as background process.  `grep` and `echo`  were executed independently by the parent process calls twice fork(). Then, the parent execute `sleep 3` on foreground, hesh will suspend 3 sec and execute `jobs` .

### Function

**hesh** has in internal command:

```
*   jobs	 		  // show all backgroud process  
*   fg #job_id        // stop job
*   cg #job_id        // restart job
```



You also use`Ctrl+C`  or `Ctrl+Z`  to control foreground process.
Like other shell，hesh provides '&', "&&", '>', '<', ">>", '|'。

