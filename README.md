# bool-search

A command line tool that searches things with boolean expressions. Primarily inspired by a section in the [De Morgan's laws](https://en.wikipedia.org/wiki/De_Morgan%27s_laws) Wikipedia article.

## Quick Start
```bash
mkdir build
cd build
cmake -DBOOL_SEARCH_COMPILE_TESTS=ON -DCMAKE_BUILD_TYPE=Release ..
make
ctest --output-on-failure # run tests
sudo make install
```


## Usage
```
bool-search - A command line tool that searches things with boolean expressions.

Usage: ./bin/bool-search  [-rh] EXPR [FILE]...
  -r, --recursive           recusivly search given directories
  -h, --help                display this help and exit
  EXPR                      The expression that is used to search
  FILE                      The file or directory (if has -r option) to search from

When FILE is absent, read in input from standard input. Read from ".", if -r option is specified.

```

## Examples
```bash
 # searches a single file
bool-search "not ( cats or dogs ) and rabbit" sample-text/dir1/random111.txt

 # searches multiple file
bool-search "not ( cats or not camels ) and dogs" sample-text/dir1/random45.txt sample-text/random444.txt sample-text/random650.txt

# searches all files in a directory, skipping given directories
bool-search "( not cats ) and ( not dogs ) or giraffes" sample-text/*

# searches given directory recursibly
bool-search -r "not ( not cats or ( dogs or camels ) ) or shark" sample-text/

# searches via stdin
printf "%s\n%s\n%s\n%s\n" cat dog shark cat | bool-search "cat or dog"
```

## Excerpt from Wikipedia

### Text searching
De Morgan's laws commonly apply to text searching using Boolean operators AND, OR, and NOT. Consider a set of documents containing the words "cats" and "dogs". De Morgan's laws hold that these two searches will return the same set of documents:  

**Search A**: NOT (cats OR dogs)  
**Search B**: (NOT cats) AND (NOT dogs)  

The corpus of documents containing "cats" or "dogs" can be represented by four documents:  

**Document 1**: Contains only the word "cats".  
**Document 2**: Contains only "dogs".  
**Document 3**: Contains both "cats" and "dogs".  
**Document 4**: Contains neither "cats" nor "dogs".  

To evaluate **Search A**, clearly the search "(cats OR dogs)" will hit on **Documents** **1**, **2**, and **3**. So the negation of that search (which is **Search A**) will hit everything else, which is **Document 4**.  

Evaluating **Search B**, the search "(NOT cats)" will hit on documents that do not contain "cats", which is **Documents 2** and **4**. Similarly the search "(NOT dogs)" will hit on **Documents** 1 and **4**. Applying the AND operator to these two searches (which is **Search B**) will hit on the documents that are common to these two searches, which is **Document 4**.  

A similar evaluation can be applied to show that the following two searches will both return **Documents 1**, **2**, and **4**:  

**Search C**: NOT (cats AND dogs),  
**Search D**: (NOT cats) OR (NOT dogs).  

### Informal proof of De Morgan's Law
De Morgan's Law can be informally proven by using the `bool-search` command along with the `find` command. There are some sample text files in the `test/sample-text` directory. We will use that for the proof.
```bash
bool-search -r "not ( cats or dogs )" sample-text/ > test1.txt
bool-search -r "not cats and not dogs" sample-text/ > test2.txt

# Take the diff of both test files to prove that they produce the same output!
diff test1.txt test2.txt
```

## Boolean Expression
The command line takes an EXPR parameter. This is where you define the boolean expression that will be used to search files. There are six main keywords that are used to define expressions: 'and', 'or', 'not', '(', ')', and an identifier. In order for parentheses to register, they must be surrounded by spaces. The precendence of each operator goes like this: '()' first, 'not' second, 'and' third, and 'or' forth. The expression is evaluated in that order. So, "cat or dog and pig", the 'and' operation is done first. In, "cat and not dog or fish", the 'not' operation is done first, then the 'and', and finally the 'or'. The identifier is a search term and is defined to be any printable characters. If all you want an open parethesis to be an identifier, then you can escape it by doing the following: "\\(". An example would be: "main and not \\(".



### TODO
- add test for failed parsing
- add install instructions

