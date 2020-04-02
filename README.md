# JSOON - JavaScript Ordered Object Notation
A JSON library for structured data

For the cases when you just want to read/write C structs to/from JSON with 0 dynamic allocations.

C structs are not dictionaries.  This makes working with JSON more painful than it ought to be.  If the order of struct members is enforced in the JSON data, then it is easy to read them without storing any intermediate state like a hash map or token list.

Enforcing member order does break general JSON compatibility. However, it improves a few things:

- Avoids calls to strcmp() to check which struct member to assign the data too (assuming no intermediate hash map).
- Allows 'size' fields to precede arrays so you can reserve the exact amount of memory needed for the array.

For data that is primarily dictated by one piece of software, these concerns outweight the ability to read unordered JSON.  Any JSON reader will still be able to read the data this library outputs.  Furthermore, it does not inhibit the perks of a plain-text format, namely the ability to inspect & modify it by hand.

# Features

- C99
- no allocations
- write to FILE stream, memory buffers, or custom callbacks
- straight-forward error checking, with easy-to-implement error 'stack traces'
- examples

# Known issues

- null values are not handled
- numbers with exponents are not handled
- number parsing is simplistic & will allow non-digit characters in places it shouldn't
