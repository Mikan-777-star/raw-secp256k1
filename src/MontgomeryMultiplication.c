
#include "secp256k1_utils.h"

void mul_mont(UInt256* res, const UInt256* A, const UInt256* B){
    uint64_t t[5] = {0};
    const uint64_t* a = A->limbs;
    const uint64_t* b = B->limbs;
    const uint64_t* p = SECP256K1.limbs;

    for(int i = 0; i < 4; i++){
        uint64_t carry = 0;
        for(int j = 0; j < 4; j++){
            uint128_t sum = (uint128_t)t[j]+
                            (uint128_t)a[i] * b[j] + carry;
            t[j] = (uint64_t)sum;
            carry = (uint64_t)(sum >> 64);
        }
        uint128_t sum_carry = (uint128_t)t[4] + carry;
        t[4] = (uint64_t) sum_carry;
        uint64_t m = t[0] * P_DASH;

        uint128_t carry_r = 0;
        for(int j = 0; j < 4; j++){
            uint128_t sum = (uint128_t)t[j] +
                            (uint128_t)m * p[j] +
                            carry_r;
            if(j > 0) t[j - 1] = (uint64_t)sum;
            carry_r = (uint64_t)(sum >> 64);
        }
        uint128_t sum_final = (uint128_t)t[4] + carry_r;
        t[3] = (uint64_t)sum_final;
        t[4] = (uint64_t)(sum_final >> 64);
    }
    UInt256 temp_res;
    for(int k = 0; k < 4; k++)temp_res.limbs[k] = t[k];
    *res = temp_res; 

    // 引き算した値を作る
    sub_UInt256(&temp_res, &SECP256K1);

    // 判定: (オーバーフローしている) OR (値がP以上である)
    // cmp >= 0 は 「temp >= P」 を意味する
    int cmp = cmp_UInt256(res, &SECP256K1); // 注: 引数は元の値(res)を使うこと！
    int need_sub = (t[4] != 0) | (cmp >= 0);

    // need_sub が 1 なら 0xFF...FF (-1), 0 なら 0x00...00
    uint64_t mask = -(uint64_t)need_sub;

    for(int i = 0; i < 4; i++){
        // マスクがFF(引くべき)なら 引いた後のtemp_resを採用
        // マスクが00(引かない)なら 元のresを採用
        res->limbs[i] = (mask & temp_res.limbs[i]) | (~mask & res->limbs[i]);
    }
};


// デバッグ用 mul_mont
void mul_mont_debug(UInt256* res, const UInt256* A, const UInt256* B) {
    uint64_t t[5] = {0}; 
    const uint64_t* a = A->limbs;
    const uint64_t* b = B->limbs;
    const uint64_t* p = SECP256K1.limbs;

    printf("\n--- DEBUG MUL START ---\n");
    
    for (int i = 0; i < 4; i++) {
        // 1. Multiplication Phase
        uint64_t carry = 0;
        for (int j = 0; j < 4; j++) {
            uint128_t sum = (uint128_t)t[j] + (uint128_t)a[i] * b[j] + carry;
            t[j] = (uint64_t)sum;
            carry = (uint64_t)(sum >> 64);
        }
        uint128_t sum_carry = (uint128_t)t[4] + carry;
        t[4] = (uint64_t)sum_carry;
        
        printf("i=%d (Mul) : %016lx %016lx %016lx %016lx %016lx\n", 
               i, t[4], t[3], t[2], t[1], t[0]);

        // 2. Reduction Phase
        uint64_t m = t[0] * P_DASH;
        printf("      m    : %016lx\n", m);

        uint128_t carry_r = 0;
        for (int j = 0; j < 4; j++) {
            uint128_t sum = (uint128_t)t[j] + (uint128_t)m * p[j] + carry_r;
            if (j > 0) t[j-1] = (uint64_t)sum; // Shift
            carry_r = (uint64_t)(sum >> 64);
        }
        uint128_t sum_final = (uint128_t)t[4] + carry_r;
        t[3] = (uint64_t)sum_final;
        t[4] = (uint64_t)(sum_final >> 64);

        printf("i=%d (Red) : %016lx %016lx %016lx %016lx %016lx\n", 
               i, t[4], t[3], t[2], t[1], t[0]);
    }

    UInt256 temp_res;
    for(int k=0; k<4; k++) temp_res.limbs[k] = t[k];
    
    // 条件付き減算のチェック
    if (t[4] != 0 || cmp_UInt256(&temp_res, &SECP256K1) >= 0) {
        printf("Final Subtraction executed.\n");
        sub_UInt256(&temp_res, &SECP256K1);
    }
    *res = temp_res;
    printf("--- DEBUG MUL END ---\n\n");
}

void to_mont(UInt256* res, const UInt256* A){
    UInt256 RR;
    legacy_mod_mul(&RR, &SECP256K1_R, &SECP256K1_R);
    mul_mont(res, A, &RR);
}

void from_mont(UInt256* res, const UInt256* montA){
    UInt256 ONE= {{1,0,0,0}};
    mul_mont(res, montA, &ONE);
}