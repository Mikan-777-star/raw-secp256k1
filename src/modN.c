
#include "secp256k1_utils.h"

// 修正案: scalar_add
void scalar_add(UInt256* res, const UInt256* A, const UInt256* B){
    UInt256 tmp = *A;
    uint64_t carry = add_UInt256(&tmp, B); // A + B
    
    // 補正用: tmp - N を計算してみる
    UInt256 sub_n = tmp;
    uint64_t borrow = sub_UInt256(&sub_n, &SECP256K1_N);
    
    // 選択ロジック:
    // 1. キャリーが出た (carry=1) -> 256bit溢れ確定 -> Nを引く必要がある
    // 2. 引いてもボローが出ない (borrow=0) -> 結果がN以上だった -> Nを引く必要がある
    // つまり、(carry=1 OR borrow=0) なら sub_n を採用。それ以外なら tmp を採用。
    
    // mask生成: 引く必要がある(1) なら 0xFF..FF, 不需要(0) なら 0x00..00
    // (borrow == 0) は (1 - borrow) と書ける (borrowは0か1なので)
    uint64_t condition = carry | (1 - borrow);
    uint64_t mask = -condition; // 0x00...00 or 0xFF...FF

    for(int i = 0; i < 4; i++){
        res->limbs[i] = (mask & sub_n.limbs[i]) | (~mask & tmp.limbs[i]);
    }
}

void scalar_sub(UInt256* res, const UInt256* A, const UInt256* B){
    *res = *A;
    uint64_t borrow = sub_UInt256(res, B);
    UInt256 tmp = *res;
    add_UInt256(&tmp, &SECP256K1_N);
    uint64_t mask = -borrow;
    for(int i = 0; i < 4; i++){
        res->limbs[i] = (mask & tmp.limbs[i]) | (~mask & res->limbs[i]);
    }
}

void scalar_mul(UInt256* res, const UInt256* A, const UInt256* B){
    UInt256 sum, term = *A, ZERO;
    zero_UInt256(&ZERO);
    zero_UInt256(&sum);

    for(int i = 0; i < 256; i++){
        int bit = get_bit_UInt256(B, i);
        
        // ビットが1なら、現在の term (A * 2^i) を足す
        // ビットが0なら、0を足す
        // 注意: あなたの元のコードだと sum が第一引数と第二引数で重複して倍加していた。
        // ここでは sum = sum + (bit ? term : 0) という形にする。
        scalar_add(&sum, &sum, bit ? &term : &ZERO);

        // 次のループのために term を2倍にする (A * 2^(i+1))
        scalar_add(&term, &term, &term);
    }
    *res = sum; 
}


void scalar_pow(UInt256* res, const UInt256* base, const UInt256* exp){
    UInt256 acc, term = *base, ONE;
    zero_UInt256(&ONE);
    ONE.limbs[0] = 0x1;
    
    // べき乗の初期値は 1 
    acc = ONE; 

    for(int i = 0; i < 256; i++){
        int bit = get_bit_UInt256(exp, i);
        
        // ビットが1なら、現在の term (base^(2^i)) を掛ける
        // ビットが0なら、1を掛ける (値を変えない)
        scalar_mul(&acc, &acc, bit ? &term : &ONE);

        // 次のループのために term を2乗する
        scalar_mul(&term, &term, &term);
    }
    *res = acc;
}

void scalar_inv(UInt256* res, const UInt256* A){
    UInt256 u2={{2,0,0,0}}, tmp = SECP256K1_N;
    sub_UInt256(&tmp, &u2);
    scalar_pow(res, A, &tmp);
}
