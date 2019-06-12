pragma solidity ^0.5.0;

/*
 * Reference papers:
 *
 * \[AGRRT16]:
 * "MiMC: Efficient Encryption and Cryptographic Hashing with Minimal Multiplicative Complexity",
 * Martin Albrecht, Lorenzo Grassi, Christian Rechberger, Arnab Roy, and Tyge Tiessen,
 * ASIACRYPT 2016,
 * <https://eprint.iacr.org/2016/492.pdf>
 *
 * "One-way compression function"
 * Section: "Miyaguchi–Preneel"
 * <https://en.wikipedia.org/wiki/One-way_compression_function#Miyaguchi%E2%80%93Preneel>
**/

contract MiMC7 {
  function MiMCHash(bytes32[] memory x, bytes32 iv) public pure returns (bytes32) {
    uint r = 21888242871839275222246405745257275088548364400416034343698204186575808495617;
    bytes32 seed = keccak256("mimc");
    bytes32 message; // Input message processed
    bytes32 previousH = iv; // Previous hash (ie: round key)

    for( uint i = 0; i < x.length; i++ ) {
      message = x[i];

      assembly {
        // Load the "free memory pointer" to point to the next free memory address
        let roundConstant := mload(0x40)
        // 0x40 (free memory pointer) now becomes the next memory location
        mstore(0x40, add(roundConstant, 32))
        // We store the seed in the memory word/address pointed by roundConstant
        mstore(roundConstant, seed)

        // Round function f(message) = (message + previousH + roundConstant)^d
        // d (= exponent) = 7; #rounds = 91
        //
        // Note on the exponent: gcd(7, r - 1) = 1 which confirms that the monomial x^7 is a permutation in Fr
        // See: Proposition 1, Section 4 and section 5; https://eprint.iacr.org/2016/492.pdf
        let outPermutation := message
        for {let j := 0} slt(j, 91) {j := add(j,1)} {
          // roundConstant = H(roundConstant); we derive the (round) constants by iterative hash on the seed
          mstore(roundConstant, keccak256(roundConstant, 32))
          // a = outPermutation + roundConstant + previousH mod r
          let a :=  addmod(addmod(outPermutation, mload(roundConstant), r), previousH, r)
          // b = a^2 mod r
          let b := mulmod(a, a, r)
          // outPermutation = a^7 mod r (x^7 is the permutation polynomial used)
          outPermutation :=  mulmod(mulmod(mulmod(b, b, r), b, r), a, r)
        }

        // Merged last round of the permutation with Myjaguchi-Prenell step
        // Compute H_i from H_{i-1} to generate the round key for the next entry in the input slice x
        // In MiMC the output of the last round is mixed with the round key: This corresponds to the `outMiMCCipher = addmod(outPermutation, previousH, r)`
        // And, the Myjaguchi-Prenell OWCF is ran: `addmod(addmod(outMiMCCipher, message, r), previousH, r)`
        previousH := addmod(addmod(addmod(outPermutation, previousH, r), message, r), previousH, r)
      }
    }

    // When we exit the `for` loop, the value of the previousH is the last H_i produced
    // by the hash function. Thus, this is the final hash that we return
    return previousH
  }
}
