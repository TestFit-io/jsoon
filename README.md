# JSOON - JavaScript Ordered Object Notation
A JSON library for structured data

For the cases when you just want to read/write C structs to/from JSON with 0 dynamic allocations.

C structs are not dictionaries.  This makes working with JSON more painful than it ought to be.  If the order of struct members is enforced in the JSON data, then it is easy to read them without storing any intermediate state like a hash map or token list.

Enforcing member order does break general JSON compatibility. However, it improves a few things:

- Avoids calls to strcmp() to check which struct member to assign the data too (assuming no intermediate hash map).
- Allows 'size' fields to precede arrays so you can reserve the exact amount of memory needed for the array.
