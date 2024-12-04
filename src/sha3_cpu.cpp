#include "./headers/sha3_cpu.h"

#define KECCAKF_ROUNDS 24
#define ROTL64(x, y) (((x) << (y)) | ((x) >> (64 - (y))))

// Compression function - To update the state with a specific number of rounds
void keccakf(uint64_t *state){
    // constants
    const uint64_t keccakf_rndc[24] = {
        0x0000000000000001, 0x0000000000008082, 0x800000000000808a,
        0x8000000080008000, 0x000000000000808b, 0x0000000080000001,
        0x8000000080008081, 0x8000000000008009, 0x000000000000008a,
        0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
        0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
        0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
        0x000000000000800a, 0x800000008000000a, 0x8000000080008081,
        0x8000000000008080, 0x0000000080000001, 0x8000000080008008
    };
    const uint8_t keccakf_rotc[24] = {
        1,  3,  6,  10, 15, 21, 28, 36, 45, 55, 2,  14,
        27, 41, 56, 8,  25, 43, 62, 18, 39, 61, 20, 44
    };
    const uint8_t keccakf_piln[24] = {
        10, 7,  11, 17, 18, 3, 5,  16, 8,  21, 24, 4,
        15, 23, 19, 13, 12, 2, 20, 14, 22, 9,  6,  1
    };

    // variables
    uint64_t temp, intermediate[5];
#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
    uint8_t *endian_little;

    // Endian conversion. If your machine is little-endian, this part becomes redundant
    #pragma GCC unroll 25
    for(int i=0;i<25;i++){
        endian_little = (uint8_t *) &state[i];
        state[i] = ((uint64_t)  endian_little[0])          |
                (((uint64_t) endian_little[1]) <<  8)   |
                (((uint64_t) endian_little[2]) << 16)   | 
                (((uint64_t) endian_little[3]) << 24)   |
                (((uint64_t) endian_little[4]) << 32)   |
                (((uint64_t) endian_little[5]) << 40)   |
                (((uint64_t) endian_little[6]) << 48)   |
                (((uint64_t) endian_little[7]) << 56);
    }
#endif
    // cryptographic iterations
    for(int r=0;r<KECCAKF_ROUNDS;r++){
        // theta block
        #pragma GCC unroll 5
        for(int i=0;i<5;i++)
            intermediate[i] = state[i] ^ state[i+5] ^ state[i+10] ^ state[i+15] ^ state[i+20];

        #pragma GCC unroll 5
        for (int i=0;i<5;i++){
            temp = intermediate[(i+4)%5] ^ ROTL64(intermediate[(i+1)%5], 1);
            #pragma GCC unroll 5
            for (int j=0;j<25;j+=5)
                state[j+i] ^= temp;
        }

        // combined rho & pi block
        int k;
        temp = state[1];

        #pragma GCC unroll 24
        for(int i=0;i<24;i++){
            k = keccakf_piln[i];
            intermediate[0] = state[k];
            state[k] = ROTL64(temp, keccakf_rotc[i]);
            temp = intermediate[0];
        }

        //  chi block
        #pragma GCC unroll 5
        for (int j=0;j<25;j+=5){
            #pragma GCC unroll 5
            for(int i=0;i<5;i++)
                intermediate[i] = state[j+i];
            
            #pragma GCC unroll 5
            for(int i=0;i<5;i++)
                state[j+i] ^= (~intermediate[(i+1)%5]) & intermediate[(i+2)%5];
        }

        //  iota block
        state[0] ^= keccakf_rndc[r];
    }

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
    // Endian conversion - again
    #pragma GCC unroll 25
    for(int i=0;i<25;i++){
        endian_little = (uint8_t *) &state[i];
        temp = state[i];
        endian_little[0] = temp & 0xFF;
        endian_little[1] = (temp >>  8) & 0xFF;
        endian_little[2] = (temp >> 16) & 0xFF;
        endian_little[3] = (temp >> 24) & 0xFF;
        endian_little[4] = (temp >> 32) & 0xFF;
        endian_little[5] = (temp >> 40) & 0xFF;
        endian_little[6] = (temp >> 48) & 0xFF;
        endian_little[7] = (temp >> 56) & 0xFF;
    }
#endif
}

// Initialize the context for SHA3 - digest_size is the the hash length in bytes
int SHA3_init(sha3_ctx_t *c, int digest_size){
    for(int i=0;i<25;i++){
        c->state.q[i] = 0;
    }
    c->digest_size = digest_size;
    c->rsiz = 200 - 2 * digest_size;
    c->pt = 0;

    return 1;
}

// update state with more data
int SHA3_update(sha3_ctx_t *c, const void *data, size_t data_len){
    int j = c->pt;
    for(size_t i=0;i<data_len;i++){
        c->state.b[j++] ^= ((const uint8_t *) data)[i];
        if(j >= c->rsiz){
            keccakf(c->state.q);
            j = 0;
        }
    }
    c->pt = j;

    return 1;
}

// finalize and output a hash - digest goes to md
int SHA3_final(void *md, sha3_ctx_t *c){
    c->state.b[c->pt] ^= 0x06;
    c->state.b[c->rsiz - 1] ^= 0x80;
    keccakf(c->state.q);

    for(int i=0;i<c->digest_size;i++){
        ((uint8_t *) md)[i] = c->state.b[i];
    }
    return 1;
}

// A function to specifically target 16 byte data-types (i.e. 128-bit UUID values) and hash them in one go
void SHA3_16B_cpu(void *digest, const void *bytearr, const int digest_size, const int object_size, size_t num_objects){
    #pragma omp parallel for
    for(size_t _i=0; _i<num_objects; _i++){
        uint8_t b[200] = {0};
        for(int i=0;i<object_size;i++){
            b[i] = ((uint8_t *)bytearr)[_i*object_size + i];
        }
        int rsiz = 200 - 2 * digest_size;
        b[object_size] ^= 0x06;
        b[rsiz - 1] ^= 0x80;
        keccakf((uint64_t *)b);
        for(int i=0;i<digest_size;i++){
            ((uint8_t *)digest)[_i*digest_size + i] = b[i];
        }
    }
}

void SHA3_16B_validation_v1(void *digest, const void *bytearr, const int digest_size, const int object_size, size_t num_objects){
    #pragma omp parallel for
    for(size_t _i=0; _i<num_objects; _i++){
        sha3_ctx_t sha3;
        SHA3_init(&sha3, 64);
        SHA3_update(&sha3, &(((uint8_t *)bytearr)[_i*object_size]), object_size);
        SHA3_final(&(((uint8_t *)digest)[_i*digest_size]), &sha3);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
