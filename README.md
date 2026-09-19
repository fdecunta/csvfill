# csvfill

Fill empty cells in a CSV file with values from another CSV file, matching rows by an ID column. Existing values are never overwritten. Only empty cells ("", NA, NaN) are replaced.

## Usage

```sh
usage: csvfill [-1 col1] [-2 col2] [-d delim] file1 [file2]

Fill empty cells ("", NA, NaN) in file1 with values from file2,
matching rows by their ID column, and print the result.
Existing values are never overwritten.
If file2 is omitted, it is read from standard input.

  -1 col1    ID column in file1, 1-based (default: 1)
  -2 col2    ID column in file2, 1-based (default: 1)
  -d delim   field delimiter (default: ,)
  -h         show this help
```

## Bugs

- Can't handle embedded new lines in fields. 
