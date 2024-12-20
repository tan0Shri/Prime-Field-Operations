#include"utilities.h"
#include<string.h>

// Initialize base as g = 2
uint32_t g[10] = {0x2, 0};

//  Function to check if given num in base 29 is zero or not
int IsZero(uint32_t* num, int length) {
    for (int i = 0; i < length; i++) {
        if (num[i] != 0) {
            return 0; // Return 0 if any digit is non-zero
        }
    }
    return 1; // Return 1 if all digits are zero
}

// Function to get the bit length of the exponent
int BitLength(uint32_t* exp) {
    for (int i = 8; i >= 0; i--) {  // Traverse from the most significant chunk
        if (exp[i] != 0) {
            for (int j = 28; j >= 0; j--) {  // Check each bit in the 29-bit chunk
                if ((exp[i] >> j) & 1) {
                    return i * 29 + j + 1;  // Return the bit position + 1 as the length
                }
            }
        }
    }
    return 0;  // If exp is zero, bit length is 0
}

// Function check 2 <= exp <= p-2, i.e, 1 < exp < p-1
int IsCompatible(uint32_t* exp){
    uint32_t one[10] = {0x1,0};
    uint32_t p_1[10] = {0};
    p_1[0] = p[0] - 1;
    for(int i = 1; i < 9; i++){
        p_1[i] = p[i];
    }
    int flag = (IsGreater(exp, one) && IsGreater(p_1, exp));
    return flag;
}

// Function to perform modular exponentiation in a prime field (left to right square and multiply)
void FieldExp_left2right(uint32_t* base, uint32_t* exp, uint32_t* result) { 
    // Initialize result to 1 in packed base-29 format
    result[0] = 0x1;

    // Get the bit length of the exponent
    int expBitLength = BitLength(exp);

    //Loop through each bit of the exponent from the most significant to the least
    for (int i = expBitLength - 1; i >= 0; i--) {
        // Square step
        FieldMult(result, result, result);  // tempResult = tempResult^2 (mod p)
    
        // Multiply step if the i-th bit of exp is set
        ((exp[i / 29] >> (i % 29)) & 1)? FieldMult(result, base, result) : NULL;
    }
}


// Function to perform modular exponentiation in a prime field (right to left square and multiply)
void FieldExp_right2left(uint32_t* base, uint32_t* exp, uint32_t* result) { 
    //copy base from g
    uint32_t b[10] = {0};
    for(int i = 0; i < 9; i++){
        b[i] = base[i];
    }

    // Initialize result to 1 in packed base-29 format
    result[0] = 0x1;

    // Get the bit length of the exponent
    int expBitLength = BitLength(exp);

    //Loop through each bit of the exponent from the least significant to the most
    for (int i = 0; i < expBitLength; i++) 
    {
        // Multiply step if the i-th bit of exp is set 
        ((exp[i / 29] >> (i % 29)) & 1)? FieldMult(result, b, result) : NULL;

        // Square step
        FieldMult(b, b, b);
    }
}

// Function to perform modular exponentiation in a prime field (using Montgomery Ladder)
void FieldExp_Montgomery(uint32_t* base, uint32_t* exp, uint32_t* result) { 
    if (IsZero(exp, 10)) {
    result[0] = 1; // Initialize result as 1 (mod p)
    for (int i = 1; i < 9; i++) {
        result[i] = 0;
    }
    return;
    }
    // Initialize S to 1 and R to base in packed base-29 format
    uint32_t S[10] = {0};
    // S represents the current result, initialized to base
    for(int i = 0; i < 9; i++){
        S[i] = base[i];
    }
    uint32_t R[10] = {0};       // R represents the "next" result 
    FieldMult(S, S, R);   // initialized to the base^2

    // Get the bit length of the exponent
    int expBitLength = BitLength(exp);
    // Loop through each bit of the exponent from the most significant to the least
    for (int i = expBitLength - 2; i >= 0; i--) 
    {
        // Determine the bit value of exp at position i
        int bit = (exp[i / 29] >> (i % 29)) & 1;

        if (bit == 1) {
            // If bit is 1: S = S * R (mod p), R = R * R (mod p)
            FieldMult(S, R, S); // Multiply S by R
            FieldMult(R, R, R); // Square R
        } else {
            // If bit is 0: R = S * R (mod p), S = S * S (mod p)
            FieldMult(R, S, R); // Multiply R by S
            FieldMult(S, S, S); // Square S
        }
    }
    // Copy S to result
    for(int i = 0; i < 9; i++){
        result[i] = S[i];
    }
}

// Without BRANCHING: Function to perform modular exponentiation in a prime field (using Montgomery Ladder)
void FieldExp_Montgomery_noBranching(uint32_t* base, uint32_t* exp, uint32_t* result) { 
    if (IsZero(exp, 10)) {
    result[0] = 1; // Initialize result as 1 (mod p)
    for (int i = 1; i < 9; i++) {
        result[i] = 0;
    }
    return;
    }
    uint32_t S[10] = {0};
    // S is initialized to base
    for(int i = 0; i < 9; i++){
        S[i] = base[i];
    }
    uint32_t R[10] = {0}; // R initialized to base^2
    FieldMult(S, S, R);

    int expBitLength = BitLength(exp);

    for (int i = expBitLength - 2; i >= 0; i--) {
        // Get the bit value of exp at position i
        int bit = (exp[i / 29] >> (i % 29)) & 1;

        // Temporary variables to hold potential new values for S and R
        uint32_t tempSR[10] = {0};
        uint32_t tempR[10] = {0};

        if (bit == 0) {
                // Swap S and R by copying elements
                for (int j = 0; j < 10; j++) {
                    uint32_t temp = S[j];
                    S[j] = R[j];
                    R[j] = temp;
                }
            }

        // Compute S * R and R * R
        FieldMult(S, R, tempSR);  // tempSR = S * R
        FieldMult(R, R, tempR);  // tempR = R * R

        // Select the new values of S and R without branching
        for (int j = 0; j < 10; j++) {
            S[j] = (bit * tempSR[j]) + ((1 - bit) * tempR[j]);
            R[j] = (bit * tempR[j]) + ((1 - bit) * tempSR[j]);
        }
    }
    // Copy S to result
    for(int i = 0; i < 9; i++){
        result[i] = S[i];
    }
}


