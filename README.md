# ArithmeticCoding ![build status](https://travis-ci.org/nibrunie/PhotonHash.svg?branch=master)
simple implementation to learn basis of arithmetic coding

# A word on arithmetic coding
Arithmetic coding can be seen as a conversion. It converts a byte array to a number. Rather than converting each byte to a fixed-size digit (such as in Huffman coding) it converts each byte to a digit whose size is proportionnal to -log(probability(byte value)) which makes it a very efficient entropy coder with very good compression ratios (optimal under certain hypothesises).

Arithmetic coding relies on a probability table which may be static or dynamic. This last technic is called adaptative arithmetic coding. It allows a coder to follow the source variation more closely and thus improve compression ratios.
  Obviously, updating the table will slow-down the encoding process. This may be alleviated by post-poning the cumul probabilities table update to less frequent intervals.

# Implementation details
The first implementation has been inspired (a lot) by the excellent Introduction to arithmetic coding-theory and practive [1].

To try this code, just do *make test*

## Encode/Decode with update ##

In those functions, rather than using a statically initialized probability table, the coder/decoded uses a dynamic table which is updated according to the occurence count of symbols encountered during encoding/decoding. The encode and decode function MUST be called with identical update parameters (*update_range* and *range_clear*) to be functionnal.


# References
[1] Said, Amir. "Introduction to arithmetic coding-theory and practice." Hewlett Packard Laboratories Report (2004).
