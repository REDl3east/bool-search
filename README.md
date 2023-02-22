# ez-search

A simple cli search tool that was inspired by a section in the [De Morgan's laws](https://en.wikipedia.org/wiki/De_Morgan%27s_laws) Wikipedia article.

## Quick Start
```bash
mkdir build
cd build
cmake ..
make
```

### Excerpt from Wikipedia

```
Text searching
De Morgan's laws commonly apply to text searching using Boolean operators AND, OR, and NOT. Consider a set of documents containing the words "cats" and "dogs". De Morgan's laws hold that these two searches will return the same set of documents:

Search A: NOT (cats OR dogs)
Search B: (NOT cats) AND (NOT dogs)
The corpus of documents containing "cats" or "dogs" can be represented by four documents:

Document 1: Contains only the word "cats".
Document 2: Contains only "dogs".
Document 3: Contains both "cats" and "dogs".
Document 4: Contains neither "cats" nor "dogs".
To evaluate Search A, clearly the search "(cats OR dogs)" will hit on Documents 1, 2, and 3. So the negation of that search (which is Search A) will hit everything else, which is Document 4.

Evaluating Search B, the search "(NOT cats)" will hit on documents that do not contain "cats", which is Documents 2 and 4. Similarly the search "(NOT dogs)" will hit on Documents 1 and 4. Applying the AND operator to these two searches (which is Search B) will hit on the documents that are common to these two searches, which is Document 4.

A similar evaluation can be applied to show that the following two searches will both return Documents 1, 2, and 4:

Search C: NOT (cats AND dogs),
Search D: (NOT cats) OR (NOT dogs).
```

## Command Line
```bash
ez-search INPUT SEARCH
```

## Running Tests
```bash
ctest --output-on-failure
```

### TODO
- add test for failed parsing