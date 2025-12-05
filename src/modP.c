
#include "secp256k1_utils.h"
// 安全な mod_sub P ロジックの概念図
//
void conditional_sub_p(UInt256* val, const UInt256* P) {
    // 1. とりあえず引いてみる
    UInt256 temp = *val;
    int borrow = sub_UInt256(&temp, P); 
    
    // 2. 引いてボローが出た（つまり val < P だった）なら、引いちゃダメだった
    //    引いてボローが出ない（つまり val >= P だった）なら、引いた結果を採用
    
    // borrow が 1 なら「引く前の val」を採用
    // borrow が 0 なら「引いた後の temp」を採用
    
    uint64_t mask = -borrow; // borrow=1 -> All 1s, borrow=0 -> All 0s (※intが64bit環境か要確認)
    
    // 全桁に対してマスク処理で選択 (CMOV的な動作)
    for(int i=0; i<4; i++) {
        // borrow(ダメだった)なら: (val & 111) | (temp & 000) -> val
        // no borrow(OK)    なら: (val & 000) | (temp & 111) -> temp
        // ※ maskの論理に注意。今回は borrow=1 のとき元の値を残したい
        val->limbs[i] = (val->limbs[i] & mask) | (temp.limbs[i] & ~mask);
    }
}

/**
 * res: 結果
 * A: 引数1
 * B: 引数2
 * P: Mod
 */
void mod_add(UInt256* result, const UInt256* A, const UInt256* B){
    UInt256 res = *A;
    uint64_t carry = add_UInt256(&res, B);
    UInt256 tmp = res;
    add_UInt256(&tmp, &SECP256K1_CORRECTION);
    uint64_t mask = -carry;
    for(int i = 0; i < 4; i++){
        res.limbs[i] = (~mask & res.limbs[i]) | (mask & tmp.limbs[i]);
    }
    conditional_sub_p(&res, &SECP256K1);
    *result = res;
}

void mod_sub(UInt256* result, const UInt256* A, const UInt256* B){
    UInt256 res = *A, tmp;
    uint64_t borrow = sub_UInt256(&res, B);
    tmp = res;
    add_UInt256(&tmp, &SECP256K1);
    uint64_t mask = -borrow;
    for(int i = 0; i < 4; i++){
        result->limbs[i] = (mask & tmp.limbs[i]) | (~mask & res.limbs[i]);
    }
    //conditional_sub_p(result, &SECP256K1);
}
//モンゴメリーする
void mod_mul(UInt256* res, const UInt256* A, const UInt256* B){
    mul_mont(res, A, B);
}

void legacy_mod_mul(UInt256* res, const UInt256* A, const UInt256* B){
    UInt256 sum, term = *A, ZERO;
    zero_UInt256(&ZERO);
    zero_UInt256(&sum);

    for(int i = 0; i < 256; i++){
        int bit = get_bit_UInt256(B, i);
        
        // ビットが1なら、現在の term (A * 2^i) を足す
        // ビットが0なら、0を足す
        // 注意: あなたの元のコードだと sum が第一引数と第二引数で重複して倍加していた。
        // ここでは sum = sum + (bit ? term : 0) という形にする。
        mod_add(&sum, &sum, bit ? &term : &ZERO);

        // 次のループのために term を2倍にする (A * 2^(i+1))
        mod_add(&term, &term, &term);
    }
    *res = sum; 
}

void mod_pow(UInt256* res,  const UInt256* base, const UInt256* exp){
    UInt256 acc = SECP256K1_R, term = *base, tmp;
    for(int i = 0; i < 256; i++){
        int bit = get_bit_UInt256(exp, i);
        uint64_t mask = -bit;
        mul_mont(&tmp, &acc, &term);
        for(int j = 0; j < 4; j++)
            acc.limbs[j] = (mask & tmp.limbs[j]) | (~mask & acc.limbs[j]);
        mul_mont(&term, &term, &term);
    }
    *res = acc;
}

// a^exp mod p を計算する
void legacy_mod_pow(UInt256* res, const UInt256* base, const UInt256* exp){
    UInt256 acc, term = *base, ONE;
    zero_UInt256(&ONE);
    ONE.limbs[0] = 0x1;
    
    // べき乗の初期値は 1 
    acc = ONE; 

    for(int i = 0; i < 256; i++){
        int bit = get_bit_UInt256(exp, i);
        
        // ビットが1なら、現在の term (base^(2^i)) を掛ける
        // ビットが0なら、1を掛ける (値を変えない)
        mod_mul(&acc, &acc, bit ? &term : &ONE);

        // 次のループのために term を2乗する
        mod_mul(&term, &term, &term);
    }
    *res = acc;
}

// a^(-1) mod p を計算する (mod_powを利用)
// 内部で p-2 を計算して mod_pow に渡すこと
void mod_inv(UInt256* res, const UInt256* A){
    mod_pow(res, A, &secp256k1_2);
}