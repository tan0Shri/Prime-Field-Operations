#include<stdio.h>
#include "utilities.h"

int main()
{
    //Taking prime related inputs for defining the Field
    PrimeInputs();

    // Opening input file
    FILE *in = fopen("input.txt", "r");
    if (in == NULL) {
        perror("Error opening file");
        return 1;
    }

    printf("Default Argument for Exponentiation is fixed");

    //Exponentiation of primitive element g=2
    uint8_t exponent[32];
    for (int i = 31; i >= 0; i--){
        fscanf(in, "%02hhx",&exponent[i]);
    }
    printf("\nGiven exponent for exponentiation:\n");
    printBytes(exponent, 32);

    // Converting the exponent to base 29
    uint32_t exp[10] = {0};
    ToBase29(exponent, exp, 32);

    //computation of g^exponent (left 2 right)
    uint8_t res1[32] = {0};
    FieldExp_left2right(g, exp, res1);
    printf("\nRequired result (using left-to-right square and multiply algorithm):\n");
    printBytes(res1, 32);

    //computation of g^exponent (right 2 left)
    uint8_t res2[32] = {0};
    FieldExp_right2left(g, exp, res2);
    printf("\nRequired result (using right-to_left square and multiply algorithm):\n");
    printBytes(res2, 32);

    //computation of g^exponent (Montgomery Scalar multiplication)
    uint8_t res3[32] = {0};
    FieldExp_Montgomery(g, exp, res3);
    printf("\nRequired result (using montgomery scalar multiplication):\n");
    printBytes(res3, 32);

    //computation of g^exponent (Montgomery Scalar multiplication without BRANCHING)
    uint8_t res4[32] = {0};
    FieldExp_Montgomery_noBranching(g, exp, res4);
    printf("\nRequired result (using montgomery scalar multiplication WITHOUT BRANCHING):\n");
    printBytes(res4, 32);

    fclose(in);
    return 0;
}

