# Welcome to Homework 0!

For these questions you'll need the mini course and  "Linux-In-TheBrowser" virtual machine (yes it really does run in a web page using javascript) at -

http://cs-education.github.io/sys/

Let's take a look at some C code (with apologies to a well known song)-
```C
// An array to hold the following bytes. "q" will hold the address of where those bytes are.
// The [] mean set aside some space and copy these bytes into teh array array
char q[] = "Do you wanna build a C99 program?";

// This will be fun if our code has the word 'or' in later...
#define or "go debugging with gdb?"

// sizeof is not the same as strlen. You need to know how to use these correctly, including why you probably want strlen+1

static unsigned int i = sizeof(or) != strlen(or);

// Reading backwards, ptr is a pointer to a character. (It holds the address of the first byte of that string constant)
char* ptr = "lathe"; 

// Print something out
size_t come = fprintf(stdout,"%s door", ptr+2);

// Challenge: Why is the value of away equal to 1?
int away = ! (int) * "";


// Some system programming - ask for some virtual memory

int* shared = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
munmap(shared,sizeof(int*));

// Now clone our process and run other programs
if(!fork()) { execlp("man","man","-3","ftell", (char*)0); perror("failed"); }
if(!fork()) { execlp("make","make", "snowman", (char*)0); execlp("make","make", (char*)0)); }

// Let's get out of it?
exit(0);
```

## So you want to master System Programming? And get a better grade than B?
```C
int main(int argc, char** argv) {
	puts("Great! We have plenty of useful resources for you, but it's up to you to");
	puts(" be an active learner and learn how to solve problems and debug code.");
	puts("Bring your near-completed answers the problems below");
	puts(" to the first lab to show that you've been working on this.");
	printf("A few \"don't knows\" or \"unsure\" is fine for lab 1.\n"); 
	puts("Warning: you and your peers will work hard in this class.");
	puts("This is not CS225; you will be pushed much harder to");
	puts(" work things out on your own.");
	fprintf(stdout,"This homework is a stepping stone to all future assignments.\n");
	char p[] = "So, you will want to clear up any confusions or misconceptions.\n";
	write(1, p, strlen(p) );
	char buffer[1024];
	sprintf(buffer,"For grading purposes, this homework 0 will be graded as part of your lab %d work.\n", 1);
	write(1, buffer, strlen(buffer));
	printf("Press Return to continue\n");
	read(0, buffer, sizeof(buffer));
	return 0;
}
```
## Watch the videos and write up your answers to the following questions

**Important!**

The virtual machine-in-your-browser and the videos you need for HW0 are here:

http://cs-education.github.io/sys/

The coursebook:

http://cs341.cs.illinois.edu/coursebook/index.html

Questions? Comments? Use Ed: (you'll need to accept the sign up link I sent you)
https://edstem.org/

The in-browser virtual machine runs entirely in Javascript and is fastest in Chrome. Note the VM and any code you write is reset when you reload the page, *so copy your code to a separate document.* The post-video challenges (e.g. Haiku poem) are not part of homework 0 but you learn the most by doing (rather than just passively watching) - so we suggest you have some fun with each end-of-video challenge.

HW0 questions are below. Copy your answers into a text document (which the course staff will grade later) because you'll need to submit them later in the course. More information will be in the first lab.

## Chapter 1

In which our intrepid hero battles standard out, standard error, file descriptors and writing to files.

### Hello, World! (system call style)
1. Write a program that uses `write()` to print out "Hi! My name is `<Your Name>`".

```C
#include <unistd.h>
int main() {
	write(1,"Hi! My name is Chen Yang",26);
	return 0;}
```

### Hello, Standard Error Stream!
2. Write a function to print out a triangle of height `n` to standard error.
   - Your function should have the signature `void write_triangle(int n)` and should use `write()`.
   - The triangle should look like this, for n = 3:
```C
void write_triangle(int n) {
	int i ,j;
	for(i = 0;i < n ;++i) {
		for(j = -1;j < i;++j) {
			write(2, "*",1);
		}
		write(2, "\n",1);
	}
		
}
```
### Writing to files
3. Take your program from "Hello, World!" modify it write to a file called `hello_world.txt`.
   - Make sure to to use correct flags and a correct mode for `open()` (`man 2 open` is your friend).

```C
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
int main() {
	mode_t mode = S_IRUSR | S_IWUSR; 
	int fildes = open("hello_world.txt",O_CREAT |O_TRUNC | O_RDWR,mode);
	write(fildes,"Hi! My name is Chen Yang",26);
	return 0;}
```

### Not everything is a system call
4. Take your program from "Writing to files" and replace `write()` with `printf()`.
   - Make sure to print to the file instead of standard out!
```C
int main() {
	mode_t mode = S_IRUSR | S_IWUSR; 
	close(1);
	open("hello_world.txt",O_CREAT |O_TRUNC | O_RDWR,mode);
	printf("Hi! My name is Chen Yang");
	return 0;}
```
5. What are some differences between `write()` and `printf()`?

write() is a system call while printf() is not.Moreover, write can write to any file we want, and printf() would only print to the output that we are currently at.

## Chapter 2

Sizing up C types and their limits, `int` and `char` arrays, and incrementing pointers

### Not all bytes are 8 bits?
1. How many bits are there in a byte?
It depends on the machine we use, but mostly 8 bits.
2. How many bytes are there in a `char`?
1 byte.
3. How many bytes the following are on your machine?
   - `int`, `double`, `float`, `long`, and `long long`
   4，8，4，4，8
### Follow the int pointer
4. On a machine with 8 byte integers:
```C
int main(){
    int data[8];
} 
```
If the address of data is `0x7fbd9d40`, then what is the address of `data+2`?
0x7fbd9d50;

5. What is `data[3]` equivalent to in C?
   - Hint: what does C convert `data[3]` to before dereferencing the address?
   data + 3

### `sizeof` character arrays, incrementing pointers
  
Remember, the type of a string constant `"abc"` is an array.

6. Why does this segfault?
```C
char *ptr = "hello";
*ptr = 'J';
```
ptr refers to a string constant, while re-writing this is forbideen by C.
7. What does `sizeof("Hello\0World")` return?
12
8. What does `strlen("Hello\0World")` return?
5
9. Give an example of X such that `sizeof(X)` is 3.
char X[] = "ab";
10. Give an example of Y such that `sizeof(Y)` might be 4 or 8 depending on the machine.
int y;

## Chapter 3

Program arguments, environment variables, and working with character arrays (strings)

### Program arguments, `argc`, `argv`
1. What are two ways to find the length of `argv`?
first: print argc in %d manner.
second: just count the number of argument and plus one.
2. What does `argv[0]` represent?
program name
### Environment Variables
3. Where are the pointers to environment variables stored (on the stack, the heap, somewhere else)?
somewhere above stack.

### String searching (strings are just char arrays)
4. On a machine where pointers are 8 bytes, and with the following code:
```C
char *ptr = "Hello";
char array[] = "Hello";
```
What are the values of `sizeof(ptr)` and `sizeof(array)`? Why?
sizeof(ptr): 4 bytes, sizeof(array): 6bytes. 
ptr is a pointer which has 4 bytes size, and array has the size that is necessary to store the whole string, which is "Hello\0", in total 6 bytes. 
### Lifetime of automatic variables

5. What data structure manages the lifetime of automatic variables?
Stack


## Chapter 4

Heap and stack memory, and working with structs

### Memory allocation using `malloc`, the heap, and time
1. If I want to use data after the lifetime of the function it was created in ends, where should I put it? How do I put it there?
you should put the data in heap, by using malloc to assign some memory space for data. eg. char* str = malloc(32)
2. What are the differences between heap and stack memory?
stack memory would be cleared up when function call is complete, and it is manipulated by system. heap memory is manual and larger than stack memory, we have to manually free the memory we allocate.
3. Are there other kinds of memory in a process?
static memory, which stores the global varaiable.
4. Fill in the blank: "In a good C program, for every malloc, there is a ___".
free
### Heap allocation gotchas
5. What is one reason `malloc` can fail?
there is no enough memory in heap
6. What are some differences between `time()` and `ctime()`?
time() returns a time_t varaiable, while ctime() return a char* which is more readable. 
7. What is wrong with this code snippet?
```C
free(ptr);
free(ptr);
```
it frees the same blocks memeory as ptr is already freed.Heap has already use that memory for it own use.
8. What is wrong with this code snippet?
```C
free(ptr);
printf("%s\n", ptr);
```
using the pointer has pointing to the memory that is already freed, that pointer is a dangling pointer.
9. How can one avoid the previous two mistakes? 
set the ptr pointing to null when we free ptr.
### `struct`, `typedef`s, and a linked list
10. Create a `struct` that represents a `Person`. Then make a `typedef`, so that `struct Person` can be replaced with a single word. A person should contain the following information: their name (a string), their age (an integer), and a list of their friends (stored as a pointer to an array of pointers to `Person`s).
```C
struct Person{
	char* name;
	int age;
	struct Person** friends;
};
typedef struct Person person_t;
```
11. Now, make two persons on the heap, "Agent Smith" and "Sonny Moore", who are 128 and 256 years old respectively and are friends with each other.
### Duplicating strings, memory allocation and deallocation of structures
```C
person_t *person1 = (person_t*)malloc(sizeof(person_t));
person_t *person2 = (person_t*)malloc(sizeof(person_t));
person1->name = "Agent Smith";
person1->age = 128;
person2->name = "Sonny Moore";
person2->age = 256;
person1->friends[0] = person2;
person2->friends[0] = person1;
free(person1);
free(person2);
```
Create functions to create and destroy a Person (Person's and their names should live on the heap).
12. `create()` should take a name and age. The name should be copied onto the heap. Use malloc to reserve sufficient memory for everyone having up to ten friends. Be sure initialize all fields (why?).
```C
person_t create(char* name, int age) {
	person_t *person = (person_t*)malloc(11*sizeof(person_t));
	person->name = name;
	person->age = age;
}
```
Because malloc just reserve the memory for the struct while doesn't initialize it.
13. `destroy()` should free up not only the memory of the person struct, but also free all of its attributes that are stored on the heap. Destroying one person should not destroy any others.
```C
void destroy(person_t *p) {
	memset(p,0,sizeof(person_t));
	free(p);
}
```

## Chapter 5 
	
Text input and output and parsing using `getchar`, `gets`, and `getline`.

### Reading characters, trouble with gets
1. What functions can be used for getting characters from `stdin` and writing them to `stdout`?
getchar() and putchar()
2. Name one issue with `gets()`.
if the input is to long, it would corrupt with other variable
### Introducing `sscanf` and friends
3. Write code that parses the string "Hello 5 World" and initializes 3 variables to "Hello", 5, and "World".
```C
char * data = "Hello 5 World";
char* first;
char* second;
int num;
int result  = sscanf(data, "%s %d %s", first,&num,second);
```
### `getline` is useful
4. What does one need to define before including `getline()`?
#define _GNU_SOURCE
5. Write a C program to print out the content of a file line-by-line using `getline()`.

```C
void get_line(char* filename) {
	char *buffer =NULL;
	size_t capacity = 0;
	ssize_t result = getline (&buffer,&capacity,filename);
	if(result > 0 && buffer[result-1]  == '\n') {
		buffer[result-1] = '\0';
	}
	free(buffer);
}
```

## C Development

These are general tips for compiling and developing using a compiler and git. Some web searches will be useful here

1. What compiler flag is used to generate a debug build?
gcc -g


2. You modify the Makefile to generate debug builds and type `make` again. Explain why this is insufficient to generate a new build.


3. Are tabs or spaces used to indent the commands after the rule in a Makefile?
Tabs

4. What does `git commit` do? What's a `sha` in the context of git?
git commit submit the staged changes to local repository.
Secure Hash Algorithm 
5. What does `git log` show you?
All the commits made to a repository
6. What does `git status` tell you and how would the contents of `.gitignore` change its output?
git status tells us state of the working directory and the staging area. The change in .gitignore would not be recognize by git.
7. What does `git push` do? Why is it not just sufficient to commit with `git commit -m 'fixed all bugs' `?
git push would push the local repository to remote repository.
"fixed all bugs" doesn't clarify what has been fixed and what needs to be donel, it is vague and useless.
8. What does a non-fast-forward error `git push` reject mean? What is the most common way of dealing with this?
This means git cannot commit your changes to the remote repository. Maybe Commit is lost or someone else is trying to push to the same branch.
Pull from the remote repository and commit and push again.
## Optional (Just for fun)
- Convert your a song lyrics into System Programming and C code and share on Ed.
- Find, in your opinion, the best and worst C code on the web and post the link to Ed.
- Write a short C program with a deliberate subtle C bug and post it on Ed to see if others can spot your bug.
- Do you have any cool/disastrous system programming bugs you've heard about? Feel free to share with your peers and the course staff on Ed.
