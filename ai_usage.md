# AI Usage Documentation - Phase 1

## 1. Tool Used
Google Gemini

## 2. Prompts Given
**Prompt 1 (For parse_condition):**
> I am writing a C program and need help writing a parsing function. Please generate a function with this exact signature: `int parse_condition(const char *input, char *field, char *op, char *value);`
> Requirements: It must take an `input` string formatted exactly as "field:operator:value". It needs to split that string and copy the three parts into the `field`, `op`, and `value` buffers. It should return 1 if successful, 0 if invalid. Please use standard C libraries but do not modify the original `input` string since it is a `const char *`

**Prompt 2 (For match_condition):**
> I need a C function to evaluate if a record matches a specific condition. Here is my struct:
> (Included the full Report struct definition)
> Please generate a function with this exact signature: `int match_condition(Report *r, const char *field, const char *op, const char *value);`
> Requirements: The function should return 1 if the record `r` satisfies the condition, and 0 otherwise. Support fields: `severity` (int), `category` (string), `inspector` (string), and `timestamp` (time_t). Support operators: `==`, `!=`, `<`, `<=`, `>`, `>=`. Convert the `value` string appropriately before comparison

## 3. What Was Generated
**Function 1 (parse_condition):**
The AI generated a function using `sscanf` instead of `strtok`. It used the format specifier `"%[^:]:%[^:]:%s"` to extract the string parts without modifying the original input.

**Function 2 (match_condition):**
The AI generated a large `if/else` block handling the numeric and string comparisons. However, it made several errors:
* It renamed my struct to `LogRecord` instead of `Report`.
* It passed the struct by value (`LogRecord record`) instead of by pointer (`Report *r`) as the function signature requires.
* It completely forgot to include the logic for the `inspector` and `timestamp` fields, only implementing `severity` and `category`.

## 4. What I Changed and Why
**For parse_condition:**
I kept the AI's `sscanf` logic exactly as generated. I had considered using `strtok` initially, but `strtok` injects null terminators into the target string. Since `input` is passed as a `const char *`, using `strtok` would cause a segmentation fault. The `sscanf` approach is safer and cleaner.

**For match_condition:**
I had to heavily modify the generated code to make it usable:
1. I changed the struct name back to `Report`.
2. I fixed the function signature to take a pointer (`Report *r`) and updated all variable accesses inside the function to use the `->` operator instead of the `.` operator.
3. I manually wrote the missing logic for the `timestamp` field (using `atol` to convert the string to a `long` for the `time_t` comparison) and the `inspector` field (using `strcmp`).

## 5. What I Learned
1. **Technical:** `sscanf` is a highly effective tool for parsing rigidly formatted strings (`const char *`), unlike unsafe alternatives like `strtok`.
2. **AI Limitations:** While AI is good at generating repetitive, basic code (like the `if/else` for the `<, >, ==` operators), it easily loses track of specific constraints. Even though I provided the exact function signature and struct definition, it hallucinated a different struct name and ignored half of the required fields. AI code must be reviewd line by line.